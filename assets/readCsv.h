#ifndef READ_CSV_INCLUDED

  #define READ_CSV_INCLUDED
  
  #include <fstream>
  #include <sstream>
  #include "types.h"

  using namespace std;
  
  struct ReadCsv{
      string search(string filter);
      ldouble getSunElevation();
      int getNumberSensor();
  };
  
#endif
