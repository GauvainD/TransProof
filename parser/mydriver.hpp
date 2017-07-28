#ifndef DRIVER
#define DRIVER
#include <iostream>
#include <string>
#include <map>
#include "myparser.hh"

#define YY_DECL yy::myparser::symbol_type yylex(mydriver &driver)

YY_DECL;

class mydriver
{
public:

    std::map<std::string, int> variables;
    int result;
    std::string file;
    bool trace_scanning;
    bool trace_parsing;

    mydriver() : trace_scanning(false), trace_parsing(false)
    {
    }
    virtual ~mydriver() {}

    void scan_begin();
    void scan_end();

    int parse(const std::string &f)
    {
        file = f;
        scan_begin();
        yy::myparser parser(*this);
        parser.set_debug_level(trace_parsing);
        int res = parser.parse();
        scan_end();
        return res;
    }

    void error(const yy::location& l, const std::string& m)
    {
        std::cerr << l << ": " << m << "\n";
    }

    void error(const std::string& m)
    {
        std::cerr << m << "\n";
    }
};

#endif
