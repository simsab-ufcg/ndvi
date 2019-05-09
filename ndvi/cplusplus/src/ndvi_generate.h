#pragma once

#include "types.h"
#include "utils.h"
#include <math.h>
#include <string.h>

struct NDVIGenerate{
    ldouble sintheta;
    Tiff band_red, band_nir, band_bqa;
    PixelReader pixel_read_band_red, pixel_read_band_nir, pixel_read_band_bqa;
    tdata_t line_band_red, line_band_nir, line_band_bqa;

    NDVIGenerate(ldouble _sun_elevation, Tiff _band_red, Tiff _band_nir, Tiff _band_bqa);
    void processNDVI(int number_sensor, ldouble dist_sun_earth, Tiff ndvi);
    void landsat(Tiff ndvi, int width_band, int height_band, int mask, ldouble dist_sun_earth, vector<ldouble> param_band_red, vector<ldouble> param_band_nir);
    void landsat(Tiff ndvi, int width_band, int height_band, int mask);
};