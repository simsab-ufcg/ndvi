#ifndef TYPES_INCLUDED

  #define TYPES_INCLUDED
  
  #include <vector>
  #include <tiffio.h>
  #include <math.h>

  using namespace std;
  
  using ldouble = double;

  using Tiff = TIFF*;
  
  const ldouble EPS = 1e-7;

  const ldouble NaN = -sqrt(-1.0);
 
#endif