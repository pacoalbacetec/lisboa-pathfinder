## Introduction

I made this file so that anyone can understand this repository, the OSM data it works with, and the logic behind it. It also serves as a personal reference — a way to solidify and review my own understanding of the concepts involved, so I can come back to it later if anything needs refreshing.

## What is OSM? And the PBF format?

OpenStreetMap is a collaborative map of the world, created by anyone who wants to contribute. The motivation behind it was that geographic information was not publicly available, so the goal was to make it free for everyone. Years later, OSM is used by companies like Yahoo!, Amazon, and Microsoft.

OSM doesn't store a map image. Instead, it contains three core elements and a tagging system.

- **Nodes:** Specific points defined by latitude and longitude. They can represent standalone features like a tree or a mailbox, or serve as the building blocks of lines and areas.

- **Ways:** Ordered lists of nodes.
    - **Open Ways:** Lines such as rivers or railways.
    - **Closed Ways:** Areas where the first and last node are the same, like a park or a building footprint.

- **Relations:** Define how different elements interact. For example, a bus route is a relation that joins several ways together.

### OSM Tagging System

Tags provide meaning to elements, stored as simple `key=value` pairs.

| Key | Example Value | Description |
| :--- | :--- | :--- |
| `highway` | `motorway`, `residential` | Defines the type of road. |
| `building` | `yes`, `apartments`, `retail` | Identifies a structure and its use. |
| `amenity` | `cafe`, `hospital`, `pharmacy` | Facilities useful to the public. |
| `natural` | `tree`, `water`, `wood` | Geological or ecological features. |
| `name` | `Main Street` | The actual name of the feature. |

<img src="/docs/OSM.jpg" width="500"/>

Nowadays, OSM and PBF are almost always mentioned together. Protocol Buffer Binary Format is a binary compressed file format mainly used by OSM to store its data. It emerged as an alternative to the traditional XML format. PBF is 5x faster to parse and 30% smaller in size, making it ideal for large datasets like world maps. We will explore how it achieves this performance in the next section.

## PBF format
A pbf file is not a continuous data flow, It contains Blobs (Binary Large Objects). Each *Blob* is composed of a *BlobHeader* and a *Blob*.  The first 4 bytes of a blob indicate the size of the BlobHeader, followed by the BlobHeader itself, and then the Blob data.

**1.** **BlobHeader:** 
- It contains metadata about the blob, such as its type and size.
- The type indicates what kind of data the blob contains (e.g., "OSMData" for OSM data blobs).
- The size specifies the length of the blob's data in bytes.

```cpp
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
```

**2.** **Blob:**
- It contains the actual data, which can be compressed or uncompressed. Usually, the data is compressed using zlib to save space.
- The blob's data is structured according to the OSM PBF specification, which organizes the data into blocks and groups for efficient storage and retrieval.


### Visual 1: PBF File Structure

<img src="/docs/Visual1.png" width="300"/>

```cpp
if(blob_header.type() != "OSMData") {
    return true; // Skip non-OSMData blobs but continue processing the file
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

```
Since the blob data is compressed, we first need to know the uncompressed size (stored in `blob.raw_size()`), allocate a buffer for it, and then decompress using zlib's `uncompress` function.
Zlib decompression:
```cpp
    /*  bytef is a type defined in zlib, representing an unsigned char
        uLong is a type defined in zlib, representing an unsigned long
        Decompress the blob data if it is compressed using zlib */
    uLongf decompressed_size = blob.raw_size();
    vector<Bytef> buffer_decompressed(decompressed_size); 

    // Decompress the blob data using zlib, parameter: destination buffer, destination size, source buffer, source size
    int result = uncompress(buffer_decompressed.data(), &decompressed_size, (const Bytef*)blob.zlib_data().data(), blob.zlib_data().size());

    // Z_OK is a constant defined in zlib that indicates successful decompression
    if(result != Z_OK) {
        cerr << "Failed to decompress blob data!" << endl;
        return false;
    }
```
Once decompressed, `buffer_decompressed` contains the raw bytes of a `PrimitiveBlock`, which we parse using Protobuf:

3. **PrimitiveBlock:**
- This is the main data block within a blob that contains the OSM elements (nodes, ways, and relations).
- It is organized into *PrimitiveGroups* ,*StringTable* and *Granularity*:
    -  *PrimitiveGroup*: Are collections of similar OSM elements. Commonly, It will contain either nodes, ways, or relations, but not a mix of them. This organization allows for more efficient storage and retrieval of OSM data. 

    -  *StringTable*: A dictionary of unique strings used in the PrimitiveGroups to reduce redundancy. Instead of storing the same string multiple times, the PrimitiveGroups reference the StringTable by index, which helps to save space and improve performance.

    - *Granularity*: A scaling factor used to convert the integer coordinates of nodes into their actual latitude and longitude values. This allows for more efficient storage of geographic data while still maintaining precision.

### Visual 2: PrimitiveBlock Anatomy

![PrimitiveBlock structure diagram](/docs/Visual2.png)

```cpp
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
```
So the whole pipeline for reading and processing a PBF file looks like this:

![Pipeline diagram](/docs/Visual3.png)



