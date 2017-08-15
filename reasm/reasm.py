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
t_OPCODE = r'(ldi|lds|st|rjmp|lpm|add|adc|adiw|cpi|cpc|brne|sbci|subi|ret)'
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
    self._mos_pc = 4
    self._PCMAP = {}

  def mapitem(self,item,rev=False,comment=True):
    if item in _regmap:
        item = _regmap[item]
    elif item in self._labels:
        mapped = self._labels[item]
        if mapped in main_ctx._PCMAP:
            mapped = main_ctx._PCMAP[mapped]
        if comment:
          if rev:
            item = "%s /* $%04x */" % (item,mapped)
          else:
            item = "$%04x /* %s */" % (self._labels[item],item)
        else:
            item = "%d" % (mapped)
    elif item in self._identifiers:
        mapped = self._identifiers[item]
        if mapped in main_ctx._PCMAP:
            mapped = main_ctx._PCMAP[mapped]
        if comment:
          if rev:
            item = "%s /* %s */" % (item,mapped)
          else:
            item = "%s /* %s */" % (mapped,item)
        else:
            item = "%d" % (mapped)
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
_output_items = []

###############################################################################

def genabsaddr(addr):

    addrtype = type(addr)

    #print addr
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
            name = addr
            addr = int(main_ctx._identifiers[addr])
            #print "IDEN: %s:$%04x" % (name,addr)
            if addr in main_ctx._PCMAP:
                addr = main_ctx._PCMAP[addr]
        elif addr in main_ctx._labels:
            name = addr
            addr = int(main_ctx._labels[addr])
            #print "LABL: %s:$%04x" % (name,addr)
            if addr in main_ctx._PCMAP:
                addr = main_ctx._PCMAP[addr]
        else:
            addr = int(addr)
    elif addrtype == type(int):
        addr = addr
        if addr<0:
            addr = addr&0xff;

    elif isinstance(addr,expr_node):
        addr = addr.eval()
    else:
        print addr, addrtype
        assert(False)

    return addr    

###############################################################################

class expr_node:
    def __init__(self,a,op,b):
        self._a = a
        self._op = op
        self._b = b
    def __str__(self):
        if self._op:
            a = self._a #main_ctx.mapitem(self._a,comment=False)
            b = self._b #main_ctx.mapitem(self._b,comment=False)
            return "%s %s %s" % (a,self._op,b)
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
    p[0] = "-"+p[2]
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

###########################################################
# opcode
###########################################################

def gen_6502_opcode_LDI(item):
    ctx = item["ctx"]
    opcitems = item["opcitems"]
    pc = item["PC"]
    rval = ""
    dest = opcitems[0]
    if len(opcitems)==4: # reg , mod immv
        mod = opcitems[2]
        immv = int(opcitems[3])
        regno = int(dest[1:])
        if mod=="lo8":
            rval +=  "lda #<$%04x\n" % (immv)
        rval += "sta  $%02x\n" % (regno)
        main_ctx._mos_pc += 4
    elif len(opcitems)==3: # reg , immv
        imm = int(opcitems[2])
        rval +=  "lda #$%02x\n" % (imm)
        regno = int(dest[1:])
        rval += "sta  $%02x\n" % (regno)
        main_ctx._mos_pc += 4
    return rval

#############################

def gen_6502_opcode_LDS(item):
    ctx = item["ctx"]
    opcitems = item["opcitems"]
    
    addr = opcitems[2] #ctx.mapitem(opcitems[2],comment=False)
    #addr = genabsaddr(addr)


    if addr<256:
        rval =  "lda  %s\n" % (addr)
        main_ctx._mos_pc += 2
    else:
        rval =  "lda  %s\n" % (addr)
        main_ctx._mos_pc += 3

    reg = genabsaddr(opcitems[0])
    rval += "sta  $%02x" % (reg)
    main_ctx._mos_pc += 2

    return rval;

#############################

