#pragma once
#include "graph.h"
#include <random>

void shuffle(std::vector<edge>& arr) {
    std::random_device rd;
    for (int i = arr.size() - 1; i >= 0; i--) {
        std::uniform_int_distribution<int> dist(0, i);
        int pos = dist(rd);
        edge t = arr[i];
        arr[i] = arr[pos];
        arr[pos] = t;
    }
}

int max(int a, int b) {
    return a > b ? a : b;
}

int min(int a, int b) {
    return a < b ? a : b;
}