#ifndef KDTREE_H
#define KDTREE_H 

#include "type.h"
#include "graph.h"

struct KdTree {
    kdNode* root;
};

kdNode* buildRecursive(vector<pair<int64_t, LatLon>>& nodes, int depth);

void buildKdTree(KdTree& tree, const Graph& graph);

int64_t findNearest(const KdTree& tree, const Coords& target);

#endif KDTREE_H