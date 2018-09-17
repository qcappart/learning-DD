/*
 * Taken from http://www.andrew.cmu.edu/user/vanhoeve/mdd/ with the notice:
 * "The software can be freely used but comes with no warranty"
 *
 * [Adaptation] Copyright (c) 2018 Quentin Cappart, Emmanuel Goutierre, David Bergman and Louis-Martin Rousseau (MIT Licence)
*/

#include <cassert>
#include "indepset_solver.hpp"
#include "util.hpp"


void IndepSetSolver::initialize(IntSet &initial_state, int initial_longest_path) {

	final_width = -1;

	// reset active list/map
	active_vertices.clear();

	memset( in_state_counter, 0, sizeof(int)*inst->graph->n_vertices );

	// get nodes that are active in the initial state
	eligible_vertex = initial_state.get_first();
	while( eligible_vertex != initial_state.get_end() ) {

		// obtain active vertex and store it in map
		active_vertex_map[eligible_vertex] = active_vertices.size();
		active_vertices.push_back(eligible_vertex);

		// update in state counter
		in_state_counter[eligible_vertex] = 1;

		eligible_vertex = initial_state.get_next(eligible_vertex);
	}

	initial_node = new Node(initial_state, initial_longest_path);
	node_list.clear();
	node_list[&(initial_state)] = initial_node;

	current_vertex = -1;

	// reset layer
	layer = 0;
}

int IndepSetSolver::generate_next_step_restriction(int next_vertex) {


    if( ordering->order_type == MinState ) {
        current_vertex = choose_next_vertex_min_size_next_layer();

    } else if( ordering->order_type == RandMinState ) {
        current_vertex = choose_next_vertex_min_size_next_layer_random();

    } else {
        current_vertex = next_vertex;
    }

    assert( current_vertex != -1 );
    vertex_in_layer[layer] = current_vertex;

    /*
     * ===============================================================================
     * 1. Take nodes that have the current vertex in their state
     * ===============================================================================
     */

    nodes_layer.clear();

    node_it = node_list.begin();
    while( node_it != node_list.end() )	{
        if( node_it->second->state.contains(current_vertex) ) {

            if( ordering->order_type == MinState || ordering->order_type == RandMinState ) {
                // decrement active state counter
                eligible_vertex = node_it->first->get_first();
                while( eligible_vertex != node_it->first->get_end() ) {
                    in_state_counter[ eligible_vertex]--;
                    eligible_vertex = node_it->first->get_next(eligible_vertex);
                }
            }

            assert( layer >= 0 );
            //assert( (int)final_bdd.size() > layer );

            // add node to current layer list
            nodes_layer.push_back(node_it->second);

            // erase element from the list
            node_list.erase(node_it++);

        } else {
            ++node_it;
        }
    }



    // // PRINT LAYER
    // cout << "Layer " << layer << " - current vertex: " << current_vertex;
    // cout << " - pool size: " << node_list.size();
    // cout << " - before merge: " << nodes_layer.size();
    // cout << " - total: " << node_list.size() + nodes_layer.size();
    // cout << endl;


    /*
     * ===============================================================================
     * 2. Merging
     * ===============================================================================
     */



    int nodes_before = (int) nodes_layer.size();

    if( width != EXACT_BDD && (int)nodes_layer.size() > width ) {
        restrict_layer_shortestpath();
    }

    final_width = MAX(final_width, (int)nodes_layer.size());

    cur_nodes_merged = nodes_before - (int)nodes_layer.size();

    /*
     * ===============================================================================
     * 3. Branching
     * ===============================================================================
     */
    Node* branch_node;
    for( vector<Node*>::iterator it = nodes_layer.begin(); it != nodes_layer.end(); ++it ) {

        branch_node = (*it);

        // remove current vertex
        branch_node->state.remove(current_vertex);

        // **** one arc ****


        node = new Node(branch_node->state, branch_node->longest_path+inst->weights[current_vertex]);

        // we assume a node is adjacent to itself
        node->state.set &= inst->adj_mask_compl[current_vertex].set;


        existing_node_it = node_list.find(&(node->state));
        if( existing_node_it != node_list.end() ) {

            // node already exists !!!

            update_node_match(existing_node_it->second, node);

            delete node;

        } else {
            node_list[&(node->state)] = node;


            // update active state counter
            if( ordering->order_type == MinState || ordering->order_type == RandMinState ) {
                eligible_vertex = node->state.get_first();
                while( eligible_vertex != node->state.get_end() ) {
                    in_state_counter[ eligible_vertex] ++;
                    eligible_vertex = node->state.get_next(eligible_vertex);
                }
            }
        }

        // **** zero arc ****
        existing_node_it = node_list.find(&(branch_node->state));
        if( existing_node_it != node_list.end() ) {

            // node exists

            update_node_match(existing_node_it->second, branch_node);


            delete branch_node;

        } else {

            // node does not exist

            // put branch node back to pool
            node_list[&(branch_node->state)] = branch_node;

            // update eligibility list
            if( ordering->order_type == MinState || ordering->order_type == RandMinState) {
                eligible_vertex = branch_node->state.get_first();
                while( eligible_vertex != branch_node->state.get_end() ) {
                    in_state_counter[ eligible_vertex] ++;
                    eligible_vertex = branch_node->state.get_next(eligible_vertex);
                }
            }


        }

    }

    // go to next layer
    layer++;

    return final_width;


}


