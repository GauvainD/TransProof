#ifndef CSV_PARSER_HPP
#define CSV_PARSER_HPP
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include <vector>
#include <regex>
#include <map>

#include "transformation_test.hpp"
#include "graph.hpp"
#include "graph6.hpp"

namespace phoeg
{
typedef std::map<std::string, std::map<std::string, struct invariant_value> > node_list;

/**
 * Removes spaces at the begining and end of the
 * sring.
 */
std::string strip(const std::string &str)
{
    std::string res = "";
    long s = 0;
    while (s < str.length() && str[s] == ' ')
    {
        s++;
    }
    long e = str.length() - 1;
    while (e > s && str[e] == ' ')
    {
        e--;
    }
    return str.substr(s, e - s + 1);
}

/**
 * Gives the name of the headers given in parameters.
 * Removes the spaces at start and end of each title.
 */
std::vector<std::string> parseHeader(const std::string &line)
{
    if (line.length() == 0)
    {
        throw std::invalid_argument("headers are empty");
    }
    std::vector<std::string> res;
    boost::tokenizer<boost::escaped_list_separator<char> > tok(line);
    boost::tokenizer<boost::escaped_list_separator<char> >::iterator token = tok.begin();
    for (; token != tok.end(); token++)
    {
        res.push_back(strip(*token));
    }
    return res;
}

/**
 * Escapes backslash in the signature of graphs
 */
std::string cleanSig(const std::string &line)
{
    int i = 0;
    std::string result = "";
    while (i < line.length() && line[i] != ',')
    {
        if (line[i] == '\\')
        {
            result += "\\\\";
        }
        else
        {
            result += line[i];
        }
        i++;
    }
    result += line.substr(i);
    return result;
}

/**
 * Checks weither the given std::string is a graph in the g6 format
 */
bool checkFormat(const std::string &graph)
{
    //OK if empty std::string
    if (graph.size() > 0)
    {
        //TODO what if n is bigger (see graph6)
        int n = graph[0] - 63;
        //First char must be >= 63 and <= 126
        if (n >= 0 && n < 64)
        {
            //We need to have enough chars for the matrix
            int expected_size = (int) ceil((n * (n - 1) / 2) / 6.);
            if (graph.size() - 1 == expected_size)
            {
                for (int i = 1; i < graph.size(); i++)
                {
                    //Each char must be between 63 and 126
                    if (graph[i] < 63 || graph[i] > 126)
                    {
                        return false;
                    }
                }
                return true;
            }
        }
        return false;
    }
    return false;
}

/**
 * Parse the given std::string into the correct type variable
 * using boost::variant.
 * Current variants are bool, float, int and std::string
 */
struct invariant_value parseValue(const std::string &str)
{
    std::string lower = boost::algorithm::to_lower_copy(str);
    struct invariant_value res;
    std::regex f1("-?[0-9]+\\.[0-9]*");
    std::regex f2("-?[0-9]*\\.[0-9]+");
    std::regex i("-?[0-9]+");
    bool match = false;
    if (!lower.compare("true"))
    {
        res.type = BOOL;
        res.val = true;
        match = true;
    }
    else if (!lower.compare("false"))
    {
        res.type = BOOL;
        res.val = false;
        match = true;
    }
    else if (std::regex_match(str, f1) || std::regex_match(str, f2))
    {
        res.type = FLOAT;
        res.val = boost::lexical_cast<double>(str);
        match = true;
    }
    else if (std::regex_match(str, i))
    {
        res.type = INT;
        res.val = boost::lexical_cast<long>(str);
        match = true;
    }
    else if (!match)
    {
        res.type = STRING;
        res.val = str;
    }
    else
    {
        res.type = STRING;
        res.val = "";
    }
    return res;
}

/**
 * Parse a line of a csv file.
 * Each element is converted in one of the supported format.
 * The results are stored in a map with the name of the headers as keys.
 * The line will be stored with the first value as key (id)
 */
std::string parseLine(const std::string &toParse, const std::vector<std::string> &keys, node_list &nodes)
{
    if (toParse.length() == 0)
    {
        throw std::invalid_argument("line is empty");
    }
    std::string line = cleanSig(toParse);
    std::vector<std::string> res;
    boost::tokenizer<boost::escaped_list_separator<char> > tok(line);
    boost::tokenizer<boost::escaped_list_separator<char> >::iterator token = tok.begin();
    std::string sig = *token;
    ++token;
    if (!checkFormat(sig))
    {
        throw std::invalid_argument(sig + " is not a valid graph6");
    }
    int i = 1;
    std::map<std::string, struct invariant_value> node;
    unsigned long long int gtint = g6toInt(sig);
    struct invariant_value val
    {
        types::BIGINT, gtint
    };
    node.emplace(keys[0], val);
    for (; token != tok.end(); token++)
    {
        if (i > keys.size())
        {
            throw std::invalid_argument("Too many columns");
        }
        std::string p = strip(*token);
        if (p.size() > 0 && p != "NaN" && p != "inf")
        {
            node.emplace(keys[i], parseValue(p));
        }
        ++i;
    }
    if (i < keys.size())
    {
        throw std::invalid_argument("Not enough columns");
    }
    nodes.emplace(sig, node);
    return sig;
}

} // namespace phoeg

#endif
