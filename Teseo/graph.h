#pragma once

#include "teseo.hpp"
#include "teseo/memstore/memstore.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <set>
#include <chrono>
#include <unordered_set>
#include <queue>
#include <unordered_map>
#include <random>

typedef uint64_t vertex;
typedef uint64_t size;

std::string file_name = "slashdot.txt";
double insert_time = 0;
double lookup_time = 0;
double delete_time = 0;
size INF = (uint64_t)1 << 40;

class edge {
public:
	vertex src;
	vertex dst;
	double weight;
	bool operator==(const edge& e) const {
		if (src == e.src && dst == e.dst) {
			return true;
		}
		return false;
	}
	bool operator!=(const edge& e) const {
		if (src != e.src || dst != e.dst) {
			return true;
		}
		return false;
	}
	bool operator<(const edge& e) const {
		if (src == e.src) {
			return dst < e.dst;
		}
		return src < e.src;
	}
	bool operator>(const edge& e) const {
		if (src == e.src) {
			return dst > e.src;
		}
		return src > e.src;
	}
};

class Graph {
public:
    teseo::Teseo* db;
    Graph() {
        db = new teseo::Teseo();
    }
    void clear() {
        delete db;
        db = new teseo::Teseo();
    }
    bool insert_edge(edge& e) {
        //std::cout << "insert edge (" << e.src << ", " << e.dst  << ")" << std::endl;
        teseo::Transaction tx = db->start_transaction();
        try {
            tx.insert_edge(e.src, e.dst, e.weight);
            tx.commit();
            return true;
        } catch(teseo::Exception &e) {
            //tx.rollback();
            return false;
        }
    }

    bool insert_vertex(vertex u) {
        teseo::Transaction tx = db->start_transaction();
        try {
            tx.insert_vertex(u);
            tx.commit();
            return true;
        } catch(teseo::Exception &e) {
            //tx.rollback();
            return false;
        }
    }

    bool lookup_edge(edge& e) {
        //std::cout << "lookup edge (" << e.src << ", " << e.dst  << ")" << std::endl;
        teseo::Transaction tx = db->start_transaction();
        return tx.has_edge(e.src, e.dst);
    }

    bool lookup_vertex(vertex u) {
        //std::cout << "lookup vertex " << u << std::endl;
        teseo::Transaction tx = db->start_transaction();
        return tx.has_vertex(u);
    }

    int get_neighbor(vertex u) {
        //std::cout << "get nbr " << u << std::endl;
        teseo::Transaction tx = db->start_transaction();
        int ans = 0;
        teseo::Iterator it = tx.iterator();
        try {
            it.edges(u, false, [&](vertex v) {
                ans++;
            });
            it.close();
            tx.commit();
        } catch(teseo::Exception &e) {
            if (it.is_open()) {
                it.close();
            }
            //tx.rollback();
            return ans;
        }
        return ans;
    }

    bool delete_edge(edge& e) {
        //std::cout << "delete edge (" << e.src << ", " << e.dst  << ")" << std::endl;
        teseo::Transaction tx = db->start_transaction();
        try {
            tx.remove_edge(e.src, e.dst);
            tx.commit();
            return true;
        } catch(teseo::Exception &e) {
            //tx.rollback();
            return false;
        }
    }

    bool delete_vertex(vertex u) {
        //std::cout << "delete vertex " << u << std::endl;
        teseo::Transaction tx = db->start_transaction();
        try {
            tx.remove_vertex(u);
            tx.commit();
            return true;
        } catch(teseo::Exception &e) {
            //tx.rollback();
            return false;
        }
    }

    uint64_t size() {
        return teseo::context::global_context()->memstore()->size();
    }
};

// class Graph {
// public:
//     teseo::Teseo* db = new teseo::Teseo();
//     teseo::Transaction tx = db->start_transaction();
//     Graph() {}
//     void clear() {
//         tx.rollback();
//         tx.~Transaction();
//         delete db;
//         db = new teseo::Teseo();
//         tx = db->start_transaction();
//     }
//     bool insert_edge(edge& e) {
//         //std::cout << "insert edge (" << e.src << ", " << e.dst  << ")" << std::endl;
//         try {
//             if (!tx.has_vertex(e.src)) {
//                 tx.insert_vertex(e.src);
//             }
//             if (!tx.has_vertex(e.dst)) {
//                 tx.insert_vertex(e.dst);
//             }
//             tx.insert_edge(e.src, e.dst, e.weight);
//             return true;
//         } catch(teseo::Exception &e) {
//             return false;
//         }
//     }

//     bool insert_vertex(vertex u) {
//         try {
//             tx.insert_vertex(u);
//             return true;
//         } catch(teseo::Exception &e) {
//             return false;
//         }
//     }

//     bool lookup_edge(edge& e) {
//         //std::cout << "lookup edge (" << e.src << ", " << e.dst  << ")" << std::endl;
//         bool ans = tx.has_edge(e.src, e.dst);
//         return ans;
//     }

//     bool lookup_vertex(vertex u) {
//         //std::cout << "lookup vertex " << u << std::endl;
//         bool ans = tx.has_vertex(u);
//         return ans;
//     }

//     int get_neighbor(vertex u) {
//         //std::cout << "get nbr " << u << std::endl;
//         int ans = 0;
//         teseo::Iterator it = tx.iterator();
//         try {
//             it.edges(u, false, [&](vertex v) {
//                 ans++;
//             });
//             it.close();
//             return ans;
//         } catch(teseo::Exception &e) {
//             if (it.is_open()) {
//                 it.close();
//             }
//             return ans;
//         }
//     }

//     bool delete_edge(edge& e) {
//         //std::cout << "delete edge (" << e.src << ", " << e.dst  << ")" << std::endl;
//         try {
//             tx.remove_edge(e.src, e.dst);
//             return true;
//         } catch(teseo::Exception &e) {
//             return false;
//         }
//     }

//     bool delete_vertex(vertex u) {
//         //std::cout << "delete vertex " << u << std::endl;
//         try {
//             tx.remove_vertex(u);
//             return true;
//         } catch(teseo::Exception &e) {
//             return false;
//         }
//     }

//     uint64_t size() {
//         return teseo::context::global_context()->memstore()->size();
//     }
// };