int IndepSetSolver::generate_next_step_relaxation(int next_vertex) {

    if( ordering->order_type == MinState ) {
		current_vertex = choose_next_vertex_min_size_next_layer();

	} else if( ordering->order_type == RandMinState ) {
		current_vertex = choose_next_vertex_min_size_next_layer_random();

	} else {
		current_vertex = next_vertex;
	}

	assert( current_vertex != -1 );
	vertex_in_layer[layer] = current_vertex;

	/*
     * ===============================================================================
     * 1. Take nodes that have the current vertex in their state
     * ===============================================================================
     */

	nodes_layer.clear();

	node_it = node_list.begin();
	while( node_it != node_list.end() )	{
		if( node_it->second->state.contains(current_vertex) ) {

			if( ordering->order_type == MinState || ordering->order_type == RandMinState ) {
				// decrement active state counter
				eligible_vertex = node_it->first->get_first();
				while( eligible_vertex != node_it->first->get_end() ) {
					in_state_counter[ eligible_vertex]--;
					eligible_vertex = node_it->first->get_next(eligible_vertex);
				}
			}

			assert( layer >= 0 );
			//assert( (int)final_bdd.size() > layer );

			// add node to current layer list
			nodes_layer.push_back(node_it->second);

			// erase element from the list
			node_list.erase(node_it++);

		} else {
			++node_it;
		}
	}



	// // PRINT LAYER
	// cout << "Layer " << layer << " - current vertex: " << current_vertex;
	// cout << " - pool size: " << node_list.size();
	// cout << " - before merge: " << nodes_layer.size();
	// cout << " - total: " << node_list.size() + nodes_layer.size();
	// cout << endl;


	/*
     * ===============================================================================
     * 2. Merging
     * ===============================================================================
     */



    int nodes_before = (int) nodes_layer.size();

	if( width != EXACT_BDD && (int)nodes_layer.size() > width ) {
		//relax_layer_shortestpath();
		merger->merge_layer(layer, nodes_layer);

	}

	final_width = MAX(final_width, (int)nodes_layer.size());

    cur_nodes_merged = nodes_before - (int)nodes_layer.size();

	/*
     * ===============================================================================
     * 3. Branching
     * ===============================================================================
     */
	Node* branch_node;
	for( vector<Node*>::iterator it = nodes_layer.begin(); it != nodes_layer.end(); ++it ) {

		branch_node = (*it);

		// remove current vertex
		branch_node->state.remove(current_vertex);

		// **** one arc ****


		node = new Node(branch_node->state, branch_node->longest_path+inst->weights[current_vertex]);

		// we assume a node is adjacent to itself
		node->state.set &= inst->adj_mask_compl[current_vertex].set;


		existing_node_it = node_list.find(&(node->state));
		if( existing_node_it != node_list.end() ) {

			// node already exists !!!

			update_node_match(existing_node_it->second, node);

			delete node;

		} else {
			node_list[&(node->state)] = node;


			// update active state counter
			if( ordering->order_type == MinState || ordering->order_type == RandMinState ) {
				eligible_vertex = node->state.get_first();
				while( eligible_vertex != node->state.get_end() ) {
					in_state_counter[ eligible_vertex] ++;
					eligible_vertex = node->state.get_next(eligible_vertex);
				}
			}
		}

		// **** zero arc ****
		existing_node_it = node_list.find(&(branch_node->state));
		if( existing_node_it != node_list.end() ) {

			// node exists

			update_node_match(existing_node_it->second, branch_node);


			delete branch_node;

		} else {

			// node does not exist

			// put branch node back to pool
			node_list[&(branch_node->state)] = branch_node;

			// update eligibility list
			if( ordering->order_type == MinState || ordering->order_type == RandMinState) {
				eligible_vertex = branch_node->state.get_first();
				while( eligible_vertex != branch_node->state.get_end() ) {
					in_state_counter[ eligible_vertex] ++;
					eligible_vertex = branch_node->state.get_next(eligible_vertex);
				}
			}
		}
	}

	// go to next layer
	layer++;

	return final_width;
}

