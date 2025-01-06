#include "../core/alex.h"
#include "flags.h"
#include "graph_load.h"
#include "graph_query.h"
#include "utils.h"
#include "test.h"
#include "graph.h"
#include "algorithm.h"
#include "../core/alex_multimap.h"

void benchmark_read_write(std::vector<edge>& all_edge, Graph& g, bool directed, std::vector<vertex>& all_vertex,
    size init_edge_num, double read_ratio) {
    std::random_device seed;
    std::mt19937_64 engine(seed());
    std::uniform_int_distribution<unsigned> rand_edge_index(0, init_edge_num - 1);
    std::uniform_real_distribution<double> rand_double(1, 1000);
    double total_time = 0.0;
    std::unordered_set<vertex> exist_vertex;
    std::unordered_set<vertex> non_exist_vertex;
    for (int i = 0; i < init_edge_num; i++) {
        g.insert_edge(all_edge[i], directed);
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
            start = std::chrono::high_resolution_clock::now();
            bool ret = g.insert_edge(all_edge[all_edge_index], directed);
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
            bool ret = g.delete_edge(all_edge[i], directed);
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
            bool ret = g.delete_vertex(all_vertex[i], directed);
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
                bool ret = g.lookup_edge(all_edge[idx], directed);
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
        g.insert_edge(all_edge[i], directed);
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
            bool ret = g.lookup_edge(all_edge[idx], directed);
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

void bfs_test(Graph& g, std::unordered_map<vertex, int>& degree) {
    vertex root = 0;
    size max_deg = 0;
    for (auto i = degree.begin(); i != degree.end(); i++) {
        if (i->second > max_deg) {
            max_deg = i->second;
            root = i->first;
        }
    }
    auto algo_start_time = std::chrono::high_resolution_clock::now();
    bfs(g, root);
    auto algo_end_time = std::chrono::high_resolution_clock::now();
    auto algo_time = std::chrono::duration_cast<std::chrono::nanoseconds>(algo_end_time - algo_start_time).count();
    std::cout << "BFS time = " << algo_time / 1e9 << std::endl << std::endl;
}

void pagerank_test(Graph& g, std::vector<vertex>& all_vertex, std::unordered_map<vertex, int>& degree) {
    std::unordered_map<vertex, double> PR;
    for (int i = 0; i < all_vertex.size(); i++) {
        PR[all_vertex[i]] = (double)1 / all_vertex.size();
    }
    auto algo_start_time = std::chrono::high_resolution_clock::now();
    pagerank(g, all_vertex, degree, PR);
    auto algo_end_time = std::chrono::high_resolution_clock::now();
    auto algo_time = std::chrono::duration_cast<std::chrono::nanoseconds>(algo_end_time - algo_start_time).count();
    std::cout << "PageRank time = " << algo_time / 1e9 << std::endl << std::endl;
}

void triangle_counting_test(Graph& g) {
    auto algo_start_time = std::chrono::high_resolution_clock::now();
    size ans = triangle_counting(g);
    auto algo_end_time = std::chrono::high_resolution_clock::now();
    auto algo_time = std::chrono::duration_cast<std::chrono::nanoseconds>(algo_end_time - algo_start_time).count();
    std::cout << "Triangle Num = " << ans << std::endl;
    std::cout << "Triangle Counting time = " << algo_time / 1e9 << std::endl << std::endl;
}

void connected_component_test(Graph& g) {
    auto algo_start_time = std::chrono::high_resolution_clock::now();
    connected_component(g);
    auto algo_end_time = std::chrono::high_resolution_clock::now();
    auto algo_time = std::chrono::duration_cast<std::chrono::nanoseconds>(algo_end_time - algo_start_time).count();
    std::cout << "Connected Component time = " << algo_time / 1e9 << std::endl << std::endl;
}

void LCC_test(Graph& g, std::unordered_map<vertex, int>& degree) {
    vertex root = 0;
    size max_deg = 0;
    for (auto i = degree.begin(); i != degree.end(); i++) {
        if (i->second > max_deg) {
            max_deg = i->second;
            root = i->first;
        }
    }
    auto algo_start_time = std::chrono::high_resolution_clock::now();
    double ans = LCC(g, root, degree[root]);
    auto algo_end_time = std::chrono::high_resolution_clock::now();
    auto algo_time = std::chrono::duration_cast<std::chrono::nanoseconds>(algo_end_time - algo_start_time).count();
    std::cout << "LCC of vertex " << root << " = " << ans << std::endl;
    std::cout << "LCC time = " << algo_time / 1e9 << std::endl << std::endl;
}

void SSSP_test(Graph& g, std::unordered_map<vertex, int>& degree) {
    vertex root = 0;
    size max_deg = 0;
    std::unordered_map<vertex, double> dis;
    for (auto i = degree.begin(); i != degree.end(); i++) {
        if (i->second > max_deg) {
            max_deg = i->second;
            root = i->first;
        }
        dis.insert(std::pair<vertex, double>(i->first, INF));
    }
    auto algo_start_time = std::chrono::high_resolution_clock::now();
    SSSP(g, root, dis);
    auto algo_end_time = std::chrono::high_resolution_clock::now();
    auto algo_time = std::chrono::duration_cast<std::chrono::nanoseconds>(algo_end_time - algo_start_time).count();
    std::cout << "SSSP time = " << algo_time / 1e9 << std::endl << std::endl;
}

void CDLP_test(Graph& g, std::vector<vertex>& all_vertex) {
    std::unordered_map<vertex, size> label;
    for (int i = 0; i < all_vertex.size(); i++) {
        label[all_vertex[i]] = all_vertex[i];
    }
    auto algo_start_time = std::chrono::high_resolution_clock::now();
    CDLP(g, label);
    auto algo_end_time = std::chrono::high_resolution_clock::now();
    auto algo_time = std::chrono::duration_cast<std::chrono::nanoseconds>(algo_end_time - algo_start_time).count();
    std::cout << "CDLP time = " << algo_time / 1e9 << std::endl << std::endl;
}

void traverse_test(Graph& g) {
    Iterator it(g.index);
    const vertex* u = nullptr;
    const vertex* v = nullptr;
    size vertex_num = 0;
    size edge_num = 0;
    auto algo_start_time = std::chrono::high_resolution_clock::now();
    while ((u = it.next_vertex()) != nullptr) {
        vertex_num++;
        while ((v = it.next_nbr()) != nullptr) {
            edge_num++;
        }
    }
    auto algo_end_time = std::chrono::high_resolution_clock::now();
    auto algo_time = std::chrono::duration_cast<std::chrono::nanoseconds>(algo_end_time - algo_start_time).count();
    std::cout << "vertex num = " << vertex_num << std::endl;
    std::cout << "edge num = " << edge_num / 2 << std::endl;
    std::cout << "traverse time = " << algo_time / 1e9 << std::endl << std::endl;
}

int main(int argc, char* argv[]) {
    std::map<std::string, std::string> flags;
    if (argc != 1) {
        flags = parse_flags(argc, argv);
    } else {
        int option_num = 0;
        std::string* opt = read_option_from_file(option_num);
        flags = parse_flags(option_num, opt);
    }
    std::string graph_file_path = get_with_default(flags, "graph_file", "C:\\Users\\wangx\\Documents\\dataset\\Slashdot\\Slashdot0902.txt");
    bool directed = false;
    if (get_with_default(flags, "directed", "false") == "true") {
        directed = true;
    }
    std::cout << "\n";

    // Read graph from file
    std::vector<edge> all_edge;
    std::vector<vertex> all_vertex;
    std::unordered_map<vertex, int> vertex_deg;
    size total_num_edges = 0;
    size load_num_edges = INF;
    if (directed) {
        if (!load_directed_graph(graph_file_path, all_edge, all_vertex, vertex_deg, total_num_edges, load_num_edges)) {
            std::cout << "file cannot open" << std::endl;
            return -1;
        }
    }
    else {
        if (!load_undirected_graph(graph_file_path, all_edge, all_vertex, vertex_deg, total_num_edges, load_num_edges)) {
            std::cout << "file cannot open" << std::endl;
            return -1;
        }
    }
    
    std::cout << "--- Load graph finish ---\n" << std::endl;
    Graph g;
    // benchmark_read_write(all_edge, g, directed, all_vertex, 0.8 * all_edge.size(), 0);
    // g.clear();
    // std::cout << "\n" << std::endl;
    // benchmark_read_write(all_edge, g, directed, all_vertex, 0.8 * all_edge.size(), 1);
    // g.clear();
    // std::cout << "\n" << std::endl;
    // benchmark_read_only(all_edge, g, directed, all_vertex, 0.5);
    for (int i = 0; i < all_edge.size(); i++) {
       g.insert_edge(all_edge[i], directed);
    }
    //std::cout << "mem usage = " << (double)g.get_bytes() / (1024 * 1024) << "MB" << std::endl;
    for (int i = 0; i < 2; i++) {
        SSSP_test(g, vertex_deg);
    }
    for (int i = 0; i < 2; i++) {
        pagerank_test(g, all_vertex, vertex_deg);
    }
    // bfs_test(g, vertex_deg);
    // connected_component_test(g);
    // LCC_test(g, vertex_deg);
    // pagerank_test(g, all_vertex, vertex_deg);
    // CDLP_test(g, all_vertex);
#ifdef TIME
    std::cout << "lookup in first index = " << num_lookup_fir_idx << std::endl;
    std::cout << "delete in first index = " << num_delete_fir_idx << std::endl;
    std::cout << "num_delete_fill = " << num_delete_fill << std::endl;
    std::cout << "num_scan = " << num_scan << std::endl;
#endif // TIME

    return 0;
}
