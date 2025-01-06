#pragma once
#include "graph.h"
#include "utils.h"

namespace graph_query0 {
#pragma region bulk_load
	//// 从 left ~ right - 1，创建二级索引，返回索引指针
	//alex::Alex<vertex, vertex>* construct_secondary_index(edge* load_value, int left, int right) {
	//	alex::Alex<vertex, vertex>* index = new alex::Alex<vertex, vertex>();
	//	std::pair<vertex, vertex>* bulk_load_values = new std::pair<vertex, vertex>[right - left];
	//	for (int i = left; i < right; i++) {
	//		bulk_load_values[i - left].first = load_value[i].dst;
	//		bulk_load_values[i - left].second = load_value[i].dst;
	//	}
	//	index->bulk_load(bulk_load_values, right - left);
	//	return index;
	//}

	//// 为 all_edge 中的前 init_num_edges 条边创建索引，二级索引同样使用 Alex
	//void bulk_load(alex::Alex<vertex, payload>* index, std::vector<edge>& all_edge, int init_num_edges) {
	//	if (init_num_edges == 0) {
	//		return;
	//	}
	//	// 需要插入的边
	//	edge* load_values = new edge[init_num_edges];
	//	for (int i = 0; i < init_num_edges; i++) {
	//		load_values[i] = all_edge[i];
	//	}
	//	std::sort(load_values, load_values + init_num_edges);
	//	// 用于 bulk load 的数据
	//	std::vector<std::pair<vertex, payload>> load_values_vector;
	//	int left = 0, right = 0;
	//	while (right < init_num_edges) {
	//		while (right < init_num_edges && load_values[left].src == load_values[right].src) {
	//			right++;
	//		}
	//		if (right - left >= SECONDARY_INDEX_THRESHOLD) {
	//			// 从 left ~ right-1，创建二级索引，返回索引指针
	//			alex::Alex<vertex, vertex>* secondary_index = construct_secondary_index(load_values, left, right);
	//			load_values_vector.push_back(std::pair<vertex, payload>(load_values[left].src, payload(secondary_index, right - left)));
	//		}
	//		else {
	//			payload t;
	//			int idx = 0;
	//			for (int i = left; i < right; i++) {
	//				t.dst[idx] = load_values[i].dst;
	//				t.used[idx++] = true;
	//			}
	//			t.degree = idx;
	//			load_values_vector.push_back(std::pair<vertex, payload>(load_values[left].src, t));
	//		}
	//		left = right;
	//	}
	//	std::pair<vertex, payload>* do_load_values = new std::pair<vertex, payload>[load_values_vector.size()];
	//	for (int i = 0; i < load_values_vector.size(); i++) {
	//		do_load_values[i] = load_values_vector[i];
	//	}
	//	index->bulk_load(do_load_values, load_values_vector.size());
	//	delete[] load_values;
	//	delete[] do_load_values;
	//}
#pragma endregion bulk_load
	
	inline bool check_used(payload* b, int i) {
		return b->used[i];
	}
	inline void set_used(payload* b, int i) {
		b->used[i] = true;
	}
	inline void set_unused(payload* b, int i) {
		b->used[i] = false;
	}

	/***  Lookup  ***/

	bool lookup_edge(alex::Alex<vertex, payload>* index, vertex src, vertex dst) {
		payload* src_payload = nullptr;
#ifdef TIME
		auto start = std::chrono::high_resolution_clock::now();
#endif // TIME

		src_payload = index->get_payload(src, num_lookup_fir_idx);
#ifdef TIME
		auto end = std::chrono::high_resolution_clock::now();
		firindex_lookup_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME

		if (src_payload == nullptr) {
			return false;
		}
		alex::Alex<vertex, vertex>* sec_index = src_payload->sec_index;
		if (sec_index == nullptr) {
#ifdef TIME
			auto start = std::chrono::high_resolution_clock::now();
#endif
			for (int i = 0; i <= src_payload->max_pos; i++) {
#ifdef TIME
			num_lookup_arr++;
#endif
				if (check_used(src_payload, i) && src_payload->dst[i] == dst) {
#ifdef TIME
					auto end = std::chrono::high_resolution_clock::now();
					lookup_arr_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif
					return true;
				}
			}
			return false;
		}
		vertex* dst_payload = nullptr;
#ifdef TIME
		start = std::chrono::high_resolution_clock::now();
#endif // TIME

		dst_payload = sec_index->get_payload(dst, num_lookup_sec_idx, num_get_child_lookup);
#ifdef TIME
		end = std::chrono::high_resolution_clock::now();
		lookup_secindex_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME

		return dst_payload != nullptr;
	}

