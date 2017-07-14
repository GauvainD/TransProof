#!/bin/zsh
bison -d transfo.y
flex transfo.l
g++ transfo.tab.c lex.yy.c -lfl -o parser
