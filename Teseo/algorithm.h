#pragma once
#include "graph.h"

void bfs(teseo::Teseo& db, uint64_t root) {
    auto tx = db.start_transaction();
    auto it = tx.iterator();
    std::unordered_set<uint64_t> visit;
    std::queue<uint64_t> q;
    auto start = std::chrono::high_resolution_clock::now();
    q.push(root);
    visit.insert(root);
    while (!q.empty()) {
        uint64_t u = q.front();
        q.pop();
        it.edges(u, true, [&](uint64_t v) {
            if (visit.find(v) == visit.end()) {
                q.push(v);
                visit.insert(v);
            }
        });
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "BFS time = " << (double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e9 << std::endl << std::endl;
    it.close();
    tx.commit();
}

void connected_component(teseo::Teseo& db) {
    std::unordered_map<uint64_t, int> id2label;
	uint64_t label = 0;
    auto tx = db.start_transaction();
    auto it = tx.iterator();
    auto start = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < tx.num_vertices(); i++) {
        if (id2label.find(i) != id2label.end()) {
            continue;
        }
        id2label[i] = label;
        std::queue<uint64_t> q;
        q.push(i);
        while (!q.empty()) {
            uint64_t u = q.front();
            q.pop();
            it.edges(u, true, [&](uint64_t v) {
                if (id2label.find(v) == id2label.end()) {
                    id2label[v] = label;
                    q.push(v);
                }
            });
        }
        label++;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "CC time = " << (double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e9 << std::endl << std::endl;
    it.close();
    tx.commit();
}

void pagerank(teseo::Teseo& db) {
    std::cout << "start Pagerank" << std::endl;
    auto tx = db.start_transaction();
    auto it = tx.iterator();
    std::unordered_map<uint64_t, double> PR;
    for (int i = 0; i < tx.num_vertices(); i++) {
        PR[i] = (double)1 / tx.num_vertices();
    }
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 5; i++) {
        std::unordered_map<uint64_t, double> nextPR;
        for (uint64_t j = 0; j < tx.num_vertices(); j++) {
            it.edges(j, true, [&](uint64_t v) {
                if (nextPR.find(v) == nextPR.end()) {
                    nextPR[v] = (double)PR[j] / tx.degree(j, true);
                }
                else {
                    nextPR[v] += (double)PR[j] / tx.degree(j, true);
                }
                });
        }
        PR = nextPR;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "PageRank time = " << (double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e9 << std::endl << std::endl;
    it.close();
    tx.commit();
}

void LCC(teseo::Teseo& db, uint64_t root) {
    double ans = 0.0;
    auto tx = db.start_transaction();
    uint64_t u = tx.vertex_id(root);
    auto it = tx.iterator();
    auto it2 = tx.iterator();
    auto start = std::chrono::high_resolution_clock::now();
    it.edges(u, false, [&](uint64_t v) {
        it2.edges(v, false, [&](uint64_t w) {
            if (w > v && tx.has_edge(w,u)) {
                ans++;
            }
        });
    });
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "LCC = " << 2 * ans / (tx.degree(root, true) * (tx.degree(root, true) - 1)) << std::endl;
    std::cout << "LCC time = " << (double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e9 << std::endl << std::endl;
    it.close();
    it2.close();
    tx.commit();
}

void SSSP(teseo::Teseo& db, uint64_t root) {
    std::unordered_map<uint64_t, double> dis;
    auto tx = db.start_transaction();
    auto it = tx.iterator();
    for (uint64_t i = 0; i < tx.num_vertices(); i++) {
        dis[tx.vertex_id(i)] = INF;
    }
    root = tx.vertex_id(root);
    dis[root] = 0;
    auto start = std::chrono::high_resolution_clock::now();
    std::unordered_set<uint64_t> vis;
	std::priority_queue<std::pair<double,uint64_t>, std::vector<std::pair<double, uint64_t>>, std::greater<std::pair<double, uint64_t>>> q;
	q.push(std::pair<double, uint64_t>(0.0, root));
    while (!q.empty()) {
        std::pair<double, uint64_t> cur = q.top();
		q.pop();
        vis.insert(cur.second);
        //std::cout << "shortest path --> " << cur.second << std::endl;
        it.edges(cur.second, false, [&](uint64_t v) {
            double weight = tx.get_weight(cur.second, v);
            double new_weight = cur.first + weight;
            if (vis.find(v) == vis.end() && new_weight < dis[v]) {
                //std::cout << "udpate path --> " << v  << " from " << dis[v] << " to " << cur.first + weight << std::endl;
                dis[v] = new_weight;
                q.push(std::pair<double, uint64_t>(dis[v], v));
            }
        });
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "SSSP time = " << (double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e9 << std::endl << std::endl;
    it.close();
    tx.commit();
}

void CDLP(teseo::Teseo& db, int max_iter) {
    std::unordered_map<uint64_t, size> label;
    std::unordered_map<uint64_t, size> next_label;
    auto tx = db.start_transaction();
    auto it = tx.iterator();
    for (uint64_t i = 0; i < tx.num_vertices(); i++) {
        label[i] = i;
    }
    bool change = true;
    int iter = 0;
    auto start = std::chrono::high_resolution_clock::now();
    while (change && iter < max_iter) {
        change = false;
        for (uint64_t i = 0; i < tx.num_vertices(); i++) {
            std::unordered_map<size, size> label_cnt;
            it.edges(i, true, [&](uint64_t v) {
                label_cnt[label[v]]++;
            });
            size max_cnt_label = 0;
			size max_cnt = 0;
			for (auto i = label_cnt.begin(); i != label_cnt.end(); i++) {
				if (i->second > max_cnt || (i->second == max_cnt && i->first < max_cnt_label)) {
					max_cnt = i->second;
					max_cnt_label = i->first;
				}
			}
			next_label[i] = max_cnt_label;
			if (next_label[i] != label[i]) {
				change = true;
			}
        }
        label = next_label;
		iter++;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "CDLP time = " << (double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e9 << std::endl << std::endl;
    it.close();
    tx.commit();
}