%{
#include <iostream>
using namespace std;

extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;
extern int line_num;
void yyerror(const char *s);
%}

%union {
    int ival;
    double fval;
    char *sval;
}

%token <ival> INT;
%token <fval> FLOAT;
%token <sval> STRING;
%token AND
%left MULOP
%left ADDOP
%token COMP

%%

transfoset:
          transfoset transfo
          | transfo
          ;
transfo:
       transfofunction ':' transfoconds ';'
       ;
transfofunction:
               STRING '(' argset ')'
               ;
argset:
      argset ',' arg
      | arg
      ;
arg:
   STRING
   ;
transfoconds:
            transfoconds AND cond
            | cond
            ;
cond:
    exprs COMP exprs
    ;
exprs:
     addexprs
     ;
addexprs:
        addexprs ADDOP mulexprs
        | mulexprs
        ;
mulexprs:
        mulexprs MULOP simexprs
        | simexprs
        ;
simexprs:
        INT
        | FLOAT
        | STRING
        | '(' exprs ')'
        | exprfonc
        ;
exprfonc:
        STRING '(' funcargs ')'
        ;
funcargs:
        funcargs ',' funcarg
        | funcarg
        ;
funcarg:
       exprs
       ;
%%
int main(int argc, char *argv[])
{
    FILE *myfile = fopen("transfo.trs","r");
    if (!myfile)
    {
        cerr << "could not open file." << "\n";
    }
    yyin = myfile;
    do
    {
        yyparse();
    } while(!feof(yyin));
}

void yyerror(const char *s)
{
    cerr << "Error on line " << line_num << " : " << s << "\n";
    exit(-1);
}