def gen_6502_opcode_ST(item):
    ctx = item["ctx"]
    opcitems = item["opcitems"]
    d = opcitems[0]
    s = opcitems[2]
    dest = genabsaddr(d)
    src = genabsaddr(s)

    rval  = "lda $%02x\n" % (src)
    rval += "ldy #$00\n"
    rval += "sta ($%02x),y" % (dest)
    main_ctx._mos_pc += 4

    return rval

#############################

def gen_6502_opcode_ADD(item):
    ctx = item["ctx"]
    opcitems = item["opcitems"]
    dest = genabsaddr(opcitems[0])
    src = genabsaddr(opcitems[2])
    rval =  "clc\n"
    rval += "lda $%02x\n" % (src)
    rval += "adc $%02x\n" % (dest)
    rval += "sta $%02x" % (dest)
    main_ctx._mos_pc += 7
    return rval

#############################

def gen_6502_opcode_ADC(item):
    ctx = item["ctx"]
    opcitems = item["opcitems"]
    dest = genabsaddr(opcitems[0])
    src = genabsaddr(opcitems[2])
    rval  = "lda $%02x\n" % (src)
    rval += "adc $%02x\n" % (dest)
    rval += "sta $%02x" % (dest)
    main_ctx._mos_pc += 6
    return rval

#############################

def gen_6502_opcode_SBC(item):
    assert(False)
    ctx = item["ctx"]
    opcitems = item["opcitems"]
    dest = genabsaddr(opcitems[0])
    src = genabsaddr(opcitems[2])
    rval  = "lda $%02x\n" % (src)
    rval += "sbc $%02x\n" % (dest)
    rval += "sta $%02x" % (dest)
    main_ctx._mos_pc += 6
    return rval

#############################

def gen_6502_opcode_SBCI(item):
    ctx = item["ctx"]
    opcitems = item["opcitems"]
    reg = genabsaddr(opcitems[0])
    imm = opcitems[2]
    if imm == "lo8":
        imm = int(opcitems[3])
        imm = imm&0xff;
    else:
        imm = int(imm)
    if imm<0:
        imm = imm&0xff;
    rval  = "lda $%02x\n" % (reg)
    rval += "sbc #$%02x\n" % (imm)
    rval += "sta $%02x" % (reg)
    main_ctx._mos_pc += 6
    return rval

#############################

def gen_6502_opcode_SUBI(item):
    ctx = item["ctx"]
    opcitems = item["opcitems"]
    reg = genabsaddr(opcitems[0])
    imm = opcitems[2]

    if imm == "lo8":
        imm = int(opcitems[3])
        imm = imm&0xff;
    else:
        imm = int(imm)

    if imm<0:
        imm = imm&0xff;

    print "reg<%s> imm<%s>\n" % (reg,imm)
    rval  = "lda $%02x\n" % (reg)
    rval += "sec\n"
    rval += "sbc #$%02x\n" % (imm)
    rval += "sta $%02x" % (reg)

    main_ctx._mos_pc += 6
    return rval

#############################

def gen_6502_opcode_ADIW(item):
    ctx = item["ctx"]
    opcitems = item["opcitems"]
    dest = genabsaddr(opcitems[0])
    src = genabsaddr(opcitems[2])
    rval =  "clc\n"
    rval += "lda #<$%04x\n" % (src)
    rval += "adc  $%02x\n" % (dest)
    rval += "sta  $%02x\n" % (dest)
    rval += "lda #>$%04x\n" % (src)
    rval += "adc  $%02x\n" % (dest+1)
    rval += "sta  $%02x\n" % (dest+1)
    main_ctx._mos_pc += 13
    return rval

#############################

