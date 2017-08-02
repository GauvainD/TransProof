%skeleton "lalr1.cc" //C++
%require "3.0.4"
%defines
%define parser_class_name {transfoparser}
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%code requires {
#define YYDEBUG 1
#include <string>
class transfodriver;
}
%param {transfodriver &driver}
%locations
%initial-action {
@$.begin.filename = @$.end.filename = &driver.file;
};
%define parse.trace
%define parse.error verbose
%code {
#include <sstream>
#include <iterator>
#include "transfodriver.hpp"
}
%define api.token.prefix {TOK_}
%token
    END     0     "end  of  file"
    MINUS   "-"
    PLUS    "+"
    STAR    "*"
    SLASH   "/"
    LPAREN  "("
    RPAREN  ")"
    COMMA   ","
    EQUAL   "=="
    LEQ     "<="
    GEQ     ">="
    LT      "<"
    GT      ">"
    COLON   ":"
    NEQ     "!="
    SEMICOL ";"
    AND     "and"
;
%token <std::string> IDENTIFIER "identifier"
%token <std::string> NUMBER "number"
%type <std::string> transfodef transfoset transfo transfonc exp constr
%type <std::string> cstrlist explist
%printer {yyoutput << $$;} <*>;
%%
transfodef: transfoset {$1 += ";";std::swap($$,$1);driver.result = $$;};

transfoset: transfoset ";" transfo {$$ = $1 + ";" + $3;}
          | transfo {std::swap($$,$1);};

transfo: transfonc ":" cstrlist {$$ = $1 + " where " + $3;}
       | transfonc {std::swap($$,$1);};

transfonc: "identifier" "(" idlist ")" {$$ =
         driver.format_transformation(driver.trsvars.size()-1, $1,
         driver.trsvars.back()); std::vector<std::string> v;driver.trsvars.push_back(v);};

idlist: idlist "," "identifier" {driver.trsvars.back().push_back($3);}
      | "identifier" {driver.trsvars.back().push_back($1);}

cstrlist: cstrlist "and" constr {$$ = $1 + "and" + $3;}
        | constr {std::swap($$,$1);};

constr: exp "==" exp {$$ = $1 + "==" + $3;}
      | exp "<=" exp {$$ = $1 + "<=" + $3;}
      | exp ">=" exp {$$ = $1 + ">=" + $3;}
      | exp "!=" exp {$$ = $1 + "!=" + $3;}
      | exp "<" exp  {$$ = $1 + "<"  + $3;}
      | exp ">" exp  {$$ = $1 + ">"  + $3;};

%left "+" "-";
%left "*" "/";
exp: exp "+" exp  {$$ = $1 + "+" + $3;}
   | exp "-" exp  {$$ = $1 + "-" + $3;}
   | exp "*" exp  {$$ = $1 + "*" + $3;}
   | exp "/" exp  {$$ = $1 + "/" + $3;}
   | "identifier" "(" explist ")" {$$ = $1 + "(" + $3 + ")";}
   | "(" exp ")"  {$$ = "(" +  $2 + ")";}
   | "number"     {std::swap($$,$1);};

explist: explist "," exp {$$ = $1 + "," + $3;}
       | explist "," "identifier" {$$ = $1 + "," + $3;}
       | "identifier" {std::swap($$,$1);}
       | exp {std::swap($$,$1);};
       %%
void yy::transfoparser::error(const location_type &l, const std::string &m) {
    driver.error(l,m);
}
