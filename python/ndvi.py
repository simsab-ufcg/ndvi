import numpy as np
import sys
import math
from numpy import nan_to_num, subtract, add, divide, multiply
from osgeo import gdal, gdalconst
from gdal import GetDriverByName

class NDVI:

    def __init__(self, sun_elevation, red_band, nir_band, bqa_band, dim, geotransform):
        self._dim = dim
        self._red_band = red_band.ReadAsArray(0, 0, dim[0], dim[1])
        self._nir_band = nir_band.ReadAsArray(0, 0, dim[0], dim[1])
        self._bqa_band = bqa_band.ReadAsArray(0, 0, dim[0], dim[1])
        self.sintheta = math.sin(sun_elevation * math.acos(-1) / 180.0)
        self._geotransform = geotransform

    def processNDVI(self, number_sensor, dist_sun_earth, output_path):

        # Convert the np arrays to 64-bit floating point to make sure division will occur properly.
        self._red_band = self._red_band.astype(np.float64)
        self._nir_band = self._nir_band.astype(np.float64)

        if number_sensor == 8:
            self._calculate_ndvi_new(output_path)
        elif number_sensor == 7:
            self._calculate_ndvi_old(dist_sun_earth, output_path, (0.969291 , -6.07 , 1039), (0.12622 , -1.13 , 230.8))
        else:
            self._calculate_ndvi_old(dist_sun_earth, output_path, (0.876024 , -2.39 , 1031), (0.120354 , -0.49 , 220))

    def _set_invalid_pixels(self, mask, result):
        for i in xrange(len(self._bqa_band)):
            for j in xrange(len(self._bqa_band[i])):
                if self._bqa_band[i][j] != mask:
                    result[i][j] = np.nan

    def _write_tiff(self, output_path, result):

        # Initialize a geotiff driver.
        geotiff = GetDriverByName('GTiff')

        output = geotiff.Create(output_path, self._dim[0], self._dim[1], 1, gdal.GDT_Float64)
        output_band = output.GetRasterBand(1)
        output_band.SetNoDataValue(np.nan)
        output_band.WriteArray(result)

        # Set the geographic transformation as the input.
        output.SetGeoTransform(self._geotransform)

    def _ndvi_equation(self, reflectance4, reflectance5):
        
        numerator = subtract(reflectance5, reflectance4)
        denominator = add(reflectance5, reflectance4)
        return divide(numerator, denominator)        

    def _calculate_ndvi_new(self, output_path):

        reflectance4 = (self._red_band * (2 * (10 ** -5)) - 0.1) / self.sintheta
        reflectance5 = (self._nir_band * (2 * (10 ** -5)) - 0.1) / self.sintheta
    
        result = self._ndvi_equation(reflectance4, reflectance5)

        self._set_invalid_pixels(2720, result)

        self._write_tiff(output_path, result)

    def _calculate_ndvi_old(self, dist_sun_earth, output_path, params_band4, params_band5):
        
        GRESCALE = 0
        BRESCALE = 1
        ESUN = 2

        radiance4 = self._red_band * (params_band4[GRESCALE] + params_band4[BRESCALE])
        radiance5 = self._nir_band * (params_band5[GRESCALE] + params_band5[BRESCALE])

        radiance4[radiance4 < 0] = 0
        radiance5[radiance5 < 0] = 0

        reflectance4 = math.acos(-1) * radiance4 * (dist_sun_earth ** 2) / self.sintheta * params_band4[ESUN]
        reflectance5 = math.acos(-1) * radiance5 * (dist_sun_earth ** 2) / self.sintheta * params_band5[ESUN]

        result = self._ndvi_equation(reflectance4, reflectance5)

        self._set_invalid_pixels(672, result)

        self._write_tiff(output_path, result)
