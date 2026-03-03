#ifndef GRAPH_H
#define GRAPH_H
#include "type.h"
#include <unordered_map>
#include <vector>
using namespace std;

struct Graph{
    unordered_map<int64_t, Node> nodes;     
    unordered_map<int64_t, vector<pair<int64_t,string>>> adjacencyList; // nodeId -> list of adjacent nodeIds and their highway types
};
#endif 