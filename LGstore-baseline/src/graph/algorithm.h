#pragma once
#include "graph.h"
#define MAX_ITER 5

void bfs_from_root(vertex root, std::unordered_set<vertex>& visit, Iterator& it) {
	std::queue<vertex> q;
	q.push(root);
	visit.insert(root);
	while (!q.empty()) {
		vertex u = q.front();
		q.pop();
		Iterator it2 = it.get_iterator(u);
		vertex* dst = nullptr;
		while ((dst = it2.next_nbr()) != nullptr) {
			if (visit.find(*dst) != visit.end()) {
				continue;
			}
			visit.insert(*dst);
			q.push(*dst);
		}
	}
}

void bfs(Graph& g, vertex root) {
	std::unordered_set<vertex> visit;
	Iterator it(g.index);
	bfs_from_root(root, visit, it);
}

void one_connected_component(vertex root, std::unordered_map<vertex, int>& id2label, Iterator& it, int label) {
	std::queue<vertex> q;
	q.push(root);
	while (!q.empty()) {
		vertex u = q.front();
		q.pop();
		Iterator it2 = it.get_iterator(u);
		vertex* dst = nullptr;
		while ((dst = it2.next_nbr()) != nullptr) {
			if (id2label.find(*dst) != id2label.end()) {
				continue;
			}
			id2label[*dst] = label;
			q.push(*dst);
		}
	}
}

void connected_component(Graph& g) {
	std::unordered_map<vertex, int> id2label;
	int label = 0;
	Iterator it(g.index);
	const vertex* src = nullptr;
	while ((src = it.next_vertex()) != nullptr) {
		if (id2label.find(*src) != id2label.end()) {
			continue;
		}
		id2label[*src] = label;
		one_connected_component(*src, id2label, it, label);
		label++;
	}
}

void pagerank(Graph& g, std::vector<vertex>& all_vertex, std::unordered_map<vertex, int>& degree, std::unordered_map<vertex, double>& PR) {
	for (int i = 0; i < MAX_ITER; i++) {
		std::unordered_map<vertex, double> nextPR;
		Iterator it(g.index);
		const vertex* src = nullptr;
		while ((src = it.next_vertex()) != nullptr) {
			const vertex* dst = nullptr;
			while ((dst = it.next_nbr()) != nullptr) {
				if (nextPR.find(*dst) == nextPR.end()) {
					nextPR[*dst] = PR[*src] / degree[*src];
				}
				else {
					nextPR[*dst] += PR[*src] / degree[*src];
				}
			}
		}
		PR = nextPR;
	}
}

size triangle_counting(Graph& g) {
	size ans = 0;
	Iterator it(g.index);
	const vertex* u = nullptr;
	while ((u = it.next_vertex()) != nullptr) {
		vertex* v = nullptr;
		while ((v = it.next_nbr()) != nullptr) {
			if (*u > *v) {
				Iterator it3 = it.get_iterator(*v);
				vertex* w = nullptr;
				while ((w = it3.next_nbr()) != nullptr) {
					if (*v > *w) {
						if (g.lookup_edge(*u, *w)) {
							ans++;
						}
					}
				}
			}
		}
	}
	return ans;
}

double LCC(Graph& g, vertex u, int degree = 1) {
	size triangle_num = 0;
	vertex* v = nullptr;
	Iterator it(g.index);
	Iterator it2 = it.get_iterator(u);
	while ((v = it2.next_nbr()) != nullptr) {
		vertex* w = nullptr;
		Iterator it3 = it.get_iterator(*v);
		while ((w = it3.next_nbr()) != nullptr) {
			if (*w > *v && g.lookup_edge(*w, u)) {
				triangle_num++;
			}
		}
	}
	return (double)2 * triangle_num / (degree * (degree - 1));
}

void SSSP(Graph& g, vertex u, std::unordered_map<vertex,double>& dis) {
	std::unordered_set<vertex> vis;
	std::priority_queue<std::pair<double,vertex>, std::vector<std::pair<double, vertex>>, std::greater<std::pair<double, vertex>>> q;
	q.push(std::pair<double, vertex>(0.0, u));
	Iterator it(g.index);
	while (!q.empty()) {
		std::pair<double, vertex> cur = q.top();
		q.pop();
		vis.insert(cur.second);
		Iterator it2 = it.get_iterator(cur.second);
		vertex* v = nullptr;
		while ((v = it2.next_nbr()) != nullptr) {
			if (vis.find(*v) == vis.end() && cur.first + it2.get_weight() < dis[*v]) {
				dis[*v] = cur.first + it2.get_weight();
				q.push(std::pair<double, vertex>(dis[*v], *v));
			}
		}
	}
}

void CDLP(Graph& g, std::unordered_map<vertex, size>& label) {
	bool change = true;
	std::unordered_map<vertex, size> next_label;
	int iter = 0;
	while (change && iter < MAX_ITER) {
		change = false;
		Iterator it(g.index);
		const vertex* u = nullptr;
		while ((u = it.next_vertex()) != nullptr) {
			std::unordered_map<size, size> label_cnt;
			const vertex* v = nullptr;
			while ((v = it.next_nbr()) != nullptr) {
				label_cnt[label[*v]]++;
			}
			size max_cnt_label = 0;
			size max_cnt = 0;
			for (auto i = label_cnt.begin(); i != label_cnt.end(); i++) {
				if (i->second > max_cnt || (i->second == max_cnt && i->first < max_cnt_label)) {
					max_cnt = i->second;
					max_cnt_label = i->first;
				}
			}
			next_label[*u] = max_cnt_label;
			if (next_label[*u] != label[*u]) {
				change = true;
			}
		}
		label = next_label;
		iter++;
	}
}