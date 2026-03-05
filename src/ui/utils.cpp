#include "utils.h"
#include <iostream>
#include "transport.h"
#include <algorithm>
#include "astar.h"
#include "nominatin.h"
using namespace std;


bool checkBoundingBox(const Coords& coords){

    const double LAT_MIN = 38.65, LAT_MAX = 38.80;
    const double LON_MIN = -9.25, LON_MAX = -9.05;

    if(coords.lat < LAT_MIN || coords.lat > LAT_MAX || coords.lon < LON_MIN || coords.lon > LON_MAX) {
        cerr << "Coordinates outside Lisbon!" << endl;
        return false;
    } else {
        return true;
    }

}



Coords askUserForCoordinates(int8_t flag) {
    switch(flag){
        case 1:{
            string startName;
            cout << "Where are you starting from? " << endl;
            cin.ignore();
            getline(cin, startName);
            Coords startCoords = forwardGeocode(startName);
            if(!checkBoundingBox(startCoords)) return {0,0};            
            return {startCoords.lat, startCoords.lon};
        } case 2: {
            string goalName;
            cout << "Where do you want to go? " << endl;            
            getline(cin, goalName);
            Coords goalCoords = forwardGeocode(goalName);
            if(!checkBoundingBox(goalCoords)) return {0,0};            
            return {goalCoords.lat, goalCoords.lon};
        } default: break;
    }
        
    return {0,0};
}



int64_t findNearestNode(Coords target, Graph& graph, int8_t transportMethod) {
    int64_t bestId = -1;
    double bestDist = numeric_limits<double>::infinity();
    LatLon targetNano = {(int64_t)(target.lat * 1e9), (int64_t)(target.lon * 1e9)};
    
    for(auto& [id, node] : graph.nodes) {

        // skip nodes without neighbours or invalid coords
        if(graph.adjacencyList.count(id) == 0) continue;
        if(node.coords.lat == 0 && node.coords.lon == 0) continue;

        // check if node has a valid neighbour for the transport method
        bool hasValidNeighbour = false;
        const vector<string>& allowedTypes = (transportMethod == 1) ? CAR_TYPES : WALK_TYPES;
        for(auto& [nId, type] : graph.adjacencyList[id]) {
            if(find(allowedTypes.begin(), allowedTypes.end(), type) != allowedTypes.end()) {
            hasValidNeighbour = true;
            break;
            }
        }
        if(!hasValidNeighbour) continue;
        
        double dist = harvesine(node.coords, targetNano);
        if(dist < bestDist) {
            bestDist = dist;
            bestId = id;
        }
    }
    return bestId;
}

