#ifndef TRANSFORMATION_TEST_HPP
#define TRANSFORMATION_TEST_HPP

#include "phoeg/src/graph.hpp"
#include "phoeg/src/vertex_iterator.hpp"
#include <queue>
#include <boost/heap/fibonacci_heap.hpp>
#include <iostream>
#include <boost/format.hpp>

struct transformation_result {
    phoeg::Graph g;
    std::string desc;
};

/**
 * Let G = (V,E) the input
 * Let u,v,w in V st (u,v) in E and (u,w) not in E
 * returns G' as G - (u,v) + (u,w)
 */
std::vector<struct transformation_result> rotation(const phoeg::Graph &g)
{
    std::vector<struct transformation_result> results;
    for (phoeg::VertexIterator u(g); !u.last(); ++u) {
        for (phoeg::VertexIterator v(g,u); !v.last(); ++v) {
            if (v != u && edge(*u, *v, g).second) {
                for (phoeg::VertexIterator w(g,v); !w.last(); ++w) {
                    if (w != v && w != u && !edge(*w, *u, g).second) {
                        phoeg::Graph h(g);
                        remove_edge(*u, *v, h);
                        add_edge(*u, *w, h);
                        std::string desc = str(boost::format("remove (%d;%d) and add (%d;%d)")
                                               % *u % *v % *u % *w);
                        struct transformation_result res = {h, desc};
                        results.push_back(res);
                    }
                }
            }
        }
    }
    return results;
}

/**
 * Let G = (V,E) the input
 * Let (u,v) in E
 * returns G' as G - (u,v)
 */
std::vector<struct transformation_result> removeEdge(const phoeg::Graph& g)
{
    std::vector<struct transformation_result> m;
    for (phoeg::VertexIterator u(g); !u.last(); ++u) {
        for (phoeg::VertexIterator v(g,u); !v.last(); ++v) {
            if (!(v <= u) && edge(*u, *v, g).second) {
                phoeg::Graph h(g);
                remove_edge(*u, *v, h);
                std::string desc = str(boost::format("deleted edge between %ld and %ld") % *u % *v);
                struct transformation_result res = {h, desc};
                m.push_back(res);
            }
        }
    }
    return m;
}

/**
 * Let G = (V,E) the input
 * Let u,v in V st (u,v) not in E
 * returns G' as G + (u,v)
 */
std::vector<struct transformation_result> addEdge(const phoeg::Graph& g)
{
    std::vector<struct transformation_result> m;
    for (phoeg::VertexIterator u(g); !u.last(); ++u) {
        for (phoeg::VertexIterator v(g,u); !v.last(); ++v) {
            if (!(v <= u) && !edge(*u, *v, g).second) {
                phoeg::Graph h(g);
                add_edge(*u, *v, h);
                std::string desc = str(boost::format("added edge between %ld and %ld") % *u % *v);
                struct transformation_result res = {h, desc};
                m.push_back(res);
            }
        }
    }
    return m;
}

/**
 * Let G = (V,E) the input
 * Let u,v,w in V st (u,v),(v,w) in E and (u,w) not in E
 * returns G' as G - (u,v) + (u,w)
 */
std::vector<struct transformation_result> slide(const phoeg::Graph &g)
{
    std::vector<struct transformation_result> results;
    for (phoeg::VertexIterator u(g); !u.last(); ++u) {
        for (phoeg::VertexIterator v(g,u); !v.last(); ++v) {
            if (u != v && edge(*u, *v, g).second) {
                for (phoeg::VertexIterator w(g,v); !w.last(); ++w) {
                    if (w != v && w != u && !edge(*w, *u, g).second && edge(*w, *v, g).second) {
                        phoeg::Graph h(g);
                        remove_edge(*u, *v, h);
                        add_edge(*u, *w, h);
                        std::string desc = str(boost::format("remove (%d;%d) and add (%d;%d)") % *u % *v % *u % *w);
                        struct transformation_result res = {h, desc};
                        results.push_back(res);
                    }
                }
            }
        }
    }
    return results;
}

/**
 * Let G = (V,E) the input
 * Let u,v,w,x in V st (u,v) in E and (w,x) not in E
 * returns G' as G - (u,v) + (w,x)
 */
