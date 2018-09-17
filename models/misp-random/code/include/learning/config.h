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

#ifndef cfg_H
#define cfg_H

#include <iostream>
#include <cstring>
#include <fstream>
#include <set>
#include <map>
#include "util/gnn_macros.h"

typedef float Dtype;

#ifdef GPU_MODE
    typedef gnn::GPU mode;
#else
    typedef gnn::CPU mode;
#endif

struct cfg
{
    static int max_bp_iter;
    static int embed_dim;
    static int batch_size;
    static int max_iter;
    static int dev_id;
    static int max_n, min_n;
    static int n_step;
    static int num_env;
    static int mem_size;
    static int reg_hidden;
    static int node_dim;
    static int avg_global;
    static int edge_dim;
    static int edge_embed_dim;
    static int aux_dim;
    static int bdd_max_width;
    static Dtype decay;
    static Dtype learning_rate;
    static Dtype l2_penalty;
    static Dtype momentum;    
    static Dtype w_scale;
    static Dtype r_scaling;
    static const char *save_dir, *net_type, *reward_type, *bdd_type;

    static void LoadParams(const int argc, const char** argv)
    {
        for (int i = 1; i < argc; i += 2)
        {
		    if (strcmp(argv[i], "-learning_rate") == 0)
		        learning_rate = atof(argv[i + 1]);
            if (strcmp(argv[i], "-max_bp_iter") == 0)
                max_bp_iter = atoi(argv[i + 1]);        
            if (strcmp(argv[i], "-dev_id") == 0)
                dev_id = atoi(argv[i + 1]);
		    if (strcmp(argv[i], "-embed_dim") == 0)
			    embed_dim = atoi(argv[i + 1]);
            if (strcmp(argv[i], "-edge_embed_dim") == 0)
			    edge_embed_dim = atoi(argv[i + 1]);
		    if (strcmp(argv[i], "-reg_hidden") == 0)
			    reg_hidden = atoi(argv[i + 1]);
            if (strcmp(argv[i], "-max_n") == 0)
			    max_n = atoi(argv[i + 1]);
            if (strcmp(argv[i], "-min_n") == 0)
			    min_n = atoi(argv[i + 1]);
            if (strcmp(argv[i], "-mem_size") == 0)
			    mem_size = atoi(argv[i + 1]);
            if (strcmp(argv[i], "-num_env") == 0)
			    num_env = atoi(argv[i + 1]);                
            if (strcmp(argv[i], "-n_step") == 0)
			    n_step = atoi(argv[i + 1]);                
    		if (strcmp(argv[i], "-batch_size") == 0)
	       		batch_size = atoi(argv[i + 1]);
            if (strcmp(argv[i], "-max_iter") == 0)
	       		max_iter = atoi(argv[i + 1]);                   
    		if (strcmp(argv[i], "-l2") == 0)
    			l2_penalty = atof(argv[i + 1]);      
            if (strcmp(argv[i], "-decay") == 0)
    			decay = atof(argv[i + 1]);      
            if (strcmp(argv[i], "-w_scale") == 0)
                w_scale = atof(argv[i + 1]);
    		if (strcmp(argv[i], "-momentum") == 0)
    			momentum = atof(argv[i + 1]);
            if (strcmp(argv[i], "-bdd_max_width") == 0)
                bdd_max_width = atoi(argv[i + 1]);
            if (strcmp(argv[i], "-avg_global") == 0)
                avg_global = atoi(argv[i + 1]);
    		if (strcmp(argv[i], "-save_dir") == 0)
    			save_dir = argv[i + 1];
            if (strcmp(argv[i], "-net_type") == 0)
    			net_type = argv[i + 1];
            if (strcmp(argv[i], "-reward_type") == 0)
                reward_type = argv[i + 1];
            if (strcmp(argv[i], "-bdd_type") == 0)
                bdd_type = argv[i + 1];
            if (strcmp(argv[i], "-r_scaling") == 0)
                r_scaling = atof(argv[i + 1]);
        }

        if (n_step <= 0)
            n_step = max_n;
        if (edge_embed_dim < 0)
            edge_embed_dim = embed_dim;
        std::cerr << "[INFO] TRAINING PARAMETERS" << std::endl;
        std::cerr << "[INFO] decay = " << decay << std::endl;
        std::cerr << "[INFO] edge_embed_dim = " << edge_embed_dim << std::endl;
        std::cerr << "[INFO] net_type = " << net_type << std::endl;
        std::cerr << "[INFO] mem_size = " << mem_size << std::endl;
        std::cerr << "[INFO] num_env = " << num_env << std::endl;
        std::cerr << "[INFO] n_step = " << n_step << std::endl;
        std::cerr << "[INFO] min_n = " << min_n << std::endl;
        std::cerr << "[INFO] max_n = " << max_n << std::endl;
        std::cerr << "[INFO] max_iter = " << max_iter << std::endl;
        std::cerr << "[INFO] dev_id = " << dev_id << std::endl;
        std::cerr << "[INFO] max_bp_iter = " << max_bp_iter << std::endl;
        std::cerr << "[INFO] batch_size = " << batch_size << std::endl;
        std::cerr << "[INFO] embed_dim = " << embed_dim << std::endl;
    	std::cerr << "[INFO] learning_rate = " << learning_rate << std::endl;
        std::cerr << "[INFO] w_scale = " << w_scale << std::endl;
    	std::cerr << "[INFO] l2_penalty = " << l2_penalty << std::endl;
    	std::cerr << "[INFO] momentum = " << momentum << std::endl;
        std::cerr << "[INFO] avg_global = " << avg_global << std::endl;
        std::cerr << "[INFO] reward_type = " << reward_type << std::endl;
        std::cerr << "[INFO] bdd_type = " << bdd_type << std::endl;
        std::cerr << "[INFO] bdd_max_width = " << bdd_max_width << std::endl;
        std::cerr << "[INFO] r_scaling = " << r_scaling << std::endl;
    }
};

#endif
