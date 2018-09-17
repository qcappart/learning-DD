/*
 * Taken from http://www.andrew.cmu.edu/user/vanhoeve/mdd/ with the notice:
 * "The software can be freely used but comes with no warranty"
 * -------------------------------------------------
 * Graph data structure
 *
 * Simple graph structure that assumes that
 * arcs/nodes are not removed once inserted.
 * It keeps a redundant representation as an
 * adjacent matrix and list for fast iteration
 * and adjacency check.
 * -------------------------------------------------
 */

#ifndef GRAPH_HPP_
#define GRAPH_HPP_

#include <cassert>
#include <vector>

using namespace std;

/**
 * Graph structure
 */
struct Graph_BDD {

    bool**                      adj_m;              /**< adjacent matrix */
    vector< vector<int> >       adj_list;           /**< adjacent list */

    int                         n_vertices;         /**< |V| */
    int                         n_edges;            /**< |E| */


    /** Set two vertices as adjacents */
    void set_adj(int i, int j);

    /** Check if two vertices are adjancent */
    bool is_adj(int i, int j);

    /** Empty constructor */
    Graph_BDD();

    /** Create an isomorphic graph according to a vertex mapping */
    Graph_BDD(Graph_BDD* graph, vector<int>& mapping);

    /** Read graph from a DIMACS format */
    void read_dimacs(const char* filename);

    /** Export to GML format */
    void export_to_gml(const char* output);
};


/**
 * ----------------------------------------------
 * Inline implementations
 * ----------------------------------------------
 */

/**
 * Empty constructor
 */
inline Graph_BDD::Graph_BDD() : n_vertices(0), n_edges(0) {

}


/**
 * Check if two vertices are adjacent
 */
inline bool Graph_BDD::is_adj(int i, int j) {
    assert(i >= 0);
    assert(j >= 0);
    assert(i < n_vertices);
    assert(j < n_vertices);
    return adj_m[i][j];
}


/**
 * Set two vertices as adjacent
 */
inline void Graph_BDD::set_adj(int i, int j) {
    assert(i >= 0);
    assert(j >= 0);
    assert(i < n_vertices);
    assert(j < n_vertices);

    // check if already adjacent
    if( adj_m[i][j] )
        return;

    // add to adjacent matrix and list
    adj_m[i][j] = true;
    adj_list[i].push_back(j);
}


#endif

