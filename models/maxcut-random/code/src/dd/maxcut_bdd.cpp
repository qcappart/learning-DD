// --------------------------------------
// MaxCut - BDD Solver
// --------------------------------------

#define TIME_LIMIT 3600

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <queue>
#include <random>
#include "maxcut_bdd.hpp"
#include <math.h>

using namespace std;


//
// MaxCut Constructor
//
MaxCutBDD::MaxCutBDD(const int _placeID,
                     const int _ddWidth,
                     const string & _instanceName, const int ordering)
        : /*DDX10_Base(_placeID, _ddWidth, _instanceName, _cb),*/
        inst(  new MaxCutInst(_instanceName.c_str()) ),
        max_width(_ddWidth),
        bestLB(-INF),
        isLBUpdated(false),
        isExact(false),
        ordering(ordering)
{
}




//
// MaxCut Constructor
//
MaxCutBDD::MaxCutBDD(const int _placeID,
                     const int _ddWidth,
                     std::vector< std::vector< std::pair<int, double> > > _adj, const int ordering, double w_scaling)
:
inst( new MaxCutInst(_adj, w_scaling)),
max_width(_ddWidth),
bestLB(-INF),
isLBUpdated(false),
isExact(false),
ordering(ordering)
{
}


MaxCutBDD::MaxCutBDD(const int _placeID,
                     const int _ddWidth,
                     MaxCutInst* _inst, const int ordering, const char* _orderingFile)
        :
        inst(_inst),
        max_width(_ddWidth),
        bestLB(-INF),
        isLBUpdated(false),
        isExact(false),
        ordering(ordering),
        orderingFile(_orderingFile)
{
}


//
// MaxCut Instance Constructor
//

MaxCutInst::MaxCutInst(std::vector< std::vector< std::pair<int, double> > > adj, double w_scaling) {
            n_vertices =  adj.size();
            adj_matrix.resize( n_vertices, vector<int>(n_vertices, 0) );
            adj_list.resize( n_vertices );
            sum_neg_weights = 0;
            int count_edges = 0;

            for(int i = 0 ; i < n_vertices ; i++) {
                for (auto& neigh : adj[i]) {
                    int u = i;
                    int v = neigh.first;
                    int w = round(neigh.second / w_scaling);
                    adj_matrix[u][v] = w;
                    adj_matrix[v][u] = w;
                    adj_list[u].push_back( pair<int,int>(v, w) );
                    adj_list[v].push_back( pair<int,int>(u, w) );
                    count_edges += 1;

                    if (w < 0) {
                        sum_neg_weights += w;
                    }
                }
            }

            n_edges =  count_edges / 2;
            sum_neg_weights = sum_neg_weights / 2;
        }

//
// MaxCut Instance Constructor
//
MaxCutInst::MaxCutInst(const char* filename) {
    // open file
    //name = filename;
    ifstream input(filename);
    if (!input.is_open()) {
        cout << "\nCould not open " << filename << endl;
        exit(1);
    }

    // allocate matrices
    input >> n_vertices;
    input >> n_edges;
    adj_matrix.resize( n_vertices, vector<int>(n_vertices, 0) );
    adj_list.resize( n_vertices );

    // read edges
    sum_neg_weights = 0;
    int u, v, w;
    for (int i = 0; i < n_edges; ++i) {
        input >> u; u--;
        input >> v; v--;
        input >> w;

        adj_matrix[u][v] = w;
        adj_matrix[v][u] = w;

        adj_list[u].push_back( pair<int,int>(v, w) );
        adj_list[v].push_back( pair<int,int>(u, w) );

        if (w < 0) {
            sum_neg_weights += w;
        }
    }
    input.close();

    cout << endl;
    cout << "MaxCut instance" << endl;
    cout << "\tnum vertices = " << n_vertices << endl;
    cout << "\tnum edges = " << n_edges << endl;
    cout << "\tsum neg weights = " << sum_neg_weights << endl;
    cout << endl;



}



//
// Sort pair by second element
//
struct PairSecondElementDecreasingSort {
    bool operator() (pair<int,int> const& x, pair<int,int> const& y) {
        return x.second > y.second;
    }
};


//
// Reorder variables in maxcut instance
//
MaxCutInst* reorder_variables(MaxCutInst* inst) {

    // sort vertices according to sum of incoming weights
    vector< pair<int,int> > weights;
    for (int i = 0; i < inst->n_vertices; ++i) {
        int weight = 0;
        for (int j = 0; j < inst->n_vertices; ++j) {
            weight += inst->adj_matrix[i][j];
        }
        weights.push_back( pair<int,int>(i, weight) );
    }

    sort(weights.begin(), weights.end(), PairSecondElementDecreasingSort());

    MaxCutInst* newinst = new MaxCutInst;
    newinst->n_vertices = inst->n_vertices;
    newinst->n_edges = inst->n_edges;
    newinst->name = inst->name;
    newinst->sum_neg_weights = inst->sum_neg_weights;


    newinst->adj_matrix.resize( inst->n_vertices, vector<int>(inst->n_vertices, 0) );
    for (int i = 0; i < inst->n_vertices; ++i) {
        for (int j = 0; j < inst->n_vertices; ++j) {
            newinst->adj_matrix[i][j] = inst->adj_matrix[weights[i].first][weights[j].first];
        }
    }

    delete inst;
    return newinst;
}