int IndepSetSolver::get_bound() {

	int bound = node_list.begin()->second->longest_path;
	//delete node_list.begin()->second;

	return bound;
}


int IndepSetSolver::generate_relaxation(IntSet &initial_state, int initial_longest_path) {

	// ---------------------------------------
	// 1. Initialize
	// ---------------------------------------

	final_width = -1;

	// reset active list/map
	active_vertices.clear();

	memset( in_state_counter, 0, sizeof(int)*inst->graph->n_vertices );

	// get nodes that are active in the initial state
	int eligible_vertex = initial_state.get_first();
	while( eligible_vertex != initial_state.get_end() ) {

		// obtain active vertex and store it in map
		active_vertex_map[eligible_vertex] = active_vertices.size();
		active_vertices.push_back(eligible_vertex);

		// update in state counter
		in_state_counter[eligible_vertex] = 1;

		eligible_vertex = initial_state.get_next(eligible_vertex);
	}

	// ---------------------------------------
	// 2. Relaxation
	// ---------------------------------------

	//Node* initial_node = new Node(initial_state, initial_longest_path, true); // b&b
	Node* initial_node = new Node(initial_state, initial_longest_path);
	node_list.clear();
	node_list[&(initial_state)] = initial_node;

	NodeMap::iterator node_it, existing_node_it;
	Node* node;

	int current_vertex = -1;

	// reset layer
	layer = 0;

	while ( layer < inst->graph->n_vertices ) {
		//while ( current_vertex < inst->graph->n_vertices ) {

		// select next vertex
		if( ordering->order_type == MinState ) {
			current_vertex = choose_next_vertex_min_size_next_layer();

		} else if( ordering->order_type == RandMinState ) {
			current_vertex = choose_next_vertex_min_size_next_layer_random();

		} else {
			current_vertex = ordering->vertex_in_layer(NULL, layer);
		}

		assert( current_vertex != -1 );
		vertex_in_layer[layer] = current_vertex;

//		cout << "\n\n\n\n ====================================================== \n\n";
//		cout << "Layer " << layer << " - current vertex: " << current_vertex << endl;

//		// iterate through the nodes in the node list
//		cout << "(Before) state list: " << endl;
//		for( NodeMap::iterator node_it = node_list.begin();
//				node_it != node_list.end();
//				node_it++ )
//		{
//			cout << "\tstate: " << *(node_it->first);
//			cout << " - longest path: " << node_it->second->longest_path;
//			cout << " - exact? " << node_it->second->exact;
//			cout << endl;
//		}


//		cout << current_vertex << endl;

		/*
		 * ===============================================================================
		 * 1. Take nodes that have the current vertex in their state
		 * ===============================================================================
		 */
		nodes_layer.clear();

		node_it = node_list.begin();
		while( node_it != node_list.end() )	{
			if( node_it->second->state.contains(current_vertex) ) {

				if( ordering->order_type == MinState || ordering->order_type == RandMinState ) {
					// decrement active state counter
					eligible_vertex = node_it->first->get_first();
					while( eligible_vertex != node_it->first->get_end() ) {
						in_state_counter[ eligible_vertex]--;
						eligible_vertex = node_it->first->get_next(eligible_vertex);
					}
				}

				assert( layer >= 0 );
				//assert( (int)final_bdd.size() > layer );

				// add node to current layer list
				nodes_layer.push_back(node_it->second);

				// erase element from the list
				node_list.erase(node_it++);

			} else {
				++node_it;
			}
		}

//		cout << "\nBefore merging: " << endl;
//		for( vector<Node*>::iterator it = nodes_layer.begin(); it != nodes_layer.end(); ++it ) {
//			cout << "\t " << (*it)->state << " - " << (*it)->longest_path;
//			cout << " - exact? " << (*it)->exact << endl;
//		}
//		cout << endl;


		// // PRINT LAYER
		// cout << "Layer " << layer << " - current vertex: " << current_vertex;
		// cout << " - pool size: " << node_list.size();
		// cout << " - before merge: " << nodes_layer.size();
		// cout << " - total: " << node_list.size() + nodes_layer.size();
		// cout << endl;


		/*
		 * ===============================================================================
		 * 2. Merging
		 * ===============================================================================
		 */
		if( width != EXACT_BDD && (int)nodes_layer.size() > width ) {
			//relax_layer_shortestpath();
			merger->merge_layer(layer, nodes_layer);
		}

		final_width = MAX(final_width, (int)nodes_layer.size());

//		cout << " - after merge: " << nodes_layer.size() << endl;


//		cout << "\nAfter merging: " << endl;
//		for( vector<Node*>::iterator it = nodes_layer.begin(); it != nodes_layer.end(); ++it ) {
//			cout << "\t " << (*it)->state << " - " << (*it)->longest_path;
//			cout << " - exact? " << (*it)->exact << endl;
//		}
//		cout << endl;


		/*
		 * ===============================================================================
		 * 3. Branching
		 * ===============================================================================
		 */
		Node* branch_node;
		for( vector<Node*>::iterator it = nodes_layer.begin(); it != nodes_layer.end(); ++it ) {

			branch_node = (*it);

			// add node to final BDD representation (bdd-save)
			//branch_node->layer = layer;
			//final_bdd[layer].push_back(branch_node);
			//cout << "\n\nSET " << node_it->second->state << " --> layer " << layer << endl;


			// remove current vertex
			branch_node->state.remove(current_vertex);

			// **** one arc ****
			//node = new Node(branch_node->state, branch_node->longest_path+1, branch_node->exact);

			node = new Node(branch_node->state, branch_node->longest_path+inst->weights[current_vertex]);

			// we assume a node is adjacent to itself
			node->state.set &= inst->adj_mask_compl[current_vertex].set;
//			for( vector<int>::iterator v = inst->graph->adj_list[current_vertex].begin();
//					v != inst->graph->adj_list[current_vertex].end();
//					v++ )
//			{
//				node->state.remove(*v);
//			}

			existing_node_it = node_list.find(&(node->state));
			if( existing_node_it != node_list.end() ) {

				// node already exists !!!

				update_node_match(existing_node_it->second, node);

//				// update node links (bdd-save)
//				assert( branch_node->one_arc == NULL );
//				branch_node->one_arc = existing_node_it->second;
//				existing_node_it->second->one_ancestors.push_back(branch_node);

				delete node;

			} else {
				node_list[&(node->state)] = node;

				// update node links (bdd-save)
				//branch_node->one_arc = node;
				//node->one_ancestors.push_back(branch_node);

				// update active state counter
				if( ordering->order_type == MinState || ordering->order_type == RandMinState ) {
					eligible_vertex = node->state.get_first();
					while( eligible_vertex != node->state.get_end() ) {
						in_state_counter[ eligible_vertex] ++;
						eligible_vertex = node->state.get_next(eligible_vertex);
					}
				}
			}

			// **** zero arc ****
			existing_node_it = node_list.find(&(branch_node->state));
			if( existing_node_it != node_list.end() ) {

				// node exists

				update_node_match(existing_node_it->second, branch_node);

				// update node links (bdd-save)
				//branch_node->zero_arc = existing_node_it->second;
				//existing_node_it->second->zero_ancestors.push_back(branch_node);

				delete branch_node;

			} else {

				// node does not exist

				// put branch node back to pool
				node_list[&(branch_node->state)] = branch_node;

				// update eligibility list
				if( ordering->order_type == MinState || ordering->order_type == RandMinState) {
					eligible_vertex = branch_node->state.get_first();
					while( eligible_vertex != branch_node->state.get_end() ) {
						in_state_counter[ eligible_vertex] ++;
						eligible_vertex = branch_node->state.get_next(eligible_vertex);
					}
				}

				// update node links (bdd-save)
				//node = new Node(branch_node->state, branch_node->longest_path);
				//node_list[&(node->state)] = node;
				//branch_node->zero_arc = node;
				//node->zero_ancestors.push_back(branch_node);

				// update active state counter
//				eligible_vertex = node->state.get_first();
//				while( eligible_vertex != node->state.get_end() ) {
//					in_state_counter[ eligible_vertex] ++;
//					eligible_vertex = node->state.get_next(eligible_vertex);
//				}
			}

		}


//		// iterate through the nodes in the node list
//		cout << "(After) state list: " << endl;
//		for( NodeMap::iterator node_it = node_list.begin();
//				node_it != node_list.end();
//				node_it++ )
//		{
//			cout << "\tstate: " << *(node_it->first);
//			cout << " - longest path: " << node_it->second->longest_path;
//			cout << " - exact? " << node_it->second->exact;
//			cout << endl;
//		}
//		cout << endl;


//		cout << endl << "state counter: " << endl;
//		for( int i = 0; i < inst->graph->n_vertices; i++ ) {
//			if( in_state_counter[i] > 0 ) {
//				cout << "\tvertex " << i << ": " << in_state_counter[i] << endl;
//			}
//		}
//		cout << endl;

		// go to next layer
		layer++;
	}

	// add root node (bdd-save)
	//node_list.begin()->second->layer = inst->graph->n_vertices;
	//final_bdd[inst->graph->n_vertices].push_back(node_list.begin()->second);

	// update node bounds of temporary list (b&b)
//	int ub = node_list.begin()->second->longest_path;
//	for( int i = 0; i < (int)temporary_branch_nodes.size(); i++ ) {
//		temporary_branch_nodes[i]->relax_ub = ub;
//	}

	// (bdd-save)
//	for( int i = 0; i < inst->graph->n_vertices+1; i++ ) {
//		int id = 0;
//		for( vector<Node*>::iterator it = final_bdd[i].begin(); it != final_bdd[i].end(); it++ ) {
//			assert( (*it)->layer != -1 );
//			(*it)->id = id++;
//		}
//		//cout << i << " - size: " << final_bdd[i].size() << endl;
//	}

//	cout << "Temporary nodes: " << endl;
//	for( int i = 0; i < (int)temporary_branch_nodes.size(); i++ ) {
//		cout << "\t" << temporary_branch_nodes[i]->state << " - " << temporary_branch_nodes[i]->longest_path << endl;
//	}
//	cout << "Number of temporary nodes: " << temporary_branch_nodes.size() << endl;
//	cout << endl;

	//cout << endl;

	// take bound and delete last node
	int bound = node_list.begin()->second->longest_path;
	delete node_list.begin()->second;

	return bound;
}

