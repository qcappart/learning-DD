
# learning-DD

Finding tight bounds on the optimal solution is a critical element  of  practical  solution  methods  for  discrete  optimization problems.  In the last decade, decision diagrams (DDs) have brought a new perspective on obtaining upper and lower bounds that can be significantly better than classical bounding mechanisms, such as linear relaxations. It is well known that the quality of the bound achieved through this flexible bounding method is highly reliant on the ordering of variables chosen for building the diagram.

This work propose an innovative and generic approach based on deep reinforcement learning for obtaining an ordering for tightening the bounds obtained with relaxed and restricted DDs. The approach is described in (https://arxiv.org/abs/1809.03359). 

For the moment, this repository contains the implementation for the [Maximum Independent Set Problem](https://en.wikipedia.org/wiki/Maximal_independent_set) and the [Maximum Cut Problem](https://en.wikipedia.org/wiki/Maximum_cut).

## Overview of the repository

```bash
.
├── graphnn/ # graphnn library
└── models/ # Implementation of the problems
	├── maxcut-random/
	├── ...
        └── misp-random/ 
              ├── results-local/ # Folder where the trained models and results are saved
              ├── misp_evaluate_random.py # Testing script
              ├── misp_training_random.py # Training script
              ├── run_misp_eval_random.sh # Executable file for running the evaluation.
              ├── run_misp_training_random.sh # Executable file for running the training.
              └── code/ # Implementation
                  ├── Makefile
                  ├── learning_lib.py # Interface between python and C++ implementation
                  ├── include/ # Header files
                  └── src/
                      ├── learning_lib.cpp # Initialization of the RL model and algorithms
                      ├── dd/ # Construction of the decision diagram.
                      └── learning/
                          ├── learning_env.cpp # Reinforcement learning environment
                          ├── misp_qnet.cpp # Implementation of the neural network
                          └── ...
```

## Installation Instructions

The next instructions describe how to build the library for the Maximum Indepenset Problem. The procedure is the same for the other problems.

### 1. Importing the repository

```shell
git clone https://github.com/qcappart/learning-DD.git
```

### 2. Building graphnn library

This library has been developped by Dai et al. and we made no modification on it. 
Please see https://github.com/Hanjun-Dai/graphnn for the instructions about how to build it.

### 3. Building learning-DD

1. Assuming you are located at the root of the repository, go to the project library:

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

1. We use conda for managing the virtual environments. If it is not yet done, install the latest version of conda (https://conda.io/docs/index.html).

2. Create a python virtual environment:

```shell
conda create -n learning-DD-env python=3.6
```

3. Install the required packages:

```shell
conda install --name learning-DD-env numpy networkx matplotlib
```

4. Activate the virtual environment:

```shell
conda activate learning-DD-env
```

5. Once done, you can deactivate the virtual environment:

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

2. It creates a new file recaping the performances and the ordering obtained for each tested instance.

### 3. Modifying the parameters

You can also modify the previous scripts in order to train/test models with other parameters. You just have to change the values in the previous script. Be sure that the parameters in the test files are consistent with the one in the training file in order to evaluate the right model.

### 4. Comparing with other methods

Python scripts that we used in order to perform the comparison in the paper are not included in the repository. If you are interested in, we can add them.

## Current implemented problems

This list recaps the problems that are currently handled by our method.

-  ![Alt text](http://progressed.io/bar/100)  Maximum Independent Set Problem (MISP).
-  ![Alt text](http://progressed.io/bar/75)  Maximum Cut Problem (Maxcut) - Performances can be improved.
-  ![Alt text](http://progressed.io/bar/50)  Knapsack - In progress.

Basically, adding a new problem requires only to implement the related DD construction and build the RL environment.

## Future work

To the best of our knowledge, it is the first work using machine learning for the purpose of tightening optimization bounds. 
It opens new insights of research and many possibilities of future works:

- [ ] Adapt to other problems.
- [ ] Apply it to real graphs.
- [ ] Use it to other fields using DDs, such as constraint programming or verification of systems.
- [ ] Test with other RL algorithms or using other function approximators.
- [ ] ...

If you want to contribute to this project or if you have any questions, we would be happy to help you.

## Cite

Please use this reference:

```latex
@article{cappart2018improving,
  title={Improving Optimization Bounds using Machine Learning: Decision Diagrams meet Deep Reinforcement Learning},
  author={Cappart, Quentin and Goutierre, Emmanuel and Bergman, David and Rousseau, Louis-Martin},
  journal={arXiv preprint arXiv:1809.03359},
  year={2018}
}
```

## References and inspirations

This work has been inspired by the following papers:

- Khalil, Elias, et al. "Learning combinatorial optimization algorithms over graphs." Advances in Neural Information Processing Systems. 2017.
- Dai, Hanjun, Bo Dai, and Le Song. "Discriminative embeddings of latent variable models for structured data." International Conference on Machine Learning. 2016.
- Bergman, David, et al. "Optimization bounds from binary decision diagrams." INFORMS Journal on Computing 26.2 (2013): 253-268.
- Bergman, David, et al. Decision diagrams for optimization. Springer International Publishing, 2016.

Both ideas and implementations of these references have been used and adapted for our work.

## Licence

This work is under MIT licence (https://choosealicense.com/licenses/mit/). It is a short and simple very permissive license with conditions only requiring preservation of copyright and license notices. Licensed works, modifications, and larger works may be distributed under different terms and without source code. 
