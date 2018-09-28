#include <iostream>
#include "readMeta.h"
#include "readSunEarth.h"
#include "landsat/landsat_function.h"
#include "landsat/landsat8.h"
#include "landsat/landsat7.h"
#include "landsat/landsat5.h"

using namespace std;

int setMask(int number_sensor){
    if(number_sensor != 8) return 672;
    else return 2720;
}

bool analisyShadow(string path_tiff_band_bqa, int number_sensor){
    int fmask = setMask(number_sensor);

    Tiff bandBqa = TIFFOpen(path_tiff_band_bqa.c_str(), "rm");

    uint32 heightTiffBqa, widthTiffBqa;

    TIFFGetField(bandBqa, TIFFTAG_IMAGELENGTH, &heightTiffBqa);
    TIFFGetField(bandBqa, TIFFTAG_IMAGEWIDTH, &widthTiffBqa);

    ldouble *line = (ldouble*) malloc(sizeof(ldouble) * widthTiffBqa);

    long long quantPixelsInvalid = 0;

    for(int i = 0; i < heightTiffBqa; i++){
        TIFFReadScanline(bandBqa, line, i, 1);
        for(int j = 0; j < widthTiffBqa; j++){
            if(line[j] != fmask) quantPixelsInvalid++;
        }
    }

    return ((ldouble)quantPixelsInvalid/(heightTiffBqa*widthTiffBqa)) >= 0.99;
}

LandsatFunction* setLandsatFunction(int number_sensor, ldouble sun_elevation, ldouble dist_sun_earth){
    if(number_sensor == 8) return new Landsat8(sun_elevation);
    if(number_sensor == 7) return new Landsat7(sun_elevation, dist_sun_earth);
    if(number_sensor == 5) return new Landsat5(sun_elevation, dist_sun_earth);
}

int main(int argc, char *argv[]){

    const int INPUT_BAND_4_INDEX = 1;
    const int INPUT_BAND_5_INDEX = 2;
    const int INPUT_BAND_BQA_INDEX = 3;
    const int INPUT_BAND_MTL_INDEX = 4;

    //valid arguments
    if(argc < 5 || argc > 5){
        cerr << "Missing arguments for processing NDVI TIF";
        exit(0);
    }

    //load meta file
    string path_meta_file = argv[INPUT_BAND_MTL_INDEX];
    ReadMeta readerMeta = ReadMeta(path_meta_file);
    ldouble sun_elevation = readerMeta.getSunElevation();
    int number_sensor = readerMeta.getNumberSensor();
    int julian_day = readerMeta.getJulianDay();
    int year = readerMeta.getYear();

    //load distance between sun and earth
    string path_d_sun_earth = "./src/d_sun_earth";
    ReadSunEarth readerSunEarth = ReadSunEarth(path_d_sun_earth);
    ldouble dist_sun_earth = readerSunEarth.getDistance(julian_day);

    //verify quantity snow and shadows
    //string path_tiff_band_bqa = inputDirectory + tiffFiles[POS_BAND_BQA];
    //if(analisyShadow(path_tiff_band_bqa, number_sensor)){
    //    cerr << "Invalid inputs. Lots of cloud in tiff images";
    //    exit(0);
    //}

    //load band 4 (tiff)
    string path_tiff_band_4 = argv[INPUT_BAND_4_INDEX];
    Tiff band4 = TIFFOpen(path_tiff_band_4.c_str(), "rm");

    //load band 5 (tiff)
    string path_tiff_band_5 = argv[INPUT_BAND_5_INDEX];
    Tiff band5 = TIFFOpen(path_tiff_band_5.c_str(), "rm");

    //load tiff ndvi
    string path_output_tiff_ndvi = "./ndvi.tif";
    Tiff ndvi = TIFFOpen(path_output_tiff_ndvi.c_str(), "w8");

    //process NDVI
    LandsatFunction* landsat;
    landsat = setLandsatFunction(number_sensor, sun_elevation, dist_sun_earth);
    (*landsat).processNDVI(band4, band5, ndvi);

    return 0;
}
