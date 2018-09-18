#!/bin/bash


# Seed for the random generation: ensure that the test set remains the same.
sample_name=random
test_seed=100




# Characterics of the training graphs, must be the same as the training
g_type=barabasi_albert # erdos_renyi, barabasi_albert
density=4 # density for ER, attachment parameter for BA
min_n=15 # minimum number of nodes
max_n=20 # maximum number of nodes
seed=42 # Seed used for the training

# Characterics of the tested graphs
test_g_type=barabasi_albert
test_density=4
test_min_n=90
test_max_n=100

# Characterics of the DDs built during the training, must be the same as the training
reward_type=bound
bdd_type=relaxed
bdd_max_width=2


# Parameters used for the learning, must be the same as the training
r_scaling=0.01 # Reward scaling factor
batch_size=64 # max batch size for training/testing
max_bp_iter=4 # max belief propagation iteration
embed_dim=64 # embedding size
net_type=MaxcutQNet # Network type
decay=1 # discounting factor
reg_hidden=32 # number of hidden layers
learning_rate=0.0001 # learning rate
w_scale=0.01 # init weights with rand normal(0, w_scale)
n_step=1 # number of steps in Q-learning
num_env=10 # number of environments
mem_size=50000 # size of the store for experience replay
max_iter=200000 # number of iterations for the training


# Folder of the trained model (inferred from the previous parameters)
result_root=results-local/$g_type-$density/nodes-$min_n-$max_n/seed-$seed/bdd_type-$bdd_type/reward-$reward_type-$bdd_max_width
save_dir=$result_root/ntype-$net_type-embed-$embed_dim-nbp-$max_bp_iter-rh-$reg_hidden-decay-$decay-step-$n_step-batch_size-$batch_size-r_scaling-$r_scaling

# Others
dev_id=0 # gpu card id

python maxcut_evaluate_random.py \
        -net_type $net_type \
        -n_step $n_step \
        -dev_id $dev_id \
        -decay $decay \
        -test_min_n $test_min_n \
        -test_max_n $test_max_n \
        -test_g_type $test_g_type \
        -test_density $test_density \
        -num_env $num_env \
        -max_iter $max_iter \
        -mem_size $mem_size \
        -learning_rate $learning_rate \
        -max_bp_iter $max_bp_iter \
        -net_type $net_type \
        -max_iter $max_iter \
        -save_dir $save_dir \
        -embed_dim $embed_dim \
        -batch_size $batch_size \
        -reg_hidden $reg_hidden \
        -momentum 0.9 \
        -l2 0.00 \
        -seed $test_seed \
        -w_scale $w_scale \
        -test_min_n $test_min_n \
        -test_max_n $test_max_n \
        -r_scaling $r_scaling \
        -reward_type $reward_type \
        -bdd_type $bdd_type \
        -bdd_max_width $bdd_max_width
