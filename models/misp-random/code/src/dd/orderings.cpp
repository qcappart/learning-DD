/*
 * Taken from http://www.andrew.cmu.edu/user/vanhoeve/mdd/ with the notice:
 * "The software can be freely used but comes with no warranty"
 *
 * [Adaptation] Copyright (c) 2018 Quentin Cappart, Emmanuel Goutierre, David Bergman and Louis-Martin Rousseau (MIT Licence)
 * --------------------------------------------------------
 * Ordering class - implementation
 * --------------------------------------------------------
 */

#include <algorithm>
#include <fstream>

#include "orderings.hpp"

using namespace std;


// min in state heuristic
int MinInState::vertex_in_layer(BDD* bdd, int layer) {
	return -1;
}

// randomized min in state heuristic
int RandomizedMinInState::vertex_in_layer(BDD* bdd, int layer) {
	return -1;
}

// fixed ordering
void FixedOrdering::read_ordering(char* filename) {
	ifstream ordering(filename);
	if( !ordering.is_open() ) {
		cout << "ERROR - could not open ordering file " << filename << endl;
		exit(1);
	}

	cout << "Ordering: ";
	for( int v = 0; v < inst->graph->n_vertices; v++ ) {
		ordering >> v_in_layer[v];
		cout  << v_in_layer[v] << " ";
	}

	ordering.close();
}



// minimum degree ordering
void MinDegreeOrdering::construct_ordering() {

  v_in_layer.clear();

  // compute vertex degree
  vector<int> degree( inst->graph->n_vertices, 0 );
  for ( int i = 0; i < inst->graph->n_vertices; ++i ) {
    for ( int j = i+1; j < inst->graph->n_vertices; ++j ) {
      if ( inst->graph->adj_m[i][j] ) {
	++(degree[i]);
	++(degree[j]);
      }
    }
  }

  vector<bool> selected( inst->graph->n_vertices, false);

  while ( (int)v_in_layer.size() < inst->graph->n_vertices ) {
  
    int min = INF;
    int v = -1;

    for ( int i = 0; i < inst->graph->n_vertices; ++i ) {
      if ( degree[i] > 0 && degree[i] < min && !selected[i] ) {
	min = degree[i];
	v = i;
      }
    }

    if ( v == -1 ) {
      for( int i = 0; i < inst->graph->n_vertices; ++i ) {
	if ( !selected[i] ) {
	  //cout << "\n selected vertex " << i << " --> degree: " << degree[i] << endl;
	  v_in_layer.push_back( i );
	}
      }
    } else {
      selected[v] = true;
      v_in_layer.push_back(v);
      //cout << "\n selected vertex " << v << " --> degree: " << degree[v] << endl;
      for ( int i = 0; i < inst->graph->n_vertices; ++i ) {
	if ( i != v && inst->graph->adj_m[i][v] ) {
	  --(degree[i]);
	}
      }
    }
  }
}


// maximal path decomposition
void MaximalPathDecomp::construct_ordering() {

	int n_maximal_paths = 0;

	v_in_layer.resize(inst->graph->n_vertices);
	vector<bool> visited(inst->graph->n_vertices, false);

	int n = 0;  // number of vertices already considered in the path

	// partial orderings
	vector<int> left;
	vector<int> right;

	while( n < inst->graph->n_vertices ) {
		left.clear();
		right.clear();

		int middle = -1;
		// take first unvisited vertex
		for( int v = 0; v < inst->graph->n_vertices; v++ ) {
			if( !visited[v] ) {
				middle = v;
				break;
			}
		}
		visited[middle] = true;

		// right composition
		int current = middle;
		while( current != -1 ) {
			int next = -1;
			for( int v = 0; v < inst->graph->n_vertices; v++ ) {
				if( !visited[v] && inst->graph->is_adj(current, v) ) {
					next = v;
					right.push_back(next);
					visited[next] = true;
					break;
				}
			}
			current = next;
		}

		// left composition
		current = middle;
		while( current != -1 ) {
			int next = -1;
			for( int v = 0; v < inst->graph->n_vertices; v++ ) {
				if( !visited[v] && inst->graph->is_adj(current, v) ) {
					next = v;
					left.push_back(next);
					visited[next] = true;
					break;
				}
			}
			current = next;
		}

		// compose path from left to right
		for( int i = (int)left.size()-1; i>=0; i-- ) {
			v_in_layer[n++] = left[i];
		}
		v_in_layer[n++] = middle;
		for( int i = 0; i < (int)right.size(); i++ ) {
			v_in_layer[n++] = right[i];
		}
		n_maximal_paths++;
	}

	cout << "\nnumber of maximal paths in decomposition: " << n_maximal_paths << endl << endl;

	// quick check...
	if( n_maximal_paths == 1 ) {
		for( int v = 0; v < inst->graph->n_vertices-1; v++ ) {
			if( !inst->graph->is_adj(v_in_layer[v], v_in_layer[v+1]) ) {
				cout << "ERROR IN MAXIMAL PATH DECOMPOSITION\n";
				exit(1);
			}
		}
	}

}


