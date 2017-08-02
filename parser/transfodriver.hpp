#ifndef DRIVER
#define DRIVER
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include "transfoparser.hh"

#define YY_DECL yy::transfoparser::symbol_type yylex(transfodriver &driver)

YY_DECL;

class transfodriver
{
public:

    std::string result;
    std::string file;
    std::vector<std::vector<std::string> > trsvars;
    bool trace_scanning;
    bool trace_parsing;

    transfodriver() : trace_scanning(false), trace_parsing(false)
    {
        std::vector<std::string> v;
        trsvars.push_back(v);
    }
    virtual ~transfodriver() {}

    void scan_begin();
    void scan_end();

    int parse(const std::string &f)
    {
        file = f;
        scan_begin();
        yy::transfoparser parser(*this);
        parser.set_debug_level(trace_parsing);
        parser.parse();
        scan_end();
        return 1;
    }

    void error(const yy::location& l, const std::string& m)
    {
        std::cerr << l << ": " << m << "\n";
    }

    void error(const std::string& m)
    {
        std::cerr << m << "\n";
    }

    std::string format_transformation(int pos, const std::string &ident, const
                                      std::vector<std::string> &trsvars)
    {
        std::ostringstream oss;
        oss << "match ()-[r" << pos << ":" << ident << "]->() with ";
        int i = 0;
        for (auto v : trsvars)
        {
            oss << "r" << pos << "." << v << " as " << v;
            ++i;
            if (i < trsvars.size())
            {
                oss << ", ";
            }
        }
        return oss.str();
    }
};

#endif
