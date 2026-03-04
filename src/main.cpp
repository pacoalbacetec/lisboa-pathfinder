#include <iostream>
#include <fstream>
#include "utils.h"
#include "astar.h"
#include "transport.h"
#include "io/parser.h"
#include "cache.h"
#include <algorithm>
using namespace std;

int main() {
    int8_t transportMethod = 0;
    string chosen;
    while(true){
        cout << "Choose a transport method (Car, Bike, Walk): " << endl;
        cin  >> chosen;
        transform(chosen.begin(), chosen.end(), chosen.begin(), ::tolower);
        if(chosen == "car" || chosen == "bike"){
            transportMethod = 1;
            break;
        } else if(chosen == "walk"){
            transportMethod = 2;
            break;
        } else {
            cout << "Invalid input" << endl;
        }
    }
    Graph graph;

    // Load graph from PBF file
    ifstream file("lisbon.osm.pbf", ios::binary);
    if(!file.is_open()) {
        cerr << "Error opening file!" << endl;
        return 1;
    }

    int blockCount = 0;
    while(file.good()) {
        if(!readBlock(file, graph, transportMethod)) break;
            blockCount++;
            if(blockCount % 100 == 0) {
                cout << "Processed " << blockCount << " blocks, nodes: " << graph.nodes.size() << endl;
        }
    }

    file.close();


    cout << "Total nodes: " << graph.nodes.size() << endl;
    cout << "Total nodes in adjacency list: " << graph.adjacencyList.size() << endl;

    // Find nearest nodes to two points in Lisbon
    Coords startCoords = askUserForCoordinates(1);
    cout << "Start: " << startCoords.lat << ", " << startCoords.lon << endl;
    Coords goalCoords = askUserForCoordinates(2);
    cout << "Goal: " << goalCoords.lat << ", " << goalCoords.lon << endl;

    int64_t start = findNearestNode(startCoords, graph, transportMethod); // Praça do Comércio
    int64_t goal = findNearestNode(goalCoords, graph, transportMethod); // Rossio

    // Run A* pathfinding
    vector<int64_t> path = astar(start, goal, graph,transportMethod);
    cout << "Path length: " << path.size() << " nodes" << endl;

    return 0;
}