#include "landsat5.h"
#include <iostream>

void Landsat5::processNDVI(Tiff band4, Tiff band5, Tiff ndvi){

    // Load Tiff information
    uint32 height, width;

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
    ldouble pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067982148086513282306647093844609550582231725359408128481117450284102701938521105559644622948954930381964428810975665933446128475648233786783165271201909145648566923460348610454326648213393607260249141273;
    ldouble costheta = sin(sun_elevation*pi/180);

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

    for(int i = 0; i < height; i++){
        TIFFReadScanline(band4, lineb4, i);
        TIFFReadScanline(band5, lineb5, i);

        // RadianceCalc
        for(int j = 0; j < width; j++){
            unsigned long long pixelb4 = 0, pixelb5 = 0;
			memcpy(&pixelb4, lineb4 + (j * byteSizeb4), byteSizeb4);
            memcpy(&pixelb5, lineb5 + (j * byteSizeb5), byteSizeb5);

            randianceB4[j] = (pixelb4) * paramB4[GRESCALE] + paramB4[BRESCALE];
            randianceB5[j] = (pixelb5) * paramB5[GRESCALE] + paramB5[BRESCALE];

            if(randianceB4[j] < 0) randianceB4[j] = 0;
            if(randianceB5[j] < 0) randianceB5[j] = 0;
        }

        //ReflectanceCalc
        for(int j = 0; j < width; j++){
            ref4 = (pi * randianceB4[j] * (dist_sun_earth*dist_sun_earth)) / (costheta * paramB4[ESUN]);
            ref5 = (pi * randianceB5[j] * (dist_sun_earth*dist_sun_earth)) / (costheta * paramB5[ESUN]);

            lineNDVI[j] = (ref5 - ref4) / (ref5 + ref4);
            cout << lineNDVI[j] << " ";
        }
        cout << endl;

        TIFFWriteScanline(ndvi, lineNDVI, i);
    }

    TIFFClose(band4);
    TIFFClose(band5);
    TIFFClose(ndvi);
};

Sensor Landsat5::getSensor(){
    return sensor;
}
