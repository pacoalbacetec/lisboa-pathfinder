# Lisboa Pathfinder

A personal route planner for Lisbon built in C++.

## Motivation

Moving to Lisbon for a year as an Erasmus student. This started as an excuse to work with real geographic data at a low level — parsing binary formats, building graph structures, and implementing pathfinding from scratch. The end goal is something actually useful for getting around the city.

## Goals

- Parse real OpenStreetMap data (PBF format) at a low level
- Build an efficient graph of Lisbon's street network
- Implement smart pathfinding (A*) with custom heuristics
- Factor in personal preferences when routing
- Fast terminal-based interface

## Status

🚧 Active development.

**Done:**
- PBF binary parsing (BlobHeader, Blob, PrimitiveBlock)
- zlib decompression of OSM data blocks
- Dense node coordinate extraction with delta decoding
- Way parsing with highway tag filtering
- Graph construction (nodes + adjacency list)
- A* pathfinding with Haversine heuristic

**Next:**
- Graph cache (serialize/deserialize to avoid reloading PBF every run)
- Filter by pedestrian-friendly highway types
- INTERNALS.md — visual deep-dive into the PBF format, varint encoding, and delta compression
- User input for start/end points
- Personal preference weighting

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