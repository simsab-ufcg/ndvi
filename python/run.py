import gdal
import sys
import numpy as np
from numpy import nan_to_num, subtract, add, divide, multiply
import metaReader
from gdal import Open
from ndvi import NDVI

def get_mask(number_sensor):
	if number_sensor == 8:
		return 2720
	else:
		return 672

def shadow_check(bqa_band, dim, mask):
	pixels_valid = 0
	bqa_arr = bqa_band.ReadAsArray(0, 0, dim[0] , dim[1])
	for i in bqa_arr:
		for j in i:
			if j == mask:
				pixels_valid += 1
	return ((float(pixels_valid)) / (dim[0] * dim[1])) <= 0.01

def main():

	if len(sys.argv) < 7:
		print "Few arguments"
		sys.exit()

	elif len(sys.argv) > 7:
		print "Too many arguments"
		sys.exit()

	#Tiff paths
	red_path = str(sys.argv[1])
	nir_path = str(sys.argv[2])
	bqa_path = str(sys.argv[3])
	mtl_path = str(sys.argv[4])
	d_sun_earth_path = str(sys.argv[5])
	output_path = str(sys.argv[6])

	# Open red image and get its only band.
	red_tiff = Open(red_path)
	red_band = red_tiff.GetRasterBand(1)

	# Open NIR image and get its only band.
	nir_tiff = Open(nir_path)
	nir_band = nir_tiff.GetRasterBand(1)

	# Open bqa image and get its only band.
	bqa_tiff = Open(bqa_path)
	bqa_band = bqa_tiff.GetRasterBand(1)

	# Get the rows and cols from one of the images (both should always be the same)
	row, col, geotransform = nir_tiff.RasterYSize, nir_tiff.RasterXSize, nir_tiff.GetGeoTransform()

	# Reading metadata
	parameters = metaReader.readParameters(mtl_path, d_sun_earth_path)

	number_sensor = int(parameters[0])
	julian_day = int(parameters[1])
	
	sun_elevation = float(parameters[2])
	dist_sun_earth = float(parameters[3])

	mask = get_mask(number_sensor)

	if shadow_check(bqa_band, (col, row), mask):
		print "Invalid inputs. Lots of cloud in tiff images"
		sys.exit()

	ndviC = NDVI(sun_elevation, red_band, nir_band, bqa_band, (col, row), geotransform)
	ndviC.processNDVI(number_sensor, dist_sun_earth, output_path)

if __name__ == "__main__":
    main()