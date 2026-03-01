#ifndef ASTAR_H
#define ASTAR_H
#include "graph.h"
#include <queue>
#include <unordered_set>
#include <unordered_map>

double haverstine(const LatLon& a, const LatLon& b);


vector<int64_t> astar(int64_t start, int64_t goal, Graph& graph);



#endif // ASTAR_H