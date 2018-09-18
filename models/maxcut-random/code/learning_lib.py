import ctypes
import networkx as nx
import numpy as np
import os
import sys

class LearningLib(object):

    def __init__(self, args):
        dir_path = os.path.dirname(os.path.realpath(__file__))
        self.lib = ctypes.CDLL('%s/build/dll/learning_lib.so' % dir_path)

        self.lib.Fit.restype = ctypes.c_double
        self.lib.GetSol.restype = ctypes.c_double
        self.lib.GetResult.restype = ctypes.c_double
        arr = (ctypes.c_char_p * len(args))()
        arr[:] = [x.encode('utf8') for x in args]
        self.lib.Init(len(args), arr)
        self.ngraph_train = 0
        self.ngraph_test = 0

    def __CtypeNetworkX(self, g):
        edges = list(g.edges(data='weight', default=1))
        #print(len(edges))
        #print(len(g.nodes))
        sys.stdout.flush()
        e_list_from = (ctypes.c_int * len(edges))()
        e_list_to = (ctypes.c_int * len(edges))()
        weights = (ctypes.c_double * len(edges))()

        if len(edges):
            a, b, c = zip(*edges)
            e_list_from[:] = a
            e_list_to[:] = b
            weights[:] = c
        return (len(g.nodes()), len(edges), ctypes.cast(e_list_from, ctypes.c_void_p), ctypes.cast(e_list_to, ctypes.c_void_p), ctypes.cast(weights, ctypes.c_void_p))

    def TakeSnapshot(self):
        self.lib.UpdateSnapshot()

    def ClearTrainGraphs(self):
        self.ngraph_train = 0
        self.lib.ClearTrainGraphs()


    def InsertGraph(self, g, is_test):
        n_nodes, n_edges, e_froms, e_tos, weights = self.__CtypeNetworkX(g)

        if is_test:
            t = self.ngraph_test
            self.ngraph_test += 1
        else:
            t = self.ngraph_train
            self.ngraph_train += 1
        self.lib.InsertGraph(is_test, t, n_nodes, n_edges, e_froms, e_tos, weights)


    def LoadModel(self, path_to_model):
        self.lib.LoadModel(ctypes.c_char_p(path_to_model.encode('utf8')))

    def SaveModel(self, path_to_model):
        self.lib.SaveModel(ctypes.c_char_p(path_to_model.encode('utf8')))


    def GetSol(self, gid, maxn):
        sol = (ctypes.c_int * (maxn + 11))()
        val = self.lib.GetSol(gid, sol)
        return val, sol

    def GetResult(self, gid):
        sol = (ctypes.c_int * 100)()
        val = self.lib.GetResult(gid, sol)
        return val, sol

if __name__ == '__main__':
    f = LearningLib(sys.argv)
