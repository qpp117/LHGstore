#pragma once
#include "graph.h"

bool load_directed_graph(const std::string file_path, std::vector<edge>& all_edge, 
    std::vector<vertex>& all_vertex, std::unordered_map<vertex,int> &vertex_deg, size& total_num_edges, const size load_num_edges) {
    std::ifstream f(file_path.c_str(), std::ios::in);
    if (!f.is_open()) {
        return false;
    }
    total_num_edges = 0;
    int max_deg = 0;
    int avg_deg = 0;
    vertex max_deg_vtx = 0;
    std::set<edge> set;
    while (!f.eof()) {
        if (total_num_edges == load_num_edges) {
            break;
        }
        vertex a, b;
        f >> a >> b;
        if (a == b || set.find(edge{ a, b }) != set.end()) {
            continue;
        }
        all_edge.push_back(edge{ a,b });
        total_num_edges++;
        set.insert(edge{ a,b });
        if (vertex_deg.find(a) != vertex_deg.end()) {
            vertex_deg[a]++;
        } else {
            vertex_deg[a] = 1;
            all_vertex.push_back(a);
        }
        if (vertex_deg.find(b) == vertex_deg.end()) {
            vertex_deg[b] = 0;
            all_vertex.push_back(b);
        }
    }
    f.close();
    std::cout << "vertex = " << vertex_deg.size() << ", edge = " << total_num_edges << std::endl;
    for (auto i = vertex_deg.begin(); i != vertex_deg.end(); i++) {
        if (i->second > max_deg) {
            max_deg = i->second;
            max_deg_vtx = i->first;
        }
    }
    std::cout << "max degree = " << max_deg << ", average degree = " << total_num_edges / all_vertex.size() << std::endl;
    std::cout << "max degree's vertex = " << max_deg_vtx << std::endl;
    int cnt = 0;
    int has_outgoing = 0;
    for (auto i = vertex_deg.begin(); i != vertex_deg.end(); i++) {
        if (i->second != 0) {
            has_outgoing++;
        }
        if (i->second >= SECONDARY_INDEX_THRESHOLD) {
            cnt++;
        }
    }
    vertex_num_need_sec_index = cnt;
    std::cout << (double)cnt / has_outgoing * 100 << "% vertex exceed threshold" << "\n" << std::endl;
    return true;
}

bool load_undirected_graph(const std::string file_path, std::vector<edge>& all_edge,
    std::vector<vertex>& all_vertex, std::unordered_map<vertex, int>& vertex_deg, size& total_num_edges, const size load_num_edges) {
    std::ifstream f(file_path.c_str(), std::ios::in);
    std::random_device seed;
    std::mt19937_64 engine(seed());
    std::uniform_real_distribution<double> rand_double(1, 10);
    if (!f.is_open()) {
        return false;
    }
    total_num_edges = 0;
    int max_deg = 0;
    int avg_deg = 0;
    vertex max_deg_vtx = 0;
    std::set<edge> set;
    while (!f.eof()) {
        if (total_num_edges == load_num_edges) {
            break;
        }
        vertex a, b;
        f >> a >> b;
        if (a == b || set.find(edge{ a, b }) != set.end() || set.find(edge{ b,a }) != set.end()) {
            continue;
        }
        all_edge.push_back(edge{ a,b,rand_double(engine)});
        total_num_edges++;
        set.insert(edge{ a,b });
        if (vertex_deg.find(a) != vertex_deg.end()) {
            vertex_deg[a]++;
        }
        else {
            vertex_deg[a] = 1;
            all_vertex.push_back(a);
        }
        if (vertex_deg.find(b) != vertex_deg.end()) {
            vertex_deg[b]++;
        }
        else {
            vertex_deg[b] = 1;
            all_vertex.push_back(b);
        }
    }
    f.close();
    std::cout << "vertex = " << all_vertex.size() << ", edge = " << total_num_edges << std::endl;
    for (auto i = vertex_deg.begin(); i != vertex_deg.end(); i++) {
        if (i->second > max_deg) {
            max_deg = i->second;
            max_deg_vtx = i->first;
        }
    }
    std::cout << "max degree = " << max_deg << ", average degree = " << 2 * total_num_edges / all_vertex.size() << std::endl;
    std::cout << "max degree's vertex = " << max_deg_vtx << std::endl;
    int cnt = 0;
    for (auto i = vertex_deg.begin(); i != vertex_deg.end(); i++) {
        if (i->second >= SECONDARY_INDEX_THRESHOLD) {
            cnt++;
        }
    }
    vertex_num_need_sec_index = cnt;
    std::cout << (double)cnt / all_vertex.size() * 100 << "% vertex exceed threshold" << "\n" << std::endl;
    return true;
}

