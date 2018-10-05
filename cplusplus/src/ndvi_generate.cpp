#include "ndvi_generate.h"

NDVIGenerate::NDVIGenerate(ldouble _sun_elevation, Tiff _band_4, Tiff _band_5, Tiff _band_bqa){
    sintheta = sin(_sun_elevation * PI / 180);
    band_4 = _band_4;
    band_5 = _band_5;
    band_bqa = _band_bqa;
}

void NDVIGenerate::processNDVI(int number_sensor, ldouble dist_sun_earth, Tiff ndvi){
    uint32 height_band, width_band;
    uint16 sample_band_4, sample_band_5, sample_band_bqa;
    int mask = setMask(number_sensor);

    TIFFGetField(band_4, TIFFTAG_SAMPLEFORMAT, &sample_band_4);
    TIFFGetField(band_5, TIFFTAG_SAMPLEFORMAT, &sample_band_5);
    TIFFGetField(band_bqa, TIFFTAG_SAMPLEFORMAT, &sample_band_bqa);

    TIFFGetField(band_4, TIFFTAG_IMAGELENGTH, &height_band);
    TIFFGetField(band_4, TIFFTAG_IMAGEWIDTH, &width_band);

    TIFFSetField(ndvi, TIFFTAG_IMAGEWIDTH     , width_band); 
    TIFFSetField(ndvi, TIFFTAG_IMAGELENGTH    , height_band);

    unsigned short byte_size_band_4 = TIFFScanlineSize(band_4) / width_band;
    unsigned short byte_size_band_5 = TIFFScanlineSize(band_5) / width_band;
    unsigned short byte_size_band_bqa = TIFFScanlineSize(band_bqa) / width_band;

    line_band_4 = _TIFFmalloc(TIFFScanlineSize(band_4));
    line_band_5 = _TIFFmalloc(TIFFScanlineSize(band_5));
    line_band_bqa = _TIFFmalloc(TIFFScanlineSize(band_bqa));

    PixelReader pixel_read_band_4(sample_band_4, byte_size_band_4, line_band_4);
    PixelReader pixel_read_band_5(sample_band_5, byte_size_band_5, line_band_5);
    PixelReader pixel_read_band_bqa(sample_band_bqa, byte_size_band_bqa, line_band_bqa);

    switch(number_sensor){
        case 8:
            landsat(ndvi, width_band, height_band, mask);
            break;
        case 7:
            landsat(ndvi, width_band, height_band, mask, dist_sun_earth, {0.969291 , -6.07 , 1039}, {0.12622 , -1.13 , 230.8});
            break;
        case 5:
            landsat(ndvi, width_band, height_band, mask, dist_sun_earth, {0.876024 , -2.39 , 1031}, {0.120354 , -0.49 , 220});
            break;
        default:
            cerr << "Type of input bands unsupported!" << endl;
			exit(1);
    } 

    _TIFFfree(line_band_4);
    _TIFFfree(line_band_5);
    _TIFFfree(line_band_bqa);
}

void NDVIGenerate::landsat(Tiff ndvi, int width_band, int height_band, int mask, ldouble dist_sun_earth, vector<ldouble> param_band_4, vector<ldouble> param_band_5){
    // Constants
    const int GRESCALE = 0;
    const int BRESCALE = 1;
    const int ESUN = 2;

    ldouble radiance_band_4[width_band];
    ldouble radiance_band_5[width_band];
    ldouble line_ndvi[width_band];

    for(int line = 0; line < height_band; line ++){
        TIFFReadScanline(band_4, line_band_4, line);
        TIFFReadScanline(band_5, line_band_5, line);
        TIFFReadScanline(band_bqa, line_band_bqa, line);

        // RadianceCalc
        for(int col = 0; col < width_band; col ++){
            ldouble pixel_band_4 = pixel_read_band_4.readPixel(col);
            ldouble pixel_band_5 = pixel_read_band_5.readPixel(col);
            
            radiance_band_4[col] = pixel_band_4 * param_band_4[GRESCALE] + param_band_4[BRESCALE];
            radiance_band_5[col] = pixel_band_5 * param_band_5[GRESCALE] + param_band_5[BRESCALE];

            if(radiance_band_4[col] < 0) radiance_band_4[col] = 0;
            if(radiance_band_5[col] < 0) radiance_band_5[col] = 0;
        }

        //ReflectanceCalc
        for(int col = 0; col < width_band; col++){
            ldouble pixel_band_bqa = pixel_read_band_bqa.readPixel(col);
            if(fabs(pixel_band_bqa - mask) > EPS){
                line_ndvi[col] = NaN;
                continue;
            }

            ldouble reflectance_pixel_band_4, reflectance_pixel_band_5;

            reflectance_pixel_band_4 = (PI * radiance_band_4[col] * (dist_sun_earth*dist_sun_earth)) / (sintheta * param_band_4[ESUN]);
            reflectance_pixel_band_5 = (PI * radiance_band_5[col] * (dist_sun_earth*dist_sun_earth)) / (sintheta * param_band_5[ESUN]);

            line_ndvi[col] = (reflectance_pixel_band_5 - reflectance_pixel_band_4) / (reflectance_pixel_band_5 + reflectance_pixel_band_4);
        }

        TIFFWriteScanline(ndvi, line_ndvi, line);
    }
}

void NDVIGenerate::landsat(Tiff ndvi, int width_band, int height_band, int mask){

    ldouble line_ndvi[width_band];

    for(int line = 0; line < height_band; line++){
        TIFFReadScanline(band_4, line_band_4, line);
        TIFFReadScanline(band_5, line_band_5, line);
        TIFFReadScanline(band_bqa, line_band_bqa, line);

        for(int col = 0; col < width_band; col++){
            ldouble pixel_band_bqa = pixel_read_band_bqa.readPixel(col);
            if(fabs(pixel_band_bqa - mask) > EPS){
                line_ndvi[col] = NaN;
                continue;
            }

            ldouble reflectance_pixel_band_4, reflectance_pixel_band_5;
            ldouble pixel_band_4 = pixel_read_band_4.readPixel(col);
            ldouble pixel_band_5 = pixel_read_band_5.readPixel(col);

            reflectance_pixel_band_4 = (pixel_band_4 * 0.00002 - 0.1) / sintheta;
            reflectance_pixel_band_5 = (pixel_band_5 * 0.00002 - 0.1) / sintheta;

            line_ndvi[col] = (reflectance_pixel_band_5 - reflectance_pixel_band_4) / (reflectance_pixel_band_5 + reflectance_pixel_band_4);

        }
        TIFFWriteScanline(ndvi, line_ndvi, line);
    }
}