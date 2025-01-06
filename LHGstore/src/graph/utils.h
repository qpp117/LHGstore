#pragma once
#include "graph.h"
#include <random>

void shuffle(std::vector<edge>& arr) {
    std::random_device rd;
    for (int i = arr.size() - 1; i >= 0; i--) {
        std::uniform_int_distribution<int> dist(0, i);
        int pos = dist(rd);
        edge t = arr[i];
        arr[i] = arr[pos];
        arr[pos] = t;
    }
}

void print_graph(std::vector<edge> all_edge, std::string file = "graph.txt") {
    std::ofstream f(file.c_str());
    for (edge e : all_edge) {
        f << e.src << " " << e.dst << std::endl;
    }
    f.close();
}

void sec_index_leaf_stats(alex::AlexDataNode<vertex, vertex>* leaf) {
    num_shifts += leaf->num_shifts_;
    num_exp_search_iterations += leaf->num_exp_search_iterations_;
}

// 二级索引 stats
void sec_index_stats(alex::Alex<vertex, vertex>* sec_index) {
    alex::Alex<vertex, vertex>::NodeIterator* nit = new alex::Alex<vertex, vertex>::NodeIterator(sec_index);
    if (nit->cur_node_->is_leaf_) {
        sec_index_leaf_stats((alex::AlexDataNode<vertex, vertex>*)nit->cur_node_);
        sec_index_num_leaf++;
    }
    while (true) {
        alex::AlexNode<vertex, vertex>* cur_node = nit->next();
        if (cur_node == nullptr) {
            break;
        }
        if (nit->cur_node_->is_leaf_) {
            sec_index_leaf_stats((alex::AlexDataNode<vertex, vertex>*)cur_node);
            sec_index_num_leaf++;
        }
    }
    num_expand_and_scales += sec_index->stats_.num_expand_and_scales;
    num_expand_and_retrains += sec_index->stats_.num_expand_and_retrains;
    num_downward_splits += sec_index->stats_.num_downward_splits;
    num_sideways_splits += sec_index->stats_.num_sideways_splits;
    num_model_node_expansions += sec_index->stats_.num_model_node_expansions;
    splitting_time += sec_index->stats_.splitting_time;
    cost_computation_time += sec_index->stats_.cost_computation_time;
}

// 一级索引的叶子节点的 stats
void count_stats(alex::AlexDataNode<vertex, payload>* leaf) {
    num_shifts += leaf->num_shifts_;
    num_exp_search_iterations += leaf->num_exp_search_iterations_;
    for (int i = 0; i < leaf->data_capacity_; i++) {
        if (leaf->check_exists(i) && leaf->get_payload(i).sec_index != nullptr) {
            sec_index_stats(leaf->get_payload(i).sec_index);
        }
    }
}

void print_stats(alex::Alex<vertex, payload>* index) {
    alex::Alex<vertex, payload>::NodeIterator* nit = new alex::Alex<vertex, payload>::NodeIterator(index);
    if (nit->cur_node_->is_leaf_) {
        count_stats((alex::AlexDataNode<vertex, payload>*)nit->cur_node_);
        num_leaf++;
    }
    while (true) {
        alex::AlexNode<vertex, payload>* cur_node = nit->next();
        if (cur_node == nullptr) {
            break;
        }
        if (nit->cur_node_->is_leaf_) {
            count_stats((alex::AlexDataNode<vertex, payload>*)cur_node);
            num_leaf++;
        }
    }
    num_expand_and_scales += index->stats_.num_expand_and_scales;
    num_expand_and_retrains += index->stats_.num_expand_and_retrains;
    num_downward_splits += index->stats_.num_downward_splits;
    num_sideways_splits += index->stats_.num_sideways_splits;
    num_model_node_expansions += index->stats_.num_model_node_expansions;
    splitting_time += index->stats_.splitting_time;
    cost_computation_time += index->stats_.cost_computation_time;

    std::cout << "num_expand_and_scales = " << num_expand_and_scales
        << ", num_expand_and_retrains = " << num_expand_and_retrains
        << ", num_downward_splits = " << num_downward_splits
        << ", num_sideways_splits = " << num_sideways_splits << std::endl;
    std::cout << "splitting_time = " << splitting_time / 1e9
        << ", cost_computation_time = " << cost_computation_time / 1e9
        << ", num_model_node_expansions = " << num_model_node_expansions << std::endl;
    std::cout << "num_shifts = " << num_shifts
        << ", num_exp_search_iterations = " << num_exp_search_iterations << std::endl;
    std::cout << "num_leaf_nodes = " << num_leaf << ", num_sec_lead_nodes = " << sec_index_num_leaf << std::endl;
}

template <typename T>
void print_stats(alex::Alex<T, T>* index) {
    std::cout << "num data nodes = " << index->stats_.num_data_nodes
        << ", num model nodes = " << index->stats_.num_model_nodes
        << ", num downward splits = " << index->stats_.num_downward_splits
        << ", num sideway splits = " << index->stats_.num_sideways_splits
        << ", root.a = " << index->root_node_->model_.a_
        << ", root.b = " << index->root_node_->model_.b_ << std::endl;
}



void test(std::vector<edge>& all_edge, int total_num_edges) {
    std::ofstream* f = new std::ofstream("ind.txt");
    alex::Alex<vertex, vertex>* ind = new alex::Alex<vertex, vertex>();
    int cnt = 0;
    bool first = true;
    bool second = false;
    for (int i = 0; i < total_num_edges; i++) {
        if (all_edge[i].src == 29425) {
            if (all_edge[i].dst == 229984) {
                second = true;
            }
            if (first) {
                ind->bulk_load(new std::pair<vertex, vertex>(all_edge[i].dst, all_edge[i].dst), 1);
                first = false;
            }
            else {
                std::cout << all_edge[i].src << " " << all_edge[i].dst << std::endl;
                ind->insert(std::pair<vertex, vertex>(all_edge[i].dst, all_edge[i].dst));
            }
            vertex* t = ind->get_payload(all_edge[i].dst);
            if (t == nullptr) {
                std::cout << "src: " << all_edge[i].src << ", dst: " << all_edge[i].dst << " not found" << std::endl;
                exit(-1);
            }
            cnt++;
        }
    }
    std::cout << "finish: " << cnt << std::endl;
    for (int i = 0; i < total_num_edges; i++) {
        if (all_edge[i].src == 29425) {
            vertex* t = ind->get_payload(all_edge[i].dst);
            if (t == nullptr) {
                std::cout << "src: " << all_edge[i].src << ", dst: " << all_edge[i].dst << " not found" << std::endl;
                exit(-1);
            }
        }
    }
    for (int i = 0; i < total_num_edges; i++) {
        if (all_edge[i].src == 29425) {
            std::cout << all_edge[i].src << " " << all_edge[i].dst << " delete" << std::endl;
            int t = ind->erase_one(all_edge[i].dst);
            if (t == 0) {
                std::cout << all_edge[i].src << " " << all_edge[i].dst << " not delete" << std::endl;
            }
        }
    }
    (*f).close();
    delete f;
    delete ind;
}