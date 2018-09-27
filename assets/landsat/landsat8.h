#include "landsat_function.h"

struct Landsat8: LandsatFunction{

  Landsat8(ldouble sun_elevation): LandsatFunction(sun_elevation) {}
  void processNDVI(Tiff band4, Tiff band5, Tiff ndvi);
};