//
// BDD Node comparator according to longest path
//
struct BDDNodeLPSort {
    bool operator() (BDDNode* const& nodeA, BDDNode* const& nodeB) {
        return nodeA->longest_path > nodeB->longest_path;
    }
};


//
// BDD Node comparator according to ranking
//
struct BDDNodeRankSort {
    bool operator() (BDDNode* const& nodeA, BDDNode* const& nodeB) {
        return nodeA->rank > nodeB->rank;
    }
};



void MaxCutBDD::initialize(State &initial_state, int i_cost, bool s_nodes) {

    // initialize structures
    node_map[0].clear();
    node_map[1].clear();
    available_vertex.clear();
    tmp.resize( inst->n_vertices);

    for (int i = 0; i < inst->n_vertices; ++i) {
        available_vertex.insert(i);
    }

    current_map_idx = 0;
    next_map_idx = 1;

    last_exact_layer = true;
    save_nodes = s_nodes;
    initial_cost = i_cost;

    if (save_nodes) {
        localBranchNodes.clear();
    }

    // create BDD root node
    root_node = new BDDNode(initial_state, initial_cost);

    // set initial layer
    initial_layer = inst->n_vertices - initial_state.size();

}

int MaxCutBDD::generate_next_step_relaxation(int cur_vertex, int l) {

    available_vertex.erase(cur_vertex);

    /*cout <<  "Available Vertices = { ";
    for (set<int>::const_iterator i = available_vertex.begin(); i != available_vertex.end(); ++i)
    {
      cout << *i << " ";
    }
    cout << "}" << endl;*/


    if (initial_layer == 0) {
        // First BDD: set first vertex to S

        for (auto v : available_vertex) {
            root_node->state[v] = inst->adj_matrix[cur_vertex][v];

        }

        root_node->longest_path = inst->sum_neg_weights + initial_cost;
        initial_layer = 1;
        node_map[current_map_idx][&root_node->state] = root_node;

        return root_node->longest_path;
    }


    state_vec.resize(max_width*2);
    state_vec.clear();

    // get node maps
    NodeMap& map = node_map[current_map_idx];
    NodeMap& next_map = node_map[next_map_idx];
    next_map.clear();

    // collects nodes in the layer according to map
    nodes_layer.clear();
    for (NodeMap::iterator it = map.begin(); it != map.end(); ++it) {
        nodes_layer.push_back( it->second );
    }

    //cout << "Layer " << l << " - size = " << map.size() << endl;

    // if exceeds width, generate restriction
    if (max_width != -1 && (int)nodes_layer.size() > max_width) {
        if (last_exact_layer && save_nodes) {
            for (int i = 0; i < (int)nodes_layer.size(); ++i) {
                add_branch_node(nodes_layer[i]);
            }
            last_exact_layer = false;
        }
        relax_layer(l, nodes_layer, false);
        assert(nodes_layer.size() == max_width);
    }

    // process nodes in current map

    int longest = 0;
    int state_vec_idx = 0;
    width = std::max((int) nodes_layer.size(),width);

    for (vector<BDDNode*>::iterator it = nodes_layer.begin(); it != nodes_layer.end(); ++it) {

        // get BDD node
        bddnode = *it;
        State& state = bddnode->state;


        // --------------------------
        // Zero arc
        // --------------------------

        // compute transition cost
        longest_path = bddnode->longest_path + std::max(-state[cur_vertex], 0);

        for (auto v : available_vertex) {
            if (state[v] * inst->adj_matrix[cur_vertex][v] <= 0) {
                longest_path += std::min( std::abs(state[v]), std::abs(inst->adj_matrix[cur_vertex][v]));
            }
        }

        tmp.clear();
        tmp.resize(inst->n_vertices);
        for (auto v : available_vertex) {
            tmp[v] = state[v] + inst->adj_matrix[cur_vertex][v];

        }

        // add to set

        state_vec.push_back(tmp);


        node = next_map.find(&state_vec.at(state_vec_idx));



        int path_0 = 0;

        if (node == next_map.end()) {
            next_map[&state_vec.at(state_vec_idx)] = new BDDNode(state_vec.at(state_vec_idx), longest_path, bddnode->exact);
            path_0 = longest_path;
        }

        else {
            if (!bddnode->exact && node->second->exact && save_nodes) {
                add_branch_node(node->second);
                node->second->exact = false;
            }
            node->second->longest_path = std::max(node->second->longest_path, longest_path);
            path_0 = std::max(node->second->longest_path, longest_path);
        }

        state_vec_idx++;
        // --------------------------
        // One arc
        // --------------------------

        // compute transition cost
        longest_path = bddnode->longest_path + std::max(state[cur_vertex], 0);


        for (auto v : available_vertex) {
            if (state[v] * inst->adj_matrix[cur_vertex][v] >= 0) {
                longest_path += std::min( std::abs(state[v]), std::abs(inst->adj_matrix[cur_vertex][v]) );
            }
        }


        // set state

        tmp.clear();
        tmp.resize(inst->n_vertices);

        for (auto v : available_vertex) {
            //aux_state[v] = state[v] - inst->adj_matrix[cur_vertex][v];
            tmp[v] = state[v] - inst->adj_matrix[cur_vertex][v];
        }

        // add to set

        state_vec.push_back(tmp);

        node = next_map.find(&state_vec.at(state_vec_idx));

        int path_1 = 0;

        if (node == next_map.end()) {
            next_map[&state_vec.at(state_vec_idx)] = new BDDNode(state_vec.at(state_vec_idx), longest_path, bddnode->exact);
            path_1 = longest_path;
        }

        else {
            if (!bddnode->exact && node->second->exact && save_nodes) {
                add_branch_node(node->second);
                node->second->exact = false;
            }
            node->second->longest_path = std::max(node->second->longest_path, longest_path);
            path_1 = std::max(node->second->longest_path, longest_path);
        }
        state_vec_idx++;
        // delete current BDD node
        delete bddnode;
        longest = std::max(path_0,path_1);
    }

    // switch maps
    current_map_idx = !current_map_idx;
    next_map_idx = !next_map_idx;
    tmp.clear();
    return longest;

}

