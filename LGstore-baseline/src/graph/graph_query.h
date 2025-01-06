#pragma once
#include "graph.h"
#include "utils.h"

//namespace graph_query0 {
//	/***  Bulk Load  ***/
//
//	// 从 left ~ right - 1，创建二级索引，返回索引指针
//	alex::Alex<vertex, vertex>* construct_secondary_index(edge* load_value, int left, int right) {
//		alex::Alex<vertex, vertex>* index = new alex::Alex<vertex, vertex>();
//		std::pair<vertex, vertex>* bulk_load_values = new std::pair<vertex, vertex>[right - left];
//		for (int i = left; i < right; i++) {
//			bulk_load_values[i - left].first = load_value[i].dst;
//			bulk_load_values[i - left].second = load_value[i].dst;
//		}
//		index->bulk_load(bulk_load_values, right - left);
//		return index;
//	}
//
//	// 为 all_edge 中的前 init_num_edges 条边创建索引，二级索引同样使用 Alex
//	void bulk_load(alex::Alex<vertex, payload>* index, std::vector<edge>& all_edge, int init_num_edges) {
//		if (init_num_edges == 0) {
//			return;
//		}
//		// 需要插入的边
//		edge* load_values = new edge[init_num_edges];
//		for (int i = 0; i < init_num_edges; i++) {
//			load_values[i] = all_edge[i];
//		}
//		std::sort(load_values, load_values + init_num_edges);
//		// 用于 bulk load 的数据
//		std::vector<std::pair<vertex, payload>> load_values_vector;
//		int left = 0, right = 0;
//		while (right < init_num_edges) {
//			while (right < init_num_edges && load_values[left].src == load_values[right].src) {
//				right++;
//			}
//			if (right - left >= SECONDARY_INDEX_THRESHOLD) {
//				// 从 left~right - 1，创建二级索引，返回索引指针
//				alex::Alex<vertex, vertex>* secondary_index = construct_secondary_index(load_values, left, right);
//				load_values_vector.push_back(std::pair<vertex, payload>(load_values[left].src, payload{ 0, secondary_index }));
//			}
//			else {
//				for (int i = left; i < right; i++) {
//					load_values_vector.push_back(std::pair<vertex, payload>(load_values[i].src, payload{ load_values[i].dst, nullptr }));
//				}
//			}
//			left = right;
//		}
//		std::pair<vertex, payload>* do_load_values = new std::pair<vertex, payload>[load_values_vector.size()];
//		for (int i = 0; i < load_values_vector.size(); i++) {
//			do_load_values[i] = load_values_vector[i];
//		}
//		index->bulk_load(do_load_values, load_values_vector.size());
//		delete[] load_values;
//		delete[] do_load_values;
//	}
//
//	/***  Lookup  ***/
//
//	bool lookup_edge_in_secondary_index(alex::Alex<vertex, vertex>* index, vertex dst) {
//		vertex* ans = index->get_payload(dst);
//		return ans != nullptr;
//	}
//
//	// 比较 index 中存储的一条数据和查询数据
//	// return 0，找到了要查询的数据
//	// return -1，leaf 中这个位置比要查询的数据小
//	// return 1，leaf 中这个位置比要查询的数据大
//	// return 2，找到了对应的 src 位置，且这里有一个二级索引
//	int compare_slot_and_query(vertex key, payload value, vertex src, vertex dst, size& num_lookup) {
//#ifdef TIME
//		num_lookup++;
//#endif
//		if (key == src && value.sec_index != nullptr) {
//			return 2;
//		}
//		if (key == src && value.dst == dst) {
//			return 0;
//		}
//		if (key < src || (key == src && value.dst < dst)) {
//			return -1;
//		}
//		if (key > src || (key == src && value.dst > dst)) {
//			return 1;
//		}
//	}
//
//	// 在 [l,r) 中寻找边 (src, dst)，返回在 leaf 中的位置
//	// >=0 —— 在 gapped array 中的位置（这个位置可能是二级索引）
//	// -1 —— 没有这条边
//	int binary_search(alex::AlexDataNode<vertex, payload>* leaf, int l, int r, vertex src, vertex dst, size& num_lookup) {
//		while (l <= r) {
//			int mid = l + (r - l) / 2;
//			int cmp = compare_slot_and_query(leaf->get_key(mid), leaf->get_payload(mid), src, dst, num_lookup);
//			if (cmp == 2) {
//				bool have = lookup_edge_in_secondary_index(leaf->get_payload(mid).sec_index, dst);
//				if (!have) {
//					return -1;
//				}
//				return mid;
//			}
//			else if (cmp == 0) {
//				return mid;
//			}
//			else if (cmp == 1) {
//				r = mid - 1;
//			}
//			else {
//				l = mid + 1;
//			}
//		}
//		return -1;
//	}
//
//	int lookup_edge_in_leaf(alex::AlexDataNode<vertex, payload>* leaf, vertex src, vertex dst, size& num_lookup) {
//		int predict_pos = leaf->predict_position(src);
//		int bound = 1, size = 0, l = 0, r = 0, cmp = 0;
//		leaf->num_lookups_++;
//		bool have;
//		switch (compare_slot_and_query(leaf->get_key(predict_pos), leaf->get_payload(predict_pos), src, dst, num_lookup))
//		{
//		case 0:
//			return predict_pos;
//		case 1:
//			size = predict_pos;
//			cmp = 1;
//			while (bound <= size && cmp == 1) {
//				cmp = compare_slot_and_query(leaf->get_key(predict_pos - bound), leaf->get_payload(predict_pos - bound),src, dst, num_lookup);
//				if (cmp != 1) {
//					break;
//				}
//				bound *= 2;
//				leaf->num_exp_search_iterations_++;
//			}
//			if (cmp == 0) {
//				return predict_pos - bound;
//			}
//			if (cmp == 2) {
//				have = lookup_edge_in_secondary_index(leaf->get_payload(predict_pos - bound).sec_index, dst);
//				if (!have) {
//					return -1;
//				}
//				return predict_pos - bound;
//			}
//			l = predict_pos - std::min<int>(bound, size);
//			r = predict_pos - bound / 2;
//			break;
//		case -1:
//			size = leaf->data_capacity_ - predict_pos;
//			cmp = -1;
//			while (bound < size && cmp == -1) {
//				cmp = compare_slot_and_query(leaf->get_key(predict_pos + bound), leaf->get_payload(predict_pos + bound),
//					src, dst, num_lookup);
//				if (cmp != -1) {
//					break;
//				}
//				bound *= 2;
//				leaf->num_exp_search_iterations_++;
//			}
//			if (cmp == 0) {
//				return predict_pos + bound;
//			}
//			if (cmp == 2) {
//				have = lookup_edge_in_secondary_index(leaf->get_payload(predict_pos + bound).sec_index, dst);
//				if (!have) {
//					return -1;
//				}
//				return predict_pos + bound;
//			}
//			l = predict_pos + bound / 2;
//			r = predict_pos + std::min<int>(bound, size);
//			break;
//		case 2:
//			have = lookup_edge_in_secondary_index(leaf->get_payload(predict_pos).sec_index, dst);
//			if (!have) {
//				return -1;
//			}
//			return predict_pos;
//		default:
//			break;
//		}
//		return binary_search(leaf, l, r, src, dst, num_lookup);
//	}
//
//	// 查询边 (src, dst)，返回这条边在 leaf 中的位置：
//	// >=0 —— 在 gapped array 中的位置（这个位置可能是二级索引）
//	// -1 —— 没有这条边
//	int lookup_edge(alex::Alex<vertex, payload>* index, vertex src, vertex dst) {
//		alex::AlexDataNode<vertex, payload>* leaf = index->get_leaf(src);
//		return lookup_edge_in_leaf(leaf, src, dst, num_lookup_fir_idx);
//	}
//
//	int lookup_edge(alex::Alex<vertex, payload>* index, edge e, bool directed) {
//		int a = lookup_edge(index, e.src, e.dst);
//		if (a < 0) {
//			if (!directed) {
//				return lookup_edge(index, e.dst, e.src);
//			}
//		}
//		return a;
//	}
//
//
//	/***  Insert  ***/
//
//	// 在 [left, right) 区间中寻找最右侧的 <src 的位置
//	// 等价于在 [left, right) 区间中寻找最左侧的 =src 的位置的前一个位置
//	int binary_search(alex::AlexDataNode<vertex, payload>* leaf, int left, int right, vertex src) {
//		while (left < right) {
//			int mid = left + (right - left) / 2;
//			if (leaf->get_key(mid) >= src) {
//				right = mid;
//			}
//			else {
//				left = mid + 1;
//			}
//		}
//		return left - 1;
//	}
//
//	// 从 pos 开始指数搜索，找出 <src 的最右位置
//	int find_left_bound_in_leaf(alex::AlexDataNode<vertex, payload>* leaf, int pos, vertex src) {
//		int bound = 1;
//		int l, r;  // will do binary search in range [l, r)
//		if (leaf->get_key(pos) >= src) {
//			// 向左搜索
//			int size = pos;
//			// 指数搜索找到第一个 < key 的位置，作为之后二分搜索的下界
//			while (bound < size && leaf->get_key(pos - bound) >= src) {
//				bound *= 2;
//				leaf->num_exp_search_iterations_++;
//			}
//			l = pos - std::min<int>(bound, size);
//			r = pos - bound / 2;
//		}
//		else {
//			int size = leaf->data_capacity_ - pos;
//			while (bound < size && leaf->get_key(pos + bound) < src) {
//				bound *= 2;
//				leaf->num_exp_search_iterations_++;
//			}
//			l = pos + bound / 2;
//			r = pos + std::min<int>(bound, size);
//		}
//		return binary_search(leaf, l, r, src);
//	}
//
//	// 将 leaf 中 [left, right] 区间里的边转为二级索引，加上正在插入的边，共 num_edges 条邻边
//	// 这次插入的边是 (src, dst)
//	void convert_inline_to_secondary(alex::AlexDataNode<vertex, payload>* leaf, int left, int right, vertex src, vertex dst, int num_edges) {
//		assert(left <= right);
//		assert(left >= 0 && right < leaf->data_capacity_);
//		alex::Alex<vertex, vertex>* sec_index = new alex::Alex<vertex, vertex>();
//		if (sec_index == nullptr) {
//			std::cout << "Create secondary index failed" << std::endl;
//			exit(-1);
//		}
//		std::pair<vertex, vertex>* load_values = new std::pair<vertex, vertex>[num_edges];
//		if (load_values == nullptr) {
//			std::cout << "memory alloc failed" << std::endl;
//			exit(-1);
//		}
//		int idx = 0;
//		for (int i = left; i <= right; i++) {
//			if (leaf->check_exists(i)) {
//				vertex a = leaf->get_payload(i).dst;
//				load_values[idx++] = std::pair<vertex, vertex>(a, a);
//			}
//		}
//		if (idx != num_edges - 1) {
//			std::cout << "convert secondary index, idx wrong" << std::endl;
//			exit(-1);
//		}
//		load_values[idx] = std::pair<vertex, vertex>(dst, dst);
//		std::sort(load_values, load_values + num_edges);
//#ifdef PRINT_PROCESS
//		for (int i = 0; i < num_edges; i++) {
//			std::cout << "(" << load_values[i].first << "," << load_values[i].second << ")" << ", ";
//		}
//		std::cout << std::endl;
//		std::cout << "bulk load secondary index" << std::endl;
//#endif
//		if (num_edges != SECONDARY_INDEX_THRESHOLD) {
//			std::cout << "wrong num_edges in secondary index" << std::endl;
//			exit(-1);
//		}
//		sec_index->bulk_load(load_values, num_edges);
//#ifdef PRINT_PROCESS
//		std::cout << "bulk secondary index finish" << std::endl;
//#endif
//		// 在中间位置放入转化后的数据
//		int mid = (left + right) / 2;
//		leaf->ALEX_DATA_NODE_KEY_AT(mid) = src;
//		leaf->ALEX_DATA_NODE_PAYLOAD_AT(mid) = payload{ 0, sec_index };
//		leaf->set_bit(mid);
//		// 两侧的位置置为 gap
//		for (int i = mid - 1; i >= left; i--) {
//			leaf->ALEX_DATA_NODE_KEY_AT(i) = src;
//			leaf->ALEX_DATA_NODE_PAYLOAD_AT(i) = payload{ 0, sec_index };
//			leaf->unset_bit(i);
//		}
//		for (int i = mid + 1; i <= right; i++) {
//			if (right + 1 >= leaf->data_capacity_) {
//				leaf->ALEX_DATA_NODE_KEY_AT(i) = leaf->kEndSentinel_;
//				leaf->ALEX_DATA_NODE_PAYLOAD_AT(i) = payload();
//			}
//			else {
//				leaf->ALEX_DATA_NODE_KEY_AT(i) = leaf->get_key(right + 1);
//				leaf->ALEX_DATA_NODE_PAYLOAD_AT(i) = leaf->get_payload(right + 1);
//			}
//			leaf->unset_bit(i);
//		}
//		// 原来有 num_edges - 1 个数据，现在变成了 1 个数据
//		leaf->num_keys_ = leaf->num_keys_ - num_edges + 2;
//		delete[] load_values;
//	}
//
//	// 从 start 开始直到前一个 non-gap 的位置，填入 (src, value)
//	// 返回填入数据的数量
//	int fill_gap(alex::AlexDataNode<vertex, payload>* leaf, int start, vertex src, payload value) {
//		int pos = start;
//		while (pos >= 0 && !leaf->check_exists(pos)) {
//			leaf->ALEX_DATA_NODE_KEY_AT(pos) = src;
//			leaf->ALEX_DATA_NODE_PAYLOAD_AT(pos) = value;
//			pos--;
//		}
//		return start - pos - 1;
//	}
//
//	// 将 [start, end] 中的元素向左（to_left == true）或向右（to_left == false）移动一个位置
//	void shift(alex::AlexDataNode<vertex, payload>* leaf, int start, int end, bool to_left) {
//		assert(start <= end && start >= 0 && end < leaf->data_capacity_);
//		if (to_left) {
//			assert(start > 0);
//			for (int i = start; i <= end; i++) {
//				assert(i >= 0 && i < leaf->data_capacity_);
//				leaf->ALEX_DATA_NODE_KEY_AT(i - 1) = leaf->get_key(i);
//				leaf->ALEX_DATA_NODE_PAYLOAD_AT(i - 1) = leaf->get_payload(i);
//				leaf->num_shifts_++;
//			}
//			leaf->set_bit(start - 1);
//		}
//		else {
//			assert(end < leaf->data_capacity_ - 1);
//			for (int i = end; i >= start; i--) {
//				assert(i >= 0 && i < leaf->data_capacity_);
//				leaf->ALEX_DATA_NODE_KEY_AT(i + 1) = leaf->get_key(i);
//				leaf->ALEX_DATA_NODE_PAYLOAD_AT(i + 1) = leaf->get_payload(i);
//				leaf->num_shifts_++;
//			}
//			leaf->set_bit(end + 1);
//		}
//	}
//
//	// 在 pos 位置插入边 (src, dst)，且这是src的第一条邻边
//	void insert_vertex_in_leaf(alex::AlexDataNode<vertex, payload>* leaf, vertex src, vertex dst, int pos) {
//		if (SECONDARY_INDEX_THRESHOLD <= 1) {
//			// 建立二级索引在 left 位置插入
//			alex::Alex<vertex, vertex>* sec_index = construct_secondary_index(new edge[1]{ edge{src, dst} }, 0, 1);
//			leaf->ALEX_DATA_NODE_KEY_AT(pos) = src;
//			leaf->ALEX_DATA_NODE_PAYLOAD_AT(pos) = payload{ 0, sec_index };
//			fill_gap(leaf, pos - 1, src, payload{ 0, sec_index });
//		}
//		else {
//			leaf->ALEX_DATA_NODE_KEY_AT(pos) = src;
//			leaf->ALEX_DATA_NODE_PAYLOAD_AT(pos) = payload{ dst, nullptr };
//			fill_gap(leaf, pos - 1, src, payload{ dst, nullptr });
//		}
//		leaf->set_bit(pos);
//	}
//
//	void update_leaf(alex::Alex<vertex, payload>* index, alex::AlexDataNode<vertex, payload>* leaf, vertex src) {
//		leaf->num_keys_++;
//		index->stats_.num_inserts++;
//		index->stats_.num_keys++;
//		if (src > leaf->max_key_) {
//			leaf->max_key_ = src;
//			leaf->num_right_out_of_bounds_inserts_++;
//		}
//		if (src < leaf->min_key_) {
//			leaf->min_key_ = src;
//			leaf->num_left_out_of_bounds_inserts_++;
//		}
//	}
//
//	// 在叶子节点中插入一条边
//	// 返回值.first：
//	// 0 —— 成功插入
//	// 1 —— significant_cost_deviation
//	// 2 —— catastrophic_cost
//	// 3 —— 管理的数据过多
//	// 4 —— 在二级索引中插入
//	// 5 —— 有重复边
//	// 返回值.second：插入的位置
//	std::pair<int, int> insert_edge_in_leaf(alex::Alex<vertex, payload>* index, alex::AlexDataNode<vertex, payload>* leaf, vertex src, vertex dst) {
//		// leaf 已满，需要 expansion
//		if (leaf->num_keys_ >= leaf->expansion_threshold_) {
//			if (leaf->significant_cost_deviation()) {
//				return { 1, -1 };
//			}
//			if (leaf->num_keys_ > leaf->max_slots_ * leaf->kMinDensity_) {
//				return { 3, -1 };
//			}
//			// Expand
//			bool keep_left = leaf->is_append_mostly_right();
//			bool keep_right = leaf->is_append_mostly_left();
//#ifdef PRINT_PROCESS
//			std::cout << "insert in leaf, need resize" << std::endl;
//#endif
//			leaf->resize(leaf->kMinDensity_, false, keep_left, keep_right);
//			leaf->num_resizes_++;
//		}
//#ifdef PRINT_PROCESS
//		std::cout << "insert in leaf, check expansion finish" << std::endl;
//#endif
//		leaf->num_inserts_++;
//		// 从模型的预测位置开始搜索，找出 >src 的最左位置和 <src 的最右位置
//		int predict_pos = leaf->predict_position(src);
//		int right = leaf->find_upper(src);
//		int left = find_left_bound_in_leaf(leaf, predict_pos, src);
//		if (left + 1 > right - 1) {
//#ifdef PRINT_PROCESS
//			std::cout << "insert in leaf, insert vertex in leaf" << std::endl;
//#endif
//			// 顶点 src 不存在，且中间没有 gap 
//			// 在 gap_pos 位置插入
//			int gap_pos = 0;
//			if (left == -1) {
//				// 插入在 leaf 中的第一个位置，向右移动空出 right
//				assert(right == 0);
//				gap_pos = leaf->closest_gap(right);
//				shift(leaf, right, gap_pos - 1, false);
//				insert_vertex_in_leaf(leaf, src, dst, right);
//				update_leaf(index, leaf, src);
//				return std::pair<int, int>(0, right);
//			}
//			if (left == leaf->data_capacity_) {
//				// 插入在 leaf 中的最后一个位置，向左移动空出 left - 1
//				assert(right == leaf->data_capacity_);
//				gap_pos = leaf->closest_gap(left);
//				shift(leaf, gap_pos + 1, left - 1, true);
//				insert_vertex_in_leaf(leaf, src, dst, left - 1);
//				update_leaf(index, leaf, src);
//				return std::pair<int, int>(0, left - 1);
//			}
//			gap_pos = leaf->closest_gap(left);
//			if (gap_pos < left) {
//				// left 向左移动，空出 left 位置
//				shift(leaf, gap_pos + 1, left, true);
//				insert_vertex_in_leaf(leaf, src, dst, left);
//				update_leaf(index, leaf, src);
//				return std::pair<int, int>(0, left);
//			}
//			else {
//				shift(leaf, right, gap_pos - 1, false);
//				insert_vertex_in_leaf(leaf, src, dst, right);
//				update_leaf(index, leaf, src);
//				return std::pair<int, int>(0, right);
//			}
//		}
//		// 在区间 [left+1, right-1] 遍历，记录已有 src 的数量（src的度）
//		int cnt = 0;
//		int payload_left = -1, payload_right = -1;
//		int gap_pos = -1;
//		for (int i = left + 1; i < right; i++) {
//			if (leaf->check_exists(i)) {
//				assert(leaf->get_key(i) == src);
//				if (leaf->get_payload(i).sec_index != nullptr) {
//					// 顶点 src 有二级索引，在二级索引中插入
//					leaf->get_payload(i).sec_index->insert(std::pair<vertex, vertex>(dst, dst));
//					return std::pair<int, int>(4, -1);
//				}
//				if (leaf->get_payload(i).dst < dst) {
//					payload_left = i;
//				}
//				if (leaf->get_payload(i).dst > dst && payload_right == -1) {
//					payload_right = i;
//				}
//				if (leaf->get_payload(i).dst == dst) {
//					return std::pair<int, int>(5, -1);
//				}
//				cnt++;
//			}
//			else {
//				// 要求 gap_pos 在 payload_left 和 payload_right 之间
//				// 因此记录 gap_pos 时应该已经找到了 payload_left，但还没找到 payload_right
//				if (payload_left != -1 && payload_right == -1 && leaf->get_payload(i).dst > dst) {
//					gap_pos = i;
//				}
//			}
//		}
//		// payload_left == -1，说明这是最小的dst，插在 left 位置
//		if (payload_left == -1) {
//			payload_left = left;
//		}
//		// payload_right == -1，说明这是最大的dst，插在 right 的位置
//		if (payload_right == -1) {
//			payload_right = right;
//		}
//		if (gap_pos != -1) {
//			// 在区间 (payload_left,payload_right) 中的 gap 插入
//			// 在 gap_pos 插入
//			leaf->ALEX_DATA_NODE_KEY_AT(gap_pos) = src;
//			leaf->ALEX_DATA_NODE_PAYLOAD_AT(gap_pos) = payload{ dst, nullptr };
//			leaf->set_bit(gap_pos);
//			fill_gap(leaf, gap_pos - 1, src, payload{ dst, nullptr });
//			update_leaf(index, leaf, src);
//			return std::pair<int, int>(0, gap_pos);
//		}
//		assert(payload_left >= 0 || payload_right >= 0);
//		// gap_pos 可能在 payload_left 左边，也可能在 payload_right 右边
//		if (payload_left < 0) {
//			gap_pos = leaf->closest_gap(payload_right);
//		}
//		else {
//			gap_pos = leaf->closest_gap(payload_left);
//		}
//		if (gap_pos < payload_left) {
//			// [gap_pos + 1, payload_left] 中的元素向左移动一个位置，空出payload_left
//			shift(leaf, gap_pos + 1, payload_left, true);
//			leaf->ALEX_DATA_NODE_KEY_AT(payload_left) = src;
//			leaf->ALEX_DATA_NODE_PAYLOAD_AT(payload_left) = payload{ dst, nullptr };
//			fill_gap(leaf, payload_left - 1, src, payload{ dst, nullptr });
//			update_leaf(index, leaf, src);
//			return std::pair<int, int>(0, payload_left);
//		}
//		else if (payload_left < gap_pos && gap_pos < payload_right) {
//			leaf->ALEX_DATA_NODE_KEY_AT(gap_pos) = src;
//			leaf->ALEX_DATA_NODE_PAYLOAD_AT(gap_pos) = payload{ dst, nullptr };
//			leaf->set_bit(gap_pos);
//			fill_gap(leaf, gap_pos - 1, src, payload{ dst, nullptr });
//			update_leaf(index, leaf, src);
//			return std::pair<int, int>(0, gap_pos);
//		}
//		else {
//			// [payload_right, gap_pos - 1] 中的元素向右移动一个位置，空出payload_right
//			shift(leaf, payload_right, gap_pos - 1, false);
//			leaf->ALEX_DATA_NODE_KEY_AT(payload_right) = src;
//			leaf->ALEX_DATA_NODE_PAYLOAD_AT(payload_right) = payload{ dst, nullptr };
//			fill_gap(leaf, payload_right - 1, src, payload{ dst, nullptr });
//			update_leaf(index, leaf, src);
//			return std::pair<int, int>(0, payload_right);
//		}
//	}
//
//	void check_expand_root(alex::Alex<vertex, payload>* index, vertex src) {
//		if (src > index->istats_.key_domain_max_) {
//			index->istats_.num_keys_above_key_domain++;
//			if (index->should_expand_right()) {
//				index->expand_root(src, false);
//			}
//		}
//		else if (src < index->istats_.key_domain_min_) {
//			index->istats_.num_keys_below_key_domain++;
//			if (index->should_expand_left()) {
//				index->expand_root(src, true);
//			}
//		}
//	}
//
//	bool insert_edge(alex::Alex<vertex, payload>* index, vertex src, vertex dst) {
//		// 找到 key 所在的 leaf
//		alex::AlexDataNode<vertex, payload>* leaf = index->get_leaf(src);
//		std::pair<int, int> ret = insert_edge_in_leaf(index, leaf, src, dst);
//		int fail = ret.first;
//		if (fail != 0 && fail != 4) {
//			if (fail == 5) {
//				return false;
//			}
//			std::vector<alex::Alex<vertex, payload>::TraversalNode> traversal_path;
//			index->get_leaf(src, &traversal_path);
//			alex::AlexModelNode<vertex, payload>* parent = traversal_path.back().node;
//			while (fail != 0 && fail != 4) {
//				if (fail == 5) {
//					return false;
//				}
//				auto start_time = std::chrono::high_resolution_clock::now();
//				index->stats_.num_expand_and_scales += leaf->num_resizes_;
//
//				if (parent == index->superroot_) {
//					index->update_superroot_key_domain();
//				}
//				int bucketID = parent->model_.predict(src);
//				bucketID = std::min<int>(std::max<int>(bucketID, 0), parent->num_children_ - 1);
//				std::vector<alex::fanout_tree::FTNode> used_fanout_tree_nodes;
//
//				int fanout_tree_depth = 1;
//				if (index->experimental_params_.splitting_policy_method == 1) {
//					// decide between no split (i.e., expand and retrain) or splitting in 2
//					fanout_tree_depth = alex::fanout_tree::find_best_fanout_existing_node<vertex, payload>(
//						parent, bucketID, index->stats_.num_keys, used_fanout_tree_nodes, 2);
//				}
//				// 用 fanout tree 决定是否分裂
//				int best_fanout = 1 << fanout_tree_depth;
//				index->stats_.cost_computation_time += std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
//
//				if (fanout_tree_depth == 0) {
//					// expand + retrain
//					leaf->resize(alex::AlexDataNode<vertex, payload>::kMinDensity_, true,
//						leaf->is_append_mostly_right(),
//						leaf->is_append_mostly_left());
//					alex::fanout_tree::FTNode& tree_node = used_fanout_tree_nodes[0];
//					leaf->cost_ = tree_node.cost;
//					leaf->expected_avg_exp_search_iterations_ = tree_node.expected_avg_search_iterations;
//					leaf->expected_avg_shifts_ = tree_node.expected_avg_shifts;
//					leaf->reset_stats();
//					index->stats_.num_expand_and_retrains++;
//				} else {
//					// split data node
//					bool reuse_model = (fail == 3);
//					// either split sideways or downwards
//					// 需要 split down 的两种情况
//					// 1. parent 的 child 数量超过了 max_fanout (达到了 max-node-size)
//					// 2. leaf 是 root
//					bool should_split_downwards = (parent->num_children_ * best_fanout / (1 << leaf->duplication_factor_) >
//						index->derived_params_.max_fanout || parent->level_ == index->superroot_->level_);
//					if (should_split_downwards) {
//						parent = index->split_downwards(parent, bucketID, fanout_tree_depth, used_fanout_tree_nodes, reuse_model);
//					}
//					else {
//						index->split_sideways(parent, bucketID, fanout_tree_depth, used_fanout_tree_nodes, reuse_model);
//					}
//					leaf = static_cast<alex::AlexDataNode<vertex, payload>*>(parent->get_child_node(src));
//				}
//				auto end_time = std::chrono::high_resolution_clock::now();
//				auto duration = end_time - start_time;
//				index->stats_.splitting_time += std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
//				// Try again to insert the key
//				ret = insert_edge_in_leaf(index, leaf, src, dst);
//				fail = ret.first;
//				//insert_pos = ret.second;
//			}
//		}
//		//index->stats_.num_inserts++;
//		//index->stats_.num_keys++;
//		//if (src == 2488 && dst == 19858) {
//		//	print_all_leaf_to_file2(index, "after " + std::to_string(src) + " " + std::to_string(dst) + ".txt");
//		//}
//		return true;
//	}
//
//
//	bool insert_edge(alex::Alex<vertex, payload>* index, edge e, bool directed) {
//		if (!directed) {
//			return insert_edge(index, e.src, e.dst) && insert_edge(index, e.dst, e.src);
//		}
//		return insert_edge(index, e.src, e.dst);
//	}
//
//
//	/***  Delete  ***/
//	// 找到（src, dst）的 non-gap 位置
//	int get_nongap_data(alex::AlexDataNode<vertex, payload>* leaf, int pos, vertex src, vertex dst) {
//		int start = pos;
//		for (int i = start; i >= 0; i--) {
//			if (leaf->get_key(i) == src && (leaf->get_payload(i).dst == dst || leaf->get_payload(i).sec_index != nullptr)
//				&& leaf->check_exists(i)) {
//				return i;
//			}
//			if (leaf->get_key(i) != src || (leaf->get_payload(i).sec_index == nullptr && leaf->get_payload(i).dst != dst)) {
//				break;
//			}
//		}
//		for (int i = start; i < leaf->data_capacity_; i++) {
//			if (leaf->get_key(i) == src && (leaf->get_payload(i).dst == dst || leaf->get_payload(i).sec_index != nullptr)
//				&& leaf->check_exists(i)) {
//				return i;
//			}
//			if (leaf->get_key(i) != src || (leaf->get_payload(i).sec_index == nullptr && leaf->get_payload(i).dst != dst)) {
//				break;
//			}
//		}
//		return -1;
//	}
//
//
//	bool delete_edge_in_leaf(alex::AlexDataNode<vertex, payload>* leaf, vertex src, vertex dst) {
//		int pos = lookup_edge_in_leaf(leaf, src, dst, num_delete_fir_idx);
//		if (pos < 0) {
//			return false;
//		}
//		assert(leaf->get_key(pos) == src && (leaf->get_payload(pos).sec_index != nullptr || leaf->get_payload(pos).dst == dst));
//		if (!leaf->check_exists(pos)) {
//			pos = get_nongap_data(leaf, pos, src, dst);
//		}
//		if (pos < 0) {
//			std::cout << "no non-gap pos with (" << src << ", " << dst << ")" << std::endl;
//			return false;
//		}
//		assert(leaf->get_key(pos) == src && (leaf->get_payload(pos).sec_index != nullptr || leaf->get_payload(pos).dst == dst));
//		if (leaf->get_payload(pos).sec_index != nullptr) {
//			leaf->get_payload(pos).sec_index->erase(dst);
//			return true;
//		}
//		leaf->unset_bit(pos);
//		leaf->num_keys_--;
//		vertex next_key = 0;
//		payload next_payload = payload();
//		// src 在最后一个位置上
//		if (pos >= leaf->data_capacity_ - 1) {
//			next_key = leaf->kEndSentinel_;
//		}
//		else {
//			next_key = leaf->get_key(pos + 1);
//			next_payload = leaf->get_payload(pos + 1);
//		}
//		int num = fill_gap(leaf, pos, next_key, next_payload);
//#ifdef TIME
//		num_delete_fill += num;
//#endif
//		if (leaf->num_keys_ < leaf->contraction_threshold_) {
//			leaf->resize(leaf->kMaxDensity_);  // contract
//			leaf->num_resizes_++;
//		}
//		return true;
//	}
//
//	bool delete_edge(alex::Alex<vertex, payload>* index, vertex src, vertex dst) {
//		alex::AlexDataNode<vertex, payload>* leaf = index->get_leaf(src);
//		bool ok = delete_edge_in_leaf(leaf, src, dst);
//		if (ok) {
//			index->stats_.num_keys--;
//			if (leaf->num_keys_ == 0) {
//				index->merge(leaf, src);
//			}
//			if (src > index->istats_.key_domain_max_) {
//				index->istats_.num_keys_above_key_domain--;
//			}
//			else if (src < index->istats_.key_domain_min_) {
//				index->istats_.num_keys_below_key_domain--;
//			}
//			return true;
//		}
//		return false;
//	}
//
//	bool delete_edge(alex::Alex<vertex, payload>* index, edge e, bool directed) {
//		if (!directed) {
//			return delete_edge(index, e.src, e.dst) && delete_edge(index, e.dst, e.src);
//		}
//		return delete_edge(index, e.src, e.dst);
//	}
//}

