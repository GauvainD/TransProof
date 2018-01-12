#ifndef CSV_INSERTER_HPP
#define CSV_INSERTER_HPP

#include <iostream>
#include <vector>
#include <fstream>
#include "inserter.hpp"

//TODO output to file

class CsvInserter: public TransProofInserter
{
protected:

    int threads;
    int graphs;
    std::vector<std::string> headers;
    bool started_transfos;
    std::ofstream outvertex, outedge;

public:

    CsvInserter(std::string output)
    {
        threads = 0;
        graphs = 0;
        started_transfos = false;
        outvertex.open(output + "-vertex.csv");
        outedge.open(output + "-edge.csv");
    }

    CsvInserter(const CsvInserter & other)
    {
        this->threads = other.threads;
        this->graphs = other.graphs;
        this->started_transfos = other.started_transfos;
        //this->outvertex = other.outvertex;
        outvertex.copyfmt(other.outvertex);
        outvertex.clear(other.outvertex.rdstate());
        outvertex.basic_ios<char>::rdbuf(other.outvertex.rdbuf());
        //this->outedge = other.outedge;
        outedge.copyfmt(other.outedge);
        outedge.clear(other.outedge.rdstate());
        outedge.basic_ios<char>::rdbuf(other.outedge.rdbuf());
    }

    CsvInserter &operator=(const CsvInserter & other)
    {
        this->threads = other.threads;
        this->graphs = other.graphs;
        this->started_transfos = other.started_transfos;
        //this->outvertex = other.outvertex;
        outvertex.copyfmt(other.outvertex);
        outvertex.clear(other.outvertex.rdstate());
        outvertex.basic_ios<char>::rdbuf(other.outvertex.rdbuf());
        //this->outedge = other.outedge;
        outedge.copyfmt(other.outedge);
        outedge.clear(other.outedge.rdstate());
        outedge.basic_ios<char>::rdbuf(other.outedge.rdbuf());
        return *this;
    }

    ~CsvInserter()
    {
        finish();
    }

    int attach() override
    {
        return 0;
    }

    void detach(int threadNum) override
    {
    }

    virtual long createNode(int threadNum, std::map<std::string, struct
                            invariant_value> props, bool isGraph) override
    {
        if (headers.size() < props.size())
        {
            for (auto pair : props)
            {
                bool inside = false;
                for (auto h : headers)
                {
                    if (h == pair.first)
                    {
                        inside = true;
                    }
                }
                if (!inside)
                {
                    headers.push_back(pair.first);
                }
            }
            outvertex << "id";
            for (long i = 0; i < headers.size(); ++i)
            {
                outvertex << ",";
                outvertex << headers[i];
            }
            outvertex << "\n";
        }
        long id = ++graphs;
        outvertex << id;
        for (long i = 0; i < headers.size(); ++i)
        {
            outvertex << ",";
            if (props.count(headers[i]) > 0)
            {
                outvertex << props[headers[i]].val;
            }
        }
        outvertex << "\n";
        return id;
    }

    virtual void addRelationship(int threadNum, long n1, long n2, std::string
                                 type, long g, long order) override
    {
        if (!started_transfos)
        {
            outedge << "from,to,type,g,order\n";
            started_transfos = true;
        }
        outedge << n1 << "," << n2 << "," << type << ",";
        outedge << g << "," << order << "\n";
    }

    virtual void finish() override
    {
        outvertex.close();
        outedge.close();
    }

};

#endif
