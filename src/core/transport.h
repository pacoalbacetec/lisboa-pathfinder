#ifndef TRANSPORT_H
#define TRANSPORT_H
#include <vector>
#include <string>
using namespace std;

const vector<string> CAR_TYPES = {
    "primary", "secondary", "tertiary", 
    "residential", "trunk", "unclassified"
};

const vector<string> WALK_TYPES = {
    "footway", "pedestrian", "path", 
    "steps", "residential", "living_street", "track"
};

#endif 