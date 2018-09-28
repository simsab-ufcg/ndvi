#include "landsat_function.h"

LandsatFunction::LandsatFunction(ldouble sun_elevation){
    this->sun_elevation = sun_elevation;
}

void LandsatFunction::processNDVI(Tiff band4, Tiff band5, Tiff ndvi){
    TIFFClose(band4);
    TIFFClose(band5);
    TIFFClose(ndvi);
    return;
}