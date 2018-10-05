#include <iostream>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "read_meta.h"
#include "read_sun_earth.h"
#include "ndvi_generate.h"
#include "utils.h"

using namespace std;

void logger(string description){
    timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    cout << res.tv_sec << " " << description.c_str() << " " << getpid() << endl;
}

bool analisyShadow(Tiff band_bqa, int number_sensor){   
    int mask = setMask(number_sensor);
    
    uint16 sample_band_bqa;
    uint32 height_tiff_bqa, width_tiff_bqa;

    TIFFGetField(band_bqa, TIFFTAG_IMAGELENGTH, &height_tiff_bqa);
    TIFFGetField(band_bqa, TIFFTAG_IMAGEWIDTH, &width_tiff_bqa);
    TIFFGetField(band_bqa, TIFFTAG_SAMPLEFORMAT, &sample_band_bqa);

    tdata_t buf;
    unsigned short byte_size = TIFFScanlineSize(band_bqa)/width_tiff_bqa;
    buf = _TIFFmalloc(TIFFScanlineSize(band_bqa));

    long long quant_pixels_valid = 0;
    ldouble pixel;
    PixelReader prBqa = PixelReader(sample_band_bqa, byte_size, buf);

    for(int line = 0; line < height_tiff_bqa; line++){
        TIFFReadScanline(band_bqa, buf, line);
        for(int row = 0; row < width_tiff_bqa; row++){
            pixel = prBqa.readPixel(row);
            if(fabs(pixel - mask) <= EPS)quant_pixels_valid++;
        }
    }
    _TIFFfree(buf);
    
    return (((ldouble)quant_pixels_valid)/(height_tiff_bqa*width_tiff_bqa)) <= 0.01;
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
    TIFFSetField(ndvi, TIFFTAG_BITSPERSAMPLE  , 64);
    TIFFSetField(ndvi, TIFFTAG_SAMPLEFORMAT   , 3);
    TIFFSetField(ndvi, TIFFTAG_COMPRESSION    , 1);
    TIFFSetField(ndvi, TIFFTAG_PHOTOMETRIC    , 1);
    TIFFSetField(ndvi, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(ndvi, TIFFTAG_ROWSPERSTRIP   , 8);
    TIFFSetField(ndvi, TIFFTAG_RESOLUTIONUNIT , 1);
    TIFFSetField(ndvi, TIFFTAG_XRESOLUTION    , 1);
    TIFFSetField(ndvi, TIFFTAG_YRESOLUTION    , 1);
    TIFFSetField(ndvi, TIFFTAG_PLANARCONFIG   , PLANARCONFIG_CONTIG );
    
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
    ReadMeta reader_meta = ReadMeta(path_meta_file);
    ldouble sun_elevation = reader_meta.getSunElevation();
    int number_sensor = reader_meta.getNumberSensor();
    int julian_day = reader_meta.getJulianDay();
    int year = reader_meta.getYear();

    //load distance between sun and earth
    string path_d_sun_earth = "./src/d_sun_earth";
    ReadSunEarth reader_sun_earth = ReadSunEarth(path_d_sun_earth);
    ldouble dist_sun_earth = reader_sun_earth.getDistance(julian_day);

    //load band 4 (tiff)
    string path_tiff_band_4 = argv[INPUT_BAND_4_INDEX];
    Tiff band_4 = TIFFOpen(path_tiff_band_4.c_str(), "rm");

    //load band 5 (tiff)
    string path_tiff_band_5 = argv[INPUT_BAND_5_INDEX];
    Tiff band_5 = TIFFOpen(path_tiff_band_5.c_str(), "rm");

    //load band_bqa (tiff)
    string path_tiff_band_bqa = argv[INPUT_BAND_BQA_INDEX];
    Tiff band_bqa = TIFFOpen(path_tiff_band_bqa.c_str(), "rm");

    //load tiff ndvi
    string path_output_tiff_ndvi = "./ndvi.tif";
    Tiff ndvi = TIFFOpen(path_output_tiff_ndvi.c_str(), "w8w");
    setup(ndvi, band_4);

    //verify quantity snow and shadows
    if(analisyShadow(band_bqa, number_sensor)){
        cerr << "Invalid inputs. Lots of cloud in tiff images";
        exit(0);
    }

    logger("Preprocess");

    NDVIGenerate ndviGen(sun_elevation, band_4, band_5, band_bqa);
    ndviGen.processNDVI(number_sensor, dist_sun_earth, ndvi);
    
    logger("NDVICalc");

    TIFFClose(band_4);
    TIFFClose(band_5);
    TIFFClose(band_bqa);
    TIFFClose(ndvi);

    return 0;
}
