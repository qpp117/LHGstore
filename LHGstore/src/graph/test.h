#pragma once
#include <vector>
#include <unordered_map>
#include <chrono>
#include "graph.h"
#include <random>
#include <forward_list>

void test_adjacency(std::vector<edge>& all_edge, int total_num_edges) {
	vertex logicId = 0;
	std::unordered_map<vertex, vertex> map;
	std::vector<std::vector<vertex>> adjList;
	double time = 0.0;
	for (int i = 0; i < total_num_edges; i++) {
		vertex a = all_edge[i].src;
		vertex b = all_edge[i].dst;
		vertex logic_a = 0, logic_b = 0;
		auto insert_start_time = std::chrono::high_resolution_clock::now();
		if (map.find(a) == map.end()) {
			map[a] = logicId++;
			adjList.push_back(std::vector<vertex>{});
		}
		if (map.find(b) == map.end()) {
			map[b] = logicId++;
			adjList.push_back(std::vector<vertex>{});
		}
		adjList[logic_a].push_back(logic_b);
		auto insert_end_time = std::chrono::high_resolution_clock::now();
		time += std::chrono::duration_cast<std::chrono::nanoseconds>(insert_end_time - insert_start_time).count();
	}
	std::cout << "insert throughput = " << total_num_edges / time * 1e9 << std::endl;

	time = 0.0;
	for (int i = 0; i < total_num_edges; i++) {
		auto insert_start_time = std::chrono::high_resolution_clock::now();
		vertex a = map[all_edge[i].src];
		vertex b = map[all_edge[i].dst];
		for (auto i = adjList[a].begin(); i != adjList[a].end(); i++) {
			if (*i == b) {
				break;
			}
		}
		auto insert_end_time = std::chrono::high_resolution_clock::now();
		time += std::chrono::duration_cast<std::chrono::nanoseconds>(insert_end_time - insert_start_time).count();
	}
	std::cout << "lookup throughput = " << total_num_edges / time * 1e9 << std::endl;
	unsigned long long size = 0;
	for (int i = 0; i < adjList.size(); i++) {
		int cap = adjList[i].capacity();
		size += cap * sizeof(vertex);
	}
	std::cout << "size = " << size/1024/1024 << std::endl;
}

void generate_graph(int degree, std::string file_path = "graph.txt") {
	std::ofstream f(file_path.c_str());
	std::random_device seed;//硬件生成随机数种子
	std::mt19937_64 engine(seed());//利用种子生成随机数引擎
    std::poisson_distribution<> rand;//设置随机数范围，并为均匀分布
    for (int i = 0; i < degree; i++) {
		int a = 0;
		int b = rand(engine);
		if (b < 0) {
			b = -b;
		}
		f << a << " " << b << std::endl;
	}
	f.close();
}

void MicroBenchmark_Lookup(int num) {
	alex::Alex<int, int> *index = new alex::Alex<int, int>();
	int* arr = new int[num];
	std::random_device seed;
	std::mt19937_64 engine(seed());
    std::uniform_int_distribution<unsigned> rand(0, 1e6);
	for (int i = 0; i < num; i++) {
		int t = rand(engine);
		arr[i] = t;
		index->insert(t, t);
	}
	double index_time = 0.0, arr_time = 0.0;
	for (int i = 0; i < 1000; i++) {
		int t = rand(engine);
		// 在 alex 中查找
		auto start = std::chrono::high_resolution_clock::now();
		int* ans = index->get_payload(t);
		auto end = std::chrono::high_resolution_clock::now();
		if (ans != nullptr) {
			std::cout << *ans << std::endl;
		}
		index_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		int j;
		// 在数组中查找
		start = std::chrono::high_resolution_clock::now();
		for (j = 0; j < num; j++) {
			if (arr[j] == t) {
				break;
			}
		}
		end = std::chrono::high_resolution_clock::now();
		std::cout << j << std::endl;
		arr_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	}
	std::cout << "index average latency: " << index_time / 1e3 << "ns" << std::endl;
	std::cout << "arr average latency: " << arr_time / 1e3 << "ns" << std::endl;
	// index_time = 0.0;
	// arr_time = 0.0;
	// int sum = 0;
	// // ALEX 遍历
	// alex::AlexNode<int,int>* cur_node = index->root_node_;
	// while (!cur_node->is_leaf_) {
	// 	int i = 0;
	// 	for (i = 0; i < static_cast<alex::AlexModelNode<int, int>*>(cur_node)->num_children_; i++) {
	// 		if (static_cast<alex::AlexModelNode<int, int>*>(cur_node)->children_[i] != nullptr) {
	// 			break;
	// 		}
	// 	}
	// 	cur_node = static_cast<alex::AlexModelNode<int, int>*>(cur_node)->children_[i];
	// }
	// alex::AlexDataNode<int,int>* leaf = static_cast<alex::AlexDataNode<int,int>*>(cur_node);
	// auto start = std::chrono::high_resolution_clock::now();
	// while (leaf != nullptr) {
	// 	for (int i = 0; i < leaf->data_capacity_; i++) {
	// 		if (leaf->check_exists(i)) {
	// 			sum += leaf->get_key(i);
	// 		}
	// 	}
	// 	leaf = leaf->next_leaf_;
	// }
	// auto end = std::chrono::high_resolution_clock::now();
	// index_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	// std::cout << sum << std::endl;
	// // 数组遍历
	// sum = 0;
	// start = std::chrono::high_resolution_clock::now();
	// for (int i = 0; i < num; i++) {
	// 	sum += arr[i];
	// }
	// end = std::chrono::high_resolution_clock::now();
	// arr_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	// std::cout << sum << std::endl;
	// std::cout << "index scan latency: " << index_time << "ns" << std::endl;
	// std::cout << "arr scan latency: " << arr_time << "ns" << std::endl;
	delete index;
	delete arr;
}

void MicroBenchmark_Insert(int num) {
	double index_time = 0.0, arr_time = 0.0;
	std::random_device seed;
	std::mt19937_64 engine(seed());
	std::uniform_int_distribution<unsigned> rand(0, 1e6);
	for (int i = 0; i < 1000; i++) {
		alex::Alex<int, int> *index = new alex::Alex<int, int>();
		int* arr = new int[num + 1];
		int max_pos = 0;
		for (int i = 0; i < num; i++) {
			int t = rand(engine);
			index->insert(std::pair<int,int>(t, t));
			arr[max_pos++] = t;
		}
		int t = rand(engine);
		auto start = std::chrono::high_resolution_clock::now();
		index->insert(std::pair<int,int>(t, t));
		auto end = std::chrono::high_resolution_clock::now();
		index_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		start = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < max_pos; i++) {
			if (arr[i] == t) {
				std::cout << "conflict" << std::endl;
			}
		}
		arr[max_pos] = t;
		end = std::chrono::high_resolution_clock::now();
		arr_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		delete index;
		delete arr;
	}
	std::cout << "index insert latency: " << index_time / 1000 << "ns" << std::endl; 
	std::cout << "arr insert latency: " << arr_time / 1000 << "ns" << std::endl;
}