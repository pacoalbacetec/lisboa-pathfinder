#include "cache.h"
#include "proto/graph_cache.pb.h"
#include <fstream>
using namespace std;

void saveCache(const string& filename, const Graph& graph){
    lisboa::CachedGraph cached;
    for(auto& [nodeId, node] : graph.nodes) {
        lisboa::Node protoNode;
        protoNode.set_id(nodeId);
        protoNode.mutable_coords()->set_lat(node.coords.lat);
        protoNode.mutable_coords()->set_lon(node.coords.lon);
        (*cached.mutable_nodes())[nodeId] = protoNode;
    }
    for(auto& [nodeId, edge] : graph.adjacencyList) {
        lisboa::EdgeList protoEdgeList;
        for(auto& [neighbourId, highwayType] : edge) {
            lisboa::Edge* protoEdge = protoEdgeList.add_edges();
            protoEdge->set_id(neighbourId);
            protoEdge->set_type(highwayType);
        }
        (*cached.mutable_adjacency_list())[nodeId] = protoEdgeList;
    }
    ofstream out(filename, ios::binary);
    if(!out.is_open()) {
        cerr << "Failed to open cache file!" << endl;
        return;
    }
    cached.SerializeToOstream(&out);
}

bool loadCache(const string& filename,  Graph& graph){

    ifstream in(filename, ios::binary);
    if(!in.is_open()) { 
        cerr << "There is no cache file!" << endl;
        return false;
    }
    lisboa::CachedGraph cached;
    if(!cached.ParseFromIstream(&in)) {
        cerr << "Failed to parse cache file!" << endl;
        return false;
    }

    for(auto& [nodeId, protoNode] : cached.nodes()){
        Node n;
        n.coords.lat = protoNode.coords().lat();
        n.coords.lon = protoNode.coords().lon();
        graph.nodes[nodeId] = n;
    }

    for(auto& [nodeId,protoEdgeList] : cached.adjacency_list()){
        for(auto& edge : protoEdgeList.edges()){
            graph.adjacencyList[nodeId].push_back({edge.id(), edge.type()});
        }

    }

    return true;
}