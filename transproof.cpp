#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include <regex>
#include <thread>
#include <mutex>

#include "graph.hpp"
#include "graph6.hpp"
#include "nauty_interface.hpp"
#include "transformation_test.hpp"
#include "filter_test.hpp"
#include "neo4j-inserter.hpp"
#include "graph_bin.hpp"

#define max(a,b) (a) > (b) ? (a) : (b)

using namespace std;
using namespace phoeg;

typedef function<vector<struct transformation_result>(const Graph&)> transformation;
typedef function<struct filter_result(const Graph&)> filter;
typedef map<string, map<string, struct invariant_value> > node_list;

struct transformation_entry
{
    transformation transfo;
    string name;
};

struct transfo_data_result
{
    unsigned long long int start;
    unsigned long long int end;
    string type;
    string desc;
};

vector<filter> filters;
vector<struct transformation_entry> transfos;
map<string, jlong> objects;
map<unsigned long long int, jlong> graphs;
node_list nodes;
std::mutex mtx;
Neo4jInserter neo;
int ntr = 0;

/**
 * Checks weither the given string is a graph in the g6 format
 */
bool checkFormat(const string &graph)
{
    //OK if empty string
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
 * Parse the given string into the correct type variable
 * using boost::variant.
 * Current variants are bool, float, int and string
 */
struct invariant_value parseValue(const string &str)
{
    string lower = boost::algorithm::to_lower_copy(str);
    struct invariant_value res;
    regex f1("-?[0-9]+\\.[0-9]*");
    regex f2("-?[0-9]*\\.[0-9]+");
    regex i("-?[0-9]+");
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
    else if (regex_match(str, f1) || regex_match(str, f2))
    {
        res.type = FLOAT;
        res.val = boost::lexical_cast<double>(str);
        match = true;
    }
    else if (regex_match(str, i))
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
 * Removes spaces at the begining and end of the
 * sring.
 */
string strip(const string &str)
{
    string res = "";
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
 * Escapes backslash in the signature of graphs
 */
string cleanSig(const string &line)
{
    int i = 0;
    string result = "";
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
 * Gives the name of the headers given in parameters.
 * Removes the spaces at start and end of each title.
 */
vector<string> parseHeader(const string &line)
{
    if (line.length() == 0)
    {
        throw invalid_argument("headers are empty");
    }
    vector<string> res;
    boost::tokenizer<boost::escaped_list_separator<char> > tok(line);
    boost::tokenizer<boost::escaped_list_separator<char> >::iterator token = tok.begin();
    for (; token != tok.end(); token++)
    {
        res.push_back(strip(*token));
    }
    return res;
}

/**
 * Parse a line of a csv file.
 * Each element is converted in one of the supported format.
 * The results are stored in a map with the name of the headers as keys.
 * The line will be stored with the first value as key (id)
 */
string parseLine(const string &toParse, const vector<string> &keys, node_list &nodes)
{
    if (toParse.length() == 0)
    {
        throw invalid_argument("line is empty");
    }
    string line = cleanSig(toParse);
    vector<string> res;
    boost::tokenizer<boost::escaped_list_separator<char> > tok(line);
    boost::tokenizer<boost::escaped_list_separator<char> >::iterator token = tok.begin();
    string sig = *token;
    ++token;
    if (!checkFormat(sig))
    {
        throw invalid_argument(sig + " is not a valid graph6");
    }
    int i = 1;
    map<string, struct invariant_value> node;
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
            throw invalid_argument("Too many columns");
        }
        string p = strip(*token);
        if (p.size() > 0 && p != "NaN" && p != "inf")
        {
            node.emplace(keys[i], parseValue(p));
        }
        ++i;
    }
    if (i < keys.size())
    {
        throw invalid_argument("Not enough columns");
    }
    nodes.emplace(sig, node);
    return sig;
}

/**
 * Loads the graphs and their properties given in the standard entry in csv format.
 * The headers are the names of each properties and the first property is the id of the graph (graph6)
 * We suppose that the files are in csv format and the first elem is the signature.
 * We suppose that the files have headers.
 */
void loadGraphs(vector<string> &keys, node_list &nodes, map<string, jlong> &objects)
{
    string line;
    int n = 0;
    int step = 100;
    getline(cin, line);
    keys = parseHeader(line);
    while (!cin.eof())
    {
        try
        {
            getline(cin, line);
            string sig = parseLine(line, keys, nodes);
            jlong n = neo.createNode(0, nodes[sig], true);
            //objects.emplace(sig, n);
            graphs.emplace(g6toInt(sig), n);
        }
        catch (invalid_argument const& e)
        {
            cerr << n << " : " << e.what() << endl;
        }
        if (n % step == 0)
        {
            fprintf(stderr, "%d graphs loaded\n", n);
        }
        n++;
    }
}

void addTransfos(vector<struct transfo_data_result> &transfos, map<string, jlong> &objects, int threadNum)
{
    mtx.lock();
    for (auto trs : transfos)
    {
        //We suppose that the last filter is trashNode wich always accept
        //And the first one is isGraphInList
        long i = 0;
        Graph res(getOrderBin(trs.end));
        binToGraph(trs.end, res);
        long n = order(res);
        long gc = graphToInt(res);
        int order[n];
        res = cannonFormOrder(res, order);
        struct filter_result fres = filters[i](res);
        while (i < filters.size() && !fres.result)
        {
            i++;
            fres = filters[i](res);
        }
        jlong n1 = graphs[trs.start];
        jlong n2;
        if (!fres.isGraph && !objects.count(fres.dest))
        {
            struct invariant_value val = {.type = STRING, .val = fres.dest};
            std::map<string, struct invariant_value> mp;
            mp.emplace("sig", val);
            n2 = neo.createNode(threadNum, mp, false);
            objects.emplace(fres.dest, n2);
        }
        else
        {
            if (!fres.isGraph)
            {
                n2 = objects[fres.dest];
            }
            else
            {
                n2 = graphs[fres.graph];
            }
        }
        neo.addRelationship(threadNum, n1, n2, trs.type, gc, orderToInt(order, n));
        ++ntr;
    }
    transfos.clear();
    mtx.unlock();
}

void computeTransformations(int threadNum, int start, int inc)
{
    long n = start;
    long step = max(100, graphs.size() / 100);
    //long step = 1;
    if (threadNum == 0)
    {
        fprintf(stderr, "  0%%\n");
    }
    auto node = graphs.begin();
    vector<struct transfo_data_result> toAdd;
    if (n < graphs.size())
    {
        std::advance(node, start);
    }
    while (n < graphs.size())
    {
        Graph g(getOrderBin(node->first));
        binToGraph(node->first, g);
        for (auto transfo : transfos)
        {
            auto results = transfo.transfo(g);
            for (auto result : results)
            {
                //We suppose that the last filter is trashNode wich always accept
                //And the first one is isGraphInList
                long i = 0;
                string resG6 = convertToGraph6(result.g);
                struct transfo_data_result trs = {.start = node->first, .end = graphToInt(result.g), .type = transfo.name, .desc = result.desc};
                toAdd.push_back(trs);
            }
        }
        if (n % step < inc)
        {
            addTransfos(toAdd, objects, threadNum);
            if (threadNum == 0)
            {
                fprintf(stderr, "%3d%%\n", n * 100 / graphs.size());
            }
        }
        n += inc;
        if (n < graphs.size())
        {
            std::advance(node, inc);
        }
    }
    addTransfos(toAdd, objects, threadNum);
}

void runThread(int start, int inc)
{
    mtx.lock();
    int numThread = neo.attach();
    mtx.unlock();
    computeTransformations(numThread, start, inc);
    mtx.lock();
    neo.detach(numThread);
    mtx.unlock();
}

void addTransfo(transformation transfo, std::string name)
{
    struct transformation_entry trs;
    trs.transfo = transfo;
    trs.name = name;
    transfos.push_back(trs);
}

void initTransfos()
{
    //Vector containing the transformation functions.
    //Add your transformations here
    addTransfo(rotation, "rotation");
    addTransfo(removeEdge, "remove_edge");
    addTransfo(addEdge, "add_edge");
    addTransfo(slide, "slide");
    //addTransfo(moveEdge, "move");
    addTransfo(moveDistinct, "move");
    addTransfo(detour, "detour");
    addTransfo(shortcut, "shortcut");
    addTransfo(twoOpt, "two_opt");
    //addTransfo(rotationAnthony, "rotAnthony");
}

int main(int argc, char *argv[])
{
    vector<string> keys;
    if (argc > 1)
    {
        auto cinbuf = cin.rdbuf();
        for (int i = 1; i < argc; i++)
        {
            cerr << "loading file " << argv[i] << endl;
            ifstream f(argv[i]);
            cin.rdbuf(f.rdbuf());
            loadGraphs(keys, nodes, objects);
        }
    }
    else
    {
        loadGraphs(keys, nodes, objects);
    }
    initTransfos();
    //Vector containing the filters.
    //The first filter should always be isGraphInList
    //And the last one should be trashNode
    filters.push_back(bind(isGraphInList, graphs, placeholders::_1));
    //Add your filters here
    filters.push_back(bind(trashNode, "TRASH", placeholders::_1));
    int ncores = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    for (int i = 0; i < ncores - 1; i++)
    {
        threads.push_back(std::thread(runThread, i + 1, ncores));
    }
    computeTransformations(0, 0, ncores);
    for (int i = 0; i < ncores - 1; i++)
    {
        threads[i].join();
    }
    std::cout << "total : " << ntr << "\n";
}
