#ifndef SENSOR_INCLUDED

  #define SENSOR_INCLUDED
  
  #include "../types.h"

  using namespace std;
  
  struct Sensor{
    vector<ldouble> parameters_b4;
    vector<ldouble> parameters_b5;

    Sensor(vector<ldouble> _parameters_b4, vector<ldouble> _parameters_b5): parameters_b4(_parameters_b4), parameters_b5(_parameters_b5) {};

    vector<ldouble> getParamBand4();
    vector<ldouble> getParamBand5();
  };
  
#endif
