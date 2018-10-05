#ifndef LANDSAT_INCLUDED

    #define LANDSAT_INCLUDED

    #include "../types.h"
    #include <math.h>
    #include <string.h>
	#include "../utils.h"

    struct LandsatFunction{
        ldouble sun_elevation;

        LandsatFunction(ldouble sun_elevation);
        virtual void processNDVI(Tiff band4, Tiff band5, Tiff ndvi, Tiff band_bqa);
    };

#endif