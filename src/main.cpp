#include <iostream>
#include <fstream>
#include "utils.h"
#include "astar.h"
#include "transport.h"
#include "io/parser.h"
#include "io/nominatin.h"
#include <algorithm>
#include "kdTree.h"
using namespace std;

int main() {
    int8_t transportMethod = 0;
    string chosen;
    while(true){

        cout << "How are you getting around Lisbon? (car / bike / walk): ";        cin  >> chosen;
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
    cout << "Mapping every street in Lisbon. This may take a moment..." << endl;
    while(file.good()) {
        if(!readBlock(file, graph, transportMethod)) break;        
    }
    file.close();
    cout << "Lisbon loaded --> " << graph.nodes.size() << " locations, " 
    << graph.adjacencyList.size() << " road segments mapped." << endl;
    KdTree kdTree;
    buildKdTree(kdTree,graph,transportMethod);

    // Find nearest nodes to two points in Lisbon    
    int answer;
    cout << "Search by [1] name  [0] coordinates: ";    
    cin >> answer;
    if(answer){
        Coords startCoords = askUserForCoordinates(1);
        cout << "From: " << startCoords.lat << ", " << startCoords.lon << endl;
        Coords goalCoords = askUserForCoordinates(2);
        cout << "To:   " << goalCoords.lat << ", " << goalCoords.lon << endl;

        int64_t start = findNearest(kdTree, startCoords ); 
        int64_t goal = findNearest(kdTree,goalCoords ); 
        cout << "start id: " << start << " goal id: " << goal << endl;

        vector<int64_t> path = astar(start, goal, graph,transportMethod);
        cout << "Route found --> " << path.size() << " nodes across Lisbon." << endl;
        bool route;
        cout << "Would you like to see the streets of your route? [1] yes  [0] no: ";
        cin >> route;
        if(route){
        printRoute(path, graph);
    } 
    } else if(!answer) {
        double startLat,startLon, goalLat,goalLon;
        for(int i = 0; i < 4; i++){
            switch(i){
                case 0: {
                    cout << "Insert start Lat" << endl;
                    cin >> startLat;
                    break;
                } case 1: {
                    cout << "Insert start Lon" << endl;
                    cin >> startLon;
                    break;
                } case 2: {
                    cout << "Insert goal Lat" << endl;
                    cin >> goalLat;
                    break;
                } case 3: {
                    cout << "Insert goal Lon" << endl;
                    cin >> goalLon;
                    break;
                } default: break;
            }
        }
        
        Coords startDest = {startLat,startLon}; 
        Coords goalDest = {goalLat,goalLon}; 
        if(!checkBoundingBox(startDest) || !checkBoundingBox(goalDest)) return 1;

        string startName = reverseGeocode(startDest);
        string goalName = reverseGeocode(goalDest);

        cout << "From: "<< startName<< endl <<"To: " << goalName << endl;
        int64_t start = findNearest(kdTree,startDest);
        int64_t goal = findNearest(kdTree,goalDest);

        vector<int64_t> path = astar(start, goal, graph, transportMethod);
        cout << "Route found --> " << path.size() << " nodes across Lisbon." << endl;
        bool route;
    cout << "Would you like to see the streets of your route? [1] yes  [0] no: ";
    cin >> route;
    if(route){
        printRoute(path, graph);
    } 
    } else {
        cerr << "Error choosing inserting coords method file!" << endl;
    }
    
    
    return 0;
} 