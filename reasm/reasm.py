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
   'LABELDEF',
   'IDENTIFIER',
   'DIRECTIVE',
   'HEXNUMBER',
   'NUMBER',
   "COMMENT",
   "OPCODE",
   'STRING'
)

literals = "+-*/()=:;,.@"

t_LABELDEF= r'[_.]?[_a-zA-Z][_a-zA-Z0-9]*:'
t_IDENTIFIER = r'[_.a-zA-Z][_.a-zA-Z0-9]*'
t_DIRECTIVE = r'\.(stabn|stabs|stabd|stabn|file|text|global|type|section|startup|data|size|word|ident)'
t_COMMENT = r'/\*([^*]|[\r\n]|(\*+([^*/]|[\r\n])))*\*+/'
t_STRING = r'\"(.+?)?\"'
t_OPCODE = r'(ldi|lds|st|rjmp|lpm|add|adc|adiw|cpi|cpc|brne|ret)'
t_HEXNUMBER = r'0[xX][\da-f]+'
t_NUMBER = r'[\d]+'

# Define a rule so we can track line numbers
def t_newline(t):
    r'\n+'
    t.lexer.lineno += len(t.value)

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

def getTokensOnLine(tok):
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
# reassembler context data
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
    '''
    if tok.value == ".file":
        print "file(%s)" % tok.value
    if tok.value == ".global":
        print "global(%s)" % tok.value
    if tok.value == ".type":
        print "type(%s)" % tok.value
    if tok.value == ".size":
        print "size(%s)" % tok.value
    if tok.value == ".stabd":
        print "stabd(%s)" % tok.value
    if tok.value == ".stabn":
        print "stabn(%s)" % tok.value
    if tok.value == ".stabs":
        print "stabs(%s)" % tok.value
    if tok.value == ".section":
        print "section(%s)" % tok.value
    if tok.value == ".word":
        print "word(%s)" % tok.value
    '''

###############################################################################
# opcodes
###############################################################################


def onOpcode(toklist):
    global avr_pc
    tok = toklist[0]
    outstr = None
    numtok = len(toklist)
    if tok.value=="ldi": #load immediate
        reg = toklist[1].value
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
        #for item in toklist:
        #    print item
    elif tok.value=="lds": # Load Direct from data space
        reg = toklist[1].value
        if numtok == 4:
            val = toklist[3].value
            outstr = "LDS %s, %s ; Load Direct from data space" % (reg, val)
        elif numtok == 6:
            val = toklist[3].value+toklist[4].value+toklist[5].value
            outstr = "LDS %s, %s ; Load Direct from data space" % (reg, val)
        else:
            outstr = "LDS %s, tokl<%d>" % (reg,numtok)
            assert(false)
    elif tok.value=="add":
        reg = toklist[1].value
        assert(numtok == 4)
        val = toklist[3].value
        outstr = "ADD %s, %s ; add without carry" % (reg, val)
    elif tok.value=="adc":
        reg = toklist[1].value
        assert(numtok == 4)
        val = toklist[3].value
        outstr = "ADC %s, %s ; add witho carry" % (reg, val)
    elif tok.value=="adiw":
        assert(numtok == 4)
        reg = toklist[1].value
        assert(numtok == 4)
        val = toklist[3].value
        outstr = "ADIW %s, %s ; add immed w/ word" % (reg, val)
    elif tok.value=="st":
        reg = toklist[1].value
        assert(numtok == 4)
        val = toklist[3].value
        outstr = "ST %s, %s ; Store Indirect From Register to data space using Index" % (reg, val)
    elif tok.value=="cpi":
        assert(numtok == 4)
        reg = toklist[1].value
        assert(numtok == 4)
        val = toklist[3].value
        outstr = "CPI %s, %s ; compare w/ immed" % (reg, val)
    elif tok.value=="cpc":
        assert(numtok == 4)
        reg = toklist[1].value
        assert(numtok == 4)
        val = toklist[3].value
        outstr = "CPC %s, %s ; compare w/ carry" % (reg, val)
    elif tok.value=="brne":
        assert(numtok == 2)
        dest = toklist[1].value
        fi = labels[dest]
        outstr = "BRNE %s (0x%04x) ; branch if not equal" % (dest,fi)
    #else:
    #    print "opcode(%s)" % tok.value
    if outstr:
        print "[%04x]\t%s ; <%d>" % (avr_pc,outstr,numtok)
    avr_opcodes.append(outstr)
    avr_pc += 1

###############################################################################
# run lexer
###############################################################################

with open("test.tok","w") as fo:
  with open("test.avr","r") as f:
    lines = f.readlines()
    for l in lines:
        lexer.input(l)
        fo.write("############################\n")
        fo.write(l)
        fo.write("\n")
        items = list()
        while True:
            tok = lexer.token()
            if tok == None: 
                break 
            if tok.type == "OPCODE":
                onOpcode(getTokensOnLine(tok))
            elif tok.type == "DIRECTIVE":
                onDirective(getTokensOnLine(tok))
            elif tok.type == "IDENTIFIER":
                onIdentifierAssignment(getTokensOnLine(tok))
            elif tok.type == "COMMENT":
                print "%s" % tok.value
            elif tok.type == "LABELDEF":
                label = tok.value.replace(":","")
                print "\n%s: ; label (avr_pc: 0x%04x)" % (label,avr_pc)
                labels[label] = avr_pc
            else:
                print "parse error: token<%s>" % tok
                assert(False)
            fo.write(str(tok))
            fo.write("\n")
            #yacc.parse(l)
