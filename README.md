
# learning-DD

This repository is currently construction. It will be released in the next days.

## Overview of the repository

## Installation Instructions

The next instructions describe how to build the library for the Maximum Indepenset Problem. The procedure is the same for the other problems.

### 1. Importing the repository

```shell
git clone https://github.com/qcappart/learning-DD.git
```

### 2. Building graphnn library

This library has been developped by Dai et al. [X]. 
Please see https://github.com/Hanjun-Dai/graphnn for the instructions about how to build it.

### 3. Building learning-DD

1. Assuming you are located at the root of the repository, go to the project library

```shell
cd models/misp-random/code
```

2. If you want to run the learning using GPU, add this line in the makefile:

```shell
CXXFLAGS += -DGPU_MODE
```

3. Build the dynamic library:

```shell
make
```

### 4. Setting up a virtual environment

1. Create a python virtual environment

```shell
conda create -n learning-DD-env python=3.6
```

2. Install the required packages

```shell
conda install --name learning-DD-env numpy networkx matplotlib
```

3. Activate the virtual environment

```shell
conda activate learning-DD-env
```

4. Once done, you can deactivate the virtual environment

```shell
conda deactivate 
```

## Basic use

### 1. Training a model

1.Run these command lines:

```shell
chmod +x run_misp_training_random.sh
./run_misp_training_random.sh
```

2. The models built during the training are saved in the results-local folder. The complete path depends on the paramaters that are considered. The log file and the training curve are also saved.


### 2. Testing a model

1. Run these command lines:

```shell
chmod +x run_misp_eval_random.sh 
./run_misp_eval_random.sh 
```

2. It creates a new file recaping the performances obtained for each test graph.

### 3. Modifying the parameters

### 4. Comparing with other methods

Python scripts that we used in order to perform the comparison in the paper are not included in the repository. If you are interested in, we can add them.

## Current implemented problems

This list recaps the problems that are currently handled by our method.

- [x] Maximum Independent Set Problem (MISP)
- [x] Maximum Cut Problem (Maxcut)
- [ ] Knapsack - In progress

Basically, adding a new problem requires only to implement the linked DD construction and build the RL environment.

## Future work

To the best of our knowledge, it is the first work using machine learning for the purpose of tightening optimization bounds. 
It opens new insights of research and many possibilities of future work. If you would like to contribute, here are some propositions :

- [ ] idea 1
- [ ] idea 2
- [ ] idea 3
- [ ] idea 4

## Cite

## Reference

## Licence