int IndepSetSolver::generate_restriction_with_ordering(IntSet &initial_state, int initial_longest_path) {

    // ---------------------------------------
    // 1. Initialize
    // ---------------------------------------

    final_width = -1;

    // reset active list/map
    active_vertices.clear();

    memset( in_state_counter, 0, sizeof(int)*inst->graph->n_vertices );

    // get nodes that are active in the initial state
    int eligible_vertex = initial_state.get_first();
    while( eligible_vertex != initial_state.get_end() ) {

        // obtain active vertex and store it in map
        active_vertex_map[eligible_vertex] = active_vertices.size();
        active_vertices.push_back(eligible_vertex);

        // update in state counter
        in_state_counter[eligible_vertex] = 1;

        eligible_vertex = initial_state.get_next(eligible_vertex);
    }

    // ---------------------------------------
    // 2. Relaxation
    // ---------------------------------------

    //Node* initial_node = new Node(initial_state, initial_longest_path, true); // b&b
    Node* initial_node = new Node(initial_state, initial_longest_path);
    node_list.clear();
    node_list[&(initial_state)] = initial_node;

    NodeMap::iterator node_it, existing_node_it;
    Node* node;

    int current_vertex = -1;

    // reset layer
    layer = 0;

    while ( layer < inst->graph->n_vertices ) {
        //while ( current_vertex < inst->graph->n_vertices ) {

        // select next vertex
        if( ordering->order_type == MinState ) {
            current_vertex = choose_next_vertex_min_size_next_layer();

        } else if( ordering->order_type == RandMinState ) {
            current_vertex = choose_next_vertex_min_size_next_layer_random();

        } else {
            current_vertex = ordering->vertex_in_layer(NULL, layer);
        }

        assert( current_vertex != -1 );
        vertex_in_layer[layer] = current_vertex;

//		cout << "\n\n\n\n ====================================================== \n\n";
//		cout << "Layer " << layer << " - current vertex: " << current_vertex << endl;

//		// iterate through the nodes in the node list
//		cout << "(Before) state list: " << endl;
//		for( NodeMap::iterator node_it = node_list.begin();
//				node_it != node_list.end();
//				node_it++ )
//		{
//			cout << "\tstate: " << *(node_it->first);
//			cout << " - longest path: " << node_it->second->longest_path;
//			cout << " - exact? " << node_it->second->exact;
//			cout << endl;
//		}


//		cout << current_vertex << endl;

        /*
         * ===============================================================================
         * 1. Take nodes that have the current vertex in their state
         * ===============================================================================
         */
        nodes_layer.clear();

        node_it = node_list.begin();
        while( node_it != node_list.end() )	{
            if( node_it->second->state.contains(current_vertex) ) {

                if( ordering->order_type == MinState || ordering->order_type == RandMinState ) {
                    // decrement active state counter
                    eligible_vertex = node_it->first->get_first();
                    while( eligible_vertex != node_it->first->get_end() ) {
                        in_state_counter[ eligible_vertex]--;
                        eligible_vertex = node_it->first->get_next(eligible_vertex);
                    }
                }

                assert( layer >= 0 );
                //assert( (int)final_bdd.size() > layer );

                // add node to current layer list
                nodes_layer.push_back(node_it->second);

                // erase element from the list
                node_list.erase(node_it++);

            } else {
                ++node_it;
            }
        }

//		cout << "\nBefore merging: " << endl;
//		for( vector<Node*>::iterator it = nodes_layer.begin(); it != nodes_layer.end(); ++it ) {
//			cout << "\t " << (*it)->state << " - " << (*it)->longest_path;
//			cout << " - exact? " << (*it)->exact << endl;
//		}
//		cout << endl;


        // // PRINT LAYER
        // cout << "Layer " << layer << " - current vertex: " << current_vertex;
        // cout << " - pool size: " << node_list.size();
        // cout << " - before merge: " << nodes_layer.size();
        // cout << " - total: " << node_list.size() + nodes_layer.size();
        // cout << endl;


        if( width != EXACT_BDD && (int)nodes_layer.size() > width ) {
            restrict_layer_shortestpath();
        }



        /*
         * ===============================================================================
         * 3. Branching
         * ===============================================================================
         */
        Node* branch_node;
        for( vector<Node*>::iterator it = nodes_layer.begin(); it != nodes_layer.end(); ++it ) {

            branch_node = (*it);

            // add node to final BDD representation (bdd-save)
            //branch_node->layer = layer;
            //final_bdd[layer].push_back(branch_node);
            //cout << "\n\nSET " << node_it->second->state << " --> layer " << layer << endl;


            // remove current vertex
            branch_node->state.remove(current_vertex);

            // **** one arc ****
            //node = new Node(branch_node->state, branch_node->longest_path+1, branch_node->exact);

            node = new Node(branch_node->state, branch_node->longest_path+inst->weights[current_vertex]);

            // we assume a node is adjacent to itself
            node->state.set &= inst->adj_mask_compl[current_vertex].set;
//			for( vector<int>::iterator v = inst->graph->adj_list[current_vertex].begin();
//					v != inst->graph->adj_list[current_vertex].end();
//					v++ )
//			{
//				node->state.remove(*v);
//			}

            existing_node_it = node_list.find(&(node->state));
            if( existing_node_it != node_list.end() ) {

                // node already exists !!!

                update_node_match(existing_node_it->second, node);

//				// update node links (bdd-save)
//				assert( branch_node->one_arc == NULL );
//				branch_node->one_arc = existing_node_it->second;
//				existing_node_it->second->one_ancestors.push_back(branch_node);

                delete node;

            } else {
                node_list[&(node->state)] = node;

                // update node links (bdd-save)
                //branch_node->one_arc = node;
                //node->one_ancestors.push_back(branch_node);

                // update active state counter
                if( ordering->order_type == MinState || ordering->order_type == RandMinState ) {
                    eligible_vertex = node->state.get_first();
                    while( eligible_vertex != node->state.get_end() ) {
                        in_state_counter[ eligible_vertex] ++;
                        eligible_vertex = node->state.get_next(eligible_vertex);
                    }
                }
            }

            // **** zero arc ****
            existing_node_it = node_list.find(&(branch_node->state));
            if( existing_node_it != node_list.end() ) {

                // node exists

                update_node_match(existing_node_it->second, branch_node);

                // update node links (bdd-save)
                //branch_node->zero_arc = existing_node_it->second;
                //existing_node_it->second->zero_ancestors.push_back(branch_node);

                delete branch_node;

            } else {

                // node does not exist

                // put branch node back to pool
                node_list[&(branch_node->state)] = branch_node;

                // update eligibility list
                if( ordering->order_type == MinState || ordering->order_type == RandMinState) {
                    eligible_vertex = branch_node->state.get_first();
                    while( eligible_vertex != branch_node->state.get_end() ) {
                        in_state_counter[ eligible_vertex] ++;
                        eligible_vertex = branch_node->state.get_next(eligible_vertex);
                    }
                }

                // update node links (bdd-save)
                //node = new Node(branch_node->state, branch_node->longest_path);
                //node_list[&(node->state)] = node;
                //branch_node->zero_arc = node;
                //node->zero_ancestors.push_back(branch_node);

                // update active state counter
//				eligible_vertex = node->state.get_first();
//				while( eligible_vertex != node->state.get_end() ) {
//					in_state_counter[ eligible_vertex] ++;
//					eligible_vertex = node->state.get_next(eligible_vertex);
//				}
            }

        }


//		// iterate through the nodes in the node list
//		cout << "(After) state list: " << endl;
//		for( NodeMap::iterator node_it = node_list.begin();
//				node_it != node_list.end();
//				node_it++ )
//		{
//			cout << "\tstate: " << *(node_it->first);
//			cout << " - longest path: " << node_it->second->longest_path;
//			cout << " - exact? " << node_it->second->exact;
//			cout << endl;
//		}
//		cout << endl;


//		cout << endl << "state counter: " << endl;
//		for( int i = 0; i < inst->graph->n_vertices; i++ ) {
//			if( in_state_counter[i] > 0 ) {
//				cout << "\tvertex " << i << ": " << in_state_counter[i] << endl;
//			}
//		}
//		cout << endl;

        // go to next layer
        layer++;
    }

    // add root node (bdd-save)
    //node_list.begin()->second->layer = inst->graph->n_vertices;
    //final_bdd[inst->graph->n_vertices].push_back(node_list.begin()->second);

    // update node bounds of temporary list (b&b)
//	int ub = node_list.begin()->second->longest_path;
//	for( int i = 0; i < (int)temporary_branch_nodes.size(); i++ ) {
//		temporary_branch_nodes[i]->relax_ub = ub;
//	}

    // (bdd-save)
//	for( int i = 0; i < inst->graph->n_vertices+1; i++ ) {
//		int id = 0;
//		for( vector<Node*>::iterator it = final_bdd[i].begin(); it != final_bdd[i].end(); it++ ) {
//			assert( (*it)->layer != -1 );
//			(*it)->id = id++;
//		}
//		//cout << i << " - size: " << final_bdd[i].size() << endl;
//	}

//	cout << "Temporary nodes: " << endl;
//	for( int i = 0; i < (int)temporary_branch_nodes.size(); i++ ) {
//		cout << "\t" << temporary_branch_nodes[i]->state << " - " << temporary_branch_nodes[i]->longest_path << endl;
//	}
//	cout << "Number of temporary nodes: " << temporary_branch_nodes.size() << endl;
//	cout << endl;

    //cout << endl;

    // take bound and delete last node
    int bound = node_list.begin()->second->longest_path;
    delete node_list.begin()->second;

    return bound;
}



