/*
 * Taken from http://www.andrew.cmu.edu/user/vanhoeve/mdd/ with the notice:
 * "The software can be freely used but comes with no warranty"
 * solver.hpp
 *
 *  Created on: Jan 28, 2012
 *      Author: Bergman, Cire
 */

#ifndef INDEPSET_SOLVER_HPP_
#define INDEPSET_SOLVER_HPP_

#define EXACT_BDD -1

#include "bdd.hpp"
#include "instance.hpp"
#include "stats.hpp"
#include "intset.hpp"
#include "orderings.hpp"
#include "merge.hpp"

#include <boost/random/discrete_distribution.hpp>
#include <vector>
#include <map>
#include <queue>



using namespace std;


/**
 * Node comparator by relax ub
 */
struct CompareNodesRelaxUB {
  bool operator()(Node* nodeA, Node* nodeB) const {
	  if( nodeA->relax_ub == nodeB->relax_ub ) {
		return( nodeA->state.get_size() > nodeB->state.get_size() );
	  }
	  return nodeA->relax_ub < nodeB->relax_ub;
  }
};


typedef priority_queue<Node*, vector<Node*>, CompareNodesRelaxUB> NodeQueue;

struct Bounds {
	int lb;
	int ub;
};

struct IndepSetSolver {

	NodeMap  						node_list;

	vector<int>						active_vertices;
	int*							in_state_counter;
	int*							active_vertex_map;

	vector<Node*>					nodes_layer;			      /**< nodes in a layer */

	vector<Node*>					temporary_branch_nodes;

	IndepSetInst* 					inst;
	int								width;
	bool						    relax;

	int								layer;


	IS_Ordering*					ordering;			         /**< ordering */
	IS_Merging*						merger;						 /**< merging technique */

	/**
	 * Branch and bound attributes
	 */
	int								global_LB;
	int								global_UB;


	Bounds							branch_and_bound();

	//vector< vector<Node*> >		final_bdd;

	vector<int>						vertex_in_layer;
	int								final_width;
	int								cur_nodes_merged;

	vector<int>						selectable_vertices;

	// added for RL

	int eligible_vertex;
	Node* initial_node;
	NodeMap::iterator node_it;
	NodeMap::iterator existing_node_it;
	int current_vertex;
	Node* node;

	// auxiliaries
	int  generate_relaxation(IntSet &initial_state, int initial_longest_path);
	int  generate_restriction_with_ordering(IntSet &initial_state, int initial_longest_path);
	int  generate_restriction(IntSet &initial_state, int initial_longest_path);

	int  choose_next_vertex_min_size_next_layer();		 /**< for min state ordering */

	int  choose_next_vertex_min_size_next_layer_random(); /**< for random min state ordering */

	void relax_layer_shortestpath();
	void restrict_layer_shortestpath();

	void update_node_match(Node* nodeA, Node* nodeB);

	void initialize(IntSet &initial_state, int initial_longest_path);
	int generate_next_step_relaxation(int next_vertex);
	int generate_next_step_restriction(int next_vertex);
	int get_bound();

	IndepSetSolver(IndepSetInst* _inst, int _width);
};


inline IndepSetSolver::IndepSetSolver(IndepSetInst* _inst, int _width) : inst(_inst), width(_width)
{
	relax = false;

	in_state_counter = new int[inst->graph->n_vertices];
	active_vertex_map = new int[inst->graph->n_vertices];

	if( width != EXACT_BDD )
		nodes_layer.reserve(2*width*100);
	else {
		nodes_layer.reserve(100000000);
	}


	vertex_in_layer.resize(inst->graph->n_vertices+1);

	selectable_vertices.reserve(inst->graph->n_vertices);
}



inline void IndepSetSolver::update_node_match(Node* nodeA, Node* nodeB) {

	nodeA->longest_path = MAX(nodeA->longest_path, nodeB->longest_path);
	nodeB->longest_path = nodeA->longest_path;

}


inline void add_without_repetition(vector<Node*> &v, Node* node) {
	for( vector<Node*>::iterator it = v.begin(); it != v.end(); it++ ) {
		if( (*it) == node )
			return;
	}
	v.push_back(node);
}


/**
 * Merge nodes to meet maximum width.
 */
inline void IndepSetSolver::relax_layer_shortestpath() {

	sort(nodes_layer.begin(), nodes_layer.end(), CompareNodesLongestPath());


	IntSet* state = &(nodes_layer[width-1]->state);
	for( vector<Node*>::iterator node = nodes_layer.begin()+width; node != nodes_layer.end(); ++node) {
		state->union_with((*node)->state);

			delete (*node);
//		}
	}
	nodes_layer.resize(width);

	/**
	 * 2. Equivalence test on this new node
	 */
	Node* node;
	for( int i = 0; i <= width-2; i++ ) {

		node = nodes_layer[i];

		// check if this state already exists in layer nodes
		if( node->state.equals_to(*state) ) {

			// delete the last node
			delete nodes_layer.back();

			// remove it from queue
			nodes_layer.pop_back();

			break;
		}
	}

}


inline int IndepSetSolver::choose_next_vertex_min_size_next_layer() {


	int min = INF;
	int selected_vertex = -1;
	for( int i = 0; i < inst->graph->n_vertices; i++ ) {
		if( in_state_counter[i] > 0 && in_state_counter[i] < min ) {
			selected_vertex = i;
			min = in_state_counter[i];
		}
	}


	return selected_vertex;
}

inline int IndepSetSolver::choose_next_vertex_min_size_next_layer_random() {

	int selected_vertex = -1;

	RandomizedMinInState* min_state = (RandomizedMinInState*) ordering;

	// check if we will randomize our selection or not
	double probabilities[] = {min_state->prob, 1.0-min_state->prob};

	boost::random::discrete_distribution<> dist(probabilities);
	if( dist(min_state->gen) == 0 ) {

		// min state !
		int min = INF;

		for( int i = 0; i < inst->graph->n_vertices; i++ ) {
			if( in_state_counter[i] > 0 && in_state_counter[i] < min ) {
				selected_vertex = i;
				min = in_state_counter[i];
			}
		}

	} else {

		// randomized!
		selectable_vertices.clear();
		for( int v = 0; v < inst->graph->n_vertices; v++ ) {
			if( in_state_counter[v] > 0 ) {
				selectable_vertices.push_back(v);
			}
		}

		assert( selectable_vertices.size() > 0 );
		boost::random::uniform_int_distribution<> vertex_selector(0, (selectable_vertices.size()-1));

		return selectable_vertices[ vertex_selector(min_state->gen) ];
	}


	return selected_vertex;
}




#endif /* INDEPSET_SOLVER_HPP_ */
