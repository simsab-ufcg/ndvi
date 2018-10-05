#include "landsat_function.h"

LandsatFunction::LandsatFunction(ldouble sun_elevation){
    this->sun_elevation = sun_elevation;
}

void LandsatFunction::processNDVI(Tiff band4, Tiff band5, Tiff ndvi, Tiff band_bqa){
    return;
}