# Lisboa Pathfinder

A low-level route planner for Lisbon, built from scratch in C++.

## Motivation

Moving to Lisbon for a year as an Erasmus student. This started as an excuse to work with real geographic data at a low level — parsing binary formats, building graph structures, and implementing pathfinding from scratch. The direction of the project is still evolving, but the end goal is something closer to a router that factors in real-world conditions like live traffic, public transport, and city data.

> 📖 Check out [INTERNALS.md](/docs/INTERNALS.md) for a deep dive into how the PBF format works, how the graph is built, and how A* finds the path.

## Status

🚧 Active development. The project is still finding its shape — priority right now is learning and building solid foundations.

**Done:**
- Full PBF binary parser from scratch — BlobHeader, Blob, zlib decompression, delta decoding
- Graph construction from OSM ways with transport mode filtering (car / bike / walk)
- A* pathfinding with Haversine heuristic and k-d tree for O(log n) nearest node lookup
- Nominatim API integration — forward and reverse geocoding, search by place name or coordinates
- Street names in route output, extracted directly from OSM way tags


**Planned:**
- Waypoints — intermediate stops by name or coordinates
- Road weighting and turn restrictions — realistic routing using OSM data
- Bidirectional A* and Contraction Hierarchies — performance optimizations
- GPX export and Leaflet.js interactive demo — GitHub Pages, click to pick points
- WebAssembly — run C++ pathfinding in the browser via Emscripten
- Lisboa open data — Carris buses, parking availability, live traffic
- Favourites and route history — save places and recent searches
- Trie autocomplete for place name input

## Dependencies

- [MSYS2](https://www.msys2.org/) — package manager for Windows
- GCC (g++)
- CMake
- Protobuf
- zlib
- libcurl (for Nominatim API integration)

Install dependencies via MSYS2:
```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-protobuf mingw-w64-ucrt-x86_64-zlib mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-curl

```
Install dependencies via apt (Linux):
```bash
sudo apt install cmake libprotobuf-dev protobuf-compiler zlib1g-dev g++ libcurl4-openssl-dev
```
## Data

Download the Lisbon OSM dataset from [openstreetmap.fr](http://download.openstreetmap.fr/extracts/europe/portugal/lisbon.osm.pbf) and place `lisbon.osm.pbf` in the project root.

## Building

Windows:
```bash
cmake -B build -DCMAKE_PREFIX_PATH="C:/msys64/ucrt64"
cmake --build build
```
Linux:
```bash
cmake -B build
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