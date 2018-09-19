# MIT License
#
# [Initial work] Copyright (c) 2018 Dai, Hanjun and Khalil, Elias B and Zhang, Yuyu and Dilkina, Bistra and Song, Le
# [Adaptation] Copyright (c) 2018 Quentin Cappart, Emmanuel Goutierre, David Bergman and Louis-Martin Rousseau
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import numpy as np
import networkx as nx
import pickle as cp
import ctypes
import os
import sys
import time
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt

sys.path.append( '%s/code' % os.path.dirname(os.path.realpath(__file__)) )

from learning_lib import LearningLib

n_valid = 100

MAX_VAL = 1000000
MIN_VAL = -1000000

def gen_graph(opt):
    max_n = int(opt['max_n'])
    min_n = int(opt['min_n'])
    g_type = opt['g_type']

    graph_id = np.random.randint(MAX_VAL)
    cur_n = np.random.randint(max_n - min_n + 1) + min_n

    if g_type == 'erdos_renyi':
        p = float(opt['density'])
        e_g = nx.erdos_renyi_graph(n = cur_n, p = p, seed = graph_id)
        lcc = max(nx.connected_component_subgraphs(e_g), key=len) # We only keep the biggest connected subgraph
        g = nx.convert_node_labels_to_integers(lcc)

    elif g_type == 'barabasi_albert':
        p = int(opt['density'])

        if p == 0:
            max_p = 16
            min_p = 1
            p = np.random.randint(max_p - min_p + 1) + min_p
        g = nx.barabasi_albert_graph(n = cur_n, m = p, seed = seed)

    else:
        raise Exception("Graph type not defined: " + g_type)
    return g


def gen_new_graphs(opt):
    api.ClearTrainGraphs()
    for i in range(1000):
        g = gen_graph(opt)
        api.InsertGraph(g, is_test=False)

def PrepareValidData(opt):
    for i in range(n_valid):
        g = gen_graph(opt)
        api.InsertGraph(g, is_test=True)

if __name__ == '__main__':

    start_time  = time.time()

    api = LearningLib(sys.argv)

    opt = {}
    for i in range(1, len(sys.argv), 2):
        opt[sys.argv[i][1:]] = sys.argv[i + 1]



    seed = int(opt['seed'])
    np.random.seed(seed)

    print("***********************************************************")
    print("[INFO] TRAINING ON RANDOM GRAPHS")
    print("[INFO] Graph type: " + opt['g_type'])
    print("[INFO] Density parameter: " + opt['density'])
    print("[INFO] Number of nodes: [" + opt['min_n'] + " " + opt['max_n'] + "]")
    print("***********************************************************")

    sys.stdout.flush()

    # Build the validation set
    PrepareValidData(opt)

    # Generate the training set
    gen_new_graphs(opt)

    for i in range(10):
        api.lib.PlayGame(100, ctypes.c_double(1.0))
    api.TakeSnapshot()

    eps_start = 1.0
    eps_end = 0.05
    eps_step = 10000.0

    lr = float(opt['learning_rate'])

    print('[INFO]','iter', 'time', 'lr', 'eps', 'avg-width','avg-bound','avg-reward')
    sys.stdout.flush()

    best_reward = (0,0,0,0,0,0,MIN_VAL)


    if int(opt["plot_training"]) == 1:
        fig = plt.figure()
        iter_list = []
        reward_list = []

    for iter in range(int(opt['max_iter'])):
        eps = eps_end + max(0., (eps_start - eps_end) * (eps_step - iter) / eps_step)
        if iter % 10 == 0:
            api.lib.PlayGame(10, ctypes.c_double(eps))

        if iter % 100 == 0:
            sys.stdout.flush()
            width, bound, reward = 0.0, 0.0, 0.0
            for idx in range(n_valid):
                val, sol = api.GetResult(idx)
                width += sol[0]
                bound += sol[1]
                reward += val

            width, bound, reward = (width/n_valid, bound/n_valid, reward/n_valid)
            cur_time = round(time.time() - start_time,2)
            it_data = (iter, cur_time, lr, eps, width, bound, reward)

            print("[DATA]", " ".join(map(str,it_data)))

            if reward > best_reward[-1]:
                best_reward = it_data

            sys.stdout.flush()
            model_path = '%s/model_iter_%d.model' % (opt['save_dir'], iter)
            api.SaveModel(model_path)



            if int(opt["plot_training"]) == 1:
                iter_list.append(iter)
                reward_list.append(reward)
                plt.clf()
                plt.plot(iter_list, reward_list)
                out_file = '%s/log_training_curve_reward.png' % opt['save_dir']
                plt.savefig(out_file, dpi = 300)

        if iter % 1000 == 0:
            api.TakeSnapshot()
            lr = lr * 0.95


        if iter and iter % 5000 == 0:
            print("[LOG] Refreshing Training set")
            gen_new_graphs(opt)

        api.lib.Fit(ctypes.c_double(lr))


    print("[BEST-REWARD]", " ".join(map(str,best_reward)))


