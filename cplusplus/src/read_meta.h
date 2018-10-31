#pragma once

#include <fstream>
#include <sstream>
#include "types.h"

using namespace std;

struct ReadMeta{
    string path_meta_file;

    ReadMeta(string _path_meta_file): path_meta_file(_path_meta_file) {};

    string search(string filter);
    ldouble getSunElevation();
    ldouble getDistEarthSun();
    vector<ldouble> getReflectanceBand(int band);
    int getNumberSensor();
};


