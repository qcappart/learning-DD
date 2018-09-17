/*
 * Taken from http://www.andrew.cmu.edu/user/vanhoeve/mdd/ with the notice:
 * "The software can be freely used but comes with no warranty"
 *
 * bdd.hpp
 *      Author: Bergman, Cire
 */

#ifndef BDD_HPP_
#define BDD_HPP_

#include <map>
#include <vector>
#include "intset.hpp"
#include "util.hpp"

using namespace std;



/**
 * Node
 */
struct Node {

//	int				id;

	IntSet			state;
	int				longest_path;
//	bool			exact;

	int				relax_ub;


	/**
	 * Node constructor if one wishes only to create a relaxation
	 */
	Node(IntSet &_state, int _longest_path)	: state(_state), longest_path(_longest_path)
	{
	}
};


struct BDD {

};


typedef map<IntSet*, Node*, IntSetLexLessThan> NodeMap;


/**
 * Node comparator by longest path
 */
struct CompareNodesLongestPath {
	bool operator()(const Node* nodeA, const Node* nodeB) const {
		return nodeA->longest_path > nodeB->longest_path;
	}
};

/**
 * Node comparator by state size
 */
struct CompareNodesStateSizeAscending {
	bool operator()(Node* nodeA, Node* nodeB) const {
		if( nodeA->state.get_size() != nodeB->state.get_size() )
			return nodeA->state.get_size() < nodeB->state.get_size();
		return nodeA->longest_path > nodeB->longest_path;
	}
};

/**
 * Node comparator by state size
 */
struct CompareNodesStateSizeDescending {
	bool operator()(Node* nodeA, Node* nodeB) const {
		if( nodeA->state.get_size() != nodeB->state.get_size() )
			return nodeA->state.get_size() > nodeB->state.get_size();
		return nodeA->longest_path > nodeB->longest_path;
	}
};



#endif /* BDD_HPP_ */
