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
labels = {}
avr_opcodes = []
avr_pc = 0
MOS_OPC = 0

###############################################################################
# identifier (assignments)
###############################################################################

def onIdentifierAssignment(toklist):
    tok = toklist[0].value
    equ = toklist[1].value
    val = toklist[2].value
    numtok = len(toklist)
    assert(numtok==3)
    assert(equ=="=")
    identifiers[tok]=val
    print "%s = %s ; assign identifier" % (tok, val)


###############################################################################
# directives
###############################################################################

def onDirective(toklist):
    tok = toklist[0]
    if tok.value == ".word":
       print "word(%s) : %s" % (tok.value,toklist[1].value)

###############################################################################

_regmap = {
    "r26": "XL",
    "r27": "XH",
    "r28": "YL",
    "r29": "YH",
    "r30": "ZL",
    "r31": "ZH"
}

def mapreg(reg):
    if reg in _regmap:
        reg = _regmap[reg]
    return reg

###############################################################################
# opcodes
###############################################################################


def onOpcode(toklist):
    global avr_pc
    tok = toklist[0]
    outstr = None
    numtok = len(toklist)
    this_avr_pc = avr_pc
    if tok.value=="ldi": #load immediate
        reg = mapreg(toklist[1].value)
        if numtok == 4:
            imm = toklist[3].value
            outstr = "LDI %s, %s ; load immediate" % (reg, imm)
        elif numtok == 7:
            assert(toklist[4].value=="(")
            assert(toklist[6].value==")")
            mod = toklist[3].value # lo8 - bits 0..7 
                                   # hi8 - bits 8..15
                                   # hh8 - bits 16..23
                                   # hhi8 -bits 24..31
            val = toklist[5].value
            outstr = "LDI %s, %s(%s) ; load immediate(sub)" % (reg,mod,val)
        else:
            outstr = "LDI %s, tokl<%d>" % (reg,numtok)
            assert(false)
        avr_pc += 2
        #for item in toklist:
        #    print item
    elif tok.value=="lds": # Load Direct from data space
        reg = mapreg(toklist[1].value)
        if numtok == 4:
            val = toklist[3].value
            outstr = "LDS %s, %s ; Load Direct from data space" % (reg, val)
        elif numtok == 6:
            val = toklist[3].value+toklist[4].value+toklist[5].value
            outstr = "LDS %s, %s ; Load Direct from data space" % (reg, val)
        else:
            outstr = "LDS %s, tokl<%d>" % (reg,numtok)
            assert(false)
        avr_pc += 4
    elif tok.value=="add":
        reg = mapreg(toklist[1].value)
        assert(numtok == 4)
        val = toklist[3].value
        outstr = "ADD %s, %s ; add without carry" % (reg, val)
        avr_pc += 2
    elif tok.value=="adc":
        reg = mapreg(toklist[1].value)
        assert(numtok == 4)
        val = toklist[3].value
        outstr = "ADC %s, %s ; add witho carry" % (reg, val)
        avr_pc += 2
    elif tok.value=="adiw":
        assert(numtok == 4)
        reg = mapreg(toklist[1].value)
        assert(numtok == 4)
        val = toklist[3].value
        outstr = "ADIW %s, %s ; add immed w/ word" % (reg, val)
        avr_pc += 2
    elif tok.value=="st":
        reg = mapreg(toklist[1].value)
        assert(numtok == 4)
        val = toklist[3].value
        if reg=="Z":
            outstr = "ST %s, %s ; Store Indirect From Register to data space using Index Z" % (reg, val)
        else:
            assert(False)
        avr_pc += 2
    elif tok.value=="cpi":
        assert(numtok == 4)
        reg = mapreg(toklist[1].value)
        assert(numtok == 4)
        val = toklist[3].value
        outstr = "CPI %s, %s ; compare w/ immed" % (reg, val)
        avr_pc += 2
    elif tok.value=="cpc":
        assert(numtok == 4)
        reg = mapreg(toklist[1].value)
        assert(numtok == 4)
        val = toklist[3].value
        outstr = "CPC %s, %s ; compare w/ carry" % (reg, val)
        avr_pc += 2
    elif tok.value=="brne":
        assert(numtok == 2)
        dest = toklist[1].value
        fi = labels[dest]
        outstr = "BRNE %s (0x%04x) ; branch if not equal" % (dest,fi)
        avr_pc += 2
    #else:
    #    print "opcode(%s)" % tok.value
    if outstr:
        print "[%04x]\t%s ; <%d>" % (this_avr_pc,outstr,numtok)
    avr_opcodes.append(outstr)

###############################################################################
# Parsing rules
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
            return "expr_node( %s %s %s )" % (self._a,self._op,self._b)
        else:
            return "expr_node( %s )" % (self._a)

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
    print("\n%s:\n" % p[1])

def p_statement_assign(p):
    'statement : IDENTIFIER "=" expression'
    print("  %s = %s\n" % (p[1],p[3]))

def p_statement_opc(p):
    '''statement : OPCODE opcodeitems
       statement : OPCODE'''
    global opcitems
    outstr = ""
    for item in opcitems:
        outstr += "%s " % str(item)
    print("\t%s [ %s]" % (p[1],outstr ))
    opcitems = []

def p_statement_dir(p):
    '''statement : DIRECTIVE directiveitems
       statement : DIRECTIVE'''
    global diritems
    outstr = ""
    for item in diritems:
        outstr += "%s " % str(item)
    print("  %s [ %s]" % (p[1],outstr ))
    diritems = []

def p_statement_com(p):
    'statement : COMMENT'
    print("\n%s\n" % p[1])

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
    for line in fin:
        yacc.parse(line,debug=False)
    #yacc.parse(input)
