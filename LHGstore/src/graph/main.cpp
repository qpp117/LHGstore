#include "../core/alex.h"
#include "flags.h"
#include "graph_load.h"
#include "graph_query.h"
#include "utils.h"
#include "test.h"
#include "algorithm.h"
using namespace graph_query0;

void Insert(std::vector<edge>& all_edge, alex::Alex<vertex, payload>* index, bool directed) {
    bool ret = true;
    for (int i = 0; i < all_edge.size(); i++) {
        //std::cout << all_edge[i].src << " " << all_edge[i].dst << std::endl;
        auto insert_start_time = std::chrono::high_resolution_clock::now();
        ret = insert_edge(index, all_edge[i], directed);
        auto insert_end_time = std::chrono::high_resolution_clock::now();
        insert_time += std::chrono::duration_cast<std::chrono::nanoseconds>(insert_end_time - insert_start_time).count();
        if (!ret) {
            std::cout << "src: " << all_edge[i].src << ", dst: " << all_edge[i].dst << " insert failed" << std::endl;
            //exit(-1);
        }
    }
    std::cout << "insert throughput = " << all_edge.size() / insert_time * 1e9 << std::endl;
    std::cout << "\n--- Insert edges finish ---\n" << std::endl << std::endl;
}

void Lookup(std::vector<edge>& all_edge, alex::Alex<vertex, payload>* index, bool directed) {
    bool ret = true;
    for (int i = 0; i < all_edge.size(); i++) {
        auto lookup_start_time = std::chrono::high_resolution_clock::now();
        ret = lookup_edge(index, all_edge[i], directed);
        auto lookup_end_time = std::chrono::high_resolution_clock::now();
        lookup_time += std::chrono::duration_cast<std::chrono::nanoseconds>(lookup_end_time - lookup_start_time).count();
        if (!ret) {
            std::cout << "src: " << all_edge[i].src << ", dst: " << all_edge[i].dst << " not found" << std::endl;
            //exit(-1);
        }
    }
    std::cout << "lookup throughput = " << all_edge.size() / lookup_time * 1e9 << std::endl;
    std::cout << "\n--- Lookup edges finish ---\n" << std::endl;
}

void Delete_(std::vector<edge>& all_edge, alex::Alex<vertex, payload>* index, bool directed) {
    bool ret = true;
    for (int i = 0; i < all_edge.size(); i++) {
        auto delete_start_time = std::chrono::high_resolution_clock::now();
        ret = delete_edge(index, all_edge[i], directed);
        auto delete_end_time = std::chrono::high_resolution_clock::now();

        delete_time += std::chrono::duration_cast<std::chrono::nanoseconds>(delete_end_time - delete_start_time).count();
        total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(delete_end_time - delete_start_time).count();
        if (!ret) {
            std::cout << "src: " << all_edge[i].src << ", dst: " << all_edge[i].dst << " delete failed" << std::endl;
            //exit(-1);
        }
    }
    std::cout << "delete throughput = " << all_edge.size() / delete_time * 1e9 << std::endl;
    std::cout << "\n--- delete edges finish ---\n" << std::endl;
}

void Update(std::vector<edge>& all_edge, alex::Alex<vertex, payload>* index, bool directed) {
    std::random_device seed; // 硬件生成随机数种子
	std::mt19937_64 engine(seed()); // 利用种子生成随机数引擎
    std::uniform_int_distribution<unsigned> rand(0, all_edge.size() - 1); // 设置随机数范围，并为均匀分布
    std::vector<bool> have_inserted(all_edge.size(), true);
    int insert_num = 0, delete_num = 0;
    for (size i = 0; i < (size)5 * all_edge.size(); i++) {
        int idx = rand(engine);
        edge e = all_edge[idx];
        if (have_inserted[idx]) {
            auto update_start_time = std::chrono::high_resolution_clock::now();
            delete_edge(index, e, directed);
            auto update_end_time = std::chrono::high_resolution_clock::now();
            update_time += std::chrono::duration_cast<std::chrono::nanoseconds>(update_end_time - update_start_time).count();
            delete_num++;
            have_inserted[idx] = false;
        } else {
            auto update_start_time = std::chrono::high_resolution_clock::now();
            insert_edge(index, e, directed);
            auto update_end_time = std::chrono::high_resolution_clock::now();
            update_time += std::chrono::duration_cast<std::chrono::nanoseconds>(update_end_time - update_start_time).count();
            insert_num++;
            have_inserted[idx] = true;
        }
    }
    std::cout << "update throughput = " << all_edge.size() * (size)10 / update_time * 1e9 << std::endl;
    std::cout << "insert num = " << insert_num << ", delete num = " << delete_num << std::endl;
    std::cout << "\n--- update edges finish ---\n" << std::endl;
}

void BfsTest(alex::Alex<vertex, payload>* index, vertex root) {
    auto algo_start_time = std::chrono::high_resolution_clock::now();
    bfs(index, root);
    auto algo_end_time = std::chrono::high_resolution_clock::now();
    auto algo_time = std::chrono::duration_cast<std::chrono::nanoseconds>(algo_end_time - algo_start_time).count();
    std::cout << "BFS time = " << algo_time / 1e9 << std::endl << std::endl;
}

void PagerankTest(alex::Alex<vertex, payload>* index, std::vector<vertex>& all_vertex) {
    auto algo_start_time = std::chrono::high_resolution_clock::now();
    pagerank(index, all_vertex);
    auto algo_end_time = std::chrono::high_resolution_clock::now();
    auto algo_time = std::chrono::duration_cast<std::chrono::nanoseconds>(algo_end_time - algo_start_time).count();
    std::cout << "PageRank time = " << algo_time / 1e9 << std::endl << std::endl;
}

int main(int argc, char* argv[]) {
    std::map<std::string, std::string> flags;
    if (argc != 1) {
        flags = parse_flags(argc, argv);
    } else {
        int option_num = 0;
        std::string* opt = ReadOptionFromFile(option_num);
        flags = parse_flags(option_num, opt);
    }
    std::string graph_file_path = get_required(flags, "graph_file");
    SECONDARY_INDEX_THRESHOLD = stoi(get_required(flags, "threshold"));
    bool directed = false;
    if (get_required(flags, "directed") == "true") {
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
        if (!LoadDiredtedGraph(graph_file_path, all_edge, all_vertex, vertex_deg, total_num_edges, load_num_edges)) {
            std::cout << "file cannot open" << std::endl;
            return -1;
        }
    }
    else {
        if (!LoadUndirectedGraphNoCheck(graph_file_path, all_edge, all_vertex, vertex_deg, total_num_edges, load_num_edges)) {
            std::cout << "file cannot open" << std::endl;
            return -1;
        }
    }
    // shuffle(all_edge);
    // print_graph(all_edge, "data/Graph500-24.txt");
    std::cout << "--- Load graph " << graph_file_path << " finish ---\n" << std::endl;

    alex::Alex<vertex, payload>* index = new alex::Alex<vertex, payload>();
    Insert(all_edge, index, directed);
    //Update(all_edge, index, directed);
    Lookup(all_edge, index, directed);
    //BfsTest(index, 0);
    //PagerankTest(index, all_vertex);
    //print_stats(index);
    Delete_(all_edge, index, directed);

#ifdef TIME
    std::cout << "insert time = " << insert_arr_time / 1e9 << ", " << insert_secindex_time / 1e9 << std::endl;
    std::cout << "lookup time = " << lookup_arr_time / 1e9 << ", " << lookup_secindex_time / 1e9 << std::endl;
    std::cout << "delete time = " << delete_arr_time / 1e9 << ", " << delete_secindex_time / 1e9 << std::endl;
    std::cout << "secondary bulk load time = " << secindex_bulkload_time / 1e9 << std::endl << std::endl;

    std::cout << "lookup in arr = " << num_lookup_arr << std::endl;
    std::cout << "lookup in first index = " << num_lookup_fir_idx << std::endl;
    std::cout << "lookup in sec index = " << num_lookup_sec_idx << std::endl;
    std::cout << "num get child lookup = " << num_get_child_lookup << std::endl << std::endl;

    std::cout << "delete in arr = " << num_delete_arr << std::endl;
    std::cout << "delete in first index = " << num_delete_fir_idx << std::endl;
    std::cout << "delete in sec index = " << num_delete_sec_idx << std::endl;
    std::cout << "num get child delete = " << num_get_child_delete << std::endl;
    std::cout << "num_delete_fill = " << num_delete_fill << std::endl << std::endl;

    std::cout << "num_scan = " << num_scan << std::endl << std::endl;
#endif // TIME

    return 0;
}
