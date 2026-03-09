#include "kdTree.h"
#include <algorithm>
#include "routing/astar.h"
#include <limits>
#include "transport.h"
bool compareLat(const pair<int64_t, LatLon>& a, const pair<int64_t, LatLon>& b) {
    return a.second.lat < b.second.lat;
}

bool compareLon(const pair<int64_t, LatLon>& a, const pair<int64_t, LatLon>& b) {
    return a.second.lon < b.second.lon;
}

void searchRecursive(kdNode * node, LatLon& target, int depth, int64_t& bestId,double& bestDist){
    if(node == nullptr){
        return;
    }
    double dist = harvesine(node->coords, target);
    if(dist < bestDist) {
        bestDist = dist;
        bestId = node->id;
    }
    bool wentLeft = false;
    if(depth % 2 == 0){
        if(target.lat < node->coords.lat){
            searchRecursive(node->left, target, depth+1, bestId, bestDist);
            wentLeft = true;
        }else {
            searchRecursive(node->right, target, depth+1, bestId, bestDist);
        }
    } else {
        if(target.lon < node->coords.lon){
            searchRecursive(node->left, target, depth+1, bestId, bestDist);
            wentLeft = true;
        }else {
            searchRecursive(node->right, target, depth+1, bestId, bestDist);
        }
    }
    double splitDist = 0;
    if(depth % 2 == 0) {
        splitDist = abs(target.lat - node->coords.lat) / 1e9 * 111000;
    } else {
        splitDist = abs(target.lon - node->coords.lon) / 1e9 * 111000;
    }
    if(splitDist < bestDist) {
        kdNode* otherBranch = wentLeft ? node->right : node->left;
        searchRecursive(otherBranch, target, depth + 1, bestId, bestDist);
    }

}

kdNode* buildRecursive(vector<pair<int64_t, LatLon>>& nodes, int depth){
    if(nodes.empty()){
        return nullptr;
    }
    if(depth % 2 == 0){
        sort(nodes.begin(),nodes.end(), compareLat);
    } else {
        sort(nodes.begin(),nodes.end(), compareLon);
    }
    int64_t mid = nodes.size() / 2;
    vector<pair<int64_t, LatLon>> left(nodes.begin(), nodes.begin() + mid);
    vector<pair<int64_t, LatLon>> right(nodes.begin() + mid + 1, nodes.end());

    kdNode* node = new kdNode();
    node->id = nodes[mid].first;
    node->coords = nodes[mid].second;
    node->left = buildRecursive(left, depth + 1);
    node->right = buildRecursive(right, depth + 1);
    return node;
}

void buildKdTree(KdTree& tree, const Graph& graph,int8_t transportMethod){
    vector<pair<int64_t, LatLon>> nodes;
    for(auto& [nodeId,node] : graph.nodes){
        if(graph.adjacencyList.count(nodeId) == 0) {continue;}
        if(node.coords.lat == 0 && node.coords.lon == 0) {continue;}

        // check if node has a valid neighbour for the transport method
        bool hasValidNeighbour = false;
        const vector<string>& allowedTypes = (transportMethod == 1) ? CAR_TYPES : WALK_TYPES;
        for(auto& edge : graph.adjacencyList.at(nodeId)) {
            if(find(allowedTypes.begin(), allowedTypes.end(), edge.highwayType) != allowedTypes.end()) {
                hasValidNeighbour = true;
                break;
            }
        }
        if(!hasValidNeighbour) continue;
        nodes.push_back({nodeId,node.coords});
    }
        
    
    int depth = 0;
    tree.root = buildRecursive(nodes, depth);
}

int64_t findNearest(const KdTree& tree, const Coords& target){
    LatLon targetNano = {(int64_t)(target.lat * 1e9), (int64_t)(target.lon * 1e9)};
    int64_t bestId = -1;
    int depth = 0;
    double bestDist = numeric_limits<double>::infinity();
    searchRecursive(tree.root, targetNano, depth,bestId,bestDist);
    return bestId;
}
