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

#ifndef I_ENV_H
#define I_ENV_H

#include <vector>
#include <set>

#include "graph.h"


extern char reward_type;
extern char bdd_type;
extern int bdd_max_width;
extern double r_scaling;

class IEnv
{
public:

    IEnv() : graph(nullptr) {}

    virtual void s0(std::shared_ptr<Graph> _g, bool isTrain = true) = 0;

    virtual double step(int a) = 0;

    virtual int randomAction() = 0;

    virtual bool isTerminal() = 0;

    std::shared_ptr<Graph> graph;
    
    std::vector< std::vector<int> > state_seq;
    std::vector<int> act_seq, action_list;
    std::vector<double> reward_seq, sum_rewards;
};

#endif