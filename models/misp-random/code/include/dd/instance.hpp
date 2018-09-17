/*
 * Taken from http://www.andrew.cmu.edu/user/vanhoeve/mdd/ with the notice:
 * "The software can be freely used but comes with no warranty"
 * --------------------------------------------------------
 * Instance data structures
 * --------------------------------------------------------
 */

#ifndef INSTANCE_HPP_
#define INSTANCE_HPP_

#include "intset.hpp"

#include <fstream>
#include <iostream>
#include <set>


using namespace std;

enum VertexOrder {
  LEX,           /**< lexicographic node ordering */
  CLIQUE,        /**< clique node ordering */
  MIN_CUT,        /**< min. cut node ordering */
  HAM_PATH,       /**< ham path for indep set */
  FEW_VARS_CON    /**< pick the constraint with the fewest number of variables and order them */
};


#include <cstring>
#include <map>
#include "graph.hpp"
#include "intset.hpp"

/**
 * Independent set instance structure
 */
struct SetCoverInst {

	int             					num_vars;
	int             					num_cons;
	int*            					weights;

	vector< vector<int> >     			consts_of_a_var;
	vector< vector<int> >          		vars_in_const;
	vector< vector<bool> >          	is_var_in_const;

//	vector< vector< vector<int> > >     absorption;
//
//	vector< int >             			first_nonempty_abs;
//	int                       			first_first_nonempty_abs;

	void    read_orlib(char* filename);
	void    preprocess();
};


/**
 * Independent set instance structure
 */
struct IndepSetInst {


    Graph_BDD*              graph;             /**< independent set graph */
  int*                weights_inclusion; /**< node weights / inclusion */
  int*                weights_exclusion; /**< node weights / exclusion */

  int*			      weights;

  IntSet*			  adj_mask_compl;	 /**< complement mask of adjacencies */
  map<int, int>       node_mapping;
 
  /** Read DIMACS independent set instance */
  void read_DIMACS(const char* filename);

  /** Read DIMACS independent set instance with costs */
  int read_DIMACS_with_costs(const char* filename, const char* costs_file, const char* params_file);

  /** Build a graph from an adj list */
  void build_instance_from_edge_list(vector<pair<int, int> >, set<int>);

  void build_complete_instance(std::vector< std::vector< std::pair<int, double> > > adj);
  /** Assign weights to vertices */
  void assign_weights();



};



/*
 * ----------------------------------------
 * Inline implementations
 * ----------------------------------------
 */


/**
 * Read DIMACS independent set instance with no costs
 */
inline void IndepSetInst::read_DIMACS(const char *filename) {

  cout << "Reading instance " << filename << endl;

  // read graph
  graph = new Graph_BDD;
  graph->read_dimacs(filename);
  
  cout << "\tnumber of vertices: " << graph->n_vertices << endl;
  cout << "\tnumber of edges: " << graph->n_edges << endl;

  // allocate weights: 1 for inclusion, 0 for exclusion
  weights_inclusion = new int[graph->n_vertices];
  memset(weights_inclusion, -1, sizeof(int)*graph->n_vertices);

  weights_exclusion = new int[graph->n_vertices];
  memset(weights_exclusion, 0, sizeof(int)*graph->n_vertices);

  // create complement mask of adjacencies
  adj_mask_compl = new IntSet[graph->n_vertices];
  for( int v = 0; v < graph->n_vertices; v++ ) {

	  adj_mask_compl[v].resize(0, graph->n_vertices-1, true);
	  for( int w = 0; w < graph->n_vertices; w++ ) {
		  if( graph->is_adj(v,w) ) {
			  adj_mask_compl[v].remove(w);
		  }
	  }

	  // a vertex is adjacent to itself
	  adj_mask_compl[v].remove(v);

  }

  assign_weights();

  cout << "\tdone." << endl;
}

/**
 * Read DIMACS independent set instance with no costs
 */
inline void IndepSetInst::build_instance_from_edge_list(vector< pair<int, int> > edge_list,set<int> covered_set) {

    graph = new Graph_BDD;

    int count = 0;
    for (auto v: covered_set) {
        node_mapping.insert({v,count});
        count++;
    }



    graph->n_vertices = node_mapping.size();
    //cout << "vertices: " << n  << '\n';
    // allocate adjacent matrix
    graph->adj_m = new bool*[graph->n_vertices];
    for( int i = 0; i < graph->n_vertices; i++ ) {
        graph->adj_m[i] = new bool[graph->n_vertices];
        memset(graph->adj_m[i], false, sizeof(bool)*graph->n_vertices);
    }

    // allocate adjacent list
    graph->adj_list.resize(graph->n_vertices);


    for (auto e: edge_list) {
        //cout.flush();
        graph->set_adj(node_mapping.at(e.first),node_mapping.at(e.second));
        graph->set_adj(node_mapping.at(e.second),node_mapping.at(e.first));
    }

    for( int i = 0; i < graph->n_vertices; i++ ) {
        graph->set_adj(i, i);
    }

    int count_edges = 0;
    for( int i = 0; i < graph->n_vertices; i++ ) {
        for( int j = i+1; j < graph->n_vertices; j++ ) {
            if( graph->is_adj(i,j) ) {
                count_edges++;
            }
        }
    }

    graph->n_edges += count_edges;

    // allocate weights: 1 for inclusion, 0 for exclusion
    weights_inclusion = new int[graph->n_vertices];
    memset(weights_inclusion, -1, sizeof(int)*graph->n_vertices);

    weights_exclusion = new int[graph->n_vertices];
    memset(weights_exclusion, 0, sizeof(int)*graph->n_vertices);

    // create complement mask of adjacencies
    adj_mask_compl = new IntSet[graph->n_vertices];
    for( int v = 0; v < graph->n_vertices; v++ ) {

        adj_mask_compl[v].resize(0, graph->n_vertices-1, true);
        for( int w = 0; w < graph->n_vertices; w++ ) {
            if( graph->is_adj(v,w) ) {
                adj_mask_compl[v].remove(w);
            }
        }

        // a vertex is adjacent to itself
        adj_mask_compl[v].remove(v);

    }

    srand(0);
    weights = new int[graph->n_vertices];

    for( int i = 0; i < graph->n_vertices; i++ ) {
        //weights[i] = (1+graph->n_vertices-graph->adj_list[i].size())*3 + rand()%10;
        //weights[i] = graph->n_vertices-graph->adj_list[i].size();
        weights[i] = 1;
    }


}


