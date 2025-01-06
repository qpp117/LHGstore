// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "zipf.h"

template <class T>
bool load_binary_data(T data[], int length, const std::string& file_path) {
  std::ifstream is(file_path.c_str(), std::ios::binary | std::ios::in);
  if (!is.is_open()) {
    return false;
  }
  is.read(reinterpret_cast<char*>(data), std::streamsize(length * sizeof(T)));
  is.close();
  return true;
}

template <class T>
bool load_text_data(T array[], int length, const std::string& file_path) {
  std::ifstream is(file_path.c_str());
  if (!is.is_open()) {
    return false;
  }
  int i = 0;
  std::string str;
  while (std::getline(is, str) && i < length) {
    std::istringstream ss(str);
    ss >> array[i];
    i++;
  }
  is.close();
  return true;
}

template <class T>
T* get_search_keys(T array[], int num_keys, int num_searches) {
  std::mt19937_64 gen(std::random_device{}());
  std::uniform_int_distribution<int> dis(0, num_keys - 1);
  auto* keys = new T[num_searches];
  for (int i = 0; i < num_searches; i++) {
    int pos = dis(gen);
    keys[i] = array[pos];
  }
  return keys;
}

template <class T>
T* get_search_keys_zipf(T array[], int num_keys, int num_searches) {
  auto* keys = new T[num_searches];
  ScrambledZipfianGenerator zipf_gen(num_keys);
  for (int i = 0; i < num_searches; i++) {
    int pos = zipf_gen.nextValue();
    keys[i] = array[pos];
  }
  return keys;
}

std::string* read_option_from_file(int& argc, std::string file_name = "option.txt") {
    argc = 0;
    std::ifstream is(file_name.c_str());
    if (!is.is_open()) {
        return nullptr;
    }
    std::string str;
    while (std::getline(is, str)) {
        ++argc;
    }
    is.clear();
    is.seekg(std::ios::beg);
    std::string* ret = new std::string[argc];
    int i = 0;
    while (std::getline(is, str)) {
        ret[i] = std::string(str);
        i++;
    }
    return ret;
}

template<class T, class P>
void traverse_one_leaf_to_file(alex::AlexNode<T, P>* node, std::ofstream* f) {
    alex::AlexDataNode<T, P>* leaf = (alex::AlexDataNode<T, P>*)node;
    for (int i = 0; i < leaf->data_capacity_; i++) {
        if (leaf->check_exists(i)) {
            *f << "key = " << leaf->get_key(i) << " value = " << leaf->get_payload(i) << std::endl;
        }
    }
    *f << "\n";
}

template<class T, class P>
void traverse_one_leaf(alex::AlexNode<T, P>* node) {
    alex::AlexDataNode<T, P>* leaf = (alex::AlexDataNode<T, P>*)node;
    for (int i = 0; i < leaf->data_capacity_; i++) {
        if (leaf->check_exists(i)) {
            std::cout << "key = " << leaf->get_key(i) << " value = " << leaf->get_payload(i) << std::endl;
        }
    }
    std::cout << "\n";
}

template<class T, class P>
void traverse_all_leaf(alex::Alex<T, P>* index) {
    alex::Alex<T, P>::NodeIterator* nit = new alex::Alex<T, P>::NodeIterator(index);
    int id = 0;
    if (nit->cur_node_->is_leaf_) {
        std::cout << "leaf " << id << ":" << std::endl;
        id++;
        traverse_one_leaf(nit->cur_node_);
    }
    while (true) {
        alex::AlexNode<T, P>* cur_node = nit->next();
        if (cur_node == nullptr) {
            break;
        }
        if (cur_node->is_leaf_) {
            std::cout << "leaf " << id << ":" << std::endl;
            id++;
            alex::AlexDataNode<T, P>* cur_leaf = (alex::AlexDataNode<T, P>*)cur_node;
            traverse_one_leaf(cur_leaf);
        }
    }
}

template<class T, class P>
void traverse_all_leaf_to_file(alex::Alex<T, P>* index, std::string file = "all_leaf.txt") {
    std::ofstream f(file.c_str());
    alex::Alex<T, P>::NodeIterator* nit = new alex::Alex<T, P>::NodeIterator(index);
    int id = 0;
    if (nit->cur_node_->is_leaf_) {
        f << "leaf " << id << ":" << std::endl;
        id++;
        traverse_one_leaf(nit->cur_node_);
    }
    while (true) {
        alex::AlexNode<T, P>* cur_node = nit->next();
        if (cur_node == nullptr) {
            break;
        }
        if (cur_node->is_leaf_) {
            f << "leaf " << id << ":" << std::endl;
            id++;
            alex::AlexDataNode<T, P>* cur_leaf = (alex::AlexDataNode<T, P>*)cur_node;
            traverse_one_leaf_to_file(cur_leaf, &f);
        }
    }
    f.close();
}
