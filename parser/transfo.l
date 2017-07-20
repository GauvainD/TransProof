%{
#include "transfo.tab.h"
using namespace std;
#define YY_DECL extern "C" int yylex()
int line_num = 0;
%}
%%
[ \t]               ;
[,:;\(\)]                   {return yytext[0];}
[aA][nN][dD]        {return AND;}
[lL][eE][tT]        {return LET;}
[0-9]+              {yylval.ival = atoi(yytext); return INT;}
[0-9]+\.[0-9]*      {yylval.ival = atof(yytext); return INT;}
[*/]                {return MULOP;}
[+-]                {return ADDOP;}
[<>=!]=             {yylval.sval = strdup(yytext); return COMP;}
=                   {yylval.sval = strdup(yytext); return EQ;}
[<>]                {yylval.sval = strdup(yytext); return COMP;}
[a-zA-Z0-9]+        {yylval.sval = strdup(yytext); return STRING;}
\n                  {++line_num;}
.                   ;
%%
