#include "read_meta.h"
#include <string.h>

string ReadMeta::search(string filter){
    ifstream in(path_meta_file);
    string line;
    bool flag = false;
    while(getline(in, line)){
        stringstream lineReader(line);
        string token;
        vector<string> nline;
        while(lineReader >> token){
            if(token == filter) flag = true;
            if(flag) nline.push_back(token);
        }
        if(flag) return nline[2];
    }
}

ldouble ReadMeta::getSunElevation(){
    string resultSearch = search("SUN_ELEVATION");
    return atof(resultSearch.c_str());
}

ldouble ReadMeta::getDistEarthSun(){
    string resultSearch = search("EARTH_SUN_DISTANCE");
    return atof(resultSearch.c_str());
}

vector<ldouble> ReadMeta::getReflectanceBand(int number_band){
    string resultSearchMultBand = search("REFLECTANCE_MULT_BAND_" + to_string(number_band));
    string resultSearchAddBand = search("REFLECTANCE_ADD_BAND_" + to_string(number_band));
    return {atof(resultSearchMultBand.c_str()), atof(resultSearchAddBand.c_str())};
}

int ReadMeta::getNumberSensor(){
    string resultSearch = search("LANDSAT_SCENE_ID");
    return atoi(new char(resultSearch[3]));
}

int ReadMeta::getJulianDay(){
    string resultSearch = search("LANDSAT_SCENE_ID");
    char julianDay[3];
    strncpy(julianDay, &resultSearch[14], 3);
    return atoi(julianDay);
}

int ReadMeta::getYear(){
    string resultSearch = search("LANDSAT_SCENE_ID");
    char year[4];
    strncpy(year, &resultSearch[10], 4);
    return atoi(year);
}