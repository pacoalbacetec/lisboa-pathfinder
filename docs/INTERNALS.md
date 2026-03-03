## Introduction

We made this file so that anyone can understand this repository, the OSM data it works with, and the logic behind it. It also serves as a personal reference — a way to solidify and review our understanding of the concepts involved, so we can come back to it later if anything needs refreshing.

>All code snippets shown throughout this document are taken directly from the project source code, not simplified examples.

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

## PBF Format

A PBF file is not a continuous data flow — it contains Blobs (Binary Large Objects). Each *Blob* is composed of a *BlobHeader* and a *Blob*. The first 4 bytes indicate the size of the BlobHeader, followed by the BlobHeader itself, and then the Blob data.

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
    uLongf decompressed_size = blob.raw_size();
    vector<Bytef> buffer_decompressed(decompressed_size); 

    int result = uncompress(buffer_decompressed.data(), &decompressed_size, (const Bytef*)blob.zlib_data().data(), blob.zlib_data().size());

    if(result != Z_OK) {
        cerr << "Failed to decompress blob data!" << endl;
        return false;
    }
```

Once decompressed, `buffer_decompressed` contains the raw bytes of a `PrimitiveBlock`, which we parse using Protobuf:

**3.** **PrimitiveBlock:**
- This is the main data block within a blob that contains the OSM elements (nodes, ways, and relations).
- It is organized into *PrimitiveGroups*, *StringTable* and *Granularity*:
    - *PrimitiveGroup*: Collections of similar OSM elements. Each group will typically contain either nodes, ways, or relations, but not a mix. This organization allows for more efficient storage and retrieval of OSM data.
    - *StringTable*: A dictionary of unique strings used in the PrimitiveGroups to reduce redundancy. Instead of storing the same string multiple times, the PrimitiveGroups reference the StringTable by index, saving space and improving performance.
    - *Granularity*: A scaling factor used to convert the integer coordinates of nodes into their actual latitude and longitude values, allowing for more efficient storage while maintaining precision.

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

## Protobuf and Varint Encoding

- **What is Protocol Buffers and for what purpose is it used?**

    Developed by Google, Protocol Buffers is a language-neutral, platform-neutral, extensible mechanism for serializing structured data. Think of it as XML or JSON, but smaller, faster, and simpler.

    In JSON, every message must include the full key names:
    `{"name": "John", "id": 123}`
    The word "name" is repeated every single time, which wastes space.

    In Protobuf, the keys are defined once in a `.proto` file and assigned a tag number:
    `name = Tag 1`
    `id = Tag 2`

    When the data is transmitted, the computer doesn't send "name" — it just sends the number 1. Both sides share the same codebook (in our case `osmformat.proto` and `fileformat.proto`), so they both know that 1 always means name.

- **What is varint encoding and how does it work?**

    This is one of Protobuf's most powerful features. In most programming languages, an integer like `int32` always occupies **4 bytes**, even if the value is `5`. Varint takes only as many bytes as the value actually needs:
    - `5` → **1 byte**
    - `500` → **2 bytes**

    Since most values in OSM data are small numbers, varint can reduce file size significantly. Here is a concrete example:
  ```
    Number: 300
    Binary: 1 0010 1100  (9 bits, does not fit in 7)

    Split into 7-bit groups (right to left):
    Group 1:  010 1100
    Group 2:  000 0010

    Add the continuation bit to each group:
    Byte 1:  [1] 010 1100  → 0xAC  (1 = more bytes follow)
    Byte 2:  [0] 000 0010  → 0x02  (0 = last byte)

    Varint result: 0xAC 0x02  →  2 bytes instead of 4

    To decode, strip the continuation bits and reverse the order:
    000 0010 ++ 010 1100  =  1 0010 1100  =  300  ✓
  ```

    >**Implementation Note:** Once the PBF block is decompressed in memory, we use `ParseFromArray(buffer, size)`. This method efficiently maps the binary stream to our C++ objects by decoding the field tags and varints according to the `.proto` definitions.

## Delta Encoding in Dense Nodes

- **What is delta encoding and why is it used for dense nodes?**

    Instead of storing absolute values, delta encoding stores only the difference between consecutive values. In OSM, this is particularly useful because nodes are often geographically close together, meaning the delta values are small.

    > Delta encoding has excellent synergy with varint encoding. Together, they turn large absolute IDs and coordinates into tiny differences that compress extremely efficiently.

    **A simple example:**
  ```
      In the PBF file (delta encoded):
      Node A: id=100, lat=50.0, lon=10.0  (base node)
      Node B: id=+1,  lat=+5,   lon=+2
      Node C: id=+7,  lat=+1,   lon=0
  ```

    **And here is how we decode it:**
  ```cpp
      void extractNodes(const OSMPBF::PrimitiveBlock& primitive_block, const OSMPBF::DenseNodes& dense, Graph& graph){
          
          int64_t accumulatedId = 0;
          int64_t accumulatedLat = 0;
          int64_t accumulatedLon = 0;
          int64_t granularity = primitive_block.granularity();
          int64_t latOffset = primitive_block.lat_offset();
          int64_t lonOffset = primitive_block.lon_offset();

          for(int i = 0; i < dense.id_size(); i++){
              Node n;
              accumulatedLat += dense.lat(i);
              accumulatedLon += dense.lon(i);
              accumulatedId += dense.id(i);

              n.coords.lat = latOffset + granularity * accumulatedLat;
              n.coords.lon = lonOffset + granularity * accumulatedLon;

              graph.nodes[accumulatedId] = n;
          } 
      }
  ```

    > **Implementation Note:** Ways and relations also use delta encoding for node references and member references respectively, following the same approach.

---

With the fundamental concepts of the PBF format covered, the next sections focus on the logic used to build the graph and implement the A* algorithm.

## Building the Graph

- **Our node vs an OSM node:**

    An OSM node is a raw geographic point with no routing significance on its own. It can represent a tree, a bus stop, or simply a shape point along a road. In our graph, we store all nodes from the PBF but only build edges (`adjacencyList`) between nodes that are part of a highway way.

- **How do we build the `adjacencyList` from the ways?**

    A Way is an ordered list of node references. To build the graph, we iterate every consecutive pair of nodes in each Way and add a bidirectional edge between them:
```cpp
    if(prevNodeId != -1) {
        graph.adjacencyList[accumulatedRef].push_back({prevNodeId, highwayType});
        graph.adjacencyList[prevNodeId].push_back({accumulatedRef, highwayType});
    }
    prevNodeId = accumulatedRef;
