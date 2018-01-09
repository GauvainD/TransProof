#ifndef TRANSPROOF_INSERTER_HPP
#define TRANSPROOF_INSERTER_HPP

#include "filter_test.hpp"

class TransProofInserter
{

public :

    virtual int attach() = 0;

    virtual void detach(int threadNum) = 0;

    virtual long createNode(int threadNum, std::map<std::string, struct invariant_value> props, bool isGraph) = 0;

    virtual void addRelationship(int threadNum, long n1, long n2, std::string type, long g, long order) = 0;

    virtual void finish() = 0;
};

#endif
