#include <cstdio>
#include "GraphBin.hpp"

int main()
{
    phoeg::GraphBin g(6);
    g.add_node();
    printf("%d\n",g.is_edge(0,6));
    g.add_edge(0,6);
    printf("%d\n",g.is_edge(0,6));
    g.remove_edge(0,6);
    printf("%d\n",g.is_edge(0,6));
    printf("Hello World\n");
}
