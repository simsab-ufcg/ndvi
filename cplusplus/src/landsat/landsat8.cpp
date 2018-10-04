#include "landsat8.h"
#include <iostream>

void Landsat8::processNDVI(Tiff band4, Tiff band5, Tiff ndvi){
    
    uint32 height, width;
    uint16 sampleBand4, sampleBand5;

    TIFFGetField(band4, TIFFTAG_SAMPLEFORMAT, &sampleBand4);
    TIFFGetField(band5, TIFFTAG_SAMPLEFORMAT, &sampleBand5);

    TIFFGetField(band4, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField(band4, TIFFTAG_IMAGEWIDTH, &width);

    TIFFSetField(ndvi, TIFFTAG_IMAGEWIDTH     , width); 
    TIFFSetField(ndvi, TIFFTAG_IMAGELENGTH    , height);

    ldouble ref4, ref5;
    ldouble pi = 3.14159265358979323;
    ldouble sintheta = sin(sun_elevation * pi / 180.0);

    tdata_t lineb4, lineb5;

    unsigned short byteSizeb4 = TIFFScanlineSize(band4)/width;
    unsigned short byteSizeb5 = TIFFScanlineSize(band5)/width;
    
    lineb4 = _TIFFmalloc(TIFFScanlineSize(band4));
    lineb5 = _TIFFmalloc(TIFFScanlineSize(band5));
    
    ldouble lineNDVI[width];
    
    PixelReader pr4(sampleBand4, byteSizeb4, lineb4);
    PixelReader pr5(sampleBand5, byteSizeb5, lineb5);

    for(int line = 0; line < height; line++){
        TIFFReadScanline(band4, lineb4, line);
        TIFFReadScanline(band5, lineb5, line);

        for(int row = 0; row < width; row++){

            ldouble pixelb4 = pr4.readPixel(row);
            ldouble pixelb5 = pr5.readPixel(row);

            ref4 = (pixelb4 * 0.00002 - 0.1) / sintheta;
            ref5 = (pixelb5 * 0.00002 - 0.1) / sintheta;
            lineNDVI[row] = (ref5 - ref4) / (ref5 + ref4);

        }
        TIFFWriteScanline(ndvi, lineNDVI, line);
    }

    _TIFFfree(lineb4);
    _TIFFfree(lineb5);
};