#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include "graph.h"

bool checkBoundingBox(const Coords& coords);

void printRoute(const vector<int64_t>& path, const Graph& graph);

Coords askUserForCoordinates(const int8_t flag);


int64_t findNearestNode(Coords target, Graph& graph, int8_t transportMethod);



#endif