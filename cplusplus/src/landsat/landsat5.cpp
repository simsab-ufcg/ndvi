#include "landsat5.h"
#include <iostream>

void Landsat5::processNDVI(Tiff band4, Tiff band5, Tiff ndvi){

    // Load Tiff information
    uint32 height, width;
    uint16 sampleBand4, sampleBand5;

    TIFFGetField(band4, TIFFTAG_SAMPLEFORMAT, &sampleBand4);
    TIFFGetField(band5, TIFFTAG_SAMPLEFORMAT, &sampleBand5);

    TIFFGetField(band4, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField(band4, TIFFTAG_IMAGEWIDTH, &width);

    TIFFSetField(ndvi, TIFFTAG_IMAGEWIDTH     , width); 
    TIFFSetField(ndvi, TIFFTAG_IMAGELENGTH    , height);

    // Constants
    const int GRESCALE = 0;
    const int BRESCALE = 1;
    const int ESUN = 2;

    // Auxiliars variables
    ldouble ref4, ref5;
    ldouble pi = 3.14159265358979323;
    ldouble sintheta = sin(sun_elevation*pi/180);

    // Auxiliars arrays 
    tdata_t lineb4, lineb5;
    unsigned short byteSizeb4 = TIFFScanlineSize(band4)/width;
    unsigned short byteSizeb5 = TIFFScanlineSize(band5)/width;
    lineb4 = _TIFFmalloc(TIFFScanlineSize(band4));
    lineb5 = _TIFFmalloc(TIFFScanlineSize(band5));

    ldouble randianceB4[width];
    ldouble randianceB5[width];

    ldouble lineNDVI[width];

    vector<ldouble> paramB4 = getSensor().getParamBand4();
    vector<ldouble> paramB5 = getSensor().getParamBand5();

    PixelReader pr4(sampleBand4, byteSizeb4, lineb4);
    PixelReader pr5(sampleBand5, byteSizeb5, lineb5);

    for(int i = 0; i < height; i++){
        TIFFReadScanline(band4, lineb4, i);
        TIFFReadScanline(band5, lineb5, i);

        // RadianceCalc
        for(int j = 0; j < width; j++){
            ldouble pixelb4 = pr4.readPixel(j);
            ldouble pixelb5 = pr5.readPixel(j);

            randianceB4[j] = (pixelb4) * paramB4[GRESCALE] + paramB4[BRESCALE];
            randianceB5[j] = (pixelb5) * paramB5[GRESCALE] + paramB5[BRESCALE];

            if(randianceB4[j] < 0) randianceB4[j] = 0;
            if(randianceB5[j] < 0) randianceB5[j] = 0;
        }

        //ReflectanceCalc
        for(int j = 0; j < width; j++){
            ref4 = (pi * randianceB4[j] * (dist_sun_earth*dist_sun_earth)) / (sintheta * paramB4[ESUN]);
            ref5 = (pi * randianceB5[j] * (dist_sun_earth*dist_sun_earth)) / (sintheta * paramB5[ESUN]);

            lineNDVI[j] = (ref5 - ref4) / (ref5 + ref4);
        }

        TIFFWriteScanline(ndvi, lineNDVI, i);
    }

    _TIFFfree(lineb4);
    _TIFFfree(lineb5);
};

Sensor Landsat5::getSensor(){
    return sensor;
}
