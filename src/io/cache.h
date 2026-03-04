#ifndef LISBOA_CACHE_H
#define LISBOA_CACHE_H
#include "graph.h"
#include <iostream>
using namespace std;

void saveCache(const string& filename, const Graph& graph);

bool loadCache(const string& filename, Graph& graph);


#endif 