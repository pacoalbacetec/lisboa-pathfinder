#include <iostream>
 #include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include "proto/fileformat.pb.h"
#include "proto/osmformat.pb.h"
#include <zlib.h>
 using namespace std;

struct LatLon {
    int64_t lat;
    int64_t lon;
};

LatLon calculateLatLon(const OSMPBF::PrimitiveBlock& primitive_block, const OSMPBF::DenseNodes& dense) {
int64_t granularity = primitive_block.granularity();
int64_t lat_offset = primitive_block.lat_offset();
int64_t lon_offset = primitive_block.lon_offset();

int64_t lat = lat_offset + granularity * dense.lat(0);
int64_t lon = lon_offset + granularity * dense.lon(0);

return {lat, lon};

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

//flow: read 4 bytes ->header_size -> read header -> parse header -> print blob type and sizes

void readBlock(istream& file) {

    // Read the first 4 bytes to get the header size
    char buffer[4];
    if(!file.read(buffer, 4)) {
        return;
    }
    uint32_t header_size = ((uint8_t)buffer[0] << 24)  | 
                           ((uint8_t)buffer[1] << 16)  | 
                           ((uint8_t)buffer[2] << 8)    | 
                           ((uint8_t)buffer[3] );

    // Read the header data
    vector<char> blob_header_buffer(header_size);
    if(!file.read(blob_header_buffer.data(), header_size)) {
        return;
    }

    // Parse the header using protobuf and print the blob type and sizes
    OSMPBF::BlobHeader blob_header;
    if(!blob_header.ParseFromArray(blob_header_buffer.data(), header_size)) {
        cerr << "Failed to parse BlobHeader!" << endl;
        return;
    }

    // Read the blob data based on the size specified in the header
    vector<char> blob_data_buffer(blob_header.datasize());
    OSMPBF::Blob blob;
    if(!file.read(blob_data_buffer.data(), blob_header.datasize())) {
        return;
    }
    if(!blob.ParseFromArray(blob_data_buffer.data(), blob_header.datasize())) {
        cerr << "Failed to parse Blob!" << endl;
        return;
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
        return;
    }

    OSMPBF::PrimitiveBlock primitive_block;
    if(!primitive_block.ParseFromArray(buffer_decompressed.data(), decompressed_size)) {
        cerr << "Failed to parse PrimitiveBlock!" << endl;
        return;
    }

    printBlobInfo(blob_header, blob, primitive_block);

}

int main() {

    ifstream file("portugal-260222.osm.pbf", ios::binary);

    if(!file.is_open()) {
        cerr << "Error opening file!" << endl;
        return 1;
    }

    while(file.good()) {
        readBlock(file);
    }

    file.close();   

    return 0;
}