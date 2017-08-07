#!/usr/bin/env python

import os
import sys
sys.path.insert(0, "../..")


os.system("avr-g++ -g -O3 test.cpp -S -o test.avr")
os.system("avr-g++ -g -O3 test.cpp -o test.o")
os.system("avr-objdump -t -S -d test.o > test.lst")

import ply.lex as lex

tokens = (
   'IDENTIFIER',
   'DIRECTIVE',
   'HEXNUMBER',
   'NUMBER',
   "COMMENT",
   "OPCODE",
   'STRING'
)

literals = "+-*/()=:;,.@"

t_IDENTIFIER = r'[_.]?[_a-zA-Z][_a-zA-Z0-9]*'
t_DIRECTIVE = r'\.(stabn|stabs|stabd|stabn|file|text|global|type|section|startup|data|size|word|ident)'
t_COMMENT = r'/\*([^*]|[\r\n]|(\*+([^*/]|[\r\n])))*\*+/'
t_STRING = r'\"(.+?)\"'
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

# run lexer

with open("test.tok","w") as fo:
  with open("test.avr","r") as f:
    lines = f.readlines()
    for l in lines:
        lexer.input(l)
        fo.write("############################\n")
        fo.write(l)
        fo.write("\n")
        while True:
            tok = lexer.token()
            if not tok: 
                break 
            fo.write(str(tok))
            fo.write("\n")
            #yacc.parse(l)