int MaxCutBDD::generate_next_step_restriction(int cur_vertex, int l) {

    available_vertex.erase(cur_vertex);

    /*cout <<  "Available Vertices = { ";
    for (set<int>::const_iterator i = available_vertex.begin(); i != available_vertex.end(); ++i)
    {
      cout << *i << " ";
    }
    cout << "}" << endl;*/


    if (initial_layer == 0) {
        // First BDD: set first vertex to S

        for (auto v : available_vertex) {
            root_node->state[v] = inst->adj_matrix[cur_vertex][v];

        }

        root_node->longest_path = inst->sum_neg_weights + initial_cost;
        initial_layer = 1;
        node_map[current_map_idx][&root_node->state] = root_node;

        return root_node->longest_path;
    }


    state_vec.resize(max_width*2);
    state_vec.clear();

    // get node maps
    NodeMap& map = node_map[current_map_idx];
    NodeMap& next_map = node_map[next_map_idx];
    next_map.clear();

    // collects nodes in the layer according to map
    nodes_layer.clear();
    for (NodeMap::iterator it = map.begin(); it != map.end(); ++it) {
        nodes_layer.push_back( it->second );
    }

    //cout << "Layer " << l << " - size = " << map.size() << endl;

    // if exceeds width, generate restriction
    if (max_width != -1 && (int)nodes_layer.size() > max_width) {
        restrict_layer(l, nodes_layer, save_nodes);
    }

    // process nodes in current map

    int longest = 0;
    int state_vec_idx = 0;
    width = std::max((int) nodes_layer.size(),width);

    for (vector<BDDNode*>::iterator it = nodes_layer.begin(); it != nodes_layer.end(); ++it) {

        // get BDD node
        bddnode = *it;
        State& state = bddnode->state;


        // --------------------------
        // Zero arc
        // --------------------------

        // compute transition cost
        longest_path = bddnode->longest_path + std::max(-state[cur_vertex], 0);

        for (auto v : available_vertex) {
            if (state[v] * inst->adj_matrix[cur_vertex][v] <= 0) {
                longest_path += std::min( std::abs(state[v]), std::abs(inst->adj_matrix[cur_vertex][v]));
            }
        }

        tmp.clear();
        tmp.resize(inst->n_vertices);
        for (auto v : available_vertex) {
            tmp[v] = state[v] + inst->adj_matrix[cur_vertex][v];

        }

        // add to set

        state_vec.push_back(tmp);


        node = next_map.find(&state_vec.at(state_vec_idx));



        int path_0 = 0;

        if (node == next_map.end()) {
            next_map[&state_vec.at(state_vec_idx)] = new BDDNode(state_vec.at(state_vec_idx), longest_path, bddnode->exact);
            path_0 = longest_path;
        }

        else {
            if (!bddnode->exact && node->second->exact && save_nodes) {
                add_branch_node(node->second);
                node->second->exact = false;
            }
            node->second->longest_path = std::max(node->second->longest_path, longest_path);
            path_0 = std::max(node->second->longest_path, longest_path);
        }

        state_vec_idx++;
        // --------------------------
        // One arc
        // --------------------------

        // compute transition cost
        longest_path = bddnode->longest_path + std::max(state[cur_vertex], 0);


        for (auto v : available_vertex) {
            if (state[v] * inst->adj_matrix[cur_vertex][v] >= 0) {
                longest_path += std::min( std::abs(state[v]), std::abs(inst->adj_matrix[cur_vertex][v]) );
            }
        }

        // set state

        tmp.clear();
        tmp.resize(inst->n_vertices);

        for (auto v : available_vertex) {
            //aux_state[v] = state[v] - inst->adj_matrix[cur_vertex][v];
            tmp[v] = state[v] - inst->adj_matrix[cur_vertex][v];
        }

        // add to set

        state_vec.push_back(tmp);

        node = next_map.find(&state_vec.at(state_vec_idx));

        int path_1 = 0;

        if (node == next_map.end()) {
            next_map[&state_vec.at(state_vec_idx)] = new BDDNode(state_vec.at(state_vec_idx), longest_path, bddnode->exact);
            path_1 = longest_path;
        }

        else {
            if (!bddnode->exact && node->second->exact && save_nodes) {
                add_branch_node(node->second);
                node->second->exact = false;
            }
            node->second->longest_path = std::max(node->second->longest_path, longest_path);
            path_1 = std::max(node->second->longest_path, longest_path);
        }
        state_vec_idx++;
        // delete current BDD node
        delete bddnode;
        longest = std::max(path_0,path_1);
    }

    // switch maps
    current_map_idx = !current_map_idx;
    next_map_idx = !next_map_idx;
    tmp.clear();
    return longest;

}