class Graph {
public:
	alex::AlexMultimap<vertex, std::pair<vertex, double>>* index;
	Graph() {
		index = new alex::AlexMultimap<vertex, std::pair<vertex, double>>();
	}
	~Graph() {
		delete index;
	}
	void clear() {
		delete index;
		index = new alex::AlexMultimap<vertex, std::pair<vertex, double>>();
	}

	bool lookup_edge(vertex src, vertex dst) {
		auto it = index->lower_bound(src);
		bool find = false;
		while (!it.is_end() && it.key() == src) {
			if (it.payload().first == dst) {
				find = true;
				break;
			}
			it++;
		}
		return find;
	}

	bool lookup_edge(edge e, bool directed) {
		if (lookup_edge(e.src, e.dst)) {
			return true;
		}
		if (directed) {
			return false;
		}
		return lookup_edge(e.dst, e.src);
	}

	bool lookup_vertex(vertex u) {
		auto it = index->lower_bound(u);
		if (it.is_end() || it.key() != u) {
			return false;
		}
		return true;
	}

	int get_neighbor(vertex u) {
		auto it = index->lower_bound(u);
		int ans = 0;
		for (; !it.is_end() && it.key() == u; it++) {
			ans++;
		}
		return ans;
	}

	bool insert_edge(vertex src, vertex dst, double weight = 0.0) {
		auto it = index->lower_bound(src);
		bool find = false;
		while (!it.is_end() && it.key() == src) {
			if (it.payload().first == dst) {
				return false;
			}
			if (it.payload().first == -1) {
				std::pair<vertex, double>& payload = it.payload();
				payload.first = dst;
				payload.second = weight;
				return true;
			}
			it++;
		}
		index->insert(src, std::pair<vertex, double>(dst, weight));
		return true;
	}

