#include <iostream>
 #include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include "proto/fileformat.pb.h"
#include "proto/osmformat.pb.h"
 using namespace std;

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
    vector<char> blob_data_buffer(blob_header.datasize());
    OSMPBF::Blob blob;
    if(!file.read(blob_data_buffer.data(), blob_header.datasize())) {
        return;
    }
    if(!blob.ParseFromArray(blob_data_buffer.data(), blob_header.datasize())) {
        cerr << "Failed to parse Blob!" << endl;
        return;
    }
    

    cout << "Blob type: " << blob_header.type() << endl;
    cout << "Blob index data size: " << blob_header.indexdata().size() << endl;
    cout << "Blob data size: " << blob_header.datasize() << endl;


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