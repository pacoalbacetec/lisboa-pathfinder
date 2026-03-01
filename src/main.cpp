#include <iostream>
 #include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include "proto/fileformat.pb.h"
#include "proto/osmformat.pb.h"
#include <zlib.h>
#include "type.h"
#include "graph.h"
#include <queue>
#include "astar.h"
 using namespace std;
 
LatLon calculateLatLon(const OSMPBF::PrimitiveBlock& primitive_block, const OSMPBF::DenseNodes& dense) {
int64_t granularity = primitive_block.granularity();
int64_t lat_offset = primitive_block.lat_offset();
int64_t lon_offset = primitive_block.lon_offset();

int64_t lat = lat_offset + granularity * dense.lat(0);
int64_t lon = lon_offset + granularity * dense.lon(0);

return {lat, lon};

    }

int64_t findNearestNode(double lat, double lon, Graph& graph) {
    int64_t best_id = -1;
    double best_dist = numeric_limits<double>::infinity();
    LatLon target = {(int64_t)(lat * 1e9), (int64_t)(lon * 1e9)};
    
    for(auto& [id, node] : graph.nodes) {
        // skip nodes without neighbours or invalid coords
        if(graph.adjacency_list.count(id) == 0) continue;
        if(node.coords.lat == 0 && node.coords.lon == 0) continue;
        
        double dist = haverstine(node.coords, target);
        if(dist < best_dist) {
            best_dist = dist;
            best_id = id;
        }
    }
    return best_id;
}

void printBlobInfo(const OSMPBF::BlobHeader& blob_header, const OSMPBF::Blob& blob, const OSMPBF::PrimitiveBlock& primitive_block) {
    cout << "Blob type: " << blob_header.type() << endl;
    cout << "Blob data size: " << blob_header.datasize() << endl;
    cout << "Blob raw size: " << blob.raw_size() << endl;
    cout << "Primitive groups: " << primitive_block.primitivegroup_size() << endl;

    if(blob_header.type() != "OSMData") return;

    const auto& group = primitive_block.primitivegroup(0);
    cout << "Nodes: " << group.nodes_size() << endl;
    cout << "Dense nodes: " << group.has_dense() << endl;
    cout << "Ways: " << group.ways_size() << endl;
    cout << "Relations: " << group.relations_size() << endl;
    const auto& dense = group.dense();
    cout << "Dense node count: " << dense.id_size() << endl;

// Calculate the latitude and longitude of the first node in the dense nodes

    LatLon first_node = calculateLatLon(primitive_block, dense);
    cout << "First node lat: " << first_node.lat / 1e9 << endl;
    cout << "First node lon: " << first_node.lon / 1e9 << endl;

}

void extractNodes(const OSMPBF::PrimitiveBlock& primitive_block, const OSMPBF::DenseNodes& dense, Graph& graph){
    
    int64_t accumulated_id = 0;
    int64_t accumulated_lat = 0;
    int64_t accumulated_lon = 0;
    int64_t granularity = primitive_block.granularity();
    int64_t lat_offset = primitive_block.lat_offset();
    int64_t lon_offset = primitive_block.lon_offset();

    for(int i = 0; i < dense.id_size();i++){
        Node n;
        accumulated_lat += dense.lat(i);
        accumulated_lon += dense.lon(i);
        accumulated_id += dense.id(i);

        n.coords.lat = lat_offset + granularity * accumulated_lat;
        n.coords.lon = lon_offset + granularity * accumulated_lon;

        int64_t id = accumulated_id;
        graph.nodes[id] = n;
    } 
}

void extractWays(const OSMPBF::PrimitiveGroup& group, Graph& graph, 
                            const OSMPBF::PrimitiveBlock& primitive_block) {
    
    const auto& string_table = primitive_block.stringtable();
    
    for(int i = 0; i < group.ways_size(); i++) {
        const auto& way = group.ways(i);
        
        // Check if this way has a "highway" tag
        bool is_highway = false;
        for(int k = 0; k < way.keys_size(); k++) {
            const string& key = string_table.s(way.keys(k));
            if(key == "highway") {
                is_highway = true;
                break;
            }
        }
        if(!is_highway) continue;  // skip non-road ways
        
        int64_t accumulated_ref = 0;
        int64_t prev_node_id = -1;

        for(int j = 0; j < way.refs_size(); j++) {
            accumulated_ref += way.refs(j);
            
            if(prev_node_id != -1) {
                graph.adjacency_list[accumulated_ref].push_back(prev_node_id);
                graph.adjacency_list[prev_node_id].push_back(accumulated_ref);
            }
            prev_node_id = accumulated_ref;
        }
    }
}

// Reads a single block from the OSM PBF file, parses it, and extracts nodes into the graph
bool readBlock(istream& file, Graph& graph) {

    // Read the first 4 bytes to get the header size
    char buffer[4];
    if(!file.read(buffer, 4))  {
        return false; 
    }
    uint32_t header_size = ((uint8_t)buffer[0] << 24)  | 
                                        ((uint8_t)buffer[1] << 16)  | 
                                        ((uint8_t)buffer[2] << 8)    | 
                                        ((uint8_t)buffer[3] );

    // Read the header data
    vector<char> blob_header_buffer(header_size);
    if(!file.read(blob_header_buffer.data(), header_size)) {
        return false;
    }

    // Parse the header using protobuf and print the blob type and sizes
    OSMPBF::BlobHeader blob_header;
    if(!blob_header.ParseFromArray(blob_header_buffer.data(), header_size)) {
        cerr << "Failed to parse BlobHeader!" << endl;
        return false; 
    }

    // Read the blob data based on the size specified in the header
    vector<char> blob_data_buffer(blob_header.datasize());
    OSMPBF::Blob blob;
    if(!file.read(blob_data_buffer.data(), blob_header.datasize())) {
        return false;
    }
    if(!blob.ParseFromArray(blob_data_buffer.data(), blob_header.datasize())) {
        cerr << "Failed to parse Blob!" << endl;
        return false;
    }

    //bytef is a type defined in zlib, representing an unsigned char
    //uLong is a type defined in zlib, representing an unsigned long
    // Decompress the blob data if it is compressed using zlib

    uLongf decompressed_size = blob.raw_size();
    vector<Bytef> buffer_decompressed(decompressed_size); 

    // Decompress the blob data using zlib, parameter: destination buffer, destination size, source buffer, source size
    int result = uncompress(buffer_decompressed.data(), &decompressed_size, 
    (const Bytef*)blob.zlib_data().data(), blob.zlib_data().size());

    // Z_OK is a constant defined in zlib that indicates successful decompression
    if(result != Z_OK) {
        cerr << "Failed to decompress blob data!" << endl;
        return false;
    }

    if(blob_header.type() != "OSMData") {
        return true; // Skip non-OSMData blobs but continue processing the file
    }
    OSMPBF::PrimitiveBlock primitive_block;
    if(!primitive_block.ParseFromArray(buffer_decompressed.data(), decompressed_size)) {
        cerr << "Failed to parse PrimitiveBlock!" << endl;
        return false;
}

for(int g = 0; g < primitive_block.primitivegroup_size(); g++) {
    const auto& group = primitive_block.primitivegroup(g);
    if(group.has_dense()) {
        extractNodes(primitive_block, group.dense(), graph);
    }
    if(group.ways_size() > 0){
        extractWays(group, graph, primitive_block);
    }
}



   //printBlobInfo(blob_header, blob, primitive_block);

    return true;
}



int main() {

    // Load graph from PBF file
    ifstream file("lisbon.osm.pbf", ios::binary);
    if(!file.is_open()) {
        cerr << "Error opening file!" << endl;
        return 1;
    }

    Graph graph;
    int block_count = 0;
    while(file.good()) {
        if(!readBlock(file, graph)) break;
        block_count++;
        if(block_count % 100 == 0) {
            cout << "Processed " << block_count << " blocks, nodes: " << graph.nodes.size() << endl;
        }
    }
    file.close();

    cout << "Total nodes: " << graph.nodes.size() << endl;
    cout << "Total nodes in adjacency list: " << graph.adjacency_list.size() << endl;



    // Find nearest nodes to two points in Lisbon
    int64_t start = findNearestNode(38.7077, -9.1365, graph); // Praça do Comércio
    int64_t goal  = findNearestNode(38.7169, -9.1399, graph); // Rossio

    cout << "Start: " << start << endl;
    cout << "Goal: " << goal << endl;
    // Run A* pathfinding
    vector<int64_t> path = astar(start, goal, graph);
    cout << "Path length: " << path.size() << " nodes" << endl;

    return 0;
}