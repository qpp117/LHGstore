#pragma once

#include "graph.h"

void benchmark_read_write(std::vector<edge>& all_edge, Graph& g, bool directed, std::vector<vertex>& all_vertex,
    size init_edge_num, double read_ratio) {
    std::random_device seed;
    std::mt19937_64 engine(seed());
    std::uniform_int_distribution<unsigned> rand_edge_index(0, init_edge_num - 1);
    std::uniform_real_distribution<double> rand_double(1, 10);
    double total_time = 0.0;
    std::unordered_set<vertex> exist_vertex;
    std::unordered_set<vertex> non_exist_vertex;
    for (int i = 0; i < init_edge_num; i++) {
        if (!g.lookup_vertex(all_edge[i].src)) {
            g.insert_vertex(all_edge[i].src);
        }
        if (!g.lookup_vertex(all_edge[i].dst)) {
            g.insert_vertex(all_edge[i].dst);
        }
        g.insert_edge(all_edge[i]);
        exist_vertex.insert(all_edge[i].src);
        exist_vertex.insert(all_edge[i].dst);
    }
    for (int i = 0; i < all_vertex.size(); i++) {
        if (exist_vertex.find(all_vertex[i]) == exist_vertex.end()) {
            non_exist_vertex.insert(all_vertex[i]);
        }
    }
    std::cout << "--- Init benchmark finish, read ratio = " << read_ratio << " ---\n" << std::endl;
    all_vertex.clear();
    all_vertex.assign(exist_vertex.begin(), exist_vertex.end());
    std::uniform_int_distribution<unsigned> rand_vertex_index(0, all_vertex.size() - 1);
    // 下一条要插入的边在 all_edge 中的下标
    int all_edge_index = init_edge_num;
    int all_edge_index2 = 0;
    int all_vertex_index = 0;
    // 选择读写操作
    int write_op = 0, read_op = 0;
    auto start = std::chrono::high_resolution_clock::now(), end = std::chrono::high_resolution_clock::now();
    // 累计 write 次数，用于决定什么时候 read
    int write_cnt = 0;
    // 总写、读、操作次数
    size total_write_cnt = 0, total_read_cnt = 0, total_cnt = 0;
    size lookup_edge_failed = 0, lookup_vertex_failed = 0, sum_nbr = 0;
    size insert_edge_failed = 0, insert_vertex_failed = 0, delete_edge_failed = 0, delete_vertex_failed = 0;
    while (true) {
        if (all_edge_index == all_edge.size() - 1) {
            break;
        }
        switch (write_op)
        {
        // add edge
        case 0: {
            //std::cout << "add edge " << all_edge[all_edge_index].src << ", " << all_edge[all_edge_index].dst << std::endl;
            if (!g.lookup_vertex(all_edge[all_edge_index].src)) {
                g.insert_vertex(all_edge[all_edge_index].src);
            }
            if (!g.lookup_vertex(all_edge[all_edge_index].dst)) {
                g.insert_vertex(all_edge[all_edge_index].dst);
            }
            start = std::chrono::high_resolution_clock::now();
            bool ret = g.insert_edge(all_edge[all_edge_index]);
            end = std::chrono::high_resolution_clock::now();
            total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            if (!ret) {
                insert_edge_failed++;
            }
            non_exist_vertex.erase(all_edge[all_edge_index].src);
            non_exist_vertex.erase(all_edge[all_edge_index].dst);
            all_edge_index++;
            write_op++;
            break;
        }
        // add vertex
        case 1: {
            int i = 0;
            auto it = non_exist_vertex.begin();
            vertex u = 0;
            if (it != non_exist_vertex.end()) {
                u = *it;
            }
            else {
                i = rand_vertex_index(engine);
                u = all_vertex[i];
            }
            //std::cout << "insert vertex " << u << std::endl;
            start = std::chrono::high_resolution_clock::now();
            bool ret = g.insert_vertex(u);
            end = std::chrono::high_resolution_clock::now();
            total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            if (it != non_exist_vertex.end()) {
                non_exist_vertex.erase(it);
            }
            if (!ret) {
                delete_vertex_failed++;
            }
            write_op++;
            break;
        }
        // delete edge
        case 2: {
            //std::cout << "delete edge " << all_edge[all_edge_index].src << ", " << all_edge[all_edge_index].dst << std::endl;
            int i = rand_edge_index(engine);
            //int i = all_edge_index2++;
            start = std::chrono::high_resolution_clock::now();
            bool ret = g.delete_edge(all_edge[i]);
            end = std::chrono::high_resolution_clock::now();
            if (!ret) {
                delete_edge_failed++;
            }
            total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            write_op++;
            break;
        }
        // delete vertex
        case 3: {
            //std::cout << "insert vertex " << all_vertex[all_vertex_index] << std::endl;
            int i = rand_vertex_index(engine);
            //int i = all_vertex_index++;
            start = std::chrono::high_resolution_clock::now();
            bool ret = g.delete_vertex(all_vertex[i]);
            end = std::chrono::high_resolution_clock::now();
            total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            if (!ret) {
                delete_vertex_failed++;
            }
            non_exist_vertex.insert(all_vertex[i]);
            write_op = 0;
            break;
        }
        default:
            break;
        }
        write_cnt++;
        total_write_cnt++;
        total_cnt++;
        int i = 0;
        for (i = 0; i < read_ratio * write_cnt; i++) {
            //std::cout << "read op = " << read_op << std::endl;
            switch (read_op)
            {
            // lookup vertex
            case 0: {
                int idx = rand_vertex_index(engine);
                start = std::chrono::high_resolution_clock::now();
                bool ret = g.lookup_vertex(all_vertex[idx]);
                end = std::chrono::high_resolution_clock::now();
                if (!ret) {
                    lookup_vertex_failed++;
                }
                total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                read_op++;
                break;
            }
            // lookup edge
            case 1: {
                int idx = rand_edge_index(engine);
                start = std::chrono::high_resolution_clock::now();
                bool ret = g.lookup_edge(all_edge[idx]);
                end = std::chrono::high_resolution_clock::now();
                if (!ret) {
                    lookup_edge_failed++;
                }
                total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                read_op++;
                break;
            }
            // get neighbor
            case 2: {
                int i = rand_vertex_index(engine);
                start = std::chrono::high_resolution_clock::now();
                int ret = g.get_neighbor(all_vertex[i]);
                end = std::chrono::high_resolution_clock::now();
                sum_nbr += ret;
                total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                read_op = 0;
                break;
            }
            default:
                break;
            }
            total_cnt++;
            total_read_cnt++;
        }
        if (i != 0) {
            write_cnt = 0;
        }
    }
    std::cout << "read cnt = " << total_read_cnt << "  " << "write cnt = " << total_write_cnt << std::endl;
    std::cout << "throughput = " << total_cnt / total_time * 1e9 << std::endl;
    std::cout << lookup_vertex_failed + lookup_edge_failed + sum_nbr + insert_edge_failed + delete_edge_failed + delete_vertex_failed + delete_vertex_failed << std::endl;
}