void RandomOrdering::construct_ordering() {
	for( int i = 0; i < inst->graph->n_vertices; i++ ) {
		v_in_layer[i] = i;
	}

	random_shuffle(v_in_layer.begin(), v_in_layer.end());
}


void CutVertexDecomposition::identify_components(vector< vector<int> > &comps, vector<bool> &is_in_graph) {

	vector<int> label(inst->graph->n_vertices, -1);
	int num_comps = -1;

	vector<int> stack;

	vector<bool> visited(inst->graph->n_vertices, false);
	for( int i = 0; i < inst->graph->n_vertices; i++ ) {

		if( is_in_graph[i] && !visited[i]) {

			num_comps++;
			stack.push_back(i);

			while( !stack.empty() ) {

				int v = stack.back();
				stack.pop_back();

				label[v] = num_comps;
				visited[v] = true;

				for( int w = 0; w < inst->graph->n_vertices; w++ ) {
					if( w == v ) continue;
					if( is_in_graph[w] && inst->graph->is_adj(v,w) && !visited[w]) {
						stack.push_back(w);
					}
				}
			}
		}
	}

	comps.clear();
	comps.resize(num_comps+1);

	for( int v = 0; v < inst->graph->n_vertices; v++ ) {
		if( label[v] != -1 ) {
			comps[label[v]].push_back(v);
		}
	}
}

vector<int> CutVertexDecomposition::find_ordering(vector<bool> is_in_graph) {

	int size = 0;
	for( int i = 0; i < inst->graph->n_vertices; i++ ) {
		size += (is_in_graph[i] ? 1 : 0 );
	}

	// find vertex with all components less than half the size of the graph
	vector< vector<int> > comps;
	for( int i = 0; i < inst->graph->n_vertices; i++ ) {
		if( is_in_graph[i] ) {

			// try removing vertex
			is_in_graph[i] = false;
			identify_components(comps, is_in_graph);

			//cout << "componentes when removing vertex " << i << endl;

			bool all_valid = true;
			for( int j = 0; j < (int)comps.size() && all_valid; j++ ) {
				all_valid = ((int)comps[j].size() <= size/2);
				//cout << "\t" << comps[j].size() << endl;
			}

			if( all_valid ) {

				vector<int> ordering;

				// compose ordering for each component separately
				vector<bool> is_in_graph_new(inst->graph->n_vertices, false);
				for( int c = 0; c < (int)comps.size(); c++ ) {

					//cout << "*** Component " << c << endl;

					for( int v = 0; v < (int)comps[c].size(); v++ ) {
						is_in_graph_new[comps[c][v]] = true;
					}

					vector<int> order_bck = find_ordering(is_in_graph_new);

					for( int v = 0; v < (int)comps[c].size(); v++ ) {
						is_in_graph_new[comps[c][v]] = false;
						ordering.push_back(order_bck[v]);
					}
				}
				ordering.push_back(i);
				return ordering;
			}

			// put vertex back again
			is_in_graph[i] = true;
		}
	}
	return( vector<int>(1,-1) );
}

void CutVertexDecomposition::construct_ordering() {

	vector<bool> is_in_graph(inst->graph->n_vertices, true);
	vector<int> ordering = find_ordering(is_in_graph);

	for( int i = 0; i < (int)ordering.size(); i++ ) {
		v_in_layer[i] = ordering[i];
	}

}


void CutVertexDecompositionGeneralGraph::identify_components(vector< vector<int> > &comps, vector<bool> &is_in_graph) {

	vector<int> label(inst->graph->n_vertices, -1);
	int num_comps = -1;

	vector<int> stack;

	vector<bool> visited(inst->graph->n_vertices, false);
	for( int i = 0; i < inst->graph->n_vertices; i++ ) {

		if( is_in_graph[i] && !visited[i]) {

			num_comps++;
			stack.push_back(i);

			while( !stack.empty() ) {

				int v = stack.back();
				stack.pop_back();

				label[v] = num_comps;
				visited[v] = true;

				for( int w = 0; w < inst->graph->n_vertices; w++ ) {
					if( w == v ) continue;
					if( is_in_graph[w] && inst->graph->is_adj(v,w) && !visited[w]) {
						stack.push_back(w);
					}
				}
			}
		}
	}

	comps.clear();
	comps.resize(num_comps+1);

	for( int v = 0; v < inst->graph->n_vertices; v++ ) {
		if( label[v] != -1 ) {
			comps[label[v]].push_back(v);
		}
	}
}

vector<int> CutVertexDecompositionGeneralGraph::find_ordering(vector<bool> is_in_graph) {

	int size = 0;
	for( int i = 0; i < inst->graph->n_vertices; i++ ) {
		size += (is_in_graph[i] ? 1 : 0 );
	}

	// find vertex with all components less than half the size of the graph
	vector< vector<int> > comps;
	for( int i = 0; i < inst->graph->n_vertices; i++ ) {
		if( is_in_graph[i] ) {

			// try removing vertex
			is_in_graph[i] = false;
			identify_components(comps, is_in_graph);

			//cout << "componentes when removing vertex " << i << endl;

			bool all_valid = true;
			for( int j = 0; j < (int)comps.size() && all_valid; j++ ) {
				all_valid = ((int)comps[j].size() <= size/2);
				//cout << "\t" << comps[j].size() << endl;
			}

			if( all_valid ) {

				vector<int> ordering;

				// compose ordering for each component separately
				vector<bool> is_in_graph_new(inst->graph->n_vertices, false);
				for( int c = 0; c < (int)comps.size(); c++ ) {

					//cout << "*** Component " << c << endl;

					for( int v = 0; v < (int)comps[c].size(); v++ ) {
						is_in_graph_new[comps[c][v]] = true;
					}

					vector<int> order_bck = find_ordering(is_in_graph_new);

					for( int v = 0; v < (int)comps[c].size(); v++ ) {
						is_in_graph_new[comps[c][v]] = false;
						ordering.push_back(order_bck[v]);
					}
				}
				ordering.push_back(i);
				return ordering;
			}

			// put vertex back again
			is_in_graph[i] = true;
		}
	}
	return( vector<int>(1,-1) );
}

void CutVertexDecompositionGeneralGraph::construct_ordering() {

	vector<bool> is_in_graph(inst->graph->n_vertices, true);
	vector<int> ordering = find_ordering(is_in_graph);

	for( int i = 0; i < (int)ordering.size(); i++ ) {
		v_in_layer[i] = ordering[i];
	}

}


void CutVertexDecompositionGeneralGraph::restrict_graph() {

	vector< pair<int,int> > edges;

	vector<int> vertices(inst->graph->n_vertices);
	vector<int> degrees(inst->graph->n_vertices);
	for( int i = 0; i < inst->graph->n_vertices; i++ ) {
		vertices[i] = i;
		degrees[i] = inst->graph->adj_list[i].size()-1;
	}
	IntComparator int_comp(degrees);
	sort(vertices.begin(), vertices.end(),int_comp);

	vector<bool> is_taken(inst->graph->n_vertices, false);
	vector<int> taken;

	cout << "vertices ordered by degree: " << endl;
	for( int i = 0; i < inst->graph->n_vertices; i++ ) {
		cout << vertices[i] << " ";
	}
	cout << endl;

	is_taken[vertices[0]] = true;
	int n_taken = 1;

	while( n_taken != inst->graph->n_vertices ) {
		for( int i = 0; i < inst->graph->n_vertices; i++ ) {

			if( !is_taken[vertices[i]] )
				continue;

			int w = vertices[i];

			bool new_edge = false;

			for( int v = 0; v < inst->graph->n_vertices; v++ ) {
				if( v != w && inst->graph->is_adj(v, w) && !is_taken[v] ) {

					pair<int,int> t;
					t.first = v;
					t.second = w;
					edges.push_back(t);

					is_taken[v] = true;
					n_taken++;

					new_edge = true;
					cout << "took " << v << " due to " << w << endl;
				}
			}

			if( new_edge )
				break;
		}
	}

	cout << endl;
	for( int i = 0; i < (int)edges.size(); i++ ) {
		cout << edges[i].first << "," << edges[i].second << endl;
	}
	cout << endl;

	bool** new_adj = new bool*[inst->graph->n_vertices];
	for( int i = 0; i < inst->graph->n_vertices; i++ ) {
		new_adj[i] = new bool[inst->graph->n_vertices];
	}

	for( int i = 0; i < inst->graph->n_vertices; i++ ) {
		for( int j = i+1; j < inst->graph->n_vertices; j++ ) {
			new_adj[i][j] = false;
			new_adj[j][i] = false;
		}
	}

	for( int i = 0; i < (int)edges.size(); i++ ) {
		new_adj[edges[i].first][edges[i].second] = true;
		new_adj[edges[i].second][edges[i].first] = true;
	}

	original_adj_matrix = inst->graph->adj_m;
	inst->graph->adj_m = new_adj;
}


void CutVertexDecompositionGeneralGraph::regenerate_graph() {
	inst->graph->adj_m = original_adj_matrix;
}



