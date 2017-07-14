#ifndef GRAPH_BIN_HPP
#define GRAPH_BIN_HPP

namespace phoeg
{

    class GraphBin
    {
        private:

            long m;

            unsigned long long int graph;

            long get_position(long i, long j)
            {
                if (i < j)
                {
                    long tmp = i;
                    i = j;
                    j = i;
                }
                return (i * (i-1))/2 + j + 4;
            }

        public:

            static const long ORDER_MAX = 11;

            GraphBin(long n);

            long order();

            long size();

            bool is_edge(long i, long j);

            void add_node();

            void add_edge(long i, long j);

            void remove_edge(long i, long j);
    };

    GraphBin::GraphBin(long n): graph(n),m(0) {}

    long GraphBin::order()
    {
        return this->graph & 15;
    }

    long GraphBin::size()
    {
        return this->m;
    }

    bool GraphBin::is_edge(long i, long j)
    {
        //TODO check if overflow
        return i!=j && (this->graph >> get_position(i,j)) & 1 > 0;
    }

    void GraphBin::add_node()
    {
        //TODO Exception if limit reached
        if (this->order() < 11)
        {
            this->graph++;
        }
    }

    void GraphBin::add_edge(long i, long j)
    {
        //TODO check for overflow and exceptions
        if (!this->is_edge(i,j) && i!=j)
        {
            this->m++;
            this->graph = this->graph | (1 << this->get_position(i,j));
        }
    }

    void GraphBin::remove_edge(long i, long j)
    {
        //TODO check for overflow and exceptions
        if (this->is_edge(i,j) && i!=j)
        {
            this->m--;
            this->graph = this->graph & !(1 << this->get_position(i,j));
        }
    }

} // namespace phoeg

#endif
