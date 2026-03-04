#include "parser.h"
#include <zlib.h>

LatLon calculateLatLon(const OSMPBF::PrimitiveBlock& primitive_block, const OSMPBF::DenseNodes& dense) {

    int64_t granularity = primitive_block.granularity();
    int64_t latOffset = primitive_block.lat_offset();
    int64_t lonOffset = primitive_block.lon_offset();
    int64_t lat = latOffset + granularity * dense.lat(0);
    int64_t lon = lonOffset + granularity * dense.lon(0);
    return {lat, lon};

}


void extractNodes(const OSMPBF::PrimitiveBlock& primitive_block, const OSMPBF::DenseNodes& dense, Graph& graph){
    
    int64_t accumulatedId = 0;
    int64_t accumulatedLat = 0;
    int64_t accumulatedLon = 0;
    int64_t granularity = primitive_block.granularity();
    int64_t latOffset = primitive_block.lat_offset();
    int64_t lonOffset = primitive_block.lon_offset();

    for(int i = 0; i < dense.id_size();i++){
        Node n;
        accumulatedLat += dense.lat(i);
        accumulatedLon += dense.lon(i);
        accumulatedId += dense.id(i);

        n.coords.lat = latOffset + granularity * accumulatedLat;
        n.coords.lon = lonOffset + granularity * accumulatedLon;

        int64_t id = accumulatedId;
        graph.nodes[id] = n;
    } 
}

void extractWays(const OSMPBF::PrimitiveGroup& group, Graph& graph, 
                            const OSMPBF::PrimitiveBlock& primitive_block, int8_t transportMethod) {
    
    const auto& stringTable = primitive_block.stringtable();
    for(int i = 0; i < group.ways_size(); i++) {
        const auto& way = group.ways(i);
        string highwayType;
        // Check if this way has a "highway" tag
        bool isHighway = false;
        for(int k = 0; k < way.keys_size(); k++) {
            const string& key = stringTable.s(way.keys(k));
            if(key == "highway") {
                isHighway = true;
                // store type of highway: primary, footway, residential...
                highwayType = stringTable.s(way.vals(k));
                break;
            }
        }        
        if(!isHighway) continue;  // skip non-road ways

        int64_t accumulatedRef = 0;
        int64_t prevNodeId = -1;

        for(int j = 0; j < way.refs_size(); j++) {
            accumulatedRef += way.refs(j);
            
            if(prevNodeId != -1) {
                graph.adjacencyList[accumulatedRef].push_back({prevNodeId,highwayType});
                graph.adjacencyList[prevNodeId].push_back({accumulatedRef,highwayType});
            }
            prevNodeId = accumulatedRef;
        }
    }
}


// Reads a single block from the OSM PBF file, parses it, and extracts nodes into the graph
bool readBlock(istream& file, Graph& graph, int8_t transportMethod) {
    
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

    if(blob_header.type() != "OSMData") {
    // read and discard the blob bytes
        vector<char> skip(blob_header.datasize());
        file.read(skip.data(), blob_header.datasize());
        return true;
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
            extractWays(group, graph, primitive_block, transportMethod);
        }
}
    return true;
}