void benchmark_read_only(std::vector<edge>& all_edge, Graph& g, bool directed, std::vector<vertex>& all_vertex, double read_ratio) {
    std::random_device seed;
    std::mt19937_64 engine(seed());
    std::uniform_int_distribution<unsigned> rand_edge_index(0, all_edge.size() - 1);
    std::uniform_real_distribution<double> rand_double(1, 1000);
    double total_time = 0.0;
    std::unordered_set<vertex> exist_vertex;
    for (int i = 0; i < all_edge.size(); i++) {
        if (!g.lookup_vertex(all_edge[i].src)) {
            g.insert_vertex(all_edge[i].src);
        }
        if (!g.lookup_vertex(all_edge[i].dst)) {
            g.insert_vertex(all_edge[i].dst);
        }
        g.insert_edge(all_edge[i]);
        exist_vertex.insert(all_edge[i].src);
        exist_vertex.insert(all_edge[i].dst);
    }
    std::cout << "--- Init read only benchmark finish ---\n" << std::endl;
    all_vertex.clear();
    all_vertex.assign(exist_vertex.begin(), exist_vertex.end());
    std::uniform_int_distribution<unsigned> rand_vertex_index(0, all_vertex.size() - 1);
    // 下一条要插入的边在 all_edge 中的下标
    int all_edge_index2 = 0;
    int all_vertex_index = 0;
    // 选择读写操作
    int read_op = 0;
    auto start = std::chrono::high_resolution_clock::now(), end = std::chrono::high_resolution_clock::now();
    // 总写、读、操作次数
    size lookup_edge_failed = 0, lookup_vertex_failed = 0, sum_nbr = 0;
    for (int i = 0; i < read_ratio * all_edge.size(); i++) {
        //std::cout << "read op = " << read_op << std::endl;
        switch (read_op)
        {
        // lookup vertex
        case 0: {
            int idx = rand_vertex_index(engine);
            start = std::chrono::high_resolution_clock::now();
            bool ret = g.lookup_vertex(all_vertex[idx]);
            end = std::chrono::high_resolution_clock::now();
            if (!ret) {
                lookup_vertex_failed++;
            }
            total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            read_op++;
            break;
        }
        // lookup edge
        case 1: {
            int idx = rand_edge_index(engine);
            start = std::chrono::high_resolution_clock::now();
            bool ret = g.lookup_edge(all_edge[idx]);
            end = std::chrono::high_resolution_clock::now();
            if (!ret) {
                lookup_edge_failed++;
            }
            total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            read_op++;
            break;
        }
        // get neighbor
        case 2: {
            int i = rand_vertex_index(engine);
            start = std::chrono::high_resolution_clock::now();
            int ret = g.get_neighbor(all_vertex[i]);
            end = std::chrono::high_resolution_clock::now();
            sum_nbr += ret;
            total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            read_op = 0;
            break;
        }
        default:
            break;
        }
    }
    std::cout << "throughput = " << read_ratio * all_edge.size() / total_time * 1e9 << std::endl;
    std::cout << lookup_vertex_failed + lookup_edge_failed + sum_nbr << std::endl;
}