	bool lookup_edge(alex::Alex<vertex, payload>* index, edge e, bool directed) {
		if (lookup_edge(index, e.src, e.dst)) {
			return true;
		}
		if (directed) {
			return false;
		}
		return lookup_edge(index, e.dst, e.src);
	}

	/***  Insert  ***/

	bool convert_into_sec_index(payload* src_payload, vertex src, vertex dst) {
#ifdef TIME
		auto start = std::chrono::high_resolution_clock::now();
#endif // TIME

		src_payload->sec_index = new alex::Alex<vertex, vertex>();
		for (int i = 0; i < SECONDARY_INDEX_THRESHOLD - 1; i++) {
			if (src_payload->used[i]) {
				vertex dst1 = src_payload->dst[i];
				src_payload->sec_index->insert(std::pair<vertex, vertex>(dst1, dst1));
			}
		}
		src_payload->sec_index->insert(std::pair<vertex, vertex>(dst, dst));
#ifdef TIME
		auto end = std::chrono::high_resolution_clock::now();
		secindex_bulkload_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME
		delete[] src_payload->dst;
		delete[] src_payload->used;
		src_payload->dst = nullptr;
		src_payload->used = nullptr;
		return true;
	}

	bool insert_edge(alex::Alex<vertex, payload>* index, vertex src, vertex dst) {
		payload* src_payload = nullptr;
#ifdef TIME
		auto start = std::chrono::high_resolution_clock::now();
#endif // TIME

		src_payload = index->get_payload(src);
#ifdef TIME
		auto end = std::chrono::high_resolution_clock::now();
		firindex_lookup_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME

		// 没有顶点 src
		if (src_payload == nullptr) {
			payload t;
			if (SECONDARY_INDEX_THRESHOLD == 1) {
				// 直接建立二级索引
#ifdef TIME
				auto start = std::chrono::high_resolution_clock::now();
#endif // TIME

				t.sec_index = new alex::Alex<vertex, vertex>();
				t.sec_index->insert(std::pair<vertex, vertex>(dst, dst));
				vertex_num_have_sec_index++;
#ifdef TIME
				auto end = std::chrono::high_resolution_clock::now();
				insert_secindex_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME

			}
			else {
				t.dst = new vertex[SECONDARY_INDEX_THRESHOLD - 1];
				t.used = new bool[SECONDARY_INDEX_THRESHOLD - 1];
				t.dst[0] = dst;
				set_used(&t, 0);
			}
#ifdef TIME
			auto start = std::chrono::high_resolution_clock::now();
#endif // TIME

			std::pair<alex::Alex<vertex, payload>::Iterator, bool> ans = index->insert(std::pair<vertex, payload>(src, t));
#ifdef TIME
			auto end = std::chrono::high_resolution_clock::now();
			firindex_insert_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME

			return ans.second;
		}
		alex::Alex<vertex, vertex>* sec_index = src_payload->sec_index;
		src_payload->degree++;
		if (sec_index == nullptr) {
			if (src_payload->degree >= SECONDARY_INDEX_THRESHOLD) {
				// 转二级索引
				assert(src_payload->degree == SECONDARY_INDEX_THRESHOLD);
				vertex_num_have_sec_index++;
				return convert_into_sec_index(src_payload, src, dst);
			}
#ifdef TIME
			start = std::chrono::high_resolution_clock::now();
#endif // TIME

			int pos = -1;
			for (int i = 0; i <= src_payload->max_pos; i++) {
				if (!check_used(src_payload, i)) {
					pos = i;
				}
				else {
					if (src_payload->dst[i] == dst) {
						return false;
					}
				}
			}
			if (pos != -1) {
				src_payload->dst[pos] = dst;
				set_used(src_payload, pos);
			}
			else {
				src_payload->max_pos++;
				src_payload->dst[src_payload->max_pos] = dst;
				set_used(src_payload, src_payload->max_pos);
			}
#ifdef TIME
			end = std::chrono::high_resolution_clock::now();
			insert_arr_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME
			return true;
		}
		else {
#ifdef TIME
			start = std::chrono::high_resolution_clock::now();
#endif // TIME
			std::pair<alex::Alex<vertex, vertex>::Iterator, bool> ans = sec_index->insert(std::pair<vertex, vertex>(dst, dst));
#ifdef TIME
			end = std::chrono::high_resolution_clock::now();
			insert_secindex_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME
			return ans.second;
		}
	}