inline void IndepSetInst::build_complete_instance(std::vector< std::vector< std::pair<int, double> > > adj) {

    graph = new Graph_BDD;



    graph->n_vertices = adj.size();
    //cout << "vertices: " << n  << '\n';
    // allocate adjacent matrix
    graph->adj_m = new bool*[graph->n_vertices];
    for( int i = 0; i < graph->n_vertices; i++ ) {
        graph->adj_m[i] = new bool[graph->n_vertices];
        memset(graph->adj_m[i], false, sizeof(bool)*graph->n_vertices);
    }

    // allocate adjacent list
    graph->adj_list.resize(graph->n_vertices);

    for(int i = 0 ; i < graph->n_vertices ; i++) {
        for (auto& neigh : adj[i]) {
            graph->set_adj(i,neigh.first);
            graph->set_adj(neigh.first,i);
            //cout << i << ' ' << neigh.first << " ; added" <<  endl;
        }
    }


    for( int i = 0; i < graph->n_vertices; i++ ) {
        graph->set_adj(i, i);
    }

    int count_edges = 0;
    for( int i = 0; i < graph->n_vertices; i++ ) {
        for( int j = i+1; j < graph->n_vertices; j++ ) {
            if( graph->is_adj(i,j) ) {
                count_edges++;
            }
        }
    }

    graph->n_edges += count_edges;

    //cout << "\tnumber of vertices: " << graph->n_vertices << endl;
    //cout << "\tnumber of edges: " << graph->n_edges << endl;
    // allocate weights: 1 for inclusion, 0 for exclusion
    weights_inclusion = new int[graph->n_vertices];
    memset(weights_inclusion, -1, sizeof(int)*graph->n_vertices);

    weights_exclusion = new int[graph->n_vertices];
    memset(weights_exclusion, 0, sizeof(int)*graph->n_vertices);

    // create complement mask of adjacencies
    adj_mask_compl = new IntSet[graph->n_vertices];
    for( int v = 0; v < graph->n_vertices; v++ ) {

        adj_mask_compl[v].resize(0, graph->n_vertices-1, true);
        for( int w = 0; w < graph->n_vertices; w++ ) {
            if( graph->is_adj(v,w) ) {
                adj_mask_compl[v].remove(w);
            }
        }

        // a vertex is adjacent to itself
        adj_mask_compl[v].remove(v);

    }

    srand(0);
    weights = new int[graph->n_vertices];

    for( int i = 0; i < graph->n_vertices; i++ ) {
        //weights[i] = (1+graph->n_vertices-graph->adj_list[i].size())*3 + rand()%10;
        //weights[i] = graph->n_vertices-graph->adj_list[i].size();
        weights[i] = 1;
    }

    //cout << "\tdone." << endl;

}


/**
 * Read DIMACS independent set instance
 */
inline int IndepSetInst::read_DIMACS_with_costs(const char *filename, const char* costs_file, const char* params_file) {

  // read graph
  graph = new Graph_BDD;
  graph->read_dimacs(filename);

  cout << "\tnumber of vertices: " << graph->n_vertices << endl;
  cout << "\tnumber of edges: " << graph->n_edges << endl;

  // allocate weights - in this case, they are all 1
  weights_inclusion = new int[graph->n_vertices];
  weights_exclusion = new int[graph->n_vertices];
    
  ifstream costsfile( costs_file );
  for( int i = 0; i < graph->n_vertices; i++ ) {
    costsfile >> weights_inclusion[i];
    costsfile >> weights_exclusion[i];
  }
  costsfile.close();
    
  ifstream paramsfile( params_file );
  int width;
  paramsfile >> width;
    
  vector<int> mapping(graph->n_vertices);
  for( int i = 0; i < graph->n_vertices; i++ ) {
    paramsfile >> mapping[i];
  }
  paramsfile.close();
    
  //reorder(mapping);

  return width;    
}



#endif /* INSTANCE_HPP_ */
