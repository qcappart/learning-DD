/* MIT License

[Initial work] Copyright (c) 2018 Dai, Hanjun and Khalil, Elias B and Zhang, Yuyu and Dilkina, Bistra and Song, Le

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

#ifndef INET_H
#define INET_H

#include <map>
#include <string>
#include <vector>
#include "config.h"
#include "tensor/tensor.h"
#include "nn/nn_all.h"
#include "util/graph_struct.h"

#include "graph.h"

using namespace gnn;

class INet
{
public:
    INet();

    virtual void BuildNet() = 0;

    virtual void SetupTrain(std::vector<int>& idxes, 
                            std::vector< std::shared_ptr<Graph> >& g_list, 
                            std::vector< std::vector<int>* >& covered, 
                            std::vector<int>& actions, 
                            std::vector<double>& target) = 0;
                            
    virtual void SetupPredAll(std::vector<int>& idxes, 
                              std::vector< std::shared_ptr<Graph> >& g_list, 
                              std::vector< std::vector<int>* >& covered) = 0;

    void UseOldModel();
    void UseNewModel();
    
    DTensor<CPU, Dtype> node_feat, edge_feat, y;
    DTensor<mode, Dtype> m_node_feat, m_edge_feat, m_y;
    GraphStruct graph;
    FactorGraph fg;
    ParamSet<mode, Dtype> model, old_model;
    AdamOptimizer<mode, Dtype>* learner;

    std::map< std::string, void* > inputs;
    std::map<std::string, std::shared_ptr< DenseData<mode, Dtype> > > param_record;
    std::shared_ptr< DTensorVar<mode, Dtype> > loss, q_pred, q_on_all;
};

#endif