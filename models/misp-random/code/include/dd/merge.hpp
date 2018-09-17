/*
 * Taken from http://www.andrew.cmu.edu/user/vanhoeve/mdd/ with the notice:
 * "The software can be freely used but comes with no warranty"
 * --------------------------------------------------------
 * Node merging class
 * --------------------------------------------------------
 */

#ifndef MERGE_HPP_
#define MERGE_HPP_

#include <cassert>
#include <cstdio>
#include <vector>
#include <list>


#include "instance.hpp"
#include "bdd.hpp"

using namespace std;


// Class representing a general ordering
struct IS_Merging {

	IndepSetInst   		*inst;
	char           		name[256];
	int					width;
	double				gap = 0;

	IS_Merging(IndepSetInst* _inst, int _width) : inst(_inst), width(_width) { }

	// returns vertex corresponding to particular layer
	virtual void merge_layer(int layer, vector<Node*> &nodes_layer) = 0;
};


// Minimum longest path
struct MinLongestPath : IS_Merging {

	MinLongestPath(IndepSetInst *_inst, int _width) : IS_Merging(_inst, _width) {
		sprintf(name, "min_lp");
	}

	void merge_layer(int layer, vector<Node*> &nodes_layer);
};


// Minimum longest path: Pair by Pair
struct PairMinLongestPath : IS_Merging {

	NodeMap	current_states;		/**< current layer states */

	PairMinLongestPath(IndepSetInst *_inst, int _width) : IS_Merging(_inst, _width) {
		sprintf(name, "pair_lp");
	}

	void merge_layer(int layer, vector<Node*> &nodes_layer);
};

// Minimum longest path: Pair by Pair
struct ConsecutivePairLongestPath : IS_Merging {

	vector<Node*> old_nodes;	/**< original set of nodes */

	ConsecutivePairLongestPath(IndepSetInst *_inst, int _width) : IS_Merging(_inst, _width) {
		sprintf(name, "consec");
		if( width != -1 ) {
			old_nodes.reserve(2*width*100);
		}
	}

	void merge_layer(int layer, vector<Node*> &nodes_layer);
};



// Minimum size merger
struct MinSizeMerger : IS_Merging {

	list<Node*> node_list;		/**< list of nodes */

	MinSizeMerger(IndepSetInst *_inst, int _width) : IS_Merging(_inst, _width) {
		sprintf(name, "min_size");
	}

	void merge_layer(int layer, vector<Node*> &nodes_layer);
};

// Maximum size merger
struct MaxSizeMerger : IS_Merging {

	list<Node*> node_list;		/**< list of nodes */

	MaxSizeMerger(IndepSetInst *_inst, int _width) : IS_Merging(_inst, _width) {
		sprintf(name, "max_size");
	}

	void merge_layer(int layer, vector<Node*> &nodes_layer);
};



// Lexicographic merger
struct LexicographicMerger : IS_Merging {

	NodeMap	current_states;		/**< current layer states */

	LexicographicMerger(IndepSetInst *_inst, int _width) : IS_Merging(_inst, _width) {
		sprintf(name, "lex");
	}

	void merge_layer(int layer, vector<Node*> &nodes_layer);
};


// Symmetric Difference merger
struct SymmetricDifferenceMerger : IS_Merging {

	typedef pair<Node*,Node*> NodePair;

	NodeMap					current_states;		/**< current layer states */
	IntSet  				aux;
	vector<NodePair> 		node_pairs;

	vector<int>				symm_diff_vals;		/**< symmetric difference */
	vector<int>				longest_path_val;	/**< value of longest path with pairs are merged */
	vector<int>				indices;

	/**
	 * Comparator by symmetric difference and longest path
	 */
	struct ComparatorNodePairSymmLP {

		vector<int>	 &symm_diff_vals;
		vector<int>	 &longest_path_val;

		ComparatorNodePairSymmLP(vector<int> &_symm_diff_vals, vector<int> &_longest_path_val)
		: symm_diff_vals(_symm_diff_vals), longest_path_val(_longest_path_val) { }

		bool operator()(int pairA, int pairB) {
			if( symm_diff_vals[pairA] != symm_diff_vals[pairB] )
				return symm_diff_vals[pairA] < symm_diff_vals[pairB];
			return longest_path_val[pairA] > longest_path_val[pairB];
		}
	};



	SymmetricDifferenceMerger(IndepSetInst *_inst, int _width) : IS_Merging(_inst, _width) {
		sprintf(name, "symmetric_diff");

		aux.resize(0, inst->graph->n_vertices-1, false);

		node_pairs.reserve(inst->graph->n_vertices * inst->graph->n_vertices+1);
		indices.reserve(inst->graph->n_vertices * inst->graph->n_vertices+1);
		if( width != -1 ) {
			symm_diff_vals.reserve(width*width+1);
			longest_path_val.reserve(width*width+1);
		}
	}

	void merge_layer(int layer, vector<Node*> &nodes_layer);
};


// Random merger
struct RandomMerger : IS_Merging {

	RandomMerger(IndepSetInst *_inst, int _width) : IS_Merging(_inst, _width) {
		sprintf(name, "random");
	}

	void merge_layer(int layer, vector<Node*> &nodes_layer);
};




#endif
