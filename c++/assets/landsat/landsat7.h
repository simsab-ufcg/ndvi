#include "landsat_function.h"
#include "../sensor/sensor_ETM.h"

struct Landsat7 : LandsatFunction{
  Sensor sensor;
  ldouble dist_sun_earth;

  Landsat7(ldouble sun_elevation, ldouble _dist_sun_earth): dist_sun_earth(_dist_sun_earth), sensor(SensorETM()), LandsatFunction(sun_elevation) {}
  void processNDVI(Tiff band4, Tiff band5, Tiff ndvi);
  Sensor getSensor();
};
