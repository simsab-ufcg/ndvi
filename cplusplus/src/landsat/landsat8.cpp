#include "landsat8.h"
#include <iostream>

void Landsat8::processNDVI(Tiff band4, Tiff band5, Tiff ndvi){
    
    uint32 height, width;

    TIFFGetField(band4, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField(band4, TIFFTAG_IMAGEWIDTH, &width);

    TIFFSetField(ndvi, TIFFTAG_IMAGEWIDTH     , width); 
    TIFFSetField(ndvi, TIFFTAG_IMAGELENGTH    , height);

    ldouble ref4, ref5;
    ldouble pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067982148086513282306647093844609550582231725359408128481117450284102701938521105559644622948954930381964428810975665933446128475648233786783165271201909145648566923460348610454326648213393607260249141273;
    ldouble costheta = sin(sun_elevation*pi/180);   

    tdata_t lineb4, lineb5;
    unsigned short byteSizeb4 = TIFFScanlineSize(band4)/width;
    unsigned short byteSizeb5 = TIFFScanlineSize(band5)/width;
    lineb4 = _TIFFmalloc(TIFFScanlineSize(band4));
    lineb5 = _TIFFmalloc(TIFFScanlineSize(band5));
    ldouble lineNDVI[width];
    
    for(int line = 0; line < height; line++){
        TIFFReadScanline(band4, lineb4, line);
        TIFFReadScanline(band5, lineb5, line);

        for(int row = 0; row < width; row++){
            unsigned long long pixelb4 = 0, pixelb5 = 0;
			memcpy(&pixelb4, lineb4 + (row * byteSizeb4), byteSizeb4);
            memcpy(&pixelb5, lineb5 + (row * byteSizeb5), byteSizeb5);

            ref4 = (pixelb4 * 0.00002 - 0.1) / costheta;
            ref5 = (pixelb5 * 0.00002 - 0.1) / costheta;
            lineNDVI[row] = (ref5 - ref4) / (ref5 + ref4);
            cout << pixelb4 << " " << pixelb5 << endl;
            if(row == 5)exit(0);
            //cout << lineNDVI[row] << " ";

        }
        //cout << endl;
        TIFFWriteScanline(ndvi, lineNDVI, line);
    }

    TIFFClose(band4);
    TIFFClose(band5);
    TIFFClose(ndvi);
};