/* MIT License

Copyright (c) 2018 Dai, Hanjun and Khalil, Elias B and Zhang, Yuyu and Dilkina, Bistra and Song, Le

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

#ifndef NSTEP_REPLAY_MEM_H
#define NSTEP_REPLAY_MEM_H

#include <vector>
#include <random>
#include "graph.h"

class IEnv;

class ReplaySample
{
public:

    std::vector< std::shared_ptr<Graph> > g_list;
    std::vector< std::vector<int>* > list_st, list_s_primes;
    std::vector<int> list_at;
    std::vector<double> list_rt;
    std::vector<bool> list_term;
};

class NStepReplayMem
{
public:
    static void Init(int memory_size);

    static void Add(std::shared_ptr<Graph> g, 
                    std::vector<int>& s_t,
                    int a_t, 
                    double r_t,
                    std::vector<int>& s_prime,
                    bool terminal);

    static void Add(IEnv* env);

    static void Sampling(int batch_size, ReplaySample& result);

    static void Clear();

    static std::vector< std::shared_ptr<Graph> > graphs;
    static std::vector<int> actions;
    static std::vector<double> rewards;
    static std::vector< std::vector<int> > states, s_primes;
    static std::vector<bool> terminals;

    static int current, count, memory_size;
    static std::default_random_engine generator;
    static std::uniform_int_distribution<int>* distribution;
};

#endif