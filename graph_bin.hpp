#ifndef GRAPH_BIN_HPP
#define GRAPH_BIN_HPP

#include "graph.hpp"
#include "graph6.hpp"
#include "invariants.hpp"

/**
 * These functions provides a way to use graphs stored in a binary format.
 * This format is the following :
 * We only store the lower triangle part of the adjencency matrix and the
 * diagonal (this is used to store informations about the vertices).
 * Each line is stored from left to right but written from right to left.
 * The lines are stored from right to left.
 * This way, adding a vertex just adds a line at the bottom of the matrix and
 * this just requires using more bits to the left and no changes.
 * ex for C_5 :
 * 0 1 0 1      0
 * 1 0 1 0   -> 1 0       -> 1010 | 010 | 01 | 0 | 0101
 * 0 1 0 1      0 1 0
 * 1 0 1 0      1 0 1 0
 */

namespace phoeg
{

/**
 * Returns the position of the bit corresponding to the cell in position (i,j)
 * in the matrix stored in a number with the format explained here above.
 */
int getPosition(int i, int j)
{
    if (i < j)
    {
        int tmp = i;
        i = j;
        j = tmp;
    }
    return (i * (i + 1)) / 2 + j + 4;
}

/**
 * Set the bit corresponding to the cell (i,j) to 1 in the number g.
 */
unsigned long long int setBit(unsigned long long int g, int i, int j)
{
    return g | (1ull << getPosition(i, j));
}

/**
 * Set the bit corresponding to the cell (i,j) to 0 in the number g.
 */
unsigned long long int unsetBit(unsigned long long g, int i, int j)
{
    return g & ~(1ull << getPosition(i, j));
}

/**
 * Returns the value of the bit corresponding to the cell (i,j) in the number g.
 */
bool getBit(unsigned long long g, int i, int j)
{
    return (g >> getPosition(i, j)) & 1;
}

int getOrderBin(unsigned long long int g)
{
    return g & 15;
}


/**
 * Converts a graph to the integer notation.
 */
unsigned long long int graphToInt(const Graph & g)
{
    long n = order(g);
    unsigned long long int num = n;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < i; j++)
        {
            if (edge(i, j, g).second)
            {
                num = setBit(num, i, j);
            }
        }
    }
    return num;
}

/**
 * Converts an integer to a graph.
 */
void binToGraph(unsigned long long int b, Graph & g)
{
    long n = order(g);
    for (long i = 0; i < n; ++i)
    {
        for (long j = 0; j < i; ++j)
        {
            if (getBit(b, i, j))
            {
                add_edge(i, j, g);
            }
        }
    }
}

/**
 * Converts a graph6 signature to the integer notation.
 */
unsigned long long int g6toInt(const std::string & s)
{
    std::string g6(s);
    long n = detail::decodeOrderGraph6(g6);
    unsigned long long int numg = n;
    if (g6.size() > 0)
    {
        char v = g6[0] - 63;
        int l = 5;
        long p = 0;
        for (long i = 1; i < n; i++)
        {
            for (long j = 0; j < i; j++)
            {
                if (l < 0)
                {
                    l = 5;
                    p++;
                    v = g6[p] - 63;
                }
                //We get the value of the bit in position l
                if ((v % (1 << (l + 1))) >> l)
                {
                    numg = setBit(numg, i, j);
                }
                l--;
            }
        }
    }
    return numg;
}


} // namespace phoeg

#endif
