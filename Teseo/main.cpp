#include "graph.h"
#include "algorithm.h"
#include "insert.h"
#include "benchmark.h"

bool LoadUndiredtedGraph(const std::string file_path, std::vector<edge>& all_edge, 
                        std::vector<vertex>& all_vertex, std::unordered_map<vertex, int>& vertex_deg, size& total_num_edges, size load_num_edges) {
    std::ifstream f(file_path.c_str(), std::ios::in);
    if (!f.is_open()) {
        return false;
    }
    std::random_device seed;
	std::mt19937_64 engine(seed());
    std::uniform_real_distribution<double> rand(1, 1000);
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
        double c;
        f >> a >> b;
        if (a == b || set.find(edge{ a, b }) != set.end() || set.find(edge{ b,a }) != set.end()) {
            continue;
        }
        all_edge.push_back(edge{ a,b,rand(engine) });
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
    return true;
}


int main(int argc, char* argv[])
{
    if (argc > 1) {
        file_name = argv[1];
    }
	std::vector<edge> all_edge;
    std::vector<vertex> all_vertex;
    std::unordered_map<vertex, int> vertex_deg;
    size total_num_edges = 0;
    size load_num_edges = INF;
	if (!LoadUndiredtedGraph(file_name, all_edge, all_vertex, vertex_deg, total_num_edges, load_num_edges)) {
		std::cout << "cannot open file" << std::endl;
		return -1;
	}
    std::cout << "load graph finish" << std::endl;
    Graph g;
    /*benchmark_read_write(all_edge, g, false, all_vertex, 0.99 * all_edge.size(), 0);
    g.clear();
    std::cout << "\n" << std::endl;
    benchmark_read_write(all_edge, g, false, all_vertex, 0.99 * all_edge.size(), 1);
    g.clear();
    std::cout << "\n" << std::endl;
    benchmark_read_only(all_edge, g, false, all_vertex, 0.1);*/
	insert(all_edge, *g.db);    
    std::cout << "mem usage = " << g.size() / (1024*1024) << "MB" << std::endl;
    
    /* teseo::Teseo db;
     insert(all_edge, db);
     uint64_t max_deg = 0;
     uint64_t root = 0;
     auto tx = db.start_transaction();
     for (uint64_t i = 0; i < tx.num_vertices(); i++) {
         if (tx.degree(i, true) > max_deg) {
             max_deg = tx.degree(i, true);
             root = i;
         }
     }
     tx.commit();
     bfs(db, root);
     pagerank(db);
     LCC(db, root);
     CDLP(db, 5);
     connected_component(db);
     SSSP(db, root);*/
	return 0;
}
