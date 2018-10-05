#pragma once

#include <fstream>
#include <sstream>
#include "types.h"

using namespace std;

struct ReadSunEarth{
    string path_d_sun_earth;

    ReadSunEarth(string _path_d_sun_earth): path_d_sun_earth(_path_d_sun_earth) {};

    ldouble getDistance(int julian_day);
};
  