int MaxCutBDD::get_final_bound() {

    BDDNode* terminal = node_map[current_map_idx].begin()->second;
    int ret = terminal->longest_path;
    //delete terminal;
    return ret;
}


int MaxCutBDD::generate_relaxation(State &initial_state,
                                   int initial_cost,
                                   bool save_nodes) {




    int static_order[inst->n_vertices];
    int idx_order = 0;

    // initialize structures
    node_map[0].clear();
    node_map[1].clear();
    available_vertex.clear();
    tmp.resize( inst->n_vertices);

    int cur_vertex = -1;
    for (int i = 0; i < inst->n_vertices; ++i) {
        available_vertex.insert(i);
    }

    if(ordering == 1) {
        srand(time(NULL));
        set<int>::iterator it = available_vertex.begin();

        for (int r = rand() % available_vertex.size(); r != 0; r--) {
            it++;
        }
        cur_vertex = *it;
        available_vertex.erase(cur_vertex);

    }

    else if(ordering == 3) {
        //int randVertex = rand() % available_vertex.size();
        //available_vertex.erase(randVertex);


        ifstream input(orderingFile);
        if (!input.is_open()) {
            cout << "\nCould not open ordering file " << orderingFile << endl;
            exit(1);
        }


        for (int i = 0; i < inst->n_vertices; ++i) {
            input >> static_order[i];
        }
        input.close();

        available_vertex.erase(static_order[idx_order]);
        cur_vertex = static_order[idx_order];
        idx_order++;


    }

    else {
        available_vertex.erase(0);
        cur_vertex = 0;
    }

    current_map_idx = 0;
    next_map_idx = 1;

    aux_state.resize(initial_state.size());
    bool last_exact_layer = true;

    if (save_nodes) {
        localBranchNodes.clear();
    }

    // create BDD root node
    BDDNode* root_node = new BDDNode(initial_state, initial_cost);

    // set initial layer
    int initial_layer = inst->n_vertices - initial_state.size();

    if (initial_layer == 0) {
        // First BDD: set first vertex to S
        //root_node->state.resize( root_node->state.size()-1 );

        for (auto v : available_vertex) {
            root_node->state[v] = inst->adj_matrix[cur_vertex][v];
        }

        /*
        for (int i = 1; i < inst->n_vertices; ++i) {
          root_node->state[i] = inst->adj_matrix[0][i];
        }*/

        root_node->longest_path = inst->sum_neg_weights + initial_cost;
        initial_layer = 1;
    } else {
        root_node->longest_path = initial_cost;
    }
    node_map[current_map_idx][&root_node->state] = root_node;

    // auxiliaries
    int longest_path = 0;
    NodeMap::iterator node;
    BDDNode* bddnode = NULL;

    // process each layer
    for (int l = initial_layer; l < inst->n_vertices; ++l) {

        // remove a vertex
        //aux_state.resize( inst->n_vertices);
        //std::fill(aux_state.begin(), aux_state.end(), 0);




        /*cout <<  "Available Vertices = { ";
        for (set<int>::const_iterator i = available_vertex.begin(); i != available_vertex.end(); ++i)
        {
          cout << *i << " ";
        }
        cout << "}" << endl;*/

        // remove a vertex

        if(ordering == 1) {
            set<int>::iterator it = available_vertex.begin();

            for (int r = rand() % available_vertex.size(); r != 0; r--) {
                it++;
            }
            cur_vertex = *it;
            available_vertex.erase(cur_vertex);

        }

        else if(ordering == 3) {
            //int randVertex = rand() % available_vertex.size();
            //available_vertex.erase(randVertex);
            available_vertex.erase(static_order[idx_order]);
            cur_vertex = static_order[idx_order];
            idx_order++;
        }


        else {
            available_vertex.erase(l);
            cur_vertex = l;
        }


        // get node maps
        NodeMap& map = node_map[current_map_idx];
        NodeMap& next_map = node_map[next_map_idx];
        next_map.clear();
        // collects nodes in the layer according to map
        nodes_layer.clear();

        for (NodeMap::iterator it = map.begin(); it != map.end(); ++it) {
            nodes_layer.push_back( it->second );
        }

        if(max_width == -1) {
            state_vec.resize(map.size());
        }
        else {
            state_vec.resize(max_width);
        }
        state_vec.clear();



        //cout << "Layer " << l << " - size = " << map.size() << endl;

        // if exceeds width, generate restriction
        if (max_width != -1 && (int)nodes_layer.size() > max_width) {
            if (last_exact_layer && save_nodes) {
                for (int i = 0; i < (int)nodes_layer.size(); ++i) {

                    add_branch_node(nodes_layer[i]);
                }
                last_exact_layer = false;
            }
            relax_layer(l, nodes_layer, false);
            assert(nodes_layer.size() == max_width);
        }
        // process nodes in current map

        int state_vec_idx = 0;
        for (vector<BDDNode*>::iterator it = nodes_layer.begin(); it != nodes_layer.end(); ++it) {

            // get BDD node
            bddnode = *it;
            State& state = bddnode->state;

            /*cout << l << "\tState = ({ ";
            for (int i = 0; i < (int)state.size(); ++i) {
              cout << state[i] << " ";
            }
            cout << "}, " << bddnode->longest_path << ")" << endl;*/

            // --------------------------
            // Zero arc
            // --------------------------

            // compute transition cost
            longest_path = bddnode->longest_path + std::max(-state[cur_vertex], 0);
            //cout << "\t\t inc: " << -state[l] << endl;
            for (auto v : available_vertex) {
                // cout << "\t\t state vertex: " << state[v] << endl;
                //cout << "\t\t weight: " << inst->adj_matrix[l][v] << endl;
                if (state[v] * inst->adj_matrix[cur_vertex][v] <= 0) {
                    //  cout << "\t\t ENTERED" << endl;
                    //  cout << "\t\t inc: " << std::min( std::abs(state[v]), std::abs(inst->adj_matrix[l][v])) << endl;
                    longest_path += std::min( std::abs(state[v]), std::abs(inst->adj_matrix[cur_vertex][v]));
                }
            }
            //cout << "Intermediate longest path: " << longest_path << endl;

            tmp.clear();
            tmp.resize( inst->n_vertices);
            for (auto v : available_vertex) {
                //aux_state[i-1] = state[i] + inst->adj_matrix[l][l+i];
                //aux_state[v] = state[v] + inst->adj_matrix[cur_vertex][v];
                tmp[v] = state[v] + inst->adj_matrix[cur_vertex][v];
                //state_vec[state_vec_idx][v] = state[v] + inst->adj_matrix[cur_vertex][v];
            }


            state_vec.push_back(tmp);


            node = next_map.find(&state_vec.at(state_vec_idx));

            //cout << "after find" << endl;
            if (node == next_map.end()) {
                //cout << "\tNew state created" << endl;
                next_map[&state_vec.at(state_vec_idx)] = new BDDNode(state_vec.at(state_vec_idx), longest_path, bddnode->exact);
            }

            else {
                if (!bddnode->exact && node->second->exact && save_nodes) {

                    add_branch_node(node->second);
                    node->second->exact = false;
                }
                node->second->longest_path = std::max(node->second->longest_path, longest_path);
            }


            state_vec_idx++;
            // --------------------------
            // One arc
            // --------------------------

            // compute transition cost
            longest_path = bddnode->longest_path + std::max(state[cur_vertex], 0);
            /*for (int i = 1; i < (int)state.size(); ++i) {
              if (state[i] * inst->adj_matrix[l][l+i] >= 0) {
                longest_path += std::min( std::abs(state[i]),
                                          std::abs(inst->adj_matrix[l][l+i]) );
              }
            }*/


            for (auto v : available_vertex) {
                if (state[v] * inst->adj_matrix[cur_vertex][v] >= 0) {
                    longest_path += std::min( std::abs(state[v]), std::abs(inst->adj_matrix[cur_vertex][v]) );
                }
            }
            // cout << "\tintermediate lp one arc: " << longest_path << endl;
            // set state
            /*for (int i = 1; i < (int)state.size(); ++i) {
              aux_state[i] = state[i] - inst->adj_matrix[l][l+i];
            }*/

            tmp.clear();
            tmp.resize( inst->n_vertices);
            for (auto v : available_vertex) {
                //aux_state[v] = state[v] - inst->adj_matrix[cur_vertex][v];
                tmp[v] = state[v] - inst->adj_matrix[cur_vertex][v];
            }

            state_vec.push_back(tmp);
            /*cout << l << "\tTmp State = ({ ";
            for (int i = 0; i < (int)state.size(); ++i) {
              cout << aux_state[i] << " ";
            }
            cout << "}, " << bddnode->longest_path << ")" << endl;*/
            // add to set


            //cout << "before find" << endl;
            //node = next_map.find(&tmp);
            node = next_map.find(&state_vec.at(state_vec_idx));
            //cout << "after find" << endl;

            if (node == next_map.end()) { // If node does noy exist yet in layer.
                //cout << "\tNew state created" << endl;
                next_map[&state_vec.at(state_vec_idx)] = new BDDNode(state_vec.at(state_vec_idx), longest_path, bddnode->exact);
            } else {
                if (!bddnode->exact && node->second->exact && save_nodes) {
                    add_branch_node(node->second);
                    node->second->exact = false;
                }
                node->second->longest_path = std::max(node->second->longest_path, longest_path);
            }
            state_vec_idx++;
            /*cout << "cur Store{" ;

            for(auto a : next_map) {
              cout << "[";
              for(auto s : *a.first) {
                cout << s << " ";
              }
              cout << "]";
            }
            cout << "}" << endl;*/

            // delete current BDD node
            delete bddnode;
        }

        // switch maps
        current_map_idx = !current_map_idx;
        next_map_idx = !next_map_idx;

    }

    // get longest path from terminal BDD node
    BDDNode* terminal = node_map[current_map_idx].begin()->second;

    longest_path = terminal->longest_path;
    isExact = terminal->exact;

    delete terminal;

    // set relaxation ub of branching nodes
    if (save_nodes) {
        for (int i = 0; i < (int)localBranchNodes.size(); ++i) {
            localBranchNodes[i]->relax_ub = longest_path;
        }
    }

    return longest_path;
}


//
// Relax layer size to maximum width
//
void MaxCutBDD::relax_layer(int layer, vector<BDDNode*> &nodes, bool save_nodes) {
    assert(nodes.size() > max_width);

    // compute node ranking
    for (int i = 0; i < (int)nodes.size(); ++i) {
        nodes[i]->rank = nodes[i]->longest_path;
        for (int j = 0; j < (int)nodes[i]->state.size(); ++j) {
            nodes[i]->rank += std::abs(nodes[i]->state[j]);
        }
    }

    // sort nodes by ranking
    sort(nodes.begin(), nodes.end(), BDDNodeRankSort());

    // relax nodes by pairs
    int lpA, lpB, aux;
    for (int i = nodes.size()-2; i >= max_width-1; --i) {

        if (save_nodes) {
            if (nodes[i]->exact) {
                add_branch_node(nodes[i]);
            }
            if (nodes[i+1]->exact) {
                add_branch_node(nodes[i+1]);
            }
        }

        State& stateA = nodes[i]->state;
        lpA = nodes[i]->longest_path;

        State& stateB = nodes[i+1]->state;
        lpB = nodes[i+1]->longest_path;

        assert( stateA.size() == stateB.size() );

        for (int k = 0; k < (int)stateA.size(); ++k) {
            if (stateA[k] >= 0 && stateB[k] >= 0) {
                aux = std::min(stateA[k], stateB[k]);
                lpA += (stateA[k] - aux);
                lpB += (stateB[k] - aux);

            } else if (stateA[k] <= 0 && stateB[k] <= 0) {
                aux = std::max(stateA[k], stateB[k]);
                lpA += ( (-1)*stateA[k] - (-1)*aux );
                lpB += ( (-1)*stateB[k] - (-1)*aux );

            } else {
                aux = 0;
                lpA += std::abs(stateA[k]);
                lpB += std::abs(stateB[k]);
            }
            stateA[k] = aux;
        }
        nodes[i]->longest_path = std::max(lpA, lpB);

        nodes[i]->exact = false;
        nodes[i+1]->exact = false;

        delete nodes[i+1];
    }

    // truncate layer
    nodes.resize(max_width);
}

//
// MaxCutBDD :: Create restriction
//
int MaxCutBDD::generate_restriction(State& initial_state, int initial_cost,
                                    bool save_nodes) {



    int static_order[inst->n_vertices];
    int idx_order = 0;

    // initialize structures
    node_map[0].clear();
    node_map[1].clear();
    available_vertex.clear();
    tmp.resize( inst->n_vertices);

    int cur_vertex = -1;
    for (int i = 0; i < inst->n_vertices; ++i) {
        available_vertex.insert(i);
    }

    if(ordering == 1) {
        srand(time(NULL));
        set<int>::iterator it = available_vertex.begin();

        for (int r = rand() % available_vertex.size(); r != 0; r--) {
            it++;
        }
        cur_vertex = *it;
        available_vertex.erase(cur_vertex);

    }

    else if(ordering == 3) {
        //int randVertex = rand() % available_vertex.size();
        //available_vertex.erase(randVertex);


        ifstream input(orderingFile);
        if (!input.is_open()) {
            cout << "\nCould not open ordering file " << orderingFile << endl;
            exit(1);
        }


        for (int i = 0; i < inst->n_vertices; ++i) {
            input >> static_order[i];
        }
        input.close();

        available_vertex.erase(static_order[idx_order]);
        cur_vertex = static_order[idx_order];
        idx_order++;
    }

    else {
        available_vertex.erase(0);
        cur_vertex = 0;
    }

    current_map_idx = 0;
    next_map_idx = 1;

    aux_state.resize(initial_state.size());

    if (save_nodes) {
        localBranchNodes.clear();
    }

    // create BDD root node
    BDDNode* root_node = new BDDNode(initial_state, initial_cost);

    // set initial layer
    int initial_layer = inst->n_vertices - initial_state.size();

    if (initial_layer == 0) {
        // First BDD: set first vertex to S
        //root_node->state.resize( root_node->state.size()-1 );

        for (auto v : available_vertex) {
            root_node->state[v] = inst->adj_matrix[cur_vertex][v];
        }

        root_node->longest_path = inst->sum_neg_weights + initial_cost;
        initial_layer = 1;
    } else {
        root_node->longest_path = initial_cost;
    }
    node_map[current_map_idx][&root_node->state] = root_node;

    // auxiliaries
    int longest_path = 0;
    NodeMap::iterator node;
    BDDNode* bddnode = NULL;

    // process each layer
    for (int l = initial_layer; l < inst->n_vertices; ++l) {

        // remove a vertex
        //aux_state.resize( inst->n_vertices);
        //std::fill(aux_state.begin(), aux_state.end(), 0);




        /*cout <<  "Available Vertices = { ";
        for (set<int>::const_iterator i = available_vertex.begin(); i != available_vertex.end(); ++i)
        {
          cout << *i << " ";
        }
        cout << "}" << endl;*/

        // remove a vertex

        if(ordering == 1) {
            set<int>::iterator it = available_vertex.begin();

            for (int r = rand() % available_vertex.size(); r != 0; r--) {
                it++;
            }
            cur_vertex = *it;
            available_vertex.erase(cur_vertex);

        }

        else if(ordering == 3) {
            //int randVertex = rand() % available_vertex.size();
            //available_vertex.erase(randVertex);
            available_vertex.erase(static_order[idx_order]);
            cur_vertex = static_order[idx_order];
            idx_order++;
        }


        else {
            available_vertex.erase(l);
            cur_vertex = l;
        }

        // get node maps
        NodeMap& map = node_map[current_map_idx];
        NodeMap& next_map = node_map[next_map_idx];
        next_map.clear();
        // collects nodes in the layer according to map
        nodes_layer.clear();

        for (NodeMap::iterator it = map.begin(); it != map.end(); ++it) {
            nodes_layer.push_back( it->second );
        }

        if(max_width == -1) {
            state_vec.resize(map.size()*2);
        }
        else {
            state_vec.resize(max_width*2);
        }
        state_vec.clear();



        //cout << "Layer " << l << " - size = " << map.size() << endl;

        // if exceeds width, generate restriction
        if (max_width != -1 && (int)nodes_layer.size() > max_width) {
            restrict_layer(l, nodes_layer, save_nodes);
        }
        // process nodes in current map

        int state_vec_idx = 0;
        for (vector<BDDNode*>::iterator it = nodes_layer.begin(); it != nodes_layer.end(); ++it) {

            // get BDD node
            bddnode = *it;
            State& state = bddnode->state;

            /*cout << l << "\tState = ({ ";
            for (int i = 0; i < (int)state.size(); ++i) {
              cout << state[i] << " ";
            }
            cout << "}, " << bddnode->longest_path << ")" << endl;*/

            // --------------------------
            // Zero arc
            // --------------------------

            // compute transition cost
            longest_path = bddnode->longest_path + std::max(-state[cur_vertex], 0);
            //cout << "\t\t inc: " << -state[l] << endl;
            for (auto v : available_vertex) {
                // cout << "\t\t state vertex: " << state[v] << endl;
                //cout << "\t\t weight: " << inst->adj_matrix[l][v] << endl;
                if (state[v] * inst->adj_matrix[cur_vertex][v] <= 0) {
                    //  cout << "\t\t ENTERED" << endl;
                    //  cout << "\t\t inc: " << std::min( std::abs(state[v]), std::abs(inst->adj_matrix[l][v])) << endl;
                    longest_path += std::min( std::abs(state[v]), std::abs(inst->adj_matrix[cur_vertex][v]));
                }
            }
            //cout << "Intermediate longest path: " << longest_path << endl;

            tmp.clear();
            tmp.resize( inst->n_vertices);
            for (auto v : available_vertex) {
                //aux_state[i-1] = state[i] + inst->adj_matrix[l][l+i];
                //aux_state[v] = state[v] + inst->adj_matrix[cur_vertex][v];
                tmp[v] = state[v] + inst->adj_matrix[cur_vertex][v];
                //state_vec[state_vec_idx][v] = state[v] + inst->adj_matrix[cur_vertex][v];
            }

            state_vec.push_back(tmp);

            node = next_map.find(&state_vec.at(state_vec_idx));

            //cout << "after find" << endl;
            if (node == next_map.end()) {
                //cout << "\tNew state created" << endl;
                next_map[&state_vec.at(state_vec_idx)] = new BDDNode(state_vec.at(state_vec_idx), longest_path, bddnode->exact);
            }

            else {
                if (!bddnode->exact && node->second->exact && save_nodes) {

                    add_branch_node(node->second);
                    node->second->exact = false;
                }
                node->second->longest_path = std::max(node->second->longest_path, longest_path);
            }

            /*cout << "cur Store{" ;

            for(auto a : next_map) {
              cout << "[";
              for(auto s : *a.first) {
                cout << s << " ";
              }
              cout << "]";
            }
            cout << "}" << endl;*/

            state_vec_idx++;
            // --------------------------
            // One arc
            // --------------------------

            // compute transition cost
            longest_path = bddnode->longest_path + std::max(state[cur_vertex], 0);
            /*for (int i = 1; i < (int)state.size(); ++i) {
              if (state[i] * inst->adj_matrix[l][l+i] >= 0) {
                longest_path += std::min( std::abs(state[i]),
                                          std::abs(inst->adj_matrix[l][l+i]) );
              }
            }*/


            for (auto v : available_vertex) {
                if (state[v] * inst->adj_matrix[cur_vertex][v] >= 0) {
                    longest_path += std::min( std::abs(state[v]), std::abs(inst->adj_matrix[cur_vertex][v]) );
                }
            }
            // cout << "\tintermediate lp one arc: " << longest_path << endl;
            // set state
            /*for (int i = 1; i < (int)state.size(); ++i) {
              aux_state[i] = state[i] - inst->adj_matrix[l][l+i];
            }*/

            tmp.clear();
            tmp.resize( inst->n_vertices);
            for (auto v : available_vertex) {
                //aux_state[v] = state[v] - inst->adj_matrix[cur_vertex][v];
                tmp[v] = state[v] - inst->adj_matrix[cur_vertex][v];
            }

            state_vec.push_back(tmp);
            /*cout << l << "\tTmp State = ({ ";
            for (int i = 0; i < (int)state.size(); ++i) {
              cout << aux_state[i] << " ";
            }
            cout << "}, " << bddnode->longest_path << ")" << endl;*/
            // add to set


            //cout << "before find" << endl;
            //node = next_map.find(&tmp);
            node = next_map.find(&state_vec.at(state_vec_idx));
            //cout << "after find" << endl;

            if (node == next_map.end()) { // If node does noy exist yet in layer.
                //cout << "\tNew state created" << endl;
                next_map[&state_vec.at(state_vec_idx)] = new BDDNode(state_vec.at(state_vec_idx), longest_path, bddnode->exact);
            } else {
                if (!bddnode->exact && node->second->exact && save_nodes) {
                    add_branch_node(node->second);
                    node->second->exact = false;
                }
                node->second->longest_path = std::max(node->second->longest_path, longest_path);
            }
            state_vec_idx++;
            /*cout << "cur Store{" ;

            for(auto a : next_map) {
              cout << "[";
              for(auto s : *a.first) {
                cout << s << " ";
              }
              cout << "]";
            }
            cout << "}" << endl;*/

            // delete current BDD node
            delete bddnode;
        }

        // switch maps
        current_map_idx = !current_map_idx;
        next_map_idx = !next_map_idx;

    }

    // get longest path from terminal BDD node
    BDDNode* terminal = node_map[current_map_idx].begin()->second;
    longest_path = terminal->longest_path;

    delete terminal;

    return longest_path;

}


//
// Restrict layer size to maximum width
//
void MaxCutBDD::restrict_layer(int layer, vector<BDDNode*> &nodes, bool save_nodes) {
    assert(nodes.size() > max_width);
    assert(max_width > 0);

    // compute node ranking
    for (int i = 0; i < (int)nodes.size(); ++i) {
        nodes[i]->rank = nodes[i]->longest_path;
        for (int j = 0; j < (int)nodes[i]->state.size(); ++j) {
            nodes[i]->rank += std::abs(nodes[i]->state[j]);
        }
    }
    // sort nodes by ranking
    sort(nodes.begin(), nodes.end(), BDDNodeRankSort());

    // truncate layer to satisfy width
    if (!save_nodes) {
        for (int i = max_width; i < (int)nodes.size(); ++i) {
            delete nodes[i];
        }
    } else {
        for (int i = max_width; i < (int)nodes.size(); ++i) {
            add_branch_node(nodes[i]);
        }
    }
    nodes.resize(max_width);
}


int main(int argc, char* argv[]) {


    // --------------------------------------------------
    // Input
    // --------------------------------------------------

    if (argc > 6) {
        cout << "\nUsage: maxcut [instance] [root-width] [width] [ordering]\n" << endl;
        exit(1);
    }
    MaxCutInst* inst = new MaxCutInst(argv[1]);
    int max_width = atoi(argv[2]);
    int ordering = atoi(argv[3]);
    const char*  ordering_file = argv[4];

    // --------------------------------------------------
    // Solving
    // --------------------------------------------------


    if(ordering == 2) {
        inst = reorder_variables(inst);
    }

    // initialize solver
    MaxCutBDD* solver = new MaxCutBDD(0, max_width, inst, ordering, ordering_file);
    //const MaxCutInst* inst = solver->get_instance();
    State initial_state(inst->n_vertices, 0);


    // --------------
    // Root node
    // --------------

    cout << "Root node: " << endl;

    int global_ub = solver->generate_relaxation(initial_state, 0, true);
    cout << "\t[UB] " << global_ub << endl;


    int global_lb = -1;
    global_lb = solver->generate_restriction(initial_state, 0);

    cout << "\t[LB] " << global_lb << endl;

    cout << "\tRoot exact? " << solver->isBDDExact() << endl;

    cout << endl;

    return 0;
}