bool load_undirected_graph(const std::string file_path, std::vector<edge>& all_edge,
    std::vector<vertex>& all_vertex, std::unordered_map<vertex, int>& vertex_deg, size& total_num_edges, size load_num_edges,
    int degree_bound, bool upper /*==true: 限制度的上界*/) {
    std::ifstream f(file_path.c_str(), std::ios::in);
    if (!f.is_open()) {
        return false;
    }
    total_num_edges = 0;
    int max_deg = 0;
    int avg_deg = 0;
    vertex max_deg_vtx = 0;
    std::set<edge> set;
    while (!f.eof()) {
        if (total_num_edges == load_num_edges) {
            break;
        }
        vertex a, b;
        f >> a >> b;
        if (vertex_deg.find(a) != vertex_deg.end()) {
            vertex_deg[a]++;
        }
        else {
            vertex_deg[a] = 1;
        }
        if (vertex_deg.find(b) != vertex_deg.end()) {
            vertex_deg[b]++;
        }
        else {
            vertex_deg[b] = 1;
        }
        total_num_edges++;
    }
    std::cout << "first time traverse" << std::endl;
    total_num_edges = 0;
    f.clear();
    f.seekg(std::ios::beg);
    std::unordered_set<vertex> have;
    while (!f.eof()) {
        if (total_num_edges == load_num_edges) {
            break;
        }
        vertex a, b;
        f >> a >> b;
        if (upper) {
            if (vertex_deg[a] > degree_bound || vertex_deg[b] > degree_bound) {
                continue;
            }
        }
        else {
            if (vertex_deg[a] < degree_bound && vertex_deg[b] < degree_bound) {
                continue;
            }
        }
        all_edge.push_back(edge{a, b});
        if (have.find(a) == have.end()) {
            have.insert(a);
            all_vertex.push_back(a);
        }
        if (have.find(b) == have.end()) {
            have.insert(b);
            all_vertex.push_back(b);
        }
        total_num_edges++;
    }
    f.close();
    std::cout << "vertex = " << all_vertex.size() << ", edge = " << total_num_edges << std::endl;
    for (int i = 0; i < all_vertex.size(); i++) {
        vertex u = all_vertex[i];
        if (vertex_deg[u] > max_deg) {
            max_deg = vertex_deg[u];
            max_deg_vtx = u;
        }
    }
    std::cout << "max degree = " << max_deg << ", average degree = " << 2 * total_num_edges / all_vertex.size() << std::endl;
    std::cout << "max degree's vertex = " << max_deg_vtx << std::endl;
    int cnt = 0;
    for (int i = 0; i < all_vertex.size(); i++) {
        vertex u = all_vertex[i];
        if (vertex_deg[u] >= SECONDARY_INDEX_THRESHOLD) {
            cnt++;
        }
    }
    vertex_num_need_sec_index = cnt;
    std::cout << (double)cnt / all_vertex.size() * 100 << "% vertex exceed threshold" << "\n" << std::endl;
    return true;
}

std::string* read_option_from_file(int& argc, std::string file_name = "option.txt") {
    argc = 0;
    std::ifstream is(file_name.c_str());
    if (!is.is_open()) {
        return nullptr;
    }
    std::string str;
    while (std::getline(is, str)) {
        if (str[0] != '#')
            ++argc;
    }
    is.clear();
    is.seekg(std::ios::beg);
    std::string* ret = new std::string[argc];
    int i = 0;
    while (std::getline(is, str)) {
        if (str[0] != '#') {
            ret[i] = std::string(str);
            i++;
        }
    }
    return ret;
}