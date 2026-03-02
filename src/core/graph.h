#ifndef GRAPH_H
#define GRAPH_H
#include "type.h"
#include <unordered_map>
#include <vector>
using namespace std;

struct Graph{
    unordered_map<int64_t, Node> nodes;     
    unordered_map<int64_t, vector<pair<int64_t,string>>> adjacency_list; // node_id -> list of adjacent node_ids and their highway types
};
#endif 