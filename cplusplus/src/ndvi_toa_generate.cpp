#include "ndvi_toa_generate.h"

NDVITOAGenerate::NDVITOAGenerate(ldouble _sun_elevation, Tiff _band_red, Tiff _band_nir, Tiff _band_bqa){
    sintheta = sin(_sun_elevation);
    band_red = _band_red;
    band_nir = _band_nir;
    band_bqa = _band_bqa;
}

void NDVITOAGenerate::processNDVI(int number_sensor, ldouble dist_sun_earth, Tiff ndvi, vector<ldouble> param_band_red, vector<ldouble> param_band_nir){
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

    landsat(ndvi, width_band, height_band, mask, dist_sun_earth, param_band_red, param_band_nir);

    _TIFFfree(line_band_red);
    _TIFFfree(line_band_nir);
    _TIFFfree(line_band_bqa);
}

void NDVITOAGenerate::landsat(Tiff ndvi, int width_band, int height_band, int mask, ldouble dist_sun_earth, vector<ldouble> param_band_red, vector<ldouble> param_band_nir){
    // Constants
    const int MULT_BAND = 0;
    const int ADD_BAND = 1;

    ldouble line_ndvi[width_band];

    for(int line = 0; line < height_band; line ++){
        TIFFReadScanline(band_red, line_band_red, line);
        TIFFReadScanline(band_nir, line_band_nir, line);
        TIFFReadScanline(band_bqa, line_band_bqa, line);
        
        //ReflectanceCalc
        for(int col = 0; col < width_band; col++){
            ldouble pixel_band_bqa = pixel_read_band_bqa.readPixel(col);
            if(fabs(pixel_band_bqa - mask) > EPS){
                line_ndvi[col] = NaN;
                continue;
            }

            ldouble reflectance_pixel_band_red, reflectance_pixel_band_nir;
            ldouble pixel_band_red = pixel_read_band_red.readPixel(col);
            ldouble pixel_band_nir = pixel_read_band_nir.readPixel(col);

            reflectance_pixel_band_red = (pixel_band_red * param_band_red[MULT_BAND] + param_band_red[ADD_BAND]) / sintheta;
            reflectance_pixel_band_nir = (pixel_band_nir * param_band_nir[MULT_BAND] + param_band_nir[ADD_BAND]) / sintheta;

            line_ndvi[col] = (reflectance_pixel_band_nir - reflectance_pixel_band_red) / (reflectance_pixel_band_nir + reflectance_pixel_band_red);

            if(line_ndvi[col] > 1 || line_ndvi[col] < -1)
                line_ndvi[col] = NaN;
        }

        TIFFWriteScanline(ndvi, line_ndvi, line);
    }
}