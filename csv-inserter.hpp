#ifndef CSV_INSERTER_HPP
#define CSV_INSERTER_HPP

#include <iostream>
#include <vector>
#include "inserter.hpp"

//TODO output to file

class CsvInserter: public TransProofInserter
{
protected:

    int threads;
    int graphs;
    std::vector<std::string> headers;
    bool started_transfos;

public:

    CsvInserter()
    {
        threads = 0;
        graphs = 0;
        started_transfos = false;
    }

    CsvInserter(const CsvInserter & other)
    {
        this->threads = other.threads;
        this->graphs = other.graphs;
        this->started_transfos = other.started_transfos;
    }

    CsvInserter &operator=(const CsvInserter & other)
    {
        this->threads = other.threads;
        this->graphs = other.graphs;
        this->started_transfos = other.started_transfos;
        return *this;
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
            std::cout << "id";
            for (long i = 0; i < headers.size(); ++i)
            {
                std::cout << ",";
                std::cout << headers[i];
            }
            std::cout << "\n";
        }
        long id = ++graphs;
        std::cout << id;
        for (long i = 0; i < headers.size(); ++i)
        {
            std::cout << ",";
            if (props.count(headers[i]) > 0)
            {
                std::cout << props[headers[i]].val;
            }
        }
        std::cout << "\n";
        return id;
    }

    virtual void addRelationship(int threadNum, long n1, long n2, std::string
                                 type, long g, long order) override
    {
        if (!started_transfos)
        {
            std::cout << "from,to,type,g,order\n";
            started_transfos = true;
        }
        std::cout << n1 << "," << n2 << "," << type << ",";
        std::cout << g << "," << order << "\n";
    }

    virtual void finish() override
    {
    }

};

#endif
