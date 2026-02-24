#ifndef GRAPH_H
#define GRAPH_H
#include "type.h"
#include <unordered_map>
using namespace std;

struct Graph{
    unordered_map<int64_t, Node> nodes;
};





#endif // GRAPH_H