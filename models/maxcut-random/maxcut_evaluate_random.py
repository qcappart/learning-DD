import numpy as np
import networkx as nx
import pickle as cp
import random
import ctypes
import os
import sys
import time
import glob
import re

sys.path.append( '%s/code' % os.path.dirname(os.path.realpath(__file__)) )

from learning_lib import LearningLib

n_test = 100
MAX_VAL = 1000000
MIN_VAL = -10000000
w_scaling = 0.01



# Select the model that has the best reward
def find_model_file(opt):
    log_file = '%s/log-training.txt' % (opt['save_dir'])

    best_r = MIN_VAL
    best_it = -1
    with open(log_file, 'r') as f:
        for line in f:
            if '[DATA]' in line:
                line = line.split(' ')
                it = int(line[1].strip())
                r = float(line[-1].strip())
                if r > best_r:
                    best_r = r
                    best_it = it
                    exec_time = line[2].strip()
    assert best_it >= 0
    print('[INFO] using iter=', best_it, 'with r=', best_r, 'at time', exec_time)
    return '%s/model_iter_%d.model' % (opt['save_dir'], best_it)

def gen_graph(opt,newVal):
    max_n = int(opt['test_max_n'])
    min_n = int(opt['test_min_n'])
    g_type = opt['test_g_type']

    max_w = 10
    min_w = 1
    cur_n = np.random.randint(max_n - min_n + 1) + min_n

    if g_type == 'erdos_renyi':
        m = float(opt['test_density'])
        e_g = nx.erdos_renyi_graph(n = cur_n, p = m, seed=newVal)
        lcc = max(nx.connected_component_subgraphs(e_g), key=len)
        g = nx.convert_node_labels_to_integers(lcc)
    elif g_type == 'barabasi_albert':
        m = int(opt['test_density'])
        g = nx.barabasi_albert_graph(n = cur_n, m = m, seed=newVal)

    else:
        raise Exception("Graph type not defined: " + g_type)

    for edge in nx.edges(g):

        weight = (np.random.randint(max_w - min_w + 1) + min_w) * w_scaling
        g[edge[0]][edge[1]]['weight'] = weight

    return g


if __name__ == '__main__':
    api = LearningLib(sys.argv)


    opt = {}
    for i in range(1, len(sys.argv), 2):
        opt[sys.argv[i][1:]] = sys.argv[i + 1]


    model_file = find_model_file(opt)

    assert model_file is not None

    sys.stdout.flush()
    api.LoadModel(model_file)

    #Initialize seed for test
    seed = int(opt['seed'])
    np.random.seed(seed)


    path = '%s/eval' % opt['save_dir']
    if not os.path.exists(path):
        os.mkdir(path)

    result_file = '%s/test-g_type-%s-density-%s-nodes-%s-%s.csv' % (path, opt['test_g_type'], opt['test_density'], opt['test_min_n'], opt['test_max_n'])

    with open(result_file, 'w') as f_out:

        sys.stdout.flush()
        idx = 0
        f_out.write("seed,graph_type,density,nodes,width,bound,ordering,time\n")
        for idx in range(n_test):
            graph_id = np.random.randint(MAX_VAL)
            g = gen_graph(opt,graph_id)
            api.InsertGraph(g, is_test=True)
            t1 = time.time()
            val, sol = api.GetSol(idx, nx.number_of_nodes(g))
            n_node = sol[0]
            t2 = time.time()
            f_out.write('%s,' % graph_id)
            f_out.write('%s,' % opt['test_g_type'])
            f_out.write('%s,' % opt['test_density'])
            f_out.write('%d,' % n_node) # number of nodes
            f_out.write('%d,' % sol[1]) # width
            f_out.write('%d,' % sol[2]) # bound
            for i in range(n_node):
                f_out.write(' %d' % sol[i + 3])
            f_out.write(',%.6f\n' % (t2 - t1))
            print("[LOG] Graph " + str(graph_id) + " processed")

