%{
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <string>
#include "transfodriver.hpp"
#include "transfoparser.hh"

static yy::location loc;
%}
%option noyywrap nounput batch debug noinput
id [a-zA-Z][a-zA-Z0-9]*
int [0-9]+
blank [ \t]
%{
#define YY_USER_ACTION loc.columns(yyleng);
%}
%%
%{
loc.step();
%}
{blank}+  loc.step();
[\n]+     loc.lines(yyleng); loc.step();
"-"       return yy::transfoparser::make_MINUS(loc);
"+"       return yy::transfoparser::make_PLUS(loc);
"*"       return yy::transfoparser::make_STAR(loc);
"/"       return yy::transfoparser::make_SLASH(loc);
"("       return yy::transfoparser::make_LPAREN(loc);
")"       return yy::transfoparser::make_RPAREN(loc);
","       return yy::transfoparser::make_COMMA(loc);
"=="      return yy::transfoparser::make_EQUAL(loc);
"<="      return yy::transfoparser::make_LEQ(loc);
">="      return yy::transfoparser::make_GEQ(loc);
">"       return yy::transfoparser::make_GT(loc);
"<"       return yy::transfoparser::make_LT(loc);
":"       return yy::transfoparser::make_COLON(loc);
";"       return yy::transfoparser::make_SEMICOL(loc);
"!="      return yy::transfoparser::make_NEQ(loc);
"and"     return yy::transfoparser::make_AND(loc);
{int}     return yy::transfoparser::make_NUMBER(yytext,loc);
{id}      return yy::transfoparser::make_IDENTIFIER(yytext,loc);
.         driver.error(loc,"invalid character");
<<EOF>>   return yy::transfoparser::make_END(loc);
%%

void transfodriver::scan_begin()
{
    yy_flex_debug = trace_scanning;
    if (file.empty() || file == "-") {
        yyin = stdin;
    }
    else if (!(yyin = fopen(file.c_str(),"r"))) {
        error("cannot open file "+file+" : "+strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void transfodriver::scan_end()
{
    fclose(yyin);
}