std::vector<struct transformation_result> moveEdge(const phoeg::Graph &g)
{
    std::vector<struct transformation_result> results;
    for (phoeg::VertexIterator u(g); !u.last(); ++u) {
        for (phoeg::VertexIterator v(g,u); !v.last(); ++v) {
            if (!(v <= u) && edge(*u, *v, g).second) {
                for (phoeg::VertexIterator w(g,v); !w.last(); ++w) {
                    if (w != v && w != u) {
                        for (phoeg::VertexIterator x(g,w); !x.last(); ++x) {
                            if (!(x <= w) && !edge(*x, *w, g).second) {
                                phoeg::Graph h(g);
                                remove_edge(*u, *v, h);
                                add_edge(*w, *x, h);
                                std::string desc = str(boost::format("remove (%d;%d) and add (%d;%d)") % *u % *v % *w % *x);
                                struct transformation_result res = {h, desc};
                                results.push_back(res);
                            }
                        }
                    }
                }
            }
        }
    }
    return results;
}

/**
 * Let G = (V,E) the input
 * Let u,v,w,x in V st (u,v) in E and (w,x) not in E and x != u and v
 * returns G' as G - (u,v) + (w,x)
 * this is the move without the rotations (when x = u or v)
 * using this transformation and rotation is equivalent to use move
 */
std::vector<struct transformation_result> moveDistinct(const phoeg::Graph &g)
{
    std::vector<struct transformation_result> results;
    for (phoeg::VertexIterator u(g); !u.last(); ++u) {
        for (phoeg::VertexIterator v(g,u); !v.last(); ++v) {
            if (!(v <= u) && edge(*u, *v, g).second) {
                for (phoeg::VertexIterator w(g,v); !w.last(); ++w) {
                    if (w != v && w != u) {
                        for (phoeg::VertexIterator x(g,w); !x.last(); ++x) {
                            if (!(x <= w) && x != v && x != u && !edge(*x, *w, g).second) {
                                phoeg::Graph h(g);
                                remove_edge(*u, *v, h);
                                add_edge(*w, *x, h);
                                std::string desc = str(boost::format("remove (%d;%d) and add (%d;%d)") % *u % *v % *w % *x);
                                struct transformation_result res = {h, desc};
                                results.push_back(res);
                            }
                        }
                    }
                }
            }
        }
    }
    return results;
}

/**
 * Let G = (V,E) the input
 * let u,v,w in V st (u,v) in E and (u,w),(w,v) not in E
 * returns G' = G - (u,v) + (u,w),(v,w)
 */
std::vector<struct transformation_result> detour(const phoeg::Graph &g)
{
    std::vector<struct transformation_result> results;
    for (phoeg::VertexIterator u(g); !u.last(); ++u) {
        for (phoeg::VertexIterator v(g,u); !v.last(); ++v) {
            if (!(v <= u) && edge(*u, *v, g).second) {
                for (phoeg::VertexIterator w(g,v); !w.last(); ++w) {
                    if (w != v && w != u && !edge(*w, *u, g).second && !edge(*v, *w, g).second) {
                        phoeg::Graph h(g);
                        remove_edge(*u, *v, h);
                        add_edge(*u, *w, h);
                        add_edge(*v, *w, h);
                        std::string desc = str(boost::format("remove (%d;%d) and add (%d;%d) and (%d;%d)") % *u % *v % *u % *w % *v % *w);
                        struct transformation_result res = {h, desc};
                        results.push_back(res);
                    }
                }
            }
        }
    }
    return results;
}

/**
 * Let G = (V,E) the input
 * let u,v,w in V st (u,w),(w,v) in E and (u,v) not in E
 * returns G' = G - (u,w),(v,w) + (u,v)
 */
std::vector<struct transformation_result> shortcut(const phoeg::Graph &g)
{
    std::vector<struct transformation_result> results;
    for (phoeg::VertexIterator u(g); !u.last(); ++u) {
        for (phoeg::VertexIterator v(g,u); !v.last(); ++v) {
            if (!(v <= u) && !edge(*u, *v, g).second) {
                for (phoeg::VertexIterator w(g,v); !w.last(); ++w) {
                    if (w != v && w != u && edge(*w, *u, g).second && edge(*v, *w, g).second) {
                        phoeg::Graph h(g);
                        remove_edge(*u, *w, h);
                        remove_edge(*v, *w, h);
                        add_edge(*u, *v, h);
                        std::string desc = str(boost::format("remove (%d;%d) and (%d;%d) and add (%d;%d)") % *u % *w % *v % *w % *u % *v);
                        struct transformation_result res = {h, desc};
                        results.push_back(res);
                    }
                }
            }
        }
    }
    return results;
}

