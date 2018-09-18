/* MIT License

[Initial work] Copyright (c) 2018 Dai, Hanjun and Khalil, Elias B and Zhang, Yuyu and Dilkina, Bistra and Song, Le
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


#include "config.h"
#include "learning_env.h"
#include "graph.h"
#include "maxcut_bdd.hpp"
#include <cassert>
#include <random>
#include <cstdlib>

int bdd_max_width = 10000; // -1 if exact
char reward_type = 'W';
char bdd_type = 'U';
double r_scaling = 1;
double w_scaling = 0.01;

LearningEnv::LearningEnv() : IEnv() {

}

void LearningEnv::s0(std::shared_ptr<Graph> _g, bool isTrain) {
    graph = _g;
    covered_set.clear();
    action_list.clear();
    state_seq.clear();
    act_seq.clear();
    reward_seq.clear();
    sum_rewards.clear();


    solver = new MaxCutBDD(0, bdd_max_width, graph->adj_list,0,w_scaling);
    inst = solver->get_instance();
    State initial_state(inst->n_vertices, 0);
    solver->initialize(initial_state, 0, true);

    l = 0;
    bound = 0;
    width = 0;

}

double LearningEnv::step(int a) {

    assert(graph);
    assert(covered_set.count(a) == 0);
    state_seq.push_back(action_list);
    act_seq.push_back(a);

    covered_set.insert(a);
    action_list.push_back(a);

    double old_width = width;
    double old_bound = bound;
    double r_t = 0;

    if(bdd_type == 'U')
        bound = solver->generate_next_step_relaxation(a,l);

    else if(bdd_type == 'L')
        bound = solver->generate_next_step_restriction(a,l);

    else {
        std::cerr << "unknown bdd_type type"  <<  bdd_type << std::endl;
        exit(0);
    }

    width = solver->width;

    l++;

    if (reward_type == 'W')
        r_t = getReward(old_width);
    else if (reward_type == 'B' && bdd_type == 'U')
        r_t = getRewardBound(old_bound);
    else if (reward_type == 'B' && bdd_type == 'L')
        r_t = getRewardLowerBound(old_bound);
    else {
        std::cerr << "unknown reward type"  <<  cfg::reward_type << std::endl;
        exit(0);
    }

    reward_seq.push_back(r_t);
    sum_rewards.push_back(r_t);

    return r_t;
}

int LearningEnv::randomAction() {
    assert(graph);
    avail_list.clear();

    for (int i = 0; i < graph->num_nodes; ++i) {
        if (covered_set.count(i) == 0) {
            avail_list.push_back(i);
        }
    }

    assert(avail_list.size());

    int idx = rand() % avail_list.size();

    return avail_list[idx];
}

bool LearningEnv::isTerminal() {

    assert(graph);
    return ((int) action_list.size() == graph->num_nodes);
}


double LearningEnv::getReward(int old_width) {
    return  - r_scaling * (width - old_width); // increase in width is penalized, decrease are rewarded
}

double LearningEnv::getRewardBound(int old_bound) {
    return - r_scaling * (bound - old_bound); // increase in width is penalized, decrease are rewarded
}

double LearningEnv::getRewardLowerBound(int old_bound) {
    return r_scaling * (bound - old_bound); // increase in width is penalized, decrease are rewarded
}
