#include "astar.h"
#include <cmath>
#include <algorithm>
#include "transport.h"


// haverstine formula to calculate the distance between two points on the earth's surface
double haverstine(const LatLon& a, const LatLon& b){

    double latA = a.lat/1e9,
        latB = b.lat/1e9,
        lonA = a.lon/1e9,
        lonB = b.lon/1e9;

    const double R = 6371000;
    // lat and lon y radians
    double dLat = (latB-latA) * M_PI / 180.0,
                dLon = (lonB-lonA) * M_PI / 180.0;
    double a_ = sin(dLat/2) * sin(dLat/2) +
                cos(latA * M_PI / 180.0) * cos(latB * M_PI / 180.0) *
                sin(dLon/2) * sin(dLon/2);
    double c = 2 * atan2(sqrt(a_), sqrt(1-a_));
    return R*c;

}

/**    
        g_score = cost from start to current node
        h_score = heuristic cost from current node to goal
        f_score = g_score + h_score
**/

vector<int64_t> astar(int64_t start, int64_t goal, Graph& graph, int8_t transportMethod){

    //set of nodes to be evaluated, ordered by f_score
    priority_queue<pair<double,int64_t>, 
                            vector<pair<double,int64_t>>, 
                            greater<pair<double,int64_t>>> 
                            open_set;

    //maps to reconstruct the path and track scores
    unordered_map<int64_t, int64_t> came_from;
    //set of nodes already evaluated
    unordered_set<int64_t> closed_set;
    //g_score and f_score maps
    unordered_map<int64_t, double> g_score;

    // intialize the first node 
    g_score[start] = 0;
    double h = haverstine(graph.nodes[start].coords, graph.nodes[goal].coords);
    open_set.push(make_pair(h, start));

    const vector<string>& allowedTypes = (transportMethod == 1) ? CAR_TYPES : WALK_TYPES;
    while(!open_set.empty()){
        auto [f,id] = open_set.top();
        open_set.pop();

        //if this node has already been evaluated, skip it
        if(closed_set.count(id)) continue; 

        if(id == goal){
            vector<int64_t> path;
            int64_t current = goal;
            while(current != start){
                path.push_back(current);
                current = came_from[current];
            }
            path.push_back(start);
            reverse(path.begin(),path.end());
            return path;
        }

        //mark current node as evaluated
            closed_set.insert({id});
        //evaluate neighbours
            for(auto& [neighbourId, highwayType] : graph.adjacency_list[id]){
                
                if(closed_set.count(neighbourId)) continue;
                if(graph.nodes[neighbourId].coords.lat == 0 && graph.nodes[neighbourId].coords.lon == 0) continue;
                if(find(allowedTypes.begin(), allowedTypes.end(), highwayType) == allowedTypes.end()) continue;

            //new g_score for neighbour is g_score of current node + distance to neighbour
                double g_new = g_score[id] + haverstine(graph.nodes[id].coords, graph.nodes[neighbourId].coords);
                if(g_score.find(neighbourId) == g_score.end() || g_new < g_score[neighbourId]) {
                    came_from[neighbourId] = id;
                    g_score[neighbourId] = g_new;
                    double f = g_new + haverstine(graph.nodes[neighbourId].coords, graph.nodes[goal].coords);
                    open_set.push(make_pair(f, neighbourId));
                }
            }
        }
        return {};

    }