/**
 * Let G = (V,E) the input
 * Let u,v,w,x in V st (u,v),(w,x) in E and (u,w),(v,x) not in E
 * returns G' as G - (u,v),(w,x) + (u,w),(v,x)
 * TODO check transformations specific symmetries
 */
std::vector<struct transformation_result> twoOpt(const phoeg::Graph &g)
{
    std::vector<struct transformation_result> results;
    for (phoeg::VertexIterator u(g); !u.last(); ++u) {
        for (phoeg::VertexIterator v(g,u); !v.last(); ++v) {
            if (!(v <= u) && edge(*u, *v, g).second) {
                for (phoeg::VertexIterator w(g,v); !w.last(); ++w) {
                    if (!(w <= u) && w != v && !edge(*u, *w, g).second) {
                        for (phoeg::VertexIterator x(g,w); !x.last(); ++x) {
                            if (x != w && x != v && !edge(*x, *v, g).second && edge(*x, *w, g).second) {
                                phoeg::Graph h(g);
                                remove_edge(*u, *v, h);
                                remove_edge(*w, *x, h);
                                add_edge(*w, *u, h);
                                add_edge(*v, *x, h);
                                std::string desc = str(boost::format("remove (%d;%d) and (%d;%d) and add (%d;%d) and (%d;%d)") % *u % *v % *w % *x % *u % *w % *v % *x);
                                struct transformation_result res = {h, desc};
                                results.push_back(res);
                            }
                        }
                    }
                }
            }
        }
    }
    return results;
}

/**
 * Let G = (V,E) the input
 * Let u,v,w,x in V st (u,v),(w,x) in E and (u,w),(v,x) not in E
 * returns G' as G - (u,v),(w,x) + (u,w),(v,x)
 */
//std::vector<struct transformation_result> twoOpt(const phoeg::Graph &g)
//{
//std::vector<struct transformation_result> results;
//std::pair<phoeg::vertex_iter, phoeg::vertex_iter> u_iter = vertices(g);
//for (; u_iter.first != u_iter.second; u_iter.first++) {
//std::pair<phoeg::vertex_iter, phoeg::vertex_iter> v_iter = vertices(g);
//for (; v_iter.first != v_iter.second; v_iter.first++) {
//if (u_iter.first != v_iter.first && edge(*u_iter.first, *v_iter.first, g).second) {
//std::pair<phoeg::vertex_iter, phoeg::vertex_iter> w_iter = vertices(g);
//for (; w_iter.first != w_iter.second; w_iter.first++) {
//if (w_iter.first != v_iter.first && w_iter.first != u_iter.first && !edge(*u_iter.first, *w_iter.first, g).second) {
//std::pair<phoeg::vertex_iter, phoeg::vertex_iter> x_iter = vertices(g);
//for (; x_iter.first != x_iter.second; x_iter.first++) {
//if (x_iter.first != v_iter.first && x_iter.first != w_iter.first && !edge(*x_iter.first, *v_iter.first, g).second && edge(*x_iter.first, *w_iter.first, g).second) {
//phoeg::Graph h(g);
//remove_edge(*u_iter.first, *v_iter.first, h);
//remove_edge(*w_iter.first, *x_iter.first, h);
//add_edge(*w_iter.first, *u_iter.first, h);
//add_edge(*v_iter.first, *x_iter.first, h);
//std::string desc = str(boost::format("remove (%d;%d) and (%d;%d) and add (%d;%d) and (%d;%d)") % *u_iter.first % *v_iter.first % *w_iter.first % *x_iter.first % *u_iter.first % *w_iter.first % *v_iter.first % *x_iter.first);
//struct transformation_result res = {h, desc};
//results.push_back(res);
//}
//}
//}
//}
//}
//}
//}
//return results;
//}

/**
 * Reconstruct paths from the predecessors lists
 */
std::vector<std::vector<long>> makeShortestsPaths(long start, long end, const std::vector<std::vector<long>> &preds)
{
    std::vector<std::vector<long>> res;
    if (end == start) {
        std::vector<long> v;
        v.push_back(start);
        res.push_back(v);
        return res;
    }
    else {
        std::vector<std::vector<long>> res;
        for (auto p : preds[end]) {
            std::vector<std::vector<long>> paths = makeShortestsPaths(start, p, preds);
            for (auto path : paths) {
                path.push_back(end);
                res.push_back(path);
            }
        }
        return res;
    }
}

