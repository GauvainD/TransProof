#!/bin/zsh
bison -d myparser.yy
flex myscanner.ll
g++ parser.cpp mydriver.hpp myparser.tab.cc lex.yy.c -lfl -o parser
