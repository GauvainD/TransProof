%{
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <string>
#include "mydriver.hpp"
#include "myparser.hh"

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
{blank}+ loc.step();
[\n]+    loc.lines(yyleng); loc.step();
"-"      return yy::myparser::make_MINUS(loc);
"+"      return yy::myparser::make_PLUS(loc);
"*"      return yy::myparser::make_STAR(loc);
"/"      return yy::myparser::make_SLASH(loc);
"("      return yy::myparser::make_LPAREN(loc);
")"      return yy::myparser::make_RPAREN(loc);
":="     return yy::myparser::make_ASSIGN(loc);
{int} {
    errno = 0;
    long n = strtol(yytext,NULL,10);
    if (!(INT_MIN <= n && n <= INT_MAX && errno != ERANGE))
    {
        driver.error(loc,"integer is out of range");
    }
    return yy::myparser::make_NUMBER(n,loc);
      }
{id}    return yy::myparser::make_IDENTIFIER(yytext,loc);
.       driver.error(loc,"invalid character");
<<EOF>> return yy::myparser::make_END(loc);
%%

void mydriver::scan_begin()
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

void mydriver::scan_end()
{
    fclose(yyin);
}
