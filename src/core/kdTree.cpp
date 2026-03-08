#include "kdTree.h"
#include <algorithm>

bool compareLat(const pair<int64_t, LatLon>& a, const pair<int64_t, LatLon>& b) {
    return a.second.lat < b.second.lat;
}

bool compareLon(const pair<int64_t, LatLon>& a, const pair<int64_t, LatLon>& b) {
    return a.second.lon < b.second.lon;
}

kdNode* buildRecursive(vector<pair<int64_t, LatLon>>& nodes, int depth){
    if(nodes.empty()){
        return nullptr;
    }
    if(depth % 2 == 1){
        sort(nodes.begin(),nodes.end(), compareLat);
    } else {
        sort(nodes.begin(),nodes.end(), compareLon);
    }
    int8_t mid = nodes.size() / 2;
    vector<pair<int64_t, LatLon>> left(nodes.begin(), nodes.begin() + mid);
    vector<pair<int64_t, LatLon>> right(nodes.begin() + mid + 1, nodes.end());

    kdNode* node = new kdNode();
    node->id = nodes[mid].first;
    node->coords = nodes[mid].second;
    node->left = buildRecursive(left, depth + 1);
    node->right = buildRecursive(right, depth + 1);
    return node;
}

void buildKdTree(KdTree& tree, const Graph& graph){
    vector<pair<int64_t, LatLon>> nodes;
    for(auto& [nodeId,node] : graph.nodes){
        nodes.push_back({nodeId,node.coords});
    }
    int depth = 0;
    tree.root = buildRecursive(nodes, depth);
}

int64_t findNearest(const KdTree& tree, const Coords& target){
    
}
