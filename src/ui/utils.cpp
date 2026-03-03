#include "utils.h"
#include <iostream>
#include "transport.h"
#include <algorithm>
#include "astar.h"
using namespace std;


Coords askUserForCoordinates(const int8_t flag) {
    double startCoordsLat, startCoordsLon, goalCoordsLat, goalCoordsLon;
    switch(flag){
        case 1:{
            cout << "Please introduce the coordinates of the start point: "<< "lat" << endl;
            cin >> startCoordsLat;
            cout << "Please introduce the coordinates of the start point: "<< "lon" << endl;
            cin >> startCoordsLon;
            return {startCoordsLat, startCoordsLon};
        } case 2:
            cout << "Please introduce the coordinates of the goal point: "<< "lat" << endl;
            cin >> goalCoordsLat;
            cout << "Please introduce the coordinates of the goal point: "<< "lon" << endl;
            cin >> goalCoordsLon;
            return {goalCoordsLat, goalCoordsLon};
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
        
        double dist = haverstine(node.coords, targetNano);
        if(dist < bestDist) {
            bestDist = dist;
            bestId = id;
        }
    }
    return bestId;
}