	bool insert_edge(alex::Alex<vertex, payload>* index, edge e, bool directed) {
		if (!directed) {
			return insert_edge(index, e.src, e.dst) && insert_edge(index, e.dst, e.src);
		}
		return insert_edge(index, e.src, e.dst);
	}

	/*** Delete ***/

	bool delete_edge(alex::Alex<vertex, payload>* index, vertex src, vertex dst) {
		payload* src_payload = nullptr;
#ifdef TIME
		auto start = std::chrono::high_resolution_clock::now();
#endif // TIME

		src_payload = index->get_payload(src, num_delete_fir_idx);
#ifdef TIME
		auto end = std::chrono::high_resolution_clock::now();
		firindex_lookup_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME

		if (src_payload == nullptr) {
			return false;
		}
		src_payload->degree--;
		if (src_payload->sec_index == nullptr) {
#ifdef TIME
			start = std::chrono::high_resolution_clock::now();
#endif // TIME
			for (int i = 0; i <= src_payload->max_pos; i++) {
#ifdef TIME
			num_delete_arr++;
#endif
				if (check_used(src_payload, i) && src_payload->dst[i] == dst) {
					set_unused(src_payload, i);
					if (i == src_payload->max_pos) {
						src_payload->max_pos--;
					}
#ifdef TIME
					auto end = std::chrono::high_resolution_clock::now();
					delete_arr_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME
					return true;
				}
			}
			return false;
		}
#ifdef TIME
		start = std::chrono::high_resolution_clock::now();
#endif // TIME
		int t = src_payload->sec_index->erase_one(dst, num_delete_sec_idx, num_delete_fill, num_get_child_delete);
#ifdef TIME
		end = std::chrono::high_resolution_clock::now();
		delete_secindex_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME
		return t == 1;
	}

	bool delete_edge(alex::Alex<vertex, payload>* index, edge e, bool directed) {
		if (!directed) {
			return delete_edge(index, e.src, e.dst) && delete_edge(index, e.dst, e.src);
		}
		return delete_edge(index, e.src, e.dst);
	}
}

namespace graph_query1 {
	inline bool check_used(payload* b, int i) {
		return b->used[i];
	}
	inline void set_used(payload* b, int i) {
		b->used[i] = true;
	}
	inline void set_unused(payload* b, int i) {
		b->used[i] = false;
	}

	/***  Lookup  ***/

	bool lookup_edge(alex::Alex<vertex, payload>* index, vertex src, vertex dst) {
		payload* src_payload = nullptr;
#ifdef TIME
		auto start = std::chrono::high_resolution_clock::now();
#endif // TIME

		src_payload = index->get_payload(src, num_lookup_fir_idx);
#ifdef TIME
		auto end = std::chrono::high_resolution_clock::now();
		firindex_lookup_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME

		if (src_payload == nullptr) {
			return false;
		}
		alex::Alex<vertex, vertex>* sec_index = src_payload->sec_index;
		if (sec_index == nullptr) {
#ifdef TIME
			auto start = std::chrono::high_resolution_clock::now();
#endif
			for (int i = 0; i <= src_payload->max_pos; i++) {
#ifdef TIME
			num_lookup_arr++;
#endif
				if (check_used(src_payload, i) && src_payload->dst[i] == dst) {
#ifdef TIME
					auto end = std::chrono::high_resolution_clock::now();
					lookup_arr_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif
					return true;
				}
			}
			return false;
		}
		vertex* dst_payload = nullptr;
#ifdef TIME
		start = std::chrono::high_resolution_clock::now();
#endif // TIME

		dst_payload = sec_index->get_payload(dst, num_lookup_sec_idx, num_get_child_lookup);
#ifdef TIME
		end = std::chrono::high_resolution_clock::now();
		lookup_secindex_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME

		return dst_payload != nullptr;
	}