def gen_6502_opcode_CPI(item):
    ctx = item["ctx"]
    opcitems = item["opcitems"]
    #reg = genabsaddr(opcitems[0])
    rval = ""
    dest = opcitems[0]
    if len(opcitems)==4: # reg , mod immv
        mod = opcitems[2]
        immv = int(opcitems[3])
        regno = int(dest[1:])
        if mod=="lo8":
            rval +=  "lda #<$%04x\n" % (immv)
        #rval += "sta  $%02x\n" % (regno)
        rval += "cmp #$%02x\n" % (regno)
    elif len(opcitems)==3: # reg , immv
        imm = int(opcitems[2])
        rval +=  "lda #$%02x\n" % (imm)
        regno = int(dest[1:])
        #rval += "sta  $%02x\n" % (regno)
        rval += "cmp #$%02x\n" % (regno)

    #imm = genabsaddr(opcitems[2])
    #rval  = "lda  $%02x\n" % (reg)
    #rval += "cmp #$%02x\n" % (imm)
    main_ctx._mos_pc += 4
    return rval

#############################

def gen_6502_opcode_CPC(item):
    ctx = item["ctx"]
    opcitems = item["opcitems"]
    reg = genabsaddr(opcitems[0])
    imm = genabsaddr(opcitems[2])
    rval  = "lda  $%02x\n" % (reg)
    rval += "sbc  $%02x\n" % (imm)
    main_ctx._mos_pc += 4
    return rval

#############################

def gen_6502_opcode_BRNE(item):
    ctx = item["ctx"]
    opcitems = item["opcitems"]
    absaddr = genabsaddr(opcitems[0])

    mapped = absaddr
    if mapped in main_ctx._PCMAP:
        mapped = main_ctx._PCMAP[absaddr] 

    curpc = main_ctx._mos_pc
    delta = mapped-curpc

    rval  = "bne  $%04x ; delta=%d\n" % (mapped,delta)
    main_ctx._mos_pc += 2
    return rval

#############################

def gen_6502_opcode_RET(item):
    main_ctx._mos_pc += 1
    return "rts         ; RET"

#############################

def gen_6502_opcode_WORD(item):
    ctx = item["ctx"]
    dump = item["dump"]
    comment = dump(item)
    opcitems = item["opcitems"][0]

    wordval = int(opcitems[1])
    if opcitems[0]=='-':
        wordval = 65536-wordval

    rval  = ".WORD $%04x; %s\n" % (wordval,comment)
    main_ctx._mos_pc += 2

    return rval

#############################

def gen_6502_opcode_LABEL(item):
    ctx = item["ctx"]
    name = item["name"]
    rval  = "%s: ; LABEL" % (name)
    return rval

#############################

def gen_6502_opcode_ASSIGN(item):
    ctx = item["ctx"]
    name = item["name"]
    val = item["val"]
    rval  = "%s = %d ; %s = %s" % (name,int(val,0),name,val)
    return rval

#############################

