#!/usr/bin/env python
###############################################################################

import os, sys
from copy import deepcopy

###############################################################################

os.system("avr-g++ -O3 test.cpp -S -o test.avr")
os.system("avr-g++ -g -O3 test.cpp -o test.o")
os.system("avr-objdump -t -S -d test.o > test.lst")

###############################################################################
# LEXER
###############################################################################

import ply.lex as lex

tokens = (
   'IDENTIFIER',
   'DIRECTIVE',
   'HEXNUMBER',
   'NUMBER',
   "COMMENT",
   "OPCODE",
   'STRING',
   'NEWLINE'
)

literals = "+-*/()=:;,.@"

t_IDENTIFIER = r'[_.a-zA-Z][_.a-zA-Z0-9]*'
t_DIRECTIVE = r'\.(stabn|stabs|stabd|stabn|file|text|global|type|section|startup|data|size|word|ident)'
t_COMMENT = r'/\*([^*]|[\r\n]|(\*+([^*/]|[\r\n])))*\*+/'
t_STRING = r'\"(.+?)?\"'
t_OPCODE = r'(ldi|lds|st|rjmp|lpm|add|adc|adiw|cpi|cpc|brne|ret)'
t_HEXNUMBER = r'0[xX][\da-f]+'
t_NUMBER = r'[\d]+'

# Define a rule so we can track line numbers
def t_NEWLINE(t):
    r'\n+'
    t.lexer.lineno += len(t.value)
    t.type = "NEWLINE"

# A string containing ignored characters (spaces and tabs)
t_ignore  = ' \t'

# Error handling rule
def t_error(t):
    print("Illegal character '%s'" % t.value[0])
    t.lexer.skip(1)


lexer = lex.lex() # Build the lexer

###############################################################################
# reassembler contextual data
###############################################################################

_regmap = {
    "r26": "XL",
    "r27": "XH",
    "r28": "YL",
    "r29": "YH",
    "r30": "ZL",
    "r31": "ZH"
}

class context:
  def __init__(self):
    self._identifiers = {}
    self._labels = {}
    self._avr_pc = 0
    self._mos_pc = 0
    self._PCMAP = {}

  def mapitem(self,item,rev=False,comment=True):
    if item in _regmap:
        item = _regmap[item]
    elif item in self._labels:
        if comment:
          if rev:
            item = "%s /* $%04x */" % (item,self._labels[item])
          else:
            item = "$%04x /* %s */" % (self._labels[item],item)
        else:
            item = "%d" % (self._labels[item])
    elif item in self._identifiers:
        if comment:
          if rev:
            item = "%s /* %s */" % (item,self._identifiers[item])
          else:
            item = "%s /* %s */" % (self._identifiers[item],item)
        else:
            item = "%d" % (self._identifiers[item])
    elif item == ".":
        if comment:
          if rev:
            item = ". /* $%04x */" % (self._avr_pc)
          else:
            item = "$%04x /* . */" % (self._avr_pc)
        else:
            item = "%d" % (self._avr_pc)
    return item

main_ctx = context()

###############################################################################
# PARSER
###############################################################################

precedence = (
    ('left', '+', '-'),
    ('left', '*', '/'),
    #('right', 'UMINUS'),
)

#######################################

diritems = []

def p_directiveitem(p):
    '''directiveitem : STRING 
                     | IDENTIFIER
                     | DIRECTIVE
                     | ","
                     | "@"
                     | "-"
                     | NUMBER
                     | HEXNUMBER'''
    diritems.append(p[1])

def p_directiveitems(p):
    '''directiveitems : directiveitem directiveitems
       directiveitems : directiveitem'''

#######################################

opcitems = []

def p_opcodeitem(p):
    '''opcodeitem : expression 
                  | ","'''
    opcitems.append(p[1])

def p_opcodeitems(p):
    '''opcodeitems : opcodeitem opcodeitems
       opcodeitems : opcodeitem'''

###############################################################################

class expr_node:
    def __init__(self,a,op,b):
        self._a = a
        self._op = op
        self._b = b
    def __str__(self):
        if self._op:
            a = main_ctx.mapitem(self._a)
            b = main_ctx.mapitem(self._b)
            return "expr_node( %s %s %s )" % (a,self._op,b)
        else:
            a = main_ctx.mapitem(self._a)
            return "expr_node( %s )" % (a)
    def eval(self):
        a = main_ctx.mapitem(self._a,comment=False)
        b = main_ctx.mapitem(self._b,comment=False)
        op = self._op
        rval = None
        if op == "+":
            rval = int(a)+int(b)
        return rval

###################
def p_expression_binop(p):
    '''expression : expression '+' expression
                  | expression '-' expression
                  | expression '*' expression
                  | expression '/' expression'''
    if p[2] == '+':
        p[0] = expr_node(p[1],"+",p[3])
    elif p[2] == '-':
        p[0] = expr_node(p[1],"-",p[3])
    elif p[2] == '*':
        p[0] = expr_node(p[1],"*",p[3])
    elif p[2] == '/':
        p[0] = expr_node(p[1],"/",p[3])
###################
def p_expression_uminus(p):
    "expression : '-' expression"
    p[0] = expr_node(0,"-",p[2])
###################
def p_expression_group(p):
    "expression : '(' expression ')'"
    p[0] = p[2]
###################
def p_expression_number(p):
    '''expression : NUMBER
                  | HEXNUMBER'''
    p[0] = p[1]
###################
def p_expression_name(p):
    "expression : IDENTIFIER"
    p[0] = p[1]

###############################################################################
# label assignment
###########################################################

_output_items = []

def p_statement_ASSIGNLABEL(p):
    'statement : IDENTIFIER ":"'
    ###################
    def dump(item):
        data = item["data"]
        label = data[0]
        pc = data[1]
        return "\n%s: /* $%04x */\n" % (label,pc)
    ###################
    global main_ctx
    label = p[1]
    data = [label,main_ctx._avr_pc]
    main_ctx._labels[label] = main_ctx._avr_pc
    _output_items.append({"dump":dump,
                          "data":data,
                          "ctx":deepcopy(main_ctx)})

###########################################################
# identifier assignment
###########################################################

def p_statement_ASSIGNIDENTIFIER(p):
    'statement : IDENTIFIER "=" expression'
    ###################
    def dump(item):
        data = item["data"]
        return "%s = %s" % (data[0],data[1])
    ###################
    data = [p[1],p[3]]
    main_ctx._identifiers[p[1]]=p[3]
    _output_items.append({"dump":dump,
                          "data":data,
                          "ctx":deepcopy(main_ctx)})

###########################################################
# opcode
###########################################################

def gen_6502_opcode_LDI(item):
    ctx = item["ctx"]
    dump = item["dump"]
    opcitems = item["opcitems"]
    pc = item["PC"]
    rval = None
    dest = opcitems[0]
    imm = opcitems[2]
    comment = dump(item)
    if len(opcitems)==4:
        immv = opcitems[3]
        rval =  "lda #$%02x\t; %s\n" % (int(immv),comment)
        regno = int(dest[1:])
        rval += "sta  $%02x\t; %s" % (regno,comment)
        main_ctx._mos_pc += 4
    elif len(opcitems)==3:
        rval =  "lda #$%02x\t; %s\n" % (int(imm),comment)
        regno = int(dest[1:])
        rval += "sta  $%02x\t; %s" % (regno,comment)
        main_ctx._mos_pc += 4
    return rval

#############################

def genabsaddr(addr):

    addrtype = type(addr)

    print addr
    if addrtype == type(str):
        if addr=='X':
            addr = 26
        elif addr=='Y':
            addr = 28
        elif addr=='Z':
            addr = 30
        elif addr[0]=='r':
            addr = int(addr[1:])
        elif addr[0]=='$':
            addr = "0x"+addr[1:]
            addr = int(addr)
        elif addr in main_ctx._identifiers:
            addr = int(main_ctx._identifiers[addr])
        elif addr in main_ctx._labels:
            addr = int(main_ctx._labels[addr])
        else:
            addr = int(addr)
    elif addrtype == type(int):
        addr = addr
    elif isinstance(addr,expr_node):
        addr = addr.eval()
    else:
        print addr, addrtype
        assert(False)

    return addr    

#############################

def gen_6502_opcode_LDS(item):
    ctx = item["ctx"]
    dump = item["dump"]
    opcitems = item["opcitems"]
    dest = genabsaddr(opcitems[0])
    pc = item["PC"]
    comment = dump(item)
    
    addr = ctx.mapitem(opcitems[2],comment=False)
    addr = genabsaddr(addr)

    if addr<256:
        rval =  "lda  $%2x\t; %s\n" % (addr,comment)
        main_ctx._mos_pc += 2
    else:
        rval =  "lda  $%04x\t; %s\n" % (addr,comment)
        main_ctx._mos_pc += 3

    rval += "sta  $%02x\t; %s" % (dest,comment)
    main_ctx._mos_pc += 2

    return rval;

#############################

def gen_6502_opcode_ST(item):
    ctx = item["ctx"]
    dump = item["dump"]
    opcitems = item["opcitems"]
    d = opcitems[0]
    s = opcitems[2]
    dest = genabsaddr(d)
    src = genabsaddr(s)
    comment = dump(item)

    numb = 1
    if d in ['X','Y','Z']:
        numb = 2

    rval  = "lda $%02x     ; %s\n" % (src,comment)
    rval += "sta $%02x     ; %s" % (dest,comment)
    main_ctx._mos_pc += 4
    if numb==2:
      rval += "\nlda $%02x     ; %s\n" % (src+1,comment)
      rval += "sta $%02x     ; %s" % (dest+1,comment)
      main_ctx._mos_pc += 4

    return rval

#############################

def gen_6502_opcode_ADD(item):
    ctx = item["ctx"]
    dump = item["dump"]
    opcitems = item["opcitems"]
    dest = genabsaddr(opcitems[0])
    src = genabsaddr(opcitems[2])
    comment = dump(item)
    rval =  "clc         ; %s\n" % comment
    rval += "lda $%02x     ; %s\n" % (src,comment)
    rval += "adc $%02x     ; %s\n" % (dest,comment)
    rval += "sta $%02x     ; %s" % (dest,comment)
    main_ctx._mos_pc += 7
    return rval

#############################

def gen_6502_opcode_ADC(item):
    ctx = item["ctx"]
    dump = item["dump"]
    opcitems = item["opcitems"]
    dest = genabsaddr(opcitems[0])
    src = genabsaddr(opcitems[2])
    comment = dump(item)
    rval  = "lda $%02x     ; %s\n" % (src,comment)
    rval += "adc $%02x     ; %s\n" % (dest,comment)
    rval += "sta $%02x     ; %s" % (dest,comment)
    main_ctx._mos_pc += 6
    return rval

#############################

def gen_6502_opcode_ADIW(item):
    ctx = item["ctx"]
    dump = item["dump"]
    opcitems = item["opcitems"]
    dest = genabsaddr(opcitems[0])
    src = genabsaddr(opcitems[2])
    comment = dump(item)
    rval =  "clc         ; %s\n" % comment
    rval += "lda #$%02x    ; %s\n" % (src,comment)
    rval += "adc  $%02x    ; %s\n" % (dest,comment)
    rval += "sta  $%02x    ; %s\n" % (dest,comment)
    rval += "lda #$00    ; %s\n" % (comment)
    rval += "adc  $%02x    ; %s\n" % (dest+1,comment)
    rval += "sta  $%02x    ; %s" % (dest+1,comment)
    main_ctx._mos_pc += 13
    return rval

#############################

def gen_6502_opcode_CPI(item):
    ctx = item["ctx"]
    dump = item["dump"]
    opcitems = item["opcitems"]
    reg = genabsaddr(opcitems[0])
    imm = genabsaddr(opcitems[2])
    comment = dump(item)
    rval  = "lda  $%02x    ; %s\n" % (reg,comment)
    rval += "cmp #$%02x    ; %s\n" % (imm,comment)
    main_ctx._mos_pc += 4
    return rval

#############################

def gen_6502_opcode_CPC(item):
    ctx = item["ctx"]
    dump = item["dump"]
    opcitems = item["opcitems"]
    reg = genabsaddr(opcitems[0])
    imm = genabsaddr(opcitems[2])
    comment = dump(item)
    rval  = "lda  $%02x    ; %s\n" % (reg,comment)
    rval += "sbc #$%02x    ; %s\n" % (imm,comment)
    main_ctx._mos_pc += 4
    return rval

#############################

def gen_6502_opcode_BRNE(item):
    ctx = item["ctx"]
    dump = item["dump"]
    opcitems = item["opcitems"]
    absaddr = genabsaddr(opcitems[0])

    mapped = absaddr
    if mapped in main_ctx._PCMAP:
        mapped = main_ctx._PCMAP[absaddr] 

    curpc = main_ctx._mos_pc
    delta = mapped-curpc

    comment = dump(item)
    rval  = "bne  $%04x    ; (delta=%d) %s\n" % (mapped,delta,comment)
    main_ctx._mos_pc += 2
    return rval

#############################

def gen_6502_opcode_RET(item):
    return "rts         ; RET"

#############################

def gen_6502_opcode_WORD(item):
    ctx = item["ctx"]
    dump = item["dump"]
    comment = dump(item)
    opcitems = item["opcitems"][0]

    print "%s" % opcitems

    wordval = int(opcitems[1])
    if opcitems[0]=='-':
        wordval = 65536-wordval

    rval  = ".WORD $%04x; %s\n" % (wordval,comment)
    main_ctx._mos_pc += 2

    return rval

#############################

_opcode_table = {
    "ldi":  { "adv": 2, "gen":gen_6502_opcode_LDI },    
    "lds":  { "adv": 4, "gen":gen_6502_opcode_LDS },    
    "add":  { "adv": 2, "gen":gen_6502_opcode_ADD },    
    "adc":  { "adv": 2, "gen":gen_6502_opcode_ADC },    
    "adiw": { "adv": 2, "gen":gen_6502_opcode_ADIW },    
    "st":   { "adv": 2, "gen":gen_6502_opcode_ST },    
    "cpi":  { "adv": 2, "gen":gen_6502_opcode_CPI },    
    "cpc":  { "adv": 2, "gen":gen_6502_opcode_CPC },    
    "brne": { "adv": 2, "gen":gen_6502_opcode_BRNE },    
    "ret":  { "adv": 1, "gen":gen_6502_opcode_RET },    
    ".word":  { "adv": 2, "gen":gen_6502_opcode_WORD },    
}

#############################

def gen_6502_opcode(item):
    avropcode = item["opcode"]
    assert(avropcode in _opcode_table)
    opcode_data = _opcode_table[avropcode]
    opcode_gen = opcode_data["gen"]
    avrpc = item["PC"]

    main_ctx._PCMAP[avrpc] = main_ctx._mos_pc
    rval  = ";;;; MOS_PC=$%04x   AVR_PC=$%04x ;;;;\n" % (main_ctx._mos_pc,avrpc)
    rval += opcode_gen(item)
    return rval

#############################

def p_statement_OPCODE(p):
    '''statement : OPCODE opcodeitems
       statement : OPCODE'''
    ###################
    def dump(item):
        ctx = item["ctx"]
        avropc = item["opcode"]
        opcitems = item["opcitems"]
        pc = item["PC"]
        outstr = ""
        for item in opcitems:
            mapped = ctx.mapitem(item)
            outstr += "%s " % mapped
        return "/* $%04x */ %s [ %s]" % (pc,avropc,outstr )
    ###################
    global opcitems, main_ctx

    _output_items.append({"dump":dump,
                          "gen6502":gen_6502_opcode,
                          "opcode":p[1],
                          "opcitems":opcitems[:],
                          "PC":main_ctx._avr_pc,
                          "ctx":main_ctx})
    opcitems = []

    opcode_name = p[1]
    assert(opcode_name in _opcode_table)
    opcode_data = _opcode_table[opcode_name]
    opcode_adv = opcode_data["adv"]
    main_ctx._avr_pc += opcode_adv

###########################################################
# directive
###########################################################

def p_statement_DIRECTIVE(p):
    '''statement : DIRECTIVE directiveitems
       statement : DIRECTIVE'''
    ###################
    def dump(item):
        ctx = item["ctx"]
        directive = item["directive"]
        diritems = item["diritems"]
        outstr = ""
        for ditem in diritems:
            mapped = ctx.mapitem(ditem,rev=True)
            outstr += "%s " % mapped
        return "%s [ %s]" % (directive,outstr )
    ###################
    def gen6502word(item):
        avropcode = item["opcode"]
        assert(avropcode in _opcode_table)
        opcode_data = _opcode_table[avropcode]
        opcode_gen = opcode_data["gen"]
        avrpc = item["PC"]

        main_ctx._PCMAP[avrpc] = main_ctx._mos_pc
        rval  = ";;;; MOS_PC=$%04x   AVR_PC=$%04x ;;;;\n" % (main_ctx._mos_pc,avrpc)
        rval += opcode_gen(item)
        return rval
    ###################
    global diritems
    directive = p[1]
    itemscopy = diritems[:]
    outdata = {"dump":dump,
               "directive": directive,
               "diritems":itemscopy,
               "ctx":deepcopy(main_ctx)
    }
    ###################
    if directive == ".word":
        global main_ctx
        outdata["PC"] = main_ctx._avr_pc
        outdata["opcode"] = ".word"
        outdata["opcitems"] = itemscopy,
        outdata["gen6502"] = gen6502word
        main_ctx._avr_pc += 2
    ###################
    diritems = []
    _output_items.append( outdata )

###########################################################
# comment
###########################################################

def p_statement_COMMENT(p):
    'statement : COMMENT'
    ###################
    def gen(item):
        data = item["data"]
        return "%s" % data
    ###################
    _output_items.append({"dump":gen,"data":p[1]})

###############################################################################
# parser top level
###############################################################################

def p_statements(p):
    '''statements : statement statements
       statements : statement'''

def p_compilationunit(p):
    "compilationunit : statements"

def p_error(p):
    if p:
        print("Syntax error at '%s'" % p.value)
    else:
        print("Syntax error at EOF")

###############################################################################
# run parser
###############################################################################

start = 'compilationunit' # parser root

import ply.yacc as yacc
yacc.yacc()

with open("test.avr","r") as fin:
  ###########################################
  # parse input, filling in _output_items
  ###########################################
  _output_items = []
  for line in fin:
    yacc.parse(line,debug=False)
  ###########################################
  # generate dump (from _output_items)
  ###########################################
  dump_strings = []
  for item in _output_items:
    dump = item["dump"]
    str = dump(item)
    dump_strings.append(str)
  # write dump
  with open("test.dump","w") as fout:
    for item in dump_strings:
        fout.write(item+"\n")
        print item
  ###########################################
  # generate dump 6502 (from _output_items)
  ###########################################
  dump_strings = []
  for item in _output_items:
    if "gen6502" in item:
      dump = item["gen6502"]
      str = dump(item)
      if str:
          dump_strings.append(str)
  # write dump6502
  with open("test.dump6502","w") as fout:
    fout.write(".ORG $0000")    
    for item in dump_strings:
        fout.write("\n"+item+"\n")


###########################################

os.system("ca65 test.dump6502 -l test.lst6502")
      
