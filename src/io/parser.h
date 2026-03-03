#ifndef PARSER_H
#define PARSER_H
#include <cstdint>
#include "proto/fileformat.pb.h"
#include "proto/osmformat.pb.h"
#include "graph.h"
using namespace std;

void extractNodes(const OSMPBF::PrimitiveBlock& primitive_block, const OSMPBF::DenseNodes& dense, Graph& graph);

void extractWays(const OSMPBF::PrimitiveGroup& group, Graph& graph, 
                            const OSMPBF::PrimitiveBlock& primitive_block, int8_t transportMethod);

bool readBlock(istream& file, Graph& graph, int8_t transportMethod);

LatLon calculateLatLon(const OSMPBF::PrimitiveBlock& primitive_block, const OSMPBF::DenseNodes& dense);
#endif 