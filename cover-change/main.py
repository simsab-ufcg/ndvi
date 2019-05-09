from osgeo import gdal, osr
import os
import sys

def get_coordinate(source_tif_path):
	'''
	Takes a tiff as parameter and returns its upper left point coordinates.
	'''

	source_tiff = gdal.Open(source_tif_path)

	width_tiff = source_tiff.RasterXSize
	height_tiff = source_tiff.RasterYSize

	coord_tiff = source_tiff.GetGeoTransform()
	
	UL = (coord_tiff[0], coord_tiff[3])
	source_tif = None

	return UL


def set_georeference(src_path, base_path, coord_upper_left):
	'''
	Sets georeference and projection of a tiff based on a source tiff.
	'''

	base_ds = gdal.Open(base_path)


	base_coord = base_ds.GetGeoTransform()


	base_proj = base_ds.GetProjection()


	src_ds = gdal.Open(src_path, gdal.GA_Update)


	result_coord = (coord_upper_left[0], 30, base_coord[2], coord_upper_left[1], base_coord[4], -30)

	src_ds.SetGeoTransform(result_coord)
	src_ds.SetProjection(base_proj)

	src_ds = None
	base_ds = None


if __name__ == '__main__':
    if len(sys.argv) != 4:
	    print "python %s <path_new_ndvi> <path_old_ndvi> <output_path_change_cover_tif>" % sys.argv[0]
    else:
        path_new_ndvi = sys.argv[1]
        path_old_ndvi = sys.argv[2]
        output_path_change_cover_tif = sys.argv[3]

        os.system(' '.join( ['./coverChangeGen', path_new_ndvi, path_old_ndvi, output_path_change_cover_tif] ) )
    	set_georeference(output_path_change_cover_tif, path_old_ndvi, get_coordinate(path_old_ndvi) )
