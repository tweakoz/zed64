#!/usr/bin/env python
###############################################################################

import os
import sys
sys.path.insert(0, "../..")

###############################################################################

os.system("avr-g++ -g -O3 test.cpp -S -o test.avr")
os.system("avr-g++ -g -O3 test.cpp -o test.o")
os.system("avr-objdump -t -S -d test.o > test.lst")

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

#t_LABELDEF= r'[_.]?[_a-zA-Z][_a-zA-Z0-9]*:'
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
# scan a lines worth of tokens
###############################################################################

def getTokensOnLine():
    tok = lexer.token()
    toks = list()
    lex2 = lexer.clone()
    if tok==None:
        return None
    toks.append(tok)
    done = False
    count = 0
    while False==done:
        ntok = lex2.token()
        #print ntok, tok
        if ntok == None:
            done = True 
        else:
          if ntok.lineno != tok.lineno:
            done = True
          else:
            count+=1
          for i in range(0,count):
            tokx = lexer.token()
            if tokx!=None:
                toks.append(tokx)
    return toks

###############################################################################
# reassembler contextual data
###############################################################################

identifiers = {}
_labels = {}
avr_opcodes = []
avr_pc = 0
MOS_OPC = 0

###############################################################################

_regmap = {
    "r26": "XL",
    "r27": "XH",
    "r28": "YL",
    "r29": "YH",
    "r30": "ZL",
    "r31": "ZH"
}

def mapitem(item):
    if item in _regmap:
        item = _regmap[item]
    elif item in _labels:
        item = "%s /* $%04x */" % (item,_labels[item])
    return item

###############################################################################
# opcodes
###############################################################################

opcode_table = {
    "ldi":  {"adv": 2},    
    "lds":  {"adv": 4},    
    "add":  {"adv": 2},    
    "adc":  {"adv": 2},    
    "adiw": {"adv": 2},    
    "st":   {"adv": 2},    
    "cpi":  {"adv": 2},    
    "cpc":  {"adv": 2},    
    "brne": {"adv": 2},    
    "ret": {"adv": 1},    
}

###############################################################################
# Parsing rules
###############################################################################

output_list = []

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
    #print("diri <%s>" % p[1])
    diritems.append(p[1])

def p_directiveitems(p):
    '''directiveitems : directiveitem directiveitems
       directiveitems : directiveitem'''
    #print("diris <%s>" % p[1])

#######################################

opcitems = []

def p_opcodeitem(p):
    '''opcodeitem : expression 
                  | ","'''
    opcitems.append(p[1])

def p_opcodeitems(p):
    '''opcodeitems : opcodeitem opcodeitems
       opcodeitems : opcodeitem'''

#######################################

class expr_node:
    def __init__(self,a,op,b):
        self._a = a
        self._op = op
        self._b = b
    def __str__(self):
        if self._op:
            a = mapitem(self._a)
            b = mapitem(self._b)
            return "expr_node( %s %s %s )" % (a,self._op,b)
        else:
            a = mapitem(self._a)
            return "expr_node( %s )" % (a)

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


def p_expression_uminus(p):
    "expression : '-' expression"
    p[0] = expr_node(None,"-",p[2])


def p_expression_group(p):
    "expression : '(' expression ')'"
    p[0] = p[2]


def p_expression_number(p):
    '''expression : NUMBER
                  | HEXNUMBER'''
    p[0] = p[1]


def p_expression_name(p):
    "expression : IDENTIFIER"
    p[0] = p[1]

#######################################

def p_statement_las(p):
    'statement : IDENTIFIER ":"'
    def gen(data):
        label = data[0]
        pc = data[1]
        return "\n%s: /* $%04x */\n" % (label,pc)
    global avr_pc
    label = p[1]
    data = [label,avr_pc]
    _labels[label] = avr_pc
    output_list.append([gen,data])

def p_statement_assign(p):
    'statement : IDENTIFIER "=" expression'
    def gen(data):
        v = data
        return "\t%s = %s" % (v[1],v[3])
    output_list.append([gen,p[:]])


def p_statement_opc(p):
    '''statement : OPCODE opcodeitems
       statement : OPCODE'''


    def gen(data):
        c_p = data[0]
        c_o = data[1]
        pc = data[2]
        outstr = ""
        for item in c_o:
            mapped = mapitem(item)
            outstr += "%s " % mapped
        return "/* $%04x */ %s [ %s]" % (pc,c_p[1],outstr )

    global opcitems
    global avr_pc
    data = [ p[:], opcitems[:], avr_pc ]
    output_list.append( [gen,data] )
    opcitems = []

    opcode_name = p[1]
    assert(opcode_name in opcode_table)
    opcode_data = opcode_table[opcode_name]
    opcode_adv = opcode_data["adv"]
    avr_pc += opcode_adv

def p_statement_dir(p):
    '''statement : DIRECTIVE directiveitems
       statement : DIRECTIVE'''

    def gen(data):
        c_p = data[0]
        c_o = data[1]
        outstr = ""
        for item in c_o:
            mapped = mapitem(item)
            outstr += "%s " % mapped
        return "  %s [ %s]" % (c_p[1],outstr )

    global diritems
    data = [ p[:], diritems[:] ]
    output_list.append( [gen,data] )
    diritems = []
    directive = p[1]
    if directive == ".word":
        global avr_pc
        avr_pc += 2

def p_statement_com(p):
    'statement : COMMENT'
    def gen(v):
        return "\n%s\n" % v[1]
    output_list.append([gen,p[:]])

def p_statements(p):
    '''statements : statement statements
       statements : statement'''

#######################################

def p_compilationunit(p):
    "compilationunit : statements"

#######################################

def p_error(p):
    if p:
        print("Syntax error at '%s'" % p.value)
    else:
        print("Syntax error at EOF")

###############################################################################
# run parser
###############################################################################

start = 'compilationunit'

import ply.yacc as yacc
yacc.yacc()

with open("test.tok","w") as fo:
  with open("test.avr","r") as fin:
    output_list = []
    for line in fin:
        yacc.parse(line,debug=False)
    for item in output_list:
        gen = item[0]
        p = item[1]
        #print gen, p
        str = gen(p)
        print str
    #yacc.parse(input)
