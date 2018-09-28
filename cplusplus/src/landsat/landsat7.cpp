#include "landsat7.h"

void Landsat7::processNDVI(Tiff band4, Tiff band5, Tiff ndvi){

    // Load Tiff information
    uint32 height, width;

    TIFFGetField(band4, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField(band4, TIFFTAG_IMAGEWIDTH, &width);

    TIFFSetField(ndvi, TIFFTAG_IMAGEWIDTH     , width); 
    TIFFSetField(ndvi, TIFFTAG_IMAGELENGTH    , height);
    TIFFSetField(ndvi, TIFFTAG_BITSPERSAMPLE  , 64);
    TIFFSetField(ndvi, TIFFTAG_SAMPLEFORMAT   , 3);
    TIFFSetField(ndvi, TIFFTAG_COMPRESSION    , 1);
    TIFFSetField(ndvi, TIFFTAG_PHOTOMETRIC    , 1);
    TIFFSetField(ndvi, TIFFTAG_ORIENTATION    , 1);
    TIFFSetField(ndvi, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(ndvi, TIFFTAG_ROWSPERSTRIP   , 8);
    TIFFSetField(ndvi, TIFFTAG_RESOLUTIONUNIT , 1);
    TIFFSetField(ndvi, TIFFTAG_XRESOLUTION    , 1);
    TIFFSetField(ndvi, TIFFTAG_YRESOLUTION    , 1);
    TIFFSetField(ndvi, TIFFTAG_PLANARCONFIG , PLANARCONFIG_CONTIG );

    // Constants
    const int GRESCALE = 0;
    const int BRESCALE = 1;
    const int ESUN = 2;

    // Auxiliars variables
    ldouble ref4, ref5;
    ldouble pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067982148086513282306647093844609550582231725359408128481117450284102701938521105559644622948954930381964428810975665933446128475648233786783165271201909145648566923460348610454326648213393607260249141273;
    ldouble costheta = sin(sun_elevation*pi/180);

    // Auxiliars arrays 
    ldouble *lineB4 = new ldouble[width];
    ldouble *lineB5 = new ldouble[width];
    ldouble randianceB4[width];
    ldouble randianceB5[width];
    ldouble lineNDVI[width];
    vector<ldouble> paramB4 = getSensor().getParamBand4();
    vector<ldouble> paramB5 = getSensor().getParamBand5();

    for(int i = 0; i < height; i++){
        TIFFReadScanline(band4, lineB4, i);
        TIFFReadScanline(band5, lineB5, i);

        // RadianceCalc
        for(int j = 0; j < width; j++){
            randianceB4[j] = lineB4[j] * paramB4[GRESCALE] + paramB4[BRESCALE];
            randianceB5[j] = lineB5[j] * paramB5[GRESCALE] + paramB5[BRESCALE];
        }

        //ReflectanceCalc
        for(int j = 0; j < width; j++){
            ref4 = (pi * randianceB4[j] * (dist_sun_earth*dist_sun_earth)) / (costheta * paramB4[ESUN]);
            ref5 = (pi * randianceB5[j] * (dist_sun_earth*dist_sun_earth)) / (costheta * paramB5[ESUN]);
            lineNDVI[j] = (ref5 - ref4) / (ref5 + ref4);
        }
        TIFFWriteScanline(ndvi, lineNDVI, i);
    }

    TIFFClose(band4);
    TIFFClose(band5);
    TIFFClose(ndvi);
};

Sensor Landsat7::getSensor(){
    return sensor;
}