/**
 * Returns all the shortests paths between two nodes.
 */
std::vector<std::vector<long>> allShortestPaths(const phoeg::Graph &g, long start, long end)
{
    long n = phoeg::order(g);
    std::vector<long> lengths(n);
    long min;
    long minInd;
    auto compareFunc = [&](long i, long j) {
        return lengths[i] > lengths[j];
    };
    //Using a fibonacci heap to make sure updates to length change the order
    boost::heap::fibonacci_heap<long, boost::heap::compare<decltype(compareFunc)>> todo(compareFunc);
    std::vector<std::vector<long>> preds(n);
    std::vector<boost::heap::fibonacci_heap<long, boost::heap::compare<decltype(compareFunc)>>::handle_type> handles(n);
    for (long i = 0; i < n; i++) {
        lengths[i] = n;
    }
    lengths[start] = 0;
    //Adding to queue after initializing the distances
    for (long i = 0; i < n; i++) {
        handles[i] = todo.push(i);
    }
    while (todo.size() > 0) {
        min = todo.top();
        todo.pop();
        for (long i = 0; i < n; i++) {
            if (edge(min, i, g).second) {
                if (lengths[min] + 1 <= lengths[i]) {
                    if (lengths[min] + 1 < lengths[i]) {
                        lengths[i] = lengths[min] + 1;
                        preds[i].clear();
                        todo.update(handles[i], i);
                    }
                    preds[i].push_back(min);
                }
            }
        }
    }
    return makeShortestsPaths(start, end, preds);
}

std::vector<std::vector<long>> diameterPaths(const phoeg::Graph &g)
{
    long diam = phoeg::diameter(g);
    long n = phoeg::order(g);
    phoeg::dMatrix dist = phoeg::distanceMatrix(g);
    std::vector<std::vector<long>> paths;
    for (long i = 0; i < n; i++) {
        for (long j = i; j < n; j++) {
            if (dist[i][j] == diam) {
                std::vector<std::vector<long>> allPaths = allShortestPaths(g, i, j);
                for (auto path : allPaths) {
                    paths.push_back(path);
                }
            }
        }
    }
    return paths;
}

std::vector<long> degreeList(const phoeg::Graph & g)
{
    std::vector<long> list(phoeg::order(g), 0);
    typedef typename boost::graph_traits<phoeg::Graph>::edge_iterator eiter;
    std::pair<eiter, eiter> ep;
    for (ep = edges(g); ep.first != ep.second; ++ep.first) {
        int u = source(*ep.first, g), v = target(*ep.first, g);
        list[u]++;
        list[v]++;
    }
    return list;
}

/**
 * Let P be a diametric path, u a node of P of degree at least 3, v a node that
 * is not in P and neighboor of u and p the extremity of P that is the furthest
 * from u. We apply the rotation (v,u,p)
 */
std::vector<struct transformation_result> rotationAnthony(const phoeg::Graph &g)
{
    std::vector<struct transformation_result> results;
    long n = phoeg::order(g);
    std::vector<std::vector<long>> diamPaths = diameterPaths(g);
    std::vector<long> degrees = degreeList(g);
    long u, p;
    float mid;
    for (auto path : diamPaths) {
        for (long i = 0; i < path.size(); i++) {
            u = path[i];
            if (degrees[u] >= 3) {
                for (long v = 0; v < n; v++) {
                    if (!(i > 0 && v == path[i - 1]) && !(i < path.size() - 1 && v == path[i + 1]) && edge(u, v, g).second) {
                        mid = (path.size() + 1) / 2.0 - 1;
                        if (i >= mid) {
                            p = path[0];
                            phoeg::Graph h(g);
                            remove_edge(u, v, h);
                            add_edge(p, v, h);
                            std::string desc = str(boost::format("remove (%d;%d) and add (%d;%d)") % u % v % p % v);
                            struct transformation_result res = {h, desc};
                            results.push_back(res);
                        }
                        if (i <= mid) {
                            p = path[path.size() - 1];
                            phoeg::Graph h(g);
                            remove_edge(u, v, h);
                            add_edge(p, v, h);
                            std::string desc = str(boost::format("remove (%d;%d) and add (%d;%d)") % u % v % p % v);
                            struct transformation_result res = {h, desc};
                            results.push_back(res);
                        }
                    }
                }
            }
        }
    }
    return results;
}

#endif
