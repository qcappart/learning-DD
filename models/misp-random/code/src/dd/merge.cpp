/*
 * Taken from http://www.andrew.cmu.edu/user/vanhoeve/mdd/ with the notice:
 * "The software can be freely used but comes with no warranty"
 *
 * [Adaptation] Copyright (c) 2018 Quentin Cappart, Emmanuel Goutierre, David Bergman and Louis-Martin Rousseau (MIT Licence)
 * --------------------------------------------------------
 * Merging class - implementation
 * --------------------------------------------------------
 */

#include <algorithm>
#include "merge.hpp"



/**
 * Minimum longest path
 */
void MinLongestPath::merge_layer(int layer, vector<Node*> &nodes_layer) {

	sort(nodes_layer.begin(), nodes_layer.end(), CompareNodesLongestPath());

	gap = 0;

	IntSet* state = &(nodes_layer[width-1]->state);

	//cout << "state: " << *state << endl;
	for( vector<Node*>::iterator node = nodes_layer.begin()+width; node != nodes_layer.end(); ++node) {
/*
        cout << "Nodes to merges" << endl;
        cout << "n1: " << *state << endl;
        cout << "n2: " << (*node)->state << endl;*/
		IntSet s_union = *state;
		IntSet s_intersect = *state;
		//cout << "Before op" << endl;
		//cout << "s_union: " << s_union << endl;
		//cout << "s_intersect: " << s_intersect << endl;
		s_union.union_with((*node)->state);
		s_intersect.intersect_with((*node)->state);
		/*
		cout << "after op" << endl;
        cout << "s_union: " << s_union << endl;
        cout << "s_intersect: " << s_intersect << endl;
        */

		gap += (s_union.get_size() - s_intersect.get_size());

		state->union_with((*node)->state);

		delete (*node);

	}
    //cout << "------------" << endl;
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



/**
 * Minimum longest path: pair by pair
 */
void PairMinLongestPath::merge_layer(int layer, vector<Node*> &nodes_layer) {

	// sort nodes
	sort(nodes_layer.begin(), nodes_layer.end(), CompareNodesLongestPath());

	// populate current states with given nodes
	current_states.clear();
	for( vector<Node*>::iterator node = nodes_layer.begin(); node != nodes_layer.end(); node++ ) {
		current_states[&((*node)->state)] = *node;
	}

	// merge nodes from the end of the list until max. width is reached

	int current_size = nodes_layer.size();
	while( current_size > width ) {

		//        int i = 0;
		//        cout << "\n\n## Current state: " << layer << "##" << endl;
		//        foreach_node_in_layer(node, layer, bdd) {
		//            cout << "\t" << i++ << " - id: " << (*node)->id << " --> SP: " << (*node)->shortest_path;
		//            cout << " - state: " << (*node)->state;
		//            cout << endl;
		//        }
		//        cout << endl;


		// erase both elements from map
		current_states.erase(&(nodes_layer[current_size-2]->state));
		current_states.erase(&(nodes_layer[current_size-1]->state));

		// merge the two last nodes. Notice that node at current_size-2 already has a larger
		// longest path
		nodes_layer[current_size-2]->state.union_with(nodes_layer[current_size-1]->state);

		//        cout << "\tmerging " << current_size - 2 << " with " << current_size-1 << endl;
		//        cout << "\tstate result: " << bdd->nodes[layer][current_size-2]->state << endl;
		//        cout << "\tshortest path: " << bdd->nodes[layer][current_size-2]->shortest_path << endl;

		// remove last node from BDD (without consistency check)
		delete nodes_layer[current_size-1];
		nodes_layer.pop_back();
		current_size--;

		// now, we must check if the state of the new node appears in any previous node
		NodeMap::iterator map_it = current_states.find(&(nodes_layer[current_size-1]->state));
		if( map_it != current_states.end() ) {

			// we just have to delete this last node. Notice that
			// we do not need to re-sort the vector, since the existing node
			// already has a larger longest path in comparison to the latter node

			// remove last node from BDD (without consistency check)
			delete nodes_layer[current_size-1];
			nodes_layer.pop_back();
			current_size--;


		} else {
			// otherwise, we add the node to the set of current states
			current_states[&(nodes_layer[current_size-1]->state)] = nodes_layer[current_size-1];
		}
	}
}

/**
 * Consecutive pair longest path
 */
void ConsecutivePairLongestPath::merge_layer(int layer, vector<Node*> &nodes_layer) {

	// auxiliaries
	Node* nodeA, *nodeB;
	bool found = false;

	// reset old nodes
	old_nodes.clear();

	// merge while maximum width is not met
	while( old_nodes.size() + nodes_layer.size() > (unsigned int)width ) {

//		cout << "old_nodes: " << old_nodes.size() << endl;
//		cout << "new_nodes: " << nodes_layer.size() << endl;

		// copy nodes to new vector
		old_nodes.resize(nodes_layer.size());
		for( int i = 0; i < (int)nodes_layer.size(); i++ ) {
			old_nodes[i] = nodes_layer[i];
		}
		nodes_layer.clear(); 	// destroy old vector

		// sort according to longest path
		sort(old_nodes.begin(), old_nodes.end(), CompareNodesLongestPath());

		// perform consecutive pairs merge while still possible
		while( old_nodes.size() >= 2 ) {

			//cout << "\tmerging... " << old_nodes.size() << endl;

			// remove the last two nodes for merging
			nodeB = old_nodes.back();
			old_nodes.pop_back();

			nodeA = old_nodes.back();
			old_nodes.pop_back();

			// merge into node A
			nodeA->state.union_with(nodeB->state);
			assert( nodeA->longest_path >= nodeB->longest_path );
			delete nodeB;

			// check if new state exists in old list
			found = false;
			for( int i = 0; !found && i < (int)old_nodes.size(); i++ ) {
				if( old_nodes[i]->state.equals_to(nodeA->state) ) {
					// if node exists in old list, we simply delete nodeA
					found = true;
					assert( old_nodes[i]->longest_path >= nodeA->longest_path );
					delete nodeA;
				}
			}

			if( !found ) {

				// check if new state exists in new list
				for( int i = 0; !found && i < (int)nodes_layer.size(); i++ ) {
					assert( nodeA != NULL );
					if( nodes_layer[i]->state.equals_to(nodeA->state) ) {
						// if node exists in new list, we need to update longest path
						found = true;
						nodes_layer[i]->longest_path = MAX(nodes_layer[i]->longest_path, nodeA->longest_path);
						delete nodeA;
					}
				}

				// if node was not found, then we add it to the new nodes list
				if( !found ) {
					nodes_layer.push_back(nodeA);
				}
			}
		}

		// we insert the remaining elements of the old node at nodes_layer
		nodes_layer.insert(nodes_layer.end(), old_nodes.begin(), old_nodes.end());
		old_nodes.clear();
	}

	assert( nodes_layer.size() <= width );

}


/**
 * Lexicographic Merger
 */
void LexicographicMerger::merge_layer(int layer, vector<Node*> &nodes_layer) {

	// populate current states with given nodes
	current_states.clear();
	for( vector<Node*>::iterator node = nodes_layer.begin(); node != nodes_layer.end(); node++ ) {
		current_states[&((*node)->state)] = *node;
	}

	// merge nodes in the map until maximum width is met
	Node* last, *previous_to_last;

	while( (int)current_states.size() > width ) {

		// take last two nodes of list
		NodeMap::reverse_iterator rev_map_it = current_states.rbegin();
		last = (rev_map_it)->second;
		previous_to_last = (++rev_map_it)->second;

		// delete last two nodes from list (use iterators!)
		current_states.erase(&(last->state));
		current_states.erase(&(previous_to_last->state));

		// merge nodes and delete last one
		previous_to_last->longest_path = MAX(previous_to_last->longest_path, last->longest_path);
		previous_to_last->state.union_with(last->state);
		delete last;

		// now, we must check if the state of the new node appears in any previous node
		NodeMap::iterator map_it = current_states.find(&(previous_to_last->state));
		if( map_it != current_states.end() ) {

			// we just have to delete this last node, updating longest path of existing one

			// remove last node from BDD (without consistency check)
			map_it->second->longest_path = MAX(map_it->second->longest_path, previous_to_last->longest_path);
			delete previous_to_last;

		} else {
			// otherwise, we add the node to the set of current states
			current_states[&(previous_to_last->state)] = previous_to_last;
		}
	}


	// replace nodes of vector from nodes in the map
	nodes_layer.clear();
	for( NodeMap::iterator node = current_states.begin(); node != current_states.end(); node++ ) {
		nodes_layer.push_back(node->second);
	}

}


/**
 * Min Size Merger
 */
void MinSizeMerger::merge_layer(int layer, vector<Node*> &nodes_layer) {

	// populate current states with given nodes
	node_list.clear();
	for( vector<Node*>::iterator node = nodes_layer.begin(); node != nodes_layer.end(); node++ ) {
		node_list.push_back(*node);
	}

	// sort list according to state size
	node_list.sort(CompareNodesStateSizeAscending());

	// auxiliaries
	list<Node*> merger_list;
	Node* last, *previous_to_last;
	bool found;

	// merge elements
	while( (int)node_list.size() > width ) {

		// take last two nodes from list
		last = node_list.back();
		node_list.pop_back();
		previous_to_last = node_list.back();
		node_list.pop_back();

		// merge nodes and delete last one
		previous_to_last->longest_path = MAX(previous_to_last->longest_path, last->longest_path);
		previous_to_last->state.union_with(last->state);
		delete last;

		// now, we must check if the state of the new node appears in any previous node.
		found = false;
		list<Node*>::iterator node_it;
		for( node_it = node_list.begin(); !found && node_it != node_list.end(); ++node_it ) {
			found = ((*node_it)->state.equals_to(previous_to_last->state));
		}

		if( found ) {

			node_it--;
			assert( ((*node_it)->state.equals_to(previous_to_last->state)) );

			// we just have to delete this last node, updating longest path of existing one

			// remove last node from BDD (without consistency check)
			(*node_it)->longest_path = MAX((*node_it)->longest_path, previous_to_last->longest_path);
			delete previous_to_last;

		} else {
			// otherwise, we add the node to the set of current states
			merger_list.clear();
			merger_list.push_back(previous_to_last);
			node_list.merge(merger_list, CompareNodesStateSizeAscending());
		}
	}

	// replace nodes of vector from nodes in the list
	nodes_layer.clear();
	for( list<Node*>::iterator node = node_list.begin(); node != node_list.end(); node++ ) {
		nodes_layer.push_back(*node);
	}
}


/**
 * Max Size Merger
 */
void MaxSizeMerger::merge_layer(int layer, vector<Node*> &nodes_layer) {

	// populate current states with given nodes
	node_list.clear();
	for( vector<Node*>::iterator node = nodes_layer.begin(); node != nodes_layer.end(); node++ ) {
		node_list.push_back(*node);
	}

	// sort list according to state size
	node_list.sort(CompareNodesStateSizeDescending());

	// auxiliaries
	list<Node*> merger_list;
	Node* last, *previous_to_last;
	bool found;

	// merge elements
	while( (int)node_list.size() > width ) {

		// take last two nodes from list
		last = node_list.back();
		node_list.pop_back();
		previous_to_last = node_list.back();
		node_list.pop_back();

		// merge nodes and delete last one
		previous_to_last->longest_path = MAX(previous_to_last->longest_path, last->longest_path);
		previous_to_last->state.union_with(last->state);
		delete last;

		// now, we must check if the state of the new node appears in any previous node.
		found = false;
		list<Node*>::iterator node_it;
		for( node_it = node_list.begin(); !found && node_it != node_list.end(); node_it++ ) {
			found = ((*node_it)->state.equals_to(previous_to_last->state));
		}

		if( found ) {

			node_it--;
			assert( ((*node_it)->state.equals_to(previous_to_last->state)) );

			// we just have to delete this last node, updating longest path of existing one

			// remove last node from BDD (without consistency check)
			(*node_it)->longest_path = MAX((*node_it)->longest_path, previous_to_last->longest_path);
			delete previous_to_last;

		} else {
			// otherwise, we add the node to the set of current states
			merger_list.clear();
			merger_list.push_back(previous_to_last);
			node_list.merge(merger_list, CompareNodesStateSizeDescending());
		}
	}

	// replace nodes of vector from nodes in the list
	nodes_layer.clear();
	for( list<Node*>::iterator node = node_list.begin(); node != node_list.end(); node++ ) {
		nodes_layer.push_back(*node);
	}
}


/**
 * Symmetric difference
 * (it is not implemented efficiently!!)
 */
void SymmetricDifferenceMerger::merge_layer(int layer, vector<Node*> &nodes_layer) {

	// populate current states with given nodes
	current_states.clear();
	for( vector<Node*>::iterator node = nodes_layer.begin(); node != nodes_layer.end(); node++ ) {
		current_states[&((*node)->state)] = *node;
	}

	ComparatorNodePairSymmLP pairs_comp(symm_diff_vals, longest_path_val);
	Node* last, *previous_to_last;

	// merge nodes until max width is satisfied
	while( (int)current_states.size() > width ) {

		// compute symmetric difference between all pairs
		node_pairs.clear();
		symm_diff_vals.clear();
		longest_path_val.clear();

		for( NodeMap::iterator nodeA = current_states.begin(); nodeA != current_states.end(); nodeA++ ) {
			NodeMap::iterator nodeB = nodeA;
			nodeB++;
			for( ; nodeB != current_states.end(); nodeB++ ) {

				// compute symmetric difference val
				aux.set = nodeA->second->state.set ^ nodeB->second->state.set;
				symm_diff_vals.push_back(aux.set.count());

//				cout << "test: ";
//				cout << nodeA->second->state.set;
//				cout << " " << nodeB->second->state.set;
//				cout << " --> " << aux.set << " - size: " << aux.set.count() << endl;

				// compute resulting longest path
				longest_path_val.push_back(MAX(nodeA->second->longest_path, nodeB->second->longest_path));

				// add to list of pairs
				node_pairs.push_back(pair<Node*,Node*>(nodeA->second, nodeB->second));
			}
		}

		assert( node_pairs.size() == symm_diff_vals.size() );
		assert( node_pairs.size() == longest_path_val.size() );

		// sort list of pairs
		indices.clear();
		for( int i = 0; i < (int)node_pairs.size(); i++ )
			indices.push_back(i);

		sort(indices.begin(), indices.end(), pairs_comp);

//		cout << endl;
//		for( int i = 0; i < (int)indices.size(); i++ ) {
//			int k = indices[i];
//			cout << node_pairs[k].first->state << " " << node_pairs[k].second->state << " --> " << symm_diff_vals[k] << endl;
//		}
//		cout << endl << endl;

		// take last pair from list
		last = node_pairs[indices.back()].first;
		previous_to_last = node_pairs[indices.back()].second;

		assert( last != NULL );
		assert( previous_to_last != NULL );

		// delete last two nodes from list (use iterators!)
		current_states.erase(&(last->state));
		current_states.erase(&(previous_to_last->state));

		// merge nodes and delete last one
		previous_to_last->longest_path = MAX(previous_to_last->longest_path, last->longest_path);
		previous_to_last->state.union_with(last->state);
		delete last;

		// now, we must check if the state of the new node appears in any previous node
		NodeMap::iterator map_it = current_states.find(&(previous_to_last->state));
		if( map_it != current_states.end() ) {

			// we just have to delete this last node, updating longest path of existing one

			// remove last node from BDD (without consistency check)
			map_it->second->longest_path = MAX(map_it->second->longest_path, previous_to_last->longest_path);
			delete previous_to_last;

		} else {
			// otherwise, we add the node to the set of current states
			current_states[&(previous_to_last->state)] = previous_to_last;
		}
	}


	// replace nodes of vector from nodes in the map
	nodes_layer.clear();
	for( NodeMap::iterator node = current_states.begin(); node != current_states.end(); node++ ) {
		nodes_layer.push_back(node->second);
	}
}


/**
 * Random merger
 */
void RandomMerger::merge_layer(int layer, vector<Node*> &nodes_layer) {

	// shuffle random
	random_shuffle(nodes_layer.begin(), nodes_layer.end());

	IntSet* state = &(nodes_layer[width-1]->state);
	for( vector<Node*>::iterator node = nodes_layer.begin()+width; node != nodes_layer.end(); ++node) {
		state->union_with((*node)->state);
		nodes_layer[width-1]->longest_path = MAX(nodes_layer[width-1]->longest_path, (*node)->longest_path);
		delete (*node);
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

			node->longest_path = MAX(node->longest_path, nodes_layer.back()->longest_path);

			// delete the last node
			delete nodes_layer.back();

			// remove it from queue
			nodes_layer.pop_back();

			break;
		}
	}
}






