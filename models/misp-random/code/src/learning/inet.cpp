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

#include "inet.h"

INet::INet()
{
    inputs.clear();
    param_record.clear();
    learner = new AdamOptimizer<mode, Dtype>(&model, cfg::learning_rate, cfg::l2_penalty);
}

void INet::UseOldModel()
{
    if (param_record.size() == 0)
    {
        for (auto& p : model.params)
        {
            param_record[p.first] = p.second->value.data;
        }
    }
    for (auto& p : model.params)
    {        
        assert(old_model.params.count(p.first));
        auto& old_ptr = old_model.params[p.first];
        p.second->SetRef(&(old_ptr->value));
    }
}

void INet::UseNewModel()
{
    assert(param_record.size());
    for (auto& p : param_record)
    {
        assert(model.params.count(p.first));
        auto& ptr = model.params[p.first];
        ptr->value.data = p.second;
    }
}