	bool lookup_edge(alex::Alex<vertex, payload>* index, edge e, bool directed) {
		if (lookup_edge(index, e.src, e.dst)) {
			return true;
		}
		if (directed) {
			return false;
		}
		return lookup_edge(index, e.dst, e.src);
	}

	/***  Insert  ***/

	bool convert_into_sec_index(payload* src_payload, vertex src, vertex dst) {
#ifdef TIME
		auto start = std::chrono::high_resolution_clock::now();
#endif // TIME

		src_payload->sec_index = new alex::Alex<vertex, vertex>();
		for (int i = 0; i < SECONDARY_INDEX_THRESHOLD - 1; i++) {
			if (src_payload->used[i]) {
				vertex dst1 = src_payload->dst[i];
				if (dst1 == dst) {
					src_payload->degree--;
					num_edge--;
					delete src_payload->sec_index;
					src_payload->sec_index = nullptr;
					return false;
				}
				src_payload->sec_index->insert(std::pair<vertex, vertex>(dst1, dst1));
			}
		}
		src_payload->sec_index->insert(std::pair<vertex, vertex>(dst, dst));
#ifdef TIME
		auto end = std::chrono::high_resolution_clock::now();
		secindex_bulkload_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME
		delete[] src_payload->dst;
		delete[] src_payload->used;
		src_payload->dst = nullptr;
		src_payload->used = nullptr;
		return true;
	}

