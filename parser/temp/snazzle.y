%{
#include <iostream>
using namespace std;
extern "C" int yylex();
extern "C" int yyparse();
void yyerror(const char *s);
%}

%union {
    int ival;
    float fval;
    char *sval;
}

%token <ival> INT;
%token <fval> FLOAT;
%token <sval> STRING;

%%

snazzle:
       snazzle INT {cout << "found int : " << $2 << "\n";}
       | snazzle FLOAT {cout << "found float : " << $2 << "\n";}
       | snazzle STRING {cout << "found string : " << $2 << "\n";}
       | INT {cout << "found int : " << $1 << "\n";}
       | FLOAT {cout << "found float : " << $1 << "\n";}
       | STRING {cout << "found string : " << $1 << "\n";}
       ;

%%

int main() {
    yyparse();
}

void yyerror(const char *s)
{
    cout << "error : " << s << "\n";
    exit(-1);
}