int IndepSetSolver::generate_restriction(IntSet &initial_state, int initial_longest_path) {

	// ---------------------------------------
	// 1. Initialize
	// ---------------------------------------

	// reset active list/map
	active_vertices.clear();
	layer = 0;

	memset( in_state_counter, 0, sizeof(int)*inst->graph->n_vertices );

	// get nodes that are active in the initial state
	int eligible_vertex = initial_state.get_first();
	while( eligible_vertex != initial_state.get_end() ) {

		// obtain active vertex and store it in map
		active_vertex_map[eligible_vertex] = active_vertices.size();
		active_vertices.push_back(eligible_vertex);

		// update counter
		in_state_counter[eligible_vertex] = 1;

		eligible_vertex = initial_state.get_next(eligible_vertex);
	}

	// ---------------------------------------
	// 2. Relaxation
	// ---------------------------------------

	//Node* initial_node = new Node(initial_state, initial_longest_path, true); // b&b
	Node* initial_node = new Node(initial_state, initial_longest_path);
	node_list.clear();
	node_list[&(initial_state)] = initial_node;

	NodeMap::iterator node_it, existing_node_it;
	Node* node;

	layer = 1;

	int current_vertex = choose_next_vertex_min_size_next_layer();
	while ( current_vertex != -1 ) {
	//while ( current_vertex < inst->graph->n_vertices ) {

//		cout << "\n\n\n\n ====================================================== \n\n";
//		cout << "Layer " << (layer++) << " - current vertex: " << current_vertex << endl;
		layer++;

//		// iterate through the nodes in the node list
//		cout << "(Before) state list: " << endl;
//		for( NodeMap::iterator node_it = node_list.begin();
//				node_it != node_list.end();
//				node_it++ )
//		{
//			cout << "\tstate: " << *(node_it->first);
//			cout << " - longest path: " << node_it->second->longest_path;
//			cout << " - exact? " << node_it->second->exact;
//			cout << endl;
//		}


//		cout << current_vertex << endl;

		/*
		 * 1. Take the nodes that have the current vertex in their state
		 */
		nodes_layer.clear();

		node_it = node_list.begin();
		while( node_it != node_list.end() )	{
			if( node_it->second->state.contains(current_vertex) ) {

				// decrement active state counter
				eligible_vertex = node_it->first->get_first();
				while( eligible_vertex != node_it->first->get_end() ) {
					in_state_counter[ eligible_vertex]--;
					eligible_vertex = node_it->first->get_next(eligible_vertex);
				}

				// add node to current layer list
				nodes_layer.push_back(node_it->second);

				// erase element from the list
				node_list.erase(node_it++);

			} else {
				++node_it;
			}
		}

//		cout << "\nBefore merging: " << endl;
//		for( vector<Node*>::iterator it = nodes_layer.begin(); it != nodes_layer.end(); ++it ) {
//			cout << "\t " << (*it)->state << " - " << (*it)->longest_path;
//			cout << " - exact? " << (*it)->exact << endl;
//		}


//		cout << "Layer " << (layer) << " - current vertex: " << current_vertex;
//		cout << " - pool size: " << node_list.size();
//		cout << " - before merge: " << nodes_layer.size();
//		cout << endl;

		/*
		 * 2. Merging
		 */
		if( width != EXACT_BDD && (int)nodes_layer.size() > width ) {
			restrict_layer_shortestpath();
		}

		//cout << " - after merge: " << nodes_layer.size() << endl;


//		cout << "\nAfter merging: " << endl;
//		for( vector<Node*>::iterator it = nodes_layer.begin(); it != nodes_layer.end(); ++it ) {
//			cout << "\t " << (*it)->state << " - " << (*it)->longest_path;
//			cout << " - exact? " << (*it)->exact << endl;
//		}
//		cout << endl;


		/*
		 * 3. Branching
		 */
		Node* branch_node;
		for( vector<Node*>::iterator it = nodes_layer.begin(); it != nodes_layer.end(); ++it ) {

			branch_node = (*it);

			// remove current vertex
			branch_node->state.remove(current_vertex);

			// **** one arc ****
			//node = new Node(branch_node->state, branch_node->longest_path+1, branch_node->exact); (b&b)
			node = new Node(branch_node->state, branch_node->longest_path+1);
			for( vector<int>::iterator v = inst->graph->adj_list[current_vertex].begin();
					v != inst->graph->adj_list[current_vertex].end();
					v++ )
			{
				node->state.remove(*v);
			}

			existing_node_it = node_list.find(&(node->state));
			if( existing_node_it != node_list.end() ) {

				existing_node_it->second->longest_path = MAX(existing_node_it->second->longest_path, node->longest_path);
				delete node;

			} else {
				node_list[&(node->state)] = node;

				// update active state counter
				eligible_vertex = node->state.get_first();
				while( eligible_vertex != node->state.get_end() ) {
					in_state_counter[ eligible_vertex] ++;
					eligible_vertex = node->state.get_next(eligible_vertex);
				}
			}

			// **** zero arc ****
			existing_node_it = node_list.find(&(branch_node->state));
			if( existing_node_it != node_list.end() ) {

				existing_node_it->second->longest_path = MAX(existing_node_it->second->longest_path, branch_node->longest_path);
				delete branch_node;

			} else {
				node_list[&(branch_node->state)] = branch_node;

				// update active state counter
				eligible_vertex = branch_node->state.get_first();
				while( eligible_vertex != branch_node->state.get_end() ) {
					in_state_counter[ eligible_vertex] ++;
					eligible_vertex = branch_node->state.get_next(eligible_vertex);
				}
			}
		}
		current_vertex = choose_next_vertex_min_size_next_layer();
	}

	return node_list.begin()->second->longest_path;
}

/**
 * Restrict nodes to meet maximum width.
 */
void IndepSetSolver::restrict_layer_shortestpath() {

	sort(nodes_layer.begin(), nodes_layer.end(), CompareNodesLongestPath());

	/**
	 * 1. Merge nodes
	 */
	for( vector<Node*>::iterator node = nodes_layer.begin()+width; node != nodes_layer.end(); ++node) {
		delete (*node);
	}
	nodes_layer.resize(width);
}
