
#include "ndvi_generate.h"

NDVIGenerate::NDVIGenerate(ldouble _sun_elevation, Tiff _band_red, Tiff _band_nir, Tiff _band_bqa){
    sintheta = sin(_sun_elevation * PI / 180);
    band_red = _band_red;
    band_nir = _band_nir;
    band_bqa = _band_bqa;
}

void NDVIGenerate::processNDVI(int number_sensor, ldouble dist_sun_earth, Tiff ndvi){
    uint32 height_band, width_band;
    uint16 sample_band_red, sample_band_nir, sample_band_bqa;
    int mask = setMask(number_sensor);

    TIFFGetField(band_red, TIFFTAG_SAMPLEFORMAT, &sample_band_red);
    TIFFGetField(band_nir, TIFFTAG_SAMPLEFORMAT, &sample_band_nir);
    TIFFGetField(band_bqa, TIFFTAG_SAMPLEFORMAT, &sample_band_bqa);

    TIFFGetField(band_red, TIFFTAG_IMAGELENGTH, &height_band);
    TIFFGetField(band_red, TIFFTAG_IMAGEWIDTH, &width_band);

    unsigned short byte_size_band_red = TIFFScanlineSize(band_red) / width_band;
    unsigned short byte_size_band_nir = TIFFScanlineSize(band_nir) / width_band;
    unsigned short byte_size_band_bqa = TIFFScanlineSize(band_bqa) / width_band;

    line_band_red = _TIFFmalloc(TIFFScanlineSize(band_red));
    line_band_nir = _TIFFmalloc(TIFFScanlineSize(band_nir));
    line_band_bqa = _TIFFmalloc(TIFFScanlineSize(band_bqa));

    pixel_read_band_red = PixelReader(sample_band_red, byte_size_band_red, line_band_red);
    pixel_read_band_nir = PixelReader(sample_band_nir, byte_size_band_nir, line_band_nir);
    pixel_read_band_bqa = PixelReader(sample_band_bqa, byte_size_band_bqa, line_band_bqa);

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
			exit(3 << 3);
    }

    _TIFFfree(line_band_red);
    _TIFFfree(line_band_nir);
    _TIFFfree(line_band_bqa);
}

void NDVIGenerate::landsat(Tiff ndvi, int width_band, int height_band, int mask, ldouble dist_sun_earth, vector<ldouble> param_band_red, vector<ldouble> param_band_nir){
    // Constants
    const int GRESCALE = 0;
    const int BRESCALE = 1;
    const int ESUN = 2;

    ldouble radiance_band_red[width_band];
    ldouble radiance_band_nir[width_band];
    ldouble line_ndvi[width_band];

    for(int line = 0; line < height_band; line ++){
        if(TIFFReadScanline(band_red, line_band_red, line) < 0){
            exit(2 << 3);
        }
        if(TIFFReadScanline(band_nir, line_band_nir, line) < 0){
            exit(2 << 3);
        }
        if(TIFFReadScanline(band_bqa, line_band_bqa, line) < 0){
            exit(2 << 3);
        }

        // RadianceCalc
        for(int col = 0; col < width_band; col ++){
            ldouble pixel_band_red = pixel_read_band_red.readPixel(col);
            ldouble pixel_band_nir = pixel_read_band_nir.readPixel(col);

            radiance_band_red[col] = pixel_band_red * param_band_red[GRESCALE] + param_band_red[BRESCALE];
            radiance_band_nir[col] = pixel_band_nir * param_band_nir[GRESCALE] + param_band_nir[BRESCALE];

            if(radiance_band_red[col] < 0) radiance_band_red[col] = 0;
            if(radiance_band_nir[col] < 0) radiance_band_nir[col] = 0;
        }

        //ReflectanceCalc
        for(int col = 0; col < width_band; col++){
            ldouble pixel_band_bqa = pixel_read_band_bqa.readPixel(col);
            if(fabs(pixel_band_bqa - mask) > EPS &&
               fabs(pixel_band_bqa - 20480) > EPS){
                line_ndvi[col] = NaN;
                continue;
            }

            ldouble reflectance_pixel_band_red, reflectance_pixel_band_nir;

            reflectance_pixel_band_red = (PI * radiance_band_red[col] * (dist_sun_earth*dist_sun_earth)) / (sintheta * param_band_red[ESUN]);
            reflectance_pixel_band_nir = (PI * radiance_band_nir[col] * (dist_sun_earth*dist_sun_earth)) / (sintheta * param_band_nir[ESUN]);

            line_ndvi[col] = (reflectance_pixel_band_nir - reflectance_pixel_band_red) / (reflectance_pixel_band_nir + reflectance_pixel_band_red);

            if(line_ndvi[col] > 1)
                line_ndvi[col] = 1;
            if(line_ndvi[col] < -1)
                line_ndvi[col] = -1;
        }

        if(TIFFWriteScanline(ndvi, line_ndvi, line) < 0){
            exit(4 << 3);
        }
    }
}

void NDVIGenerate::landsat(Tiff ndvi, int width_band, int height_band, int mask){

    ldouble line_ndvi[width_band];

    for(int line = 0; line < height_band; line++){
        if(TIFFReadScanline(band_red, line_band_red, line) < 0){
            exit(2 << 3);
        }
        if(TIFFReadScanline(band_nir, line_band_nir, line) < 0){
            exit(2 << 3);
        }
        if(TIFFReadScanline(band_bqa, line_band_bqa, line) < 0){
            exit(2 << 3);
        }

        for(int col = 0; col < width_band; col++){
            ldouble pixel_band_bqa = pixel_read_band_bqa.readPixel(col);
            if(fabs(pixel_band_bqa - mask) > EPS &&
               fabs(pixel_band_bqa - 20480) > EPS){
                line_ndvi[col] = NaN;
                continue;
            }

            ldouble reflectance_pixel_band_red, reflectance_pixel_band_nir;
            ldouble pixel_band_red = pixel_read_band_red.readPixel(col);
            ldouble pixel_band_nir = pixel_read_band_nir.readPixel(col);

            reflectance_pixel_band_red = (pixel_band_red * 0.00002 - 0.1) / sintheta;
            reflectance_pixel_band_nir = (pixel_band_nir * 0.00002 - 0.1) / sintheta;

            line_ndvi[col] = (reflectance_pixel_band_nir - reflectance_pixel_band_red) / (reflectance_pixel_band_nir + reflectance_pixel_band_red);

            if(line_ndvi[col] > 1)
                line_ndvi[col] = 1;
            if(line_ndvi[col] < -1)
                line_ndvi[col] = -1;
        }
        if (TIFFWriteScanline(ndvi, line_ndvi, line) < 0) {
            exit(4 << 3);
        }
    }
}
