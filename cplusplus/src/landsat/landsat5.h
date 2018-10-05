#include "landsat_function.h"
#include "../sensor/sensor_TM.h"

struct Landsat5 : LandsatFunction{
  Sensor sensor;
  ldouble dist_sun_earth;

  Landsat5(ldouble sun_elevation, ldouble _dist_sun_earth): dist_sun_earth(_dist_sun_earth), sensor(SensorTM()), LandsatFunction(sun_elevation) {}
  void processNDVI(Tiff band4, Tiff band5, Tiff ndvi, Tiff band_bqa);
  Sensor getSensor();
};
