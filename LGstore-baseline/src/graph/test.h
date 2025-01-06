#pragma once
#include <vector>
#include <unordered_map>
#include <chrono>
#include "graph.h"
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