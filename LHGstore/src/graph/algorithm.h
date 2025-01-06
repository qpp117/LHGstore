#pragma once
#include "graph.h"

void bfs_from_root(alex::Alex<vertex, payload>* index, vertex root, std::unordered_set<vertex>& visit, Iterator& it) {
	std::queue<vertex> q;
	q.push(root);
	visit.insert(root);
	while (!q.empty()) {
		vertex u = q.front();
		q.pop();
		Iterator it2 = it.get_iterator(u);
		vertex *dst = nullptr;
		while ((dst = it2.next_nbr()) != nullptr) {
			if (visit.find(*dst) != visit.end()) {
				continue;
			}
			visit.insert(*dst);
			q.push(*dst);
		}
	}
}

void bfs(alex::Alex<vertex, payload>* index, vertex root) {
	std::unordered_set<vertex> visit;
	Iterator it(index);
	bfs_from_root(index, root, visit, it);
}

void pagerank(alex::Alex<vertex, payload>* index, std::vector<vertex>& all_vertex) {
	std::unordered_map<vertex, double> PR;
	for (int i = 0; i < all_vertex.size(); i++) {
		PR[all_vertex[i]] = (double)1 / all_vertex.size();
	}
	for (int i = 0; i < 5; i++) {
		std::unordered_map<vertex, double> nextPR;
		Iterator it(index);
		vertex* src = nullptr;
		while ((src = it.next_vertex()) != nullptr) {
			Iterator it2 = it.get_iterator();
			vertex* dst = nullptr;
			while ((dst = it2.next_nbr()) != nullptr) {
				if (nextPR.find(*dst) == nextPR.end()) {
					nextPR[*dst] = PR[*src] / it.get_degree();
				}
				else {
					nextPR[*dst] += PR[*src] / it.get_degree();
				}
			}
		}
		PR = nextPR;
	}
}