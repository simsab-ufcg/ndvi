#include <iostream>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "tiffio.h"

using namespace std;

const double NaN = -sqrt(-1.0);

void setup(TIFF* ndvi, TIFF* bandBase){
    uint32 imageWidth, imageLength;

    TIFFGetField(bandBase, TIFFTAG_IMAGEWIDTH,      &imageWidth);
    TIFFGetField(bandBase, TIFFTAG_IMAGELENGTH,     &imageLength);
    
    TIFFSetField(ndvi, TIFFTAG_IMAGEWIDTH     , imageWidth); 
    TIFFSetField(ndvi, TIFFTAG_IMAGELENGTH    , imageLength);
    TIFFSetField(ndvi, TIFFTAG_BITSPERSAMPLE  , 64);
    TIFFSetField(ndvi, TIFFTAG_SAMPLEFORMAT   , 3);
    TIFFSetField(ndvi, TIFFTAG_COMPRESSION    , 1);
    TIFFSetField(ndvi, TIFFTAG_PHOTOMETRIC    , 1);
    TIFFSetField(ndvi, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(ndvi, TIFFTAG_ROWSPERSTRIP   , 8);
    TIFFSetField(ndvi, TIFFTAG_RESOLUTIONUNIT , 1);
    TIFFSetField(ndvi, TIFFTAG_XRESOLUTION    , 1);
    TIFFSetField(ndvi, TIFFTAG_YRESOLUTION    , 1);
    TIFFSetField(ndvi, TIFFTAG_PLANARCONFIG   , PLANARCONFIG_CONTIG);
}

double function(double value_pixel_new_ndvi, double value_pixel_old_ndvi){
    if(!isnan(value_pixel_new_ndvi) && !isnan(value_pixel_old_ndvi))
        return value_pixel_new_ndvi - value_pixel_old_ndvi;
    else
        return NaN;
}

int main(int argc, char *argv[]){

    const int INDEX_INPUT_NEW_NDVI = 1;
    const int INDEX_INPUT_OLD_NDVI = 2;
    const int INDEX_OUTPUT_CHANGE_COVER = 3;

    //valid arguments
    if(argc != 4){
        cerr << "Incorrect number of arguments for processing cover change TIF" << endl;
        exit(1);
    }

    //load new NDVI (tiff)
    string path_tiff_new_ndvi = argv[INDEX_INPUT_NEW_NDVI];
    TIFF* new_ndvi = TIFFOpen(path_tiff_new_ndvi.c_str(), "rm");

    //load old NDVI (tiff)
    string path_tiff_old_ndvi = argv[INDEX_INPUT_OLD_NDVI];
    TIFF* old_ndvi = TIFFOpen(path_tiff_old_ndvi.c_str(), "rm");

    //load change cover (tiff)
    string path_tiff_change_cover = argv[INDEX_OUTPUT_CHANGE_COVER];
    TIFF* change_cover = TIFFOpen(path_tiff_change_cover.c_str(), "w8m");
    setup(change_cover, new_ndvi);

    uint32 height_new_ndvi, width_new_ndvi;

    TIFFGetField(new_ndvi, TIFFTAG_IMAGELENGTH, &height_new_ndvi);
    TIFFGetField(new_ndvi, TIFFTAG_IMAGEWIDTH, &width_new_ndvi);

    uint32 height_old_ndvi, width_old_ndvi;

    TIFFGetField(old_ndvi, TIFFTAG_IMAGELENGTH, &height_old_ndvi);
    TIFFGetField(old_ndvi, TIFFTAG_IMAGEWIDTH, &width_old_ndvi);

    if(height_new_ndvi != height_old_ndvi || width_new_ndvi != width_old_ndvi){
        cerr << "New NDVI and old NDVI have different dimensions" << endl;
        exit(1);
    }

    double new_ndvi_line[width_new_ndvi];
    double old_ndvi_line[width_old_ndvi];
    double change_cover_line[width_old_ndvi];

    for(int line = 0; line < height_new_ndvi; line++){
        if(TIFFReadScanline(new_ndvi, new_ndvi_line, line) < 0){
            cerr << "Read problem in new NDVI TIF" << endl;
            exit(2);
        }

        if(TIFFReadScanline(old_ndvi, old_ndvi_line, line) < 0){
            cerr << "Read problem in old NDVI TIF" << endl;
            exit(2);
        }

        for(int col = 0; col < width_new_ndvi; col ++) change_cover_line[col] = function(new_ndvi_line[col], old_ndvi_line[col]);

        if (TIFFWriteScanline(change_cover, change_cover_line, line) < 0){
            cerr << "Write problem in change cover TIF" << endl;
            exit(3);
        }

    }

    TIFFClose(new_ndvi);
    TIFFClose(old_ndvi);
    TIFFClose(change_cover);

    return 0;
}