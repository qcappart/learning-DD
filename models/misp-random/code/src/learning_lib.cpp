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
#include "learning_lib.h"
#include "graph.h"
#include "nn_api.h"
#include "misp_qnet.h"
#include "nstep_replay_mem.h"
#include "simulator.h"
#include "learning_env.h"
#include <random>
#include <algorithm>
#include <cstdlib>
#include <signal.h>

using namespace gnn;
#define inf 2147483647/2

void intHandler(int dummy) {
    exit(0);
}

int LoadModel(const char* filename) {
    ASSERT(net, "please init the lib before use");    
    net->model.Load(filename);
    return 0;
}

int SaveModel(const char* filename) {
    ASSERT(net, "please init the lib before use");
    net->model.Save(filename);
    return 0;
}

std::vector< std::vector<double>* > list_pred;
LearningEnv* test_env;
int Init(const int argc, const char** argv) {
    signal(SIGINT, intHandler);
    
    cfg::LoadParams(argc, argv);
    GpuHandle::Init(cfg::dev_id, 1);

    if (!strcmp(cfg::reward_type, "width"))
        reward_type = 'W';

    else if (!strcmp(cfg::reward_type, "bound"))
        reward_type = 'B';

    else if (!strcmp(cfg::reward_type, "merge"))
        reward_type = 'M';
    else {
        std::cerr << "unknown reward type"  <<  cfg::reward_type << std::endl;
        exit(0);
    }

    if (!strcmp(cfg::bdd_type, "relaxed"))
        bdd_type = 'U';

    else if (!strcmp(cfg::bdd_type, "restricted"))
        bdd_type = 'L';
    else {
        std::cerr << "unknown bdd type"  <<  cfg::bdd_type << std::endl;
        exit(0);
    }


    bdd_max_width = cfg::bdd_max_width;
    r_scaling = cfg::r_scaling;

    if (!strcmp(cfg::net_type, "MISPQNet"))
        net = new MISPQNet();
    else {
        std::cerr << "unknown net type: " <<  cfg::net_type << std::endl;
        exit(0);
    }
    net->BuildNet();

    NStepReplayMem::Init(cfg::mem_size);
    
    Simulator::Init(cfg::num_env);
    for (int i = 0; i < cfg::num_env; ++i)
        Simulator::env_list[i] = new LearningEnv();
    test_env = new LearningEnv();

    list_pred.resize(cfg::batch_size);
    for (int i = 0; i < cfg::batch_size; ++i)
        list_pred[i] = new std::vector<double>(2010);//(cfg::max_n + 10);
    return 0;
}

int UpdateSnapshot() {
    net->old_model.DeepCopyFrom(net->model);
    return 0;
}

int InsertGraph(bool isTest, const int g_id, const int num_nodes, const int num_edges, const int* edges_from, const int* edges_to, const double* weights) {
    auto g = std::make_shared<Graph>(num_nodes, num_edges, edges_from, edges_to, weights);
    if (isTest)
        GSetTest.InsertGraph(g_id, g);
    else
        GSetTrain.InsertGraph(g_id, g);
    return 0;
}

int ClearTrainGraphs() {
    GSetTrain.graph_pool.clear();
    return 0;
}

int PlayGame(const int n_traj, const double eps) {
    Simulator::run_simulator(n_traj, eps);
    return 0;
}

ReplaySample sample;
std::vector<double> list_target;
double Fit(const double lr) {
    NStepReplayMem::Sampling(cfg::batch_size, sample);
    bool ness = false;
    for (int i = 0; i < cfg::batch_size; ++i)
        if (!sample.list_term[i]) {
            ness = true;
            break;
        }
    if (ness)
        PredictWithSnapshot(sample.g_list, sample.list_s_primes, list_pred);
    
    list_target.resize(cfg::batch_size);
    for (int i = 0; i < cfg::batch_size; ++i) {
        double q_rhs = 0;
        if (!sample.list_term[i])
            q_rhs = cfg::decay * max(sample.g_list[i]->num_nodes, list_pred[i]->data());
        q_rhs += sample.list_rt[i];
        list_target[i] = q_rhs;
    }

    return Fit(lr, sample.g_list, sample.list_st, sample.list_at, list_target);
}

double GetResult(const int gid, int* sol) {
    std::vector< std::shared_ptr<Graph> > g_list(1);
    std::vector< std::vector<int>* > states(1);

    test_env->s0(GSetTest.Get(gid),false);
    states[0] = &(test_env->action_list);
    g_list[0] = test_env->graph;

    double v = 0;
    int new_action;
    while (!test_env->isTerminal())
    {
        Predict(g_list, states, list_pred);
        auto& scores = *(list_pred[0]);
        new_action = arg_max(test_env->graph->num_nodes, scores.data());
        v += test_env->step(new_action) / r_scaling;
    }

    sol[0] = test_env->width;
    sol[1] = test_env->bound;
    return v;
}

double GetSol(const int gid, int* sol) {
    std::vector< std::shared_ptr<Graph> > g_list(1);
    std::vector< std::vector<int>* > states(1);

    test_env->s0(GSetTest.Get(gid),false);
    states[0] = &(test_env->action_list);
    g_list[0] = test_env->graph;

    double v = 0;
    int new_action;
    while (!test_env->isTerminal())
    {
        Predict(g_list, states, list_pred);
        auto& scores = *(list_pred[0]);
        new_action = arg_max(test_env->graph->num_nodes, scores.data());
        v += test_env->step(new_action) / r_scaling;
    }
    
    sol[0] = test_env->graph->num_nodes;
    sol[1] = test_env->width;
    sol[2] = test_env->bound;

    for (int i = 0; i < test_env->graph->num_nodes; ++i) {
        sol[i + 3] = test_env->action_list[i];
    }

    return -v;
}

int ClearMem() {
    NStepReplayMem::Clear();
    return 0;
}