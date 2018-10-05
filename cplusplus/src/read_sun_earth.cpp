#include "read_sun_earth.h"

ldouble ReadSunEarth::getDistance(int julian_day){
    ifstream in(path_d_sun_earth);
    string line;
    bool flag = false;
    while(getline(in, line)){
        stringstream lineReader(line);
        string token;
        vector<string> nline;
        lineReader >> token;

        if(atoi(token.c_str()) == julian_day){
            lineReader >> token;
            return atof(token.c_str());
        }
    }
}