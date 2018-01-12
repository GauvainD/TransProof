#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
#include <boost/variant.hpp>
#include <thread>
#include <mutex>
#include <memory>

#include "graph.hpp"
#include "graph6.hpp"
#include "nauty_interface.hpp"
#include "transformation_test.hpp"
#include "filter_test.hpp"
#include "neo4j-inserter.hpp"
#include "csv-inserter.hpp"
#include "graph_bin.hpp"
#include "csv-parser.hpp"
#include <docopt.h>

#define max(a,b) (a) > (b) ? (a) : (b)

static const char USAGE[] =
    R"(Transproof.

usage: transproof --output=<output> [<input>...]

--output=<output>  the file (or prefix) to use as output
<input>        the files to use as input
)";

using namespace std;
using namespace phoeg;

typedef function<vector<struct transformation_result>(const Graph&)> transformation;
typedef function<struct filter_result(const Graph&)> filter;
typedef map<string, map<string, struct invariant_value> > node_list;

struct transformation_entry {
    transformation transfo;
    string name;
};

struct transfo_data_result {
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
shared_ptr<TransProofInserter> neo;
int ntr = 0;

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
    while (!cin.eof()) {
        try {
            getline(cin, line);
            string sig = parseLine(line, keys, nodes);
            jlong n = neo->createNode(0, nodes[sig], true);
            //objects.emplace(sig, n);
            graphs.emplace(g6toInt(sig), n);
        }
        catch (invalid_argument const& e) {
            cerr << n << " : " << e.what() << endl;
        }
        if (n % step == 0) {
            fprintf(stderr, "%d graphs loaded\n", n);
        }
        n++;
    }
}

void addTransfos(vector<struct transfo_data_result> &transfos, map<string, jlong> &objects, int threadNum)
{
    mtx.lock();
    for (auto trs : transfos) {
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
        while (i < filters.size() && !fres.result) {
            i++;
            fres = filters[i](res);
        }
        jlong n1 = graphs[trs.start];
        jlong n2;
        if (!fres.isGraph && !objects.count(fres.dest)) {
            struct invariant_value val = {.type = STRING, .val = fres.dest};
            std::map<string, struct invariant_value> mp;
            mp.emplace("sig", val);
            n2 = neo->createNode(threadNum, mp, false);
            objects.emplace(fres.dest, n2);
        }
        else {
            if (!fres.isGraph) {
                n2 = objects[fres.dest];
            }
            else {
                n2 = graphs[fres.graph];
            }
        }
        neo->addRelationship(threadNum, n1, n2, trs.type, gc, orderToInt(order, n));
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
    if (threadNum == 0) {
        fprintf(stderr, "  0%%\n");
    }
    auto node = graphs.begin();
    vector<struct transfo_data_result> toAdd;
    if (n < graphs.size()) {
        std::advance(node, start);
    }
    while (n < graphs.size()) {
        Graph g(getOrderBin(node->first));
        binToGraph(node->first, g);
        for (auto transfo : transfos) {
            auto results = transfo.transfo(g);
            for (auto result : results) {
                //We suppose that the last filter is trashNode wich always accept
                //And the first one is isGraphInList
                long i = 0;
                string resG6 = convertToGraph6(result.g);
                struct transfo_data_result trs = {.start = node->first, .end = graphToInt(result.g), .type = transfo.name, .desc = result.desc};
                toAdd.push_back(trs);
            }
        }
        if (n % step < inc) {
            addTransfos(toAdd, objects, threadNum);
            if (threadNum == 0) {
                fprintf(stderr, "%3d%%\n", n * 100 / graphs.size());
            }
        }
        n += inc;
        if (n < graphs.size()) {
            std::advance(node, inc);
        }
    }
    addTransfos(toAdd, objects, threadNum);
}

void runThread(int start, int inc)
{
    mtx.lock();
    int numThread = neo->attach();
    mtx.unlock();
    computeTransformations(numThread, start, inc);
    mtx.lock();
    neo->detach(numThread);
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
    //addTransfo(rotation, "rotation");
    //addTransfo(removeEdge, "remove_edge");
    //addTransfo(addEdge, "add_edge");
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
    map<string,docopt::value> args = docopt::docopt(USAGE, {argv+1,argv+argc},true,"Transproof 0.3");
    string output = args["--output"].asString();
    neo = make_shared<CsvInserter>(output);
    //neo = make_shared<Neo4jInserter>(output);
    vector<string> keys;
    auto inputs = args["<input>"].asStringList();
    if (inputs.size() > 0) {
        auto cinbuf = cin.rdbuf();
        for (int i = 0; i < inputs.size(); i++) {
            cerr << "loading file " << inputs[i] << endl;
            ifstream f(inputs[i]);
            cin.rdbuf(f.rdbuf());
            loadGraphs(keys, nodes, objects);
            f.close();
        }
    }
    else {
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
    for (int i = 0; i < ncores - 1; i++) {
        threads.push_back(std::thread(runThread, i + 1, ncores));
    }
    computeTransformations(0, 0, ncores);
    for (int i = 0; i < ncores - 1; i++) {
        threads[i].join();
    }
    std::cout << "total : " << ntr << "\n";
}
