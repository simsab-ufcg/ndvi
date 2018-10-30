#pragma once

#include "types.h"
#include "utils.h"
#include <math.h>
#include <string.h>

struct NDVITOAGenerate{
    ldouble cosZenithAngle;
    Tiff band_4, band_5, band_bqa;
    PixelReader pixel_read_band_4, pixel_read_band_5, pixel_read_band_bqa;
    tdata_t line_band_4, line_band_5, line_band_bqa;

    NDVITOAGenerate(ldouble _sun_elevation, Tiff _band_4, Tiff _band_5, Tiff _band_bqa);
    void processNDVI(int number_sensor, ldouble dist_sun_earth, Tiff ndvi, vector<ldouble> radiometric_band_4, vector<ldouble> radiometric_band_5);
    void landsat(Tiff ndvi, int width_band, int height_band, int mask, ldouble dist_sun_earth, vector<ldouble> radiometric_band_4, vector<ldouble> radiometric_band_5);
};