	bool insert_edge(alex::Alex<vertex, payload>* index, vertex src, vertex dst) {
		num_edge++;
		payload* src_payload = nullptr;
#ifdef TIME
		auto start = std::chrono::high_resolution_clock::now();
#endif // TIME

		src_payload = index->get_payload(src);
#ifdef TIME
		auto end = std::chrono::high_resolution_clock::now();
		firindex_lookup_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME

		// 没有顶点 src
		if (src_payload == nullptr) {
			num_vertex++;
			avg_deg = num_edge / num_vertex;
			payload t;
			if (SECONDARY_INDEX_THRESHOLD == 1) {
				// 直接建立二级索引
#ifdef TIME
				auto start = std::chrono::high_resolution_clock::now();
#endif // TIME

				t.sec_index = new alex::Alex<vertex, vertex>();
				t.sec_index->insert(std::pair<vertex, vertex>(dst, dst));
				vertex_num_have_sec_index++;
#ifdef TIME
				auto end = std::chrono::high_resolution_clock::now();
				insert_secindex_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME

			}
			else {
				t.size = std::min(avg_deg, SECONDARY_INDEX_THRESHOLD - 1);
				t.dst = new vertex[t.size];
				t.used = new bool[t.size];
				t.dst[0] = dst;
				set_used(&t, 0);
			}
#ifdef TIME
			auto start = std::chrono::high_resolution_clock::now();
#endif // TIME

			std::pair<alex::Alex<vertex, payload>::Iterator, bool> ans = index->insert(std::pair<vertex, payload>(src, t));
#ifdef TIME
			auto end = std::chrono::high_resolution_clock::now();
			firindex_insert_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME

			return ans.second;
		}
		alex::Alex<vertex, vertex>* sec_index = src_payload->sec_index;
		src_payload->degree++;
		if (sec_index == nullptr) {
			if (src_payload->degree >= SECONDARY_INDEX_THRESHOLD) {
				// 转二级索引
				assert(src_payload->degree == SECONDARY_INDEX_THRESHOLD);
				// vertex_num_have_sec_index++;
				return convert_into_sec_index(src_payload, src, dst);
			}
#ifdef TIME
			start = std::chrono::high_resolution_clock::now();
#endif // TIME

			int pos = -1;
			for (int i = 0; i <= src_payload->max_pos; i++) {
				if (!check_used(src_payload, i)) {
					pos = i;
				}
				else {
					if (src_payload->dst[i] == dst) {
						src_payload->degree--;
						num_edge--;
						return false;
					}
				}
			}
			if (pos != -1) {
				src_payload->dst[pos] = dst;
				set_used(src_payload, pos);
			}
			else {
				src_payload->max_pos++;
				if (src_payload->max_pos >= src_payload->size) {
					// 这个数组已满
					int new_size = std::min(src_payload->size * 2, SECONDARY_INDEX_THRESHOLD - 1);
					vertex* new_dst = new vertex[new_size];
					bool* new_used = new bool[new_size];
					memcpy(new_dst, src_payload->dst, sizeof(vertex) * src_payload->size);
					memcpy(new_used, src_payload->used, sizeof(bool) * src_payload->size);
					delete[] src_payload->dst;
					delete[] src_payload->used;
					src_payload->size = new_size;
					src_payload->dst = new_dst;
					src_payload->used = new_used;
				}
				src_payload->dst[src_payload->max_pos] = dst;
				set_used(src_payload, src_payload->max_pos);
			}
#ifdef TIME
			end = std::chrono::high_resolution_clock::now();
			insert_arr_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME
			return true;
		}
		else {
#ifdef TIME
			start = std::chrono::high_resolution_clock::now();
#endif // TIME
			std::pair<alex::Alex<vertex, vertex>::Iterator, bool> ans = sec_index->insert(std::pair<vertex, vertex>(dst, dst));
#ifdef TIME
			end = std::chrono::high_resolution_clock::now();
			insert_secindex_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME
			return ans.second;
		}
	}

	bool insert_edge(alex::Alex<vertex, payload>* index, edge e, bool directed) {
		if (!directed) {
			return insert_edge(index, e.src, e.dst) && insert_edge(index, e.dst, e.src);
		}
		return insert_edge(index, e.src, e.dst);
	}

	/*** Delete ***/

	bool delete_edge(alex::Alex<vertex, payload>* index, vertex src, vertex dst) {
		payload* src_payload = nullptr;
#ifdef TIME
		auto start = std::chrono::high_resolution_clock::now();
#endif // TIME

		src_payload = index->get_payload(src, num_delete_fir_idx);
#ifdef TIME
		auto end = std::chrono::high_resolution_clock::now();
		firindex_lookup_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME

		if (src_payload == nullptr) {
			return false;
		}
		src_payload->degree--;
		if (src_payload->sec_index == nullptr) {
#ifdef TIME
			start = std::chrono::high_resolution_clock::now();
#endif // TIME
			for (int i = 0; i <= src_payload->max_pos; i++) {
#ifdef TIME
			num_delete_arr++;
#endif
				if (check_used(src_payload, i) && src_payload->dst[i] == dst) {
					set_unused(src_payload, i);
					if (i == src_payload->max_pos) {
						src_payload->max_pos--;
					}
#ifdef TIME
					auto end = std::chrono::high_resolution_clock::now();
					delete_arr_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME
					return true;
				}
			}
			return false;
		}
#ifdef TIME
		start = std::chrono::high_resolution_clock::now();
#endif // TIME
		int t = src_payload->sec_index->erase_one(dst, num_delete_sec_idx, num_delete_fill, num_get_child_delete);
#ifdef TIME
		end = std::chrono::high_resolution_clock::now();
		delete_secindex_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif // TIME
		return t == 1;
	}

	bool delete_edge(alex::Alex<vertex, payload>* index, edge e, bool directed) {
		if (!directed) {
			return delete_edge(index, e.src, e.dst) && delete_edge(index, e.dst, e.src);
		}
		return delete_edge(index, e.src, e.dst);
	}
}