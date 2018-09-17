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

int cfg::max_bp_iter = 1;
int cfg::embed_dim = 64;
int cfg::dev_id = 0;
int cfg::batch_size = 32;
int cfg::max_iter = 1;
int cfg::reg_hidden = 32;
int cfg::node_dim = 0;
int cfg::aux_dim = 0;
int cfg::min_n = 0;
int cfg::max_n = 0;
int cfg::mem_size = 0;
int cfg::num_env = 0;
int cfg::n_step = -1;
int cfg::edge_dim = 4;
int cfg::edge_embed_dim = -1;
int cfg::avg_global = 0;
int cfg::bdd_max_width = 10000;
Dtype cfg::r_scaling = 1.0;
Dtype cfg::learning_rate = 0.0005;
Dtype cfg::decay = 1.0;
Dtype cfg::l2_penalty = 0;
Dtype cfg::momentum = 0;
Dtype cfg::w_scale = 0.01;
const char* cfg::save_dir = "./saved";
const char* cfg::net_type = "QNet";
const char* cfg::reward_type = "width";
const char* cfg::bdd_type = "relaxed";