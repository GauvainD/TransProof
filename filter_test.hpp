#ifndef FILTER_TEST_HPP
#define FILTER_TEST_HPP

#include "graph.hpp"
#include <boost/variant.hpp>

enum types
{
    STRING,
    BOOL,
    INT,
    FLOAT,
    BIGINT,
};

struct invariant_value
{
    types type;
    boost::variant<bool, double, long, std::string, unsigned long long int> val;
};

struct filter_result
{
    bool result;
    std::string dest;
};

struct filter_result isGraphInList(const std::map<std::string, std::map<std::string, struct invariant_value> > &nodes, const std::string &g6, const phoeg::Graph &g)
{
    bool result = nodes.count(g6);
    struct filter_result res = {result, g6};
    return res;
}

struct filter_result trashNode(const std::string &trashName, const std::string &g6, const phoeg::Graph &g)
{
    struct filter_result res = {true, trashName};
    return res;
}

#endif
