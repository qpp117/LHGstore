#pragma once
#include "graph.h"

void insert(std::vector<edge>& all_edge, teseo::Teseo& db) {
    for (int i = 0; i < all_edge.size(); i++) {
        teseo::Transaction tx = db.start_transaction();
        uint64_t u = all_edge[i].src;
        uint64_t v = all_edge[i].dst;
        auto start = std::chrono::high_resolution_clock::now();
        if (!tx.has_vertex(u)) {
            tx.insert_vertex(u);
        }
        if (!tx.has_vertex(v)) {
            tx.insert_vertex(v);
        }
        tx.insert_edge(u, v, all_edge[i].weight);
        auto end = std::chrono::high_resolution_clock::now();
        insert_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        tx.commit();
    }
    std::cout << "insert throughput = " << all_edge.size() / insert_time * 1e9 << std::endl;
}

void lookup(std::vector<std::pair<uint64_t, uint64_t>>& all_edge, teseo::Teseo& db) {
    for (int i = 0; i < all_edge.size(); i++) {
        auto tx = db.start_transaction();
        uint64_t u = all_edge[i].first;
        uint64_t v = all_edge[i].second;
        auto start = std::chrono::high_resolution_clock::now();
        double w = tx.get_weight(u, v);
        auto end = std::chrono::high_resolution_clock::now();
        lookup_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        tx.commit();
    }
    std::cout << "lookup throughput = " << all_edge.size() / lookup_time * 1e9 << std::endl;
}

void delete_(std::vector<std::pair<uint64_t, uint64_t>>& all_edge, teseo::Teseo& db) {
    for (int i = 0; i < all_edge.size(); i++) {
        auto tx = db.start_transaction();
        uint64_t u = all_edge[i].first;
        uint64_t v = all_edge[i].second;
        auto start = std::chrono::high_resolution_clock::now();
        tx.remove_edge(u, v);
        auto end = std::chrono::high_resolution_clock::now();
        delete_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        tx.commit();
    }
    std::cout << "delete throughput = " << all_edge.size() / delete_time * 1e9 << std::endl;
}