_opcode_table = {
    "adc":  { "adv": 2, "gen":gen_6502_opcode_ADC },    
    "add":  { "adv": 2, "gen":gen_6502_opcode_ADD },    
    "adiw": { "adv": 2, "gen":gen_6502_opcode_ADIW },    
    "brne": { "adv": 2, "gen":gen_6502_opcode_BRNE },    
    "cpi":  { "adv": 2, "gen":gen_6502_opcode_CPI },    
    "cpc":  { "adv": 2, "gen":gen_6502_opcode_CPC },    
    "ldi":  { "adv": 2, "gen":gen_6502_opcode_LDI },    
    "lds":  { "adv": 4, "gen":gen_6502_opcode_LDS },    
    #"sbc":  { "adv": 2, "gen":gen_6502_opcode_SBC },    
    "sbci":  { "adv": 2, "gen":gen_6502_opcode_SBCI },    
    "subi":  { "adv": 2, "gen":gen_6502_opcode_SUBI },    
    "st":   { "adv": 2, "gen":gen_6502_opcode_ST },    
    "ret":  { "adv": 1, "gen":gen_6502_opcode_RET },    
    ".word":  { "adv": 2, "gen":gen_6502_opcode_WORD },    
    "LABEL":  { "adv": 2, "gen":gen_6502_opcode_LABEL },    
    "ASSIGN":  { "adv": 2, "gen":gen_6502_opcode_ASSIGN },    
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
    
    comment = None
    if "dump" in item:
        dump = item["dump"]
        comment = dump(item)
    if comment != None:
        rval += ";;;; "+comment+" ;;;;\n"

    rval += opcode_gen(item)

    return rval

#############################

def p_statement_OPCODE(p):
    '''statement : OPCODE opcodeitems
       statement : OPCODE'''
    ###################
    def dump(item):
        ctx = item["ctx"]
        opcode = item["opcode"]
        opcitems = item["opcitems"]
        pc = item["PC"]
        nocom = False
        if "nocom" in item:
            nocom = item["nocom"]
        outstr = ""
        for item in opcitems:
            mapped = ctx.mapitem(item)
            outstr += "%s " % mapped
        if nocom:
            return "%s [ %s]" % (opcode,outstr )
        else:
            return "/* $%04x */ %s [ %s]" % (pc,opcode,outstr )
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

###############################################################################
# label assignment
###########################################################

def p_statement_ASSIGNLABEL(p):
    'statement : IDENTIFIER ":"'
    ###################
    def dump(item):
        data = item["data"]
        label = data[0]
        pc = data[1]
        nocom = False
        if "nocom" in item:
            nocom=True
        if nocom:
            return None
        else:
            return "\n%s: /* $%04x */\n" % (label,pc)
    ###################
    global main_ctx
    label = p[1]
    if label=="main":
        label = "_main"
    data = [label,main_ctx._avr_pc]
    main_ctx._labels[label] = main_ctx._avr_pc


    _output_items.append({"dump":dump,
                          "PC":main_ctx._avr_pc,
                          "gen6502":gen_6502_opcode,
                          "opcode":"LABEL",
                          "name":label,
                          "data":data,
                          "ctx":deepcopy(main_ctx)})

###########################################################
# identifier assignment
###########################################################

def p_statement_ASSIGNIDENTIFIER(p):
    'statement : IDENTIFIER "=" expression'
    ###################
    def dump(item):
        name = item["name"]
        val = item["val"]
        nocom = False
        if "nocom" in item:
            nocom=True
        if nocom:
            return None
        else:
            return "%s = %s" % (name,val)
    ###################
    name = p[1]
    val = p[3]
    main_ctx._identifiers[name]=val
    _output_items.append({"dump":dump,
                          "name":name,
                          "val":val,
                          "PC":main_ctx._avr_pc,
                          "gen6502":gen_6502_opcode,
                          "opcode":"ASSIGN",
                          "ctx":deepcopy(main_ctx)})

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
    item["nocom"]=True
    dump_strings.append(str)
  # write dump
  with open("test.dump","w") as fout:
    for item in dump_strings:
        fout.write(item+"\n")
        #print item
  ###########################################
  # generate dump 6502 (from _output_items)
  ###########################################
  dump_strings = []
  for item in _output_items:
    if "gen6502" in item:
      dump = item["gen6502"]
      str = dump(item)
  dump_strings = []
  main_ctx._mos_pc = 0x1004
  for item in _output_items:
    if "gen6502" in item:
      dump = item["gen6502"]
      str = dump(item)
      if str:
          dump_strings.append(str)
  # write dump6502
  with open("test.dump6502","w") as fout:
    fout.write('.segment  "CODE"\n')
    fout.write(".ORG $1000\n")    
    fout.write(".FEATURE leading_dot_in_identifiers\n")
    fout.write("\n;;;; avr init ;;;;\n")
    fout.write("lda #$00\n")
    fout.write("sta $01\n")
    for item in dump_strings:
        fout.write("\n"+item+"\n")


###########################################

os.system("ca65 -t none crt0.s -o gen/crt0.o -l gen/crt0.lst")
os.system("ca65 test.dump6502 -l test.lst6502 -o gen/test.o")
os.system("ld65 -v -vm -C test.cfg -m gen/test1.map -o gen/test1.bin gen/crt0.o gen/test.o supervision.lib")      
