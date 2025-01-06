#pragma once
#include "alex.h"
#include "alex_nodes.h"
#include "debug.h"
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include <set>
#include <chrono>

typedef uint64_t vertex;
typedef unsigned long long size;

size INF = 1 << 30;
int SECONDARY_INDEX_THRESHOLD = 100;
size num_scan = 0;
size num_vertex = 0;
size num_edge = 0;
int avg_deg = 0.0;

struct edge {
	vertex src;
	vertex dst;
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

struct payload {
	int degree = 1;
	int max_pos = 0;
	int size = 0;
	vertex* dst = nullptr;
	bool* used = nullptr;
	alex::Alex<vertex, vertex>* sec_index = nullptr;
};

class Iterator {
private:
	// 一级索引
	alex::Alex<vertex, payload>* index;
	// 当前访问的一级索引中的叶子节点
	alex::AlexDataNode<vertex, payload>* cur_leaf;
	// 当前访问的二级索引中的叶子节点
	alex::AlexDataNode<vertex, vertex>* sec_cur_leaf;
	// 一级索引当前位置上的 payload
	payload* p;
	// 当前访问的一级索引叶子节点的位置
	// -1 —— 还没开始迭代
	// -2 —— 所有顶点都遍历完了
	int cur_idx;
	// 当前访问的二级索引叶子节点的位置（或数组中的位置）
	// -1 —— 当前顶点的邻居迭代器还没创建
	// -2 —— 当前顶点的邻居都遍历完了
	int sec_cur_idx;
	// 设置 cur_leaf 为一级索引的第一个叶子节点
	void init() {
		alex::AlexNode<vertex, payload>* cur_node = index->root_node_;
		while (!cur_node->is_leaf_) {
			int i = 0;
			for (i = 0; i < static_cast<alex::AlexModelNode<vertex, payload>*>(cur_node)->num_children_; i++) {
				if (static_cast<alex::AlexModelNode<vertex, payload>*>(cur_node)->children_[i] != nullptr) {
					break;
				}
			}
			cur_node = static_cast<alex::AlexModelNode<vertex, payload>*>(cur_node)->children_[i];
		}
		cur_leaf = static_cast<alex::AlexDataNode<vertex, payload>*>(cur_node);
		cur_idx = -1;
	}
	Iterator() {
		index = nullptr;
		cur_leaf = nullptr;
		sec_cur_leaf = nullptr;
		p = nullptr;
		cur_idx = -1;
		sec_cur_idx = -1;
	}
public:
	Iterator(alex::Alex<vertex, payload>* index) {
		this->index = index;
		cur_leaf = nullptr;
		sec_cur_leaf = nullptr;
		p = nullptr;
		cur_idx = -1;
		sec_cur_idx = -1;
		init();
	}
	
	// 得到一个位于一级索引中 src 位置上的 Iterator
	Iterator get_iterator(vertex src) {
		Iterator ret;
		ret.index = index;
		ret.cur_leaf = index->get_leaf(src);
		ret.cur_idx = ret.cur_leaf->find_key(src);
		ret.sec_cur_idx = -1;
		ret.sec_cur_leaf = nullptr;
		ret.p = &ret.cur_leaf->get_payload(ret.cur_idx);
		return ret;
	}
	
	Iterator get_iterator() {
		Iterator ret;
		ret.index = this->index;
		ret.cur_leaf = this->cur_leaf;
		ret.cur_idx = this->cur_idx;
		ret.sec_cur_idx = -1;
		ret.sec_cur_leaf = nullptr;
		ret.p = this->p;
		return ret;
	}

	// 返回一级索引中的下一个顶点
	vertex* next_vertex() {
		if (cur_idx == -2 || cur_leaf == nullptr) {
			return nullptr;
		}
		cur_idx++;
		sec_cur_idx = -1;
		sec_cur_leaf = nullptr;
		p = nullptr;
		while (cur_leaf != nullptr) {
#ifdef TIME
			num_scan++;
#endif
			// cur_leaf 中的下一个 non-gap 位置
			while (cur_idx < cur_leaf->data_capacity_ && !cur_leaf->check_exists(cur_idx)) {
#ifdef TIME
			num_scan++;
#endif
				cur_idx++;
			}
			if (cur_idx >= cur_leaf->data_capacity_) {
				// 这个 leaf 遍历完了，去下一个 leaf
				cur_leaf = cur_leaf->next_leaf_;
				cur_idx = 0;
			}
			else {
				break;
			}
		}
		if (cur_leaf == nullptr) {
			cur_idx = -2;
			return nullptr;
		}
		p = &cur_leaf->get_payload(cur_idx);
		return &cur_leaf->get_key(cur_idx);
	}

	// 返回当前源点的下一个邻居
	vertex* next_nbr() {
		if (cur_idx == -1 || sec_cur_idx == -2 || cur_leaf == nullptr) {
			return nullptr;
		}
		if (p->sec_index == nullptr) {
#ifdef TIME
			num_scan++;
#endif
			// 无二级索引，遍历数组
			sec_cur_idx++;
			while (sec_cur_idx <= p->max_pos && !p->used[sec_cur_idx]) {
#ifdef TIME
				num_scan++;
#endif
				sec_cur_idx++;
			}
			if (sec_cur_idx > p->max_pos) {
				// 已经遍历完了
				sec_cur_idx = -2;
				return nullptr;
			}
			return &p->dst[sec_cur_idx];
		}
		else {
			// 有二级索引
			if (sec_cur_leaf == nullptr) {
				// 先要找到第一个叶子节点
				alex::AlexNode<vertex, vertex>* cur_node = p->sec_index->root_node_;
				while (!cur_node->is_leaf_) {
					int i = 0;
					for (i = 0; i < static_cast<alex::AlexModelNode<vertex, vertex>*>(cur_node)->num_children_; i++) {
						if (static_cast<alex::AlexModelNode<vertex, vertex>*>(cur_node)->children_[i] != nullptr) {
							break;
						}
					}
					cur_node = static_cast<alex::AlexModelNode<vertex, vertex>*>(cur_node)->children_[i];
				}
				sec_cur_leaf = static_cast<alex::AlexDataNode<vertex, vertex>*>(cur_node);
			}
			sec_cur_idx++;
			while (sec_cur_leaf != nullptr) {
#ifdef TIME
				num_scan++;
#endif
				while (sec_cur_idx < sec_cur_leaf->data_capacity_ && !sec_cur_leaf->check_exists(sec_cur_idx)) {
#ifdef TIME
					num_scan++;
#endif
					sec_cur_idx++;
				}
				if (sec_cur_idx >= sec_cur_leaf->data_capacity_) {
					sec_cur_leaf = sec_cur_leaf->next_leaf_;
					sec_cur_idx = 0;
				} 
				else {
					break;
				}
			}
			if (sec_cur_leaf == nullptr) {
				sec_cur_idx = -2;
				return nullptr;
			}
			return &sec_cur_leaf->get_key(sec_cur_idx);
		}
	}

	// 返回当前源点的度
	int get_degree() {
		return p->degree;
	}
};

// time
double insert_time = 0.0;
double lookup_time = 0.0;
double delete_time = 0.0;
double update_time = 0.0;
double total_time = 0.0;
double lookup_arr_time = 0.0;
double insert_arr_time = 0.0;
double delete_arr_time = 0.0;
double lookup_secindex_time = 0.0;
double insert_secindex_time = 0.0;
double delete_secindex_time = 0.0;
double secindex_bulkload_time = 0.0;
double firindex_lookup_time = 0.0;
double firindex_insert_time = 0.0;
double phase1_time = 0.0;
double phase2_time = 0.0;
double phase3_time = 0.0;
int phase1_edge_num = 0;
int phase2_edge_num = 0;
int phase3_edge_num = 0;

// stats
int num_expand_and_scales = 0;
int num_expand_and_retrains = 0;
int num_downward_splits = 0;
int num_sideways_splits = 0;
int num_model_node_expansions = 0;
double splitting_time = 0.0;
double cost_computation_time = 0.0;
size num_shifts = 0;
size num_exp_search_iterations = 0;
int num_leaf = 0;
int sec_index_num_leaf = 0;

int vertex_num_need_sec_index = 0;
int vertex_num_have_sec_index = 0;

size num_lookup_fir_idx = 0;
size num_lookup_arr = 0;
size num_lookup_sec_idx = 0;
size num_delete_fir_idx = 0;
size num_delete_arr = 0;
size num_delete_sec_idx = 0;
size num_delete_fill = 0;
size num_get_child_lookup = 0;
size num_get_child_delete = 0;