	bool insert_edge(edge e, bool directed) {
		if (!directed) {
			return insert_edge(e.src, e.dst, e.weight) && insert_edge(e.dst, e.src, e.weight);
		}
		return insert_edge(e.src, e.dst);
	}

	bool insert_vertex(vertex u) {
		auto it = index->lower_bound(u);
		if (!it.is_end() && it.key() == u) {
			return false;
		}
		index->insert(u, std::pair<vertex, double>(-1, 0.0));
		return true;
	}

	bool delete_edge(vertex src, vertex dst) {
		auto it = index->lower_bound(src);
		while (!it.is_end() && it.key() == src) {
			if (it.payload().first == dst) {
				index->erase(it);
				return true;
			}
			it++;
		}
		return false;
	}

	bool delete_edge(edge e, bool directed) {
		if (!directed) {
			return delete_edge(e.src, e.dst) && delete_edge(e.dst, e.src);
		}
		return delete_edge(e.src, e.dst);
	}

	bool delete_vertex(vertex u, bool directed) {
		auto it = index->lower_bound(u);
		if (it.is_end() || it.key() != u) {
			return false;
		}
		std::vector<vertex> nbr;
		if (!directed) {
			while (!it.is_end() && it.key() == u) {
				vertex dst = it.payload().first;
				if (dst != -1) {
					nbr.push_back(dst);
				}
				it++;
			}
		}
		for (int i = 0; i < nbr.size(); i++) {
			delete_edge(nbr[i], u);
		}
		index->erase(u);
		return true;
	}

	uint64_t get_bytes() {
		return index->get_bytes();
	}
};