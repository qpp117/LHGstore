#pragma once
#include "alex.h"
#include "alex_nodes.h"
#include "debug.h"
#include "alex_multimap.h"
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include <set>
#include <chrono>

typedef int64_t vertex;
typedef unsigned long long size;
const int INF = 1 << 30;

int SECONDARY_INDEX_THRESHOLD = INF;

// time
double insert_time = 0.0;
double lookup_time = 0.0;
double delete_time = 0.0;
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
size num_delete_fir_idx = 0;
size num_delete_fill = 0;
size num_scan = 0;

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
//namespace graph_query0 {
//	typedef struct payload {
//		vertex dst;
//		alex::Alex<vertex, vertex>* sec_index;
//		payload() {
//			dst = 0;
//			sec_index = nullptr;
//		}
//		payload(vertex dst, alex::Alex<vertex, vertex>* secondary_index) {
//			this->dst = dst;
//			this->sec_index = secondary_index;
//		}
//	}payload;
//
//	class Iterator {
//	private:
//		// һ������
//		alex::Alex<vertex, payload>* index;
//		// ��ǰ���ʵ�һ�������е�Ҷ�ӽڵ�
//		alex::AlexDataNode<vertex, payload>* cur_leaf;
//		// ��ǰ���ʵ�һ������Ҷ�ӽڵ��λ��
//		// -1 ���� ��û��ʼ����
//		// -2 ���� ���ж��㶼��������
//		int cur_idx;
//		// ���� cur_leaf Ϊһ�������ĵ�һ��Ҷ�ӽڵ�
//		void init() {
//			alex::AlexNode<vertex, payload>* cur_node = index->root_node_;
//			while (!cur_node->is_leaf_) {
//				int i = 0;
//				for (i = 0; i < static_cast<alex::AlexModelNode<vertex, payload>*>(cur_node)->num_children_; i++) {
//					if (static_cast<alex::AlexModelNode<vertex, payload>*>(cur_node)->children_[i] != nullptr) {
//						break;
//					}
//				}
//				cur_node = static_cast<alex::AlexModelNode<vertex, payload>*>(cur_node)->children_[i];
//			}
//			cur_leaf = static_cast<alex::AlexDataNode<vertex, payload>*>(cur_node);
//			cur_idx = -1;
//		}
//		Iterator() {
//			index = nullptr;
//			cur_leaf = nullptr;
//			cur_idx = -1;
//		}
//		// �ҵ���һ�� non-gap ��λ��
//		void next_non_gap() {
//			while (cur_leaf != nullptr) {
//#ifdef TIME
//				num_scan++;
//#endif
//				// cur_leaf �е���һ�� non-gap λ��
//				while (cur_idx < cur_leaf->data_capacity_ && !cur_leaf->check_exists(cur_idx)) {
//#ifdef TIME
//				num_scan++;
//#endif
//					cur_idx++;
//				}
//				if (cur_idx >= cur_leaf->data_capacity_) {
//					// ��� leaf �������ˣ�ȥ��һ�� leaf
//					cur_leaf = cur_leaf->next_leaf_;
//					cur_idx = 0;
//				}
//				else {
//					break;
//				}
//			}
//		}
//	public:
//		Iterator(alex::Alex<vertex, payload>* index) {
//			this->index = index;
//			cur_leaf = nullptr;
//			cur_idx = -1;
//			init();
//		}
//		
//		// �õ�һ��λ��һ�������� src λ���ϵ� Iterator
//		Iterator get_iterator(vertex src) {
//			Iterator ret;
//			ret.index = index;
//			ret.cur_leaf = index->get_leaf(src);
//			int pos = ret.cur_leaf->predict_position(src);
//			ret.cur_idx = ret.cur_leaf->find_upper(src - 1);
//			if (ret.cur_leaf == nullptr || ret.cur_idx >= ret.cur_leaf->data_capacity_ || ret.cur_idx < 0 || ret.cur_leaf->get_key(ret.cur_idx) != src) {
//				std::cout << "wrong nbr iterator of " << src << std::endl;
//			}
//			return ret;
//		}
//		
//		Iterator get_iterator() {
//			Iterator ret;
//			ret.index = this->index;
//			ret.cur_leaf = this->cur_leaf;
//			ret.cur_idx = this->cur_idx;
//			return ret;
//		}
//
//		void next_edge(vertex **src, vertex **dst) {
//			if (cur_idx == -2 || cur_leaf == nullptr) {
//				*src = nullptr;
//				*dst = nullptr;
//				return;
//			}
//			cur_idx++;
//			next_non_gap();
//			if (cur_leaf == nullptr) {
//				cur_idx = -2;
//				*src = nullptr;
//				*dst = nullptr;
//				return;
//			}
//			*src = &cur_leaf->get_key(cur_idx);
//			*dst = &cur_leaf->get_payload(cur_idx).dst;
//		}
//	};
//}


class Iterator {
public:
	alex::AlexMultimap<vertex, std::pair<vertex, double>>* index;
	alex::AlexMultimap<vertex, std::pair<vertex, double>>::iterator it;
	vertex cur_vertex;
	double cur_weight;
	bool last_nbr;
	Iterator() {
		index = nullptr;
		cur_vertex = -1;
		cur_weight = 0.0;
		last_nbr = false;
	}
	Iterator(alex::AlexMultimap<vertex, std::pair<vertex, double>>* index) {
		this->index = index;
		last_nbr = false;
		cur_vertex = -1;
		cur_weight = 0.0;
		this->it = index->begin();
	}

	Iterator(const Iterator& it) {
		this->index = it.index;
		this->it = it.it;
		this->last_nbr = it.last_nbr;
		this->cur_vertex = it.cur_vertex;
		this->cur_weight = it.cur_weight;
	}

	// �õ�һ���� src ΪԴ��� Iterator
	Iterator get_iterator(vertex src) {
		Iterator ret;
		ret.index = index;
		ret.it = index->lower_bound(src);
		ret.cur_vertex = ret.it.key();
		ret.cur_weight = it.payload().second;
		if (ret.cur_vertex != src) {
			ret.last_nbr = true;
		}
		return ret;
	}

	// ������һ������
	const vertex* next_vertex() {
		if (it.is_end()) {
			return nullptr;
		}
		last_nbr = false;
		if (cur_vertex != it.key()) {
			cur_vertex = it.key();
			return &it.key();
		}
		while (!it.is_end() && it.key() == cur_vertex) {
			it++;
		}
		if (it.is_end()) {
			return nullptr;
		}
		cur_vertex = it.key();
		cur_weight = it.payload().second;
		return &it.key();
	}

	// ���ص�ǰԴ�����һ���ھ�
	vertex* next_nbr() {
		if (it.is_end() || last_nbr) {
			return nullptr;
		}
		vertex* ans = &it.payload().first;
		cur_weight = it.payload().second;
		it++;
		if (it.is_end() || it.key() != cur_vertex) {
			last_nbr = true;
		}
		return ans;
	}

	double get_weight() {
		return cur_weight;
	}
};