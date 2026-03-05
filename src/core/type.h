#ifndef TYPE_H
#define TYPE_H
#include <cstdint>
#include <string>
using namespace std;

struct LatLon {
    int64_t lat;
    int64_t lon;
};
struct Node{
    int64_t id;
    LatLon coords; 
};

struct Coords {
    double lat;
    double lon;
};

struct Edge {
    int64_t node;
    string highwayType;
    string wayName;
} ;

#endif 