#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include "graph.h"

Coords askUserForCoordinates(const int8_t flag);


int64_t findNearestNode(Coords target, Graph& graph, int8_t transportMethod);



#endif