```

  So for a Way with nodes `[A, B, C, D]` we create edges `A↔B`, `B↔C`, `C↔D`.

- **Why do we only collect highway types?**

    OSM ways include everything — rivers, power lines, building outlines, administrative boundaries. Without filtering, the graph would contain millions of irrelevant edges, making A* impossible to run efficiently. By keeping only `highway` ways, we reduce the graph to the routable network.

## A* Algorithm and Haversine

- **How does the A\* algorithm work?**

    A* is a pathfinding algorithm that finds the shortest path between two nodes in a graph. It uses a priority queue to explore nodes based on their estimated total cost, calculated as:

    `f(n) = g(n) + h(n)`

    Where:
    - `g(n)` is the actual cost from the start node to node `n`.
    - `h(n)` is the heuristic estimate of the remaining cost from `n` to the goal.

    > The priority queue always expands the node with the lowest `f(n)` first, guiding the search efficiently toward the goal.

- **Why Haversine as the heuristic?**

    Haversine calculates the straight-line distance between two points on the surface of a sphere, accounting for the Earth's curvature. Since the straight-line distance between two points is always less than or equal to any real path along roads, it never overestimates — making it a perfect admissible heuristic for map-based routing.

    > A simpler Euclidean distance would also work at the scale of a city, but Haversine is more accurate for geographic coordinates and is the standard choice for routing applications.
    > The full A* implementation can be found in `src/routing/astar.cpp`.

- **How does path reconstruction work once we reach the goal?**

    During the search, every time a better path to a node is found, we record where we came from:
```cpp
    cameFrom[neighbourId] = id;
```

  When the goal is reached, we follow the `cameFrom` pointers backwards from goal to start, then reverse the result:
```cpp
    int64_t current = goal;
    while(current != start) {
        path.push_back(current);
        current = cameFrom[current];
    }
    path.push_back(start);
    reverse(path.begin(), path.end());
```

  Think of `cameFrom` as a trail of breadcrumbs — each node remembers which node led to it on the best known path.

  ![A* visualization](/docs/astar.gif)

## Fun Facts

*If OSM caught your interest after reading this, you will love these short fun facts.*

**The Haiti Earthquake, 2010**<br>
On January 12, 2010, a 7.0 magnitude earthquake devastated Haiti. Relief organizations desperately needed maps of Port-au-Prince — but almost none existed. Within 48 hours of the earthquake, the OSM community began mapping the area using satellite imagery. In under a month, over 600 volunteers from around the world built what became the most detailed map of Haiti ever made, entirely from scratch. It became the default map used by the UN, the World Bank, the U.S. Marine Corps, and dozens of NGOs coordinating the relief effort. OCHA described it as "basically the best source of transportation information that we have for Haiti."

**Meta has a dedicated OSM team**<br>
Since 2017, Meta has maintained a full team of engineers and mappers dedicated to contributing to OpenStreetMap. They use AI-assisted road tracing to map roads in countries like Thailand, Indonesia, Vietnam, India and Tanzania, and have built open source tools like RapiD — an AI-powered editor that dramatically speeds up the mapping process. Facebook, Instagram and WhatsApp all run on OSM data.

**Apple has 5,000 internal volunteers mapping OSM**<br>
Apple runs an internal volunteer programme where around 5,000 staff contribute to Missing Maps, a humanitarian mapping project built on top of OpenStreetMap.

**OSM receives up to 5 million changes every day**<br>
The global OSM database receives up to 5 million edits per day from contributors around the world. Every street you walk down in Lisbon, every café, every park — all maintained by volunteers.

---

## A Note on This Document

This document reflects the current state of the project. As Lisboa Pathfinder grows and new features are implemented, we will do our best to keep it up to date. Some sections may evolve, expand, or be added entirely as the project moves forward.