#include <iostream>
#include "readMeta.h"
#include "readSunEarth.h"
#include "landsat/landsat_function.h"
#include "landsat/landsat8.h"
#include "landsat/landsat7.h"
#include "landsat/landsat5.h"
#include <time.h>
#include <unistd.h>
#include <string.h>

using namespace std;

void logger(string description){
    timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    cout << res.tv_sec << " " << description.c_str() << " " << getpid() << endl;
}

int setMask(int number_sensor){
    if(number_sensor != 8) return 672;
    else return 2720;
}

bool analisyShadow(string path_tiff_band_bqa, int number_sensor){
    int fmask = setMask(number_sensor);

    Tiff band_bqa = TIFFOpen(path_tiff_band_bqa.c_str(), "rm");

    uint32 height_tiff_bqa, width_tiff_bqa;

    TIFFGetField(band_bqa, TIFFTAG_IMAGELENGTH, &height_tiff_bqa);
    TIFFGetField(band_bqa, TIFFTAG_IMAGEWIDTH, &width_tiff_bqa);

    tdata_t buf;
    unsigned short byte_size = TIFFScanlineSize(band_bqa)/width_tiff_bqa;
    buf = _TIFFmalloc(TIFFScanlineSize(band_bqa));

    long long quant_pixels_valid = 0;
    int pixel;

    for(int line = 0; line < height_tiff_bqa; line++){
        TIFFReadScanline(band_bqa, buf, line);
        for(int row = 0; row < width_tiff_bqa; row++){
            memcpy(&pixel, (int*) buf + (row * byte_size), byte_size);
            if(pixel == fmask) quant_pixels_valid++;
        }
    }
    _TIFFfree(buf);
    TIFFClose(band_bqa);

    return ((ldouble)quant_pixels_valid/(height_tiff_bqa*width_tiff_bqa)) <= 0.01;
}

LandsatFunction* setLandsatFunction(int number_sensor, ldouble sun_elevation, ldouble dist_sun_earth){
    if(number_sensor == 8) return new Landsat8(sun_elevation);
    if(number_sensor == 7) return new Landsat7(sun_elevation, dist_sun_earth);
    if(number_sensor == 5) return new Landsat5(sun_elevation, dist_sun_earth);
}

void setup(Tiff ndvi, Tiff bandBase){
    uint32 imageWidth, imageLength, rowsPerStrip;
    uint16 samplePerPixel, bitsPerSample, sampleFormat, compression, photometric, orientation, resolutionUnit, planar_config;
    float xResolution, yResolution;

    TIFFGetField(bandBase, TIFFTAG_IMAGEWIDTH,      &imageWidth);
    TIFFGetField(bandBase, TIFFTAG_IMAGELENGTH,     &imageLength);
    TIFFGetField(bandBase, TIFFTAG_SAMPLESPERPIXEL, &samplePerPixel);
    TIFFGetField(bandBase, TIFFTAG_BITSPERSAMPLE,   &bitsPerSample);
    TIFFGetField(bandBase, TIFFTAG_SAMPLEFORMAT   , &sampleFormat);
    TIFFGetField(bandBase, TIFFTAG_COMPRESSION    , &compression);
    TIFFGetField(bandBase, TIFFTAG_PHOTOMETRIC    , &photometric);
    TIFFGetField(bandBase, TIFFTAG_ORIENTATION    , &orientation);
    TIFFGetField(bandBase, TIFFTAG_ROWSPERSTRIP   , &rowsPerStrip);
    TIFFGetField(bandBase, TIFFTAG_RESOLUTIONUNIT , &resolutionUnit);
    TIFFGetField(bandBase, TIFFTAG_XRESOLUTION    , &xResolution);
    TIFFGetField(bandBase, TIFFTAG_YRESOLUTION    , &yResolution);
    TIFFGetField(bandBase, TIFFTAG_PLANARCONFIG ,   &planar_config );
    
    TIFFSetField(ndvi, TIFFTAG_IMAGEWIDTH     , imageWidth); 
    TIFFSetField(ndvi, TIFFTAG_IMAGELENGTH    , imageLength);
    TIFFSetField(ndvi, TIFFTAG_BITSPERSAMPLE  , bitsPerSample);
    TIFFSetField(ndvi, TIFFTAG_SAMPLEFORMAT   , sampleFormat);
    TIFFSetField(ndvi, TIFFTAG_COMPRESSION    , compression);
    TIFFSetField(ndvi, TIFFTAG_PHOTOMETRIC    , photometric);
    TIFFSetField(ndvi, TIFFTAG_SAMPLESPERPIXEL, samplePerPixel);
    TIFFSetField(ndvi, TIFFTAG_ROWSPERSTRIP   , rowsPerStrip);
    TIFFSetField(ndvi, TIFFTAG_RESOLUTIONUNIT , resolutionUnit);
    TIFFSetField(ndvi, TIFFTAG_XRESOLUTION    , xResolution);
    TIFFSetField(ndvi, TIFFTAG_YRESOLUTION    , yResolution);
    TIFFSetField(ndvi, TIFFTAG_PLANARCONFIG   , PLANARCONFIG_CONTIG );

    cout << sampleFormat << endl;
    
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
    string path_tiff_band_bqa = argv[INPUT_BAND_BQA_INDEX];
    /*if(analisyShadow(path_tiff_band_bqa, number_sensor)){
        cerr << "Invalid inputs. Lots of cloud in tiff images";
        exit(0);
    }*/

    //load band 4 (tiff)
    string path_tiff_band_4 = argv[INPUT_BAND_4_INDEX];
    Tiff band4 = TIFFOpen(path_tiff_band_4.c_str(), "rm");

    //load band 5 (tiff)
    string path_tiff_band_5 = argv[INPUT_BAND_5_INDEX];
    Tiff band5 = TIFFOpen(path_tiff_band_5.c_str(), "rm");

    //load tiff ndvi
    string path_output_tiff_ndvi = "./ndvi.tif";
    Tiff ndvi = TIFFOpen(path_output_tiff_ndvi.c_str(), "w8w");
    setup(ndvi, band4);

    logger("Preprocess");

    //process NDVI
    LandsatFunction* landsat;
    landsat = setLandsatFunction(number_sensor, sun_elevation, dist_sun_earth);
    (*landsat).processNDVI(band4, band5, ndvi);

    logger("NDVICalc");
    return 0;
}
