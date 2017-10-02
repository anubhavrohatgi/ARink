#ifndef INTERPOLATION_H
#define INTERPOLATION_H


#include <iostream>
#include <math.h>
#include <opencv2/core.hpp>


// Piecewise Linear interpolation implemented on a particular Channel
inline void interpolation(uchar* lut,float* fullRange,float* Curve,float* originalValue)
{
  int i;
  for (i=0; i < 256; i++)
  {
    int j = 0;
    float a = fullRange[i];
    while(a > originalValue[j])
    {
      j++;
    }
    if (a == originalValue[j])
    {
      lut[i] = Curve[j];
      continue;
    }
    float slope = ((float)(Curve[j] - Curve[j-1]))/(originalValue[j] - originalValue[j-1]);
    float constant = Curve[j] - slope * originalValue[j];
    lut[i] = slope * fullRange[i] + constant;
  }
}





#endif // INTERPOLATION_H
