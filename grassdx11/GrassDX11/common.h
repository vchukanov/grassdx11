#pragma once

#include <math.h>
#include <stdlib.h>

// common.h
namespace com
{
#ifndef M_PI
#define M_PI (double)3.14159265358979323846
#endif
#define _2PI (double)6.283185307179586
#ifndef PI
#define PI M_PI
#endif
#define PI_2   (double)1.570796326794897
#define PI_180 (double)0.0261799387799149

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif


template <typename scalar>
inline scalar sqr(const scalar & t) { return t * t; };

template <typename scalar>
inline scalar mod (scalar a, scalar b)
{
   scalar result = a % b;
   if (result >= 0)
   {
      return result;
   }
   else
   {
      return result + b;
   }
}

inline float mod(float a, float b)
{
   return fmodf(a, b);
}


inline float frand(float Min, float Max, float prec = 0.001f)
{
   if (Min > Max)
      return 0.0f;
   if (Min == Max)
      return Min;
   return Min + prec * (float)(rand() % long((Max - Min) / prec) + 1);
}

template <typename Real>
void clamp(Real* value, const Real min, const Real max)
{
   if ((*value) > max)
   {
      (*value) = max;
   }
   else
   {
      if ((*value) < min)
      {
         (*value) = min;
      }
   }
}

template <typename scalar>
scalar maximum(const scalar a, const scalar b)
{
   return ((a > b) ? (a) : (b));
}

template <typename scalar>
scalar minimum(const scalar a, const scalar b)
{
   return ((a < b) ? (a) : (b));
}

template <typename Real>
Real Sign(const Real a)
{
   return ((a < 0.0) ? (-1.0) : ((a == 0.0) ? (0.0) : (1.0)));
}

template <typename Real>
Real coTan(const Real vIn) {
   return (Real)-tan(vIn + PI_2);
}


template <typename Real>
Real relativeEpsilon(const Real a, const Real epsilon)
{
   Real relEpsilon = max(fabs(a * epsilon), epsilon);
   return relEpsilon;
}

template <typename Real>
bool equal(const Real a, const Real b, const Real epsilon)
{
   return (fabs(a - b) < epsilon);
}

template <typename Real>
int alike(const Real a, const Real b, const Real epsilon) {
   if (a == b)
   {
      return 1;
   }
   {
      Real relEps = relativeEpsilon(a, epsilon);
      return (a - relEps <= b) && (b <= a + relEps);
   }
}

}