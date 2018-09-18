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


#ifndef LEARNING_LIB_H
#define LEARNING_LIB_H

extern "C" int Init(const int argc, const char** argv);

extern "C" int InsertGraph(bool isTest, const int g_id, const int num_nodes, const int num_edges, const int* edges_from, const int* edges_to, const double* weights);

extern "C" int LoadModel(const char* filename);

extern "C" int SaveModel(const char* filename);

extern "C" int UpdateSnapshot();

extern "C" int ClearTrainGraphs();

extern "C" int PlayGame(const int n_traj, const double eps);

extern "C" double Fit(const double lr);

extern "C" double FitWithFarthest(const double lr);

extern "C" int ClearMem();

extern "C" double GetSol(const int gid, int* sol);

extern "C" double GetResult(const int gid, int* sol);


#endif