/* MIT License

[Initial work] Copyright (c) 2018 David Bergman
[Adaptation] Copyright (c) 2018 Quentin Cappart, Emmanuel Goutierre, David Bergman and Louis-Martin Rousseau

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#ifndef MAXCUTBDD_HPP_
#define MAXCUTBDD_HPP_


#include <boost/unordered_map.hpp>
#include <vector>
#include <limits>
#include <queue>
#include <set>

using namespace std;


// =======================================================================
// Constants & Macros
// =======================================================================

const int INF = std::numeric_limits<int>::max();


// =======================================================================
// MaxCut instance
// =======================================================================

struct MaxCutInst {
    char* name;

    int n_vertices;                           // graph attributes
    int n_edges;
    vector< vector<int> > adj_matrix;
    vector< vector< pair<int,int> > > adj_list;

    int sum_neg_weights;                      // pre-processed data: sum of negative weights

    // Constructor
    MaxCutInst(const char* filename);

    // Constructor
    MaxCutInst(std::vector< std::vector< std::pair<int, double> > > adj, double w_scaling);



    // Empty Constructor
    MaxCutInst() { }

};



// =======================================================================
// State for max cut BDD nodes
// =======================================================================


typedef vector<int> State;

// Equality operator for BDD node state
struct state_equal_to : std::binary_function<State*, State*, bool> {
    bool operator()(State* const& x, State* const& y) const {
        /*cout << "in operator{" ;
        for(auto v: *x) {
            cout << v << " ";
        }
        cout << "}" << endl;
        cout << "in operator{" ;
        for(auto v: *y) {
            cout << v << " ";
        }
        cout << "}" << endl;*/
        return ((*x) == (*y));
    }
};

// Hash function for BDD node state
struct state_ihash : std::unary_function<vector<int>* const&, std::size_t> {
std::size_t operator()(State* const& x) const {
    return boost::hash_value(*x);
}
};




// =======================================================================
// BDD Node
// =======================================================================

struct BDDNode {
    State state;
    int longest_path;
    bool exact;
    int rank;

    // Constructors
    BDDNode(int &lp) : longest_path(lp) { }

    // Constructors
    BDDNode(State& _state, int &lp) : state(_state), longest_path(lp), exact(true) { }
    BDDNode(State& _state, int &lp, bool _exact)
            : state(_state), longest_path(lp), exact(_exact) { }
};



// =======================================================================
// BDD Branch Node
// =======================================================================

//
// Branching Node (for B&B)
//
struct BranchNode {
    State state;
    int longest_path;
    int relax_ub;   // upper bound on this node

    // Constructor
    BranchNode(BDDNode* node) : state(node->state),
                                longest_path(node->longest_path),
                                relax_ub(INF)
    { }

    // Empty Constructor
    // Constructor
    BranchNode() : longest_path(INF), relax_ub(INF) { }
};


//
// Branch node comparator by relaxed UB
//
struct BranchNodeComparatorUB {
    BranchNodeComparatorUB() { }
    bool operator()(const BranchNode* b1, const BranchNode* b2) {
        if (b1->relax_ub == b2->relax_ub) {
            if (b1->state.size() == b2->state.size()) {
                return b1->longest_path < b2->longest_path;
            }
            return b1->state.size() < b2->state.size();
        }
        return b1->relax_ub < b2->relax_ub;
    }
};

//
// Branch node comparator by relaxed UB (descending)
//
struct BranchNodeComparatorUBDescending {
    BranchNodeComparatorUBDescending() { }
    bool operator()(const BranchNode* b1, const BranchNode* b2) {
        if (b1->relax_ub == b2->relax_ub) {
            if (b1->state.size() == b2->state.size()) {
                return b1->longest_path > b2->longest_path;
            }
            return b1->state.size() > b2->state.size();
        }
        return b1->relax_ub > b2->relax_ub;
    }
};


//
// Queue of branch nodes ordered by relax ub
//
typedef priority_queue<BranchNode*, vector<BranchNode*>,
BranchNodeComparatorUB> BranchNodeQueue;



// =======================================================================
// MaxCut BDD Solver Class
// =======================================================================

class MaxCutBDD {

private:
    // Node Map
    typedef boost::unordered_map< State*, BDDNode*, state_ihash, state_equal_to > NodeMap;

    // Node to be processed in a layer
    typedef pair<vector<int>&, int> NodeLayer;

    // Instance info
    MaxCutInst* inst;
    int max_width;

    // BDD control
    NodeMap node_map[2];
    int current_map_idx, next_map_idx;
    vector<int> aux_state;
    vector<BDDNode*> nodes_layer;

    // lower bound control
    double  bestLB;	   // best LB found
    bool    isLBUpdated;     // used to check if LB must be sent to other cores
    bool    isExact;         // check if BDD is exact

    // Add node to branch nodes
    void add_branch_node(BDDNode* node) {
        localBranchNodes.push_back( new BranchNode(node) );
    }


public:
    // Constructor
    MaxCutBDD(const int _placeID,
              const int _ddWidth,
              const string & _instanceName, const int _ordering);

    // Constructor
    MaxCutBDD(const int _placeID, const int _ddWidth, std::vector< std::vector< std::pair<int, double> > > _adj, const int _ordering, double w_scaling);

    MaxCutBDD(const int _placeID, const int _ddWidth, MaxCutInst* _inst, const int _ordering, const char* _orderingFile);


    // Other related parallel methods
    void   emigrateBranchNodes(vector<BranchNode*>& branchNodes);

    // Branch queues
    vector<BranchNode*> localBranchNodes;    // bnodes created locally
    BranchNodeQueue     branchesToExplore;   // bnodes that must be explored locally


    // Generate relaxation
    int generate_relaxation(State &initial_state,
                            int initial_cost,
                            bool save_nodes = false);

    int generate_relaxation(BranchNode* bnode) {
        return generate_relaxation(bnode->state, bnode->longest_path, true);
    }


    // Relax layer size to maximum width
    void relax_layer(int layer, vector<BDDNode*> &nodes, bool save_nodes);

    // Generate restriction
    int generate_restriction(State &initial_state,
                             int initial_cost,
                             bool save_nodes = false);

    int generate_restriction(BranchNode* bnode) {
        return generate_restriction(bnode->state, bnode->longest_path);
    }



    // Restrict layer size to maximum width
    void restrict_layer(int layer, vector<BDDNode*> &nodes, bool save_nodes);

    // Get instance
    const MaxCutInst* get_instance()
    { return inst; }

    // Change max width
    void set_max_width(const int _max_width)
    { max_width = _max_width; }

    // Get best lower bound found
    int getBestLB()
    { return bestLB; }

    // Update local lower bound, indicating whether it was found locally or remotely
    void updateLocalLB(int _lb, bool isLocal = false)
    { if (_lb > bestLB) {  bestLB = _lb; if (isLocal) isLBUpdated = true; } }

    // Verify if relaxed BDD is exact
    bool isBDDExact()
    { return isExact; }


    // Support for learning
    void initialize(State &initial_state, int initial_cost, bool save_nodes);

    int generate_next_step_relaxation(int next_vertex, int l);

    int generate_next_step_restriction(int next_vertex, int l);

    int get_final_bound();

    bool save_nodes;
    bool last_exact_layer;
    int initial_layer;
    int longest_path = 0;
    int width = 0;
    NodeMap::iterator node;
    BDDNode* bddnode = NULL;
    BDDNode* root_node;
    int bound = 0;
    int initial_cost;
    set<int> available_vertex;
    int ordering;
    vector< vector<int> >  state_vec;
    vector <int> tmp;
    const char* orderingFile;


};




#endif
