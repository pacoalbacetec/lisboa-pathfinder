#ifndef KDTREE_H
#define KDTREE_H 

#include "type.h"
#include "graph.h"

struct KdTree {
    kdNode* root;
};

bool compareLat(const pair<int64_t, LatLon>& a, const pair<int64_t, LatLon>& b);
    
bool compareLon(const pair<int64_t, LatLon>& a, const pair<int64_t, LatLon>& b);


void searchRecursive(kdNode * node, LatLon& target, int depth, int64_t& bestId,double& bestDist);

kdNode* buildRecursive(vector<pair<int64_t, LatLon>>& nodes, int depth);

void buildKdTree(KdTree& tree, const Graph& graph, int8_t transportMethod);

int64_t findNearest(const KdTree& tree, const Coords& target);

#endif 