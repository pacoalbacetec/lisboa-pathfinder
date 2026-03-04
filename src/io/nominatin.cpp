#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "nominatin.h"
#include <curl/curl.h>
#include <string>
#include "utils.h"
#include "graph.h"

static size_t writeCallBack(char * ptr, size_t size, size_t nmemb, void* userdata){
    string * response = (string*)userdata;
    response->append(ptr,size*nmemb);
    return size * nmemb;
}

string httpGet(const string& url){
    //This is the handle, like the sesion at the http
    CURL* curl= curl_easy_init();
    string response;

    if(curl){
        //This function is to configure the options -> handle, optioin, value
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl,CURLOPT_USERAGENT, "lisboa-pathfinder/1.0");
        curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, &writeCallBack);
        curl_easy_setopt(curl,CURLOPT_WRITEDATA, &response);
        
        //Execute the action, and the always clean up to avoid memory leaks
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    return response;

}

Coords forwardGeocode(const string& query){

    //Creating a temporal handle so i can use curl_easy_escape to avoid illegal characters in the url
    CURL* curl = curl_easy_init();
    char* encoded = curl_easy_escape(curl, query.c_str(), query.length());
    string url = "https://nominatim.openstreetmap.org/search?q=" + string(encoded) + "&format=jsonv2&addressdetails=0&limit=1";
    curl_free(encoded);
    curl_easy_cleanup(curl);

    string response = httpGet(url);
    string keyLat = "\"lat\":";
    size_t Latkey = response.find(keyLat);
    size_t postFirstQuote = response.find("\"",Latkey + keyLat.length());
    size_t postSecondQuote = response.find("\"", postFirstQuote+1);
    double lat = stod(response.substr(postFirstQuote+1, postSecondQuote - postFirstQuote));


    string keyLon = "\"lon\":";
    size_t Lonkey = response.find(keyLon);
    size_t postFirstQuoteLon = response.find("\"",Lonkey + keyLon.length());
    size_t postSecondQuoteLon = response.find("\"", postFirstQuoteLon+1);
    double lon = stod(response.substr(postFirstQuoteLon+1, postSecondQuoteLon - postFirstQuoteLon));
    
    return {lat,lon};
    
}


string reverseGeocode(const Coords& coords){

    //Creating a temporal handle so i can use curl_easy_escape to avoid illegal characters in the url
    

    string url = "https://nominatim.openstreetmap.org/reverse?lat=" + 
    to_string(coords.lat) + "&lon=" +to_string(coords.lon)+  
    "&format=jsonv2&addressdetails=0&zoom=18";
    
    string response = httpGet(url);
    string keyName = "\"dislpay_name\":";
    size_t nameKey = response.find(keyName);
    size_t postFirstQuoteName = response.find("\"",nameKey + keyName.length());
    size_t postSecondQuoteName = response.find("\"", postFirstQuoteName+1);
    string name = response.substr(postFirstQuoteName+1, postSecondQuoteName - postFirstQuoteName);

    return name;
}