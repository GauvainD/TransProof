#ifndef FILTER_TEST_HPP
#define FILTER_TEST_HPP

#include <jni.h>
#include "graph.hpp"
#include "graph6.hpp"
#include "graph_bin.hpp"
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
    bool isGraph;
    std::string dest;
    unsigned long long int graph;
};

struct filter_result isGraphInList(const std::map<unsigned long long int, jlong > &nodes, const phoeg::Graph &g)
{
    unsigned long long int gi = phoeg::graphToInt(g);
    std::string g6 = phoeg::convertToGraph6(g);
    bool result = nodes.count(gi);
    struct filter_result res = {result, true, g6, gi};
    return res;
}

struct filter_result trashNode(const std::string &trashName, const phoeg::Graph &g)
{
    struct filter_result res = {true, false, trashName, 0};
    return res;
}

#endif
