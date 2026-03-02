# Lisboa Pathfinder

Exploring Lisbon through OpenStreetMap data, graph algorithms, and real city APIs.

## Motivation

Moving to Lisbon for a year as an Erasmus student. This started as an excuse to work with real geographic data at a low level — parsing binary formats, building graph structures, and implementing pathfinding from scratch. The direction of the project is still evolving, but the end goal is something closer to a dynamic router that factors in real-world conditions like live traffic, public transport, and city data.

## Status

🚧 Active development. The project is still finding its shape — priority right now is learning and building solid foundations.

**Done:**
- PBF binary parsing (BlobHeader, Blob, PrimitiveBlock)
- zlib decompression of OSM data blocks
- Dense node coordinate extraction with delta decoding
- Way parsing with highway tag filtering
- Graph construction (nodes + adjacency list)
- A* pathfinding with Haversine heuristic
- Transport method selection (car / walk) with highway type filtering
- User input for start and goal coordinates

**Planned:**
- Graph cache (serialize/deserialize to avoid reloading PBF every run)
- Nominatim API integration — search by place name instead of coordinates
- Trie for place name autocomplete
- k-d tree for efficient nearest node lookup
- Multimodal routing (walking + driving combined)
- Lisboa open data API integration:
  - Live traffic data
  - Bus and parking info
  - Traffic sign data
  - Traffic light estimation (reverse engineering / heuristics)
- INTERNALS.md — visual deep-dive into the PBF format, varint encoding, and delta compression

## Dependencies

- [MSYS2](https://www.msys2.org/) — package manager for Windows
- CMake
- Protobuf
- zlib

Install dependencies via MSYS2:
```bash
pacman -S mingw-w64-ucrt-x86_64-protobuf mingw-w64-ucrt-x86_64-zlib mingw-w64-ucrt-x86_64-cmake
```

## Data

Download the Lisbon OSM dataset from [openstreetmap.fr](http://download.openstreetmap.fr/extracts/europe/portugal/lisbon.osm.pbf) and place `lisbon.osm.pbf` in the project root.

## Building

```bash
cmake -B build -DCMAKE_PREFIX_PATH="C:/msys64/ucrt64"
cmake --build build
```

## Running

```bash
.\build\lisboa.exe
```

## Tech

- Language: C++17
- Data: OpenStreetMap (via openstreetmap.fr)
- Format: PBF (Protocol Buffers + zlib)
- Algorithms: A* with Haversine heuristic

## License

MIT