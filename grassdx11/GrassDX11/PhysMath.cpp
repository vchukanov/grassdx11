#include "PhysMath.h"

using namespace DirectX;

#define my_abs(a) ((a) > 0 ? (a) : (-a))


float2 create(float x, float y)
{
   return XMVectorSet(x, y, 0, 0);
}


float3 create(float x, float y, float z)
{
   return XMVectorSet(x, y, z, 0);
}


float4 create(float x, float y, float z, float w)
{
   return XMVectorSet(x, y, z, w);
}


float getx(const float4& v)
{
   return XMVectorGetX(v);
}


float gety(const float4& v)
{
   return XMVectorGetY(v);
}


float getz(const float4& v)
{
   return XMVectorGetZ(v);
}


float getw(const float4& v)
{
   return XMVectorGetW(v);
}


float getcoord(const float4& v, int idx)
{
   if (idx == 0)
      return getx(v);
   else if (idx == 1)
      return gety(v);
   else if (idx == 2)
      return getz(v);
   else if (idx == 3)
      return getw(v);
   else
      assert(0);
}


void setx(float4& v, float x)
{
   v *= create(0.0f, 1.0f, 1.0f, 1.0f);
   v += create(x, 0.0f, 0.0f, 0.0f);
}


void sety(float4& v, float y)
{
   v *= create(1.0f, 0.0f, 1.0f, 1.0f);
   v += create(0.0f, y, 0.0f, 0.0f);
}


void setz(float4& v, float z)
{
   v *= create(1.0f, 1.0f, 0.0f, 1.0f);
   v += create(0.0f, 0.0f, z, 0.0f);
}


void setw(float4& v, float w)
{
   v *= create(1.0f, 1.0f, 1.0f, 0.0f);
   v += create(0.0f, 0.0f, 0.0f, w);
}



void setcoord(float4& v, int idx, float val)
{
   if (idx == 0)
      return setx(v, val);
   else if (idx == 1)
      return sety(v, val);
   else if (idx == 2)
      return setz(v, val);
   else if (idx == 3)
      return setw(v, val);
   else
      assert(0);
}


float3x3 transpose(const float3x3& m)
{
   return XMMatrixTranspose(m);
}


float3x3 inverse(const float3x3& m)
{
   return XMMatrixInverse(NULL, m);
}


float3x3 identity(void)
{
   return XMMatrixIdentity();
}


float3 scale(const float s, const float3& m)
{
   return m * s;
}


float3 mul(const float3& n, const float3& m)
{
   return m * n;
}


float3 mul(const float3& v, const float3x3& m)
{
   return XMVector3Transform(v, m);
}


float3 mul(const float3x3& m, const float3& v)
{
   return XMVector3Transform(v, m);
}


float3x3 mul(const float3x3& n, const float3x3& m)
{
   return XMMatrixMultiply(n, m);
}


float3 cross(const float3& a, const float3& b)
{
   return XMVector3Cross(a, b);
}


float dot(const float3& a, const float3& b)
{
   return XMVectorGetX(XMVector3Dot(a, b));
}


float length(const float3& v)
{
   return XMVectorGetX(XMVector3Length(v));
}


float3 normalize(const float3& v)
{
   return XMVector3Normalize(v);
}


float clamp(float v, float a, float b)
{
   if (v < a) {
      v = a;
   }
   if (v > b) {
      v = b;
   }
   return v;
}


float3 oneOver(const float3& v)
{
   return create(1.0f / getx(v), 1.0f / gety(v), 1.0f / getz(v));
}


float3x3 MakeRotationMatrix(const float3& axis)
{
   if (length(axis) < EPSILON)
      return identity();

   float angle = length(axis);
   float3 nAxis = normalize(axis);

   float fCos = cos(angle);
   float l_fCos = 1 - fCos;
   float fSin = sin(angle);

   float x2 = getx(nAxis) * getx(nAxis);
   float y2 = gety(nAxis) * gety(nAxis);
   float z2 = getz(nAxis) * getz(nAxis);

   float xy = getx(nAxis) * gety(nAxis);
   float xz = getx(nAxis) * getz(nAxis);
   float yz = gety(nAxis) * getz(nAxis);

   return float3x3(
      create(fCos + l_fCos * x2, l_fCos * xy - fSin * getz(nAxis), l_fCos * xz + fSin * gety(nAxis), 0),
      create(l_fCos * xy + fSin * getz(nAxis), fCos + l_fCos * y2, l_fCos * yz - fSin * getx(nAxis), 0),
      create(l_fCos * xz - fSin * gety(nAxis), l_fCos * yz + fSin * getx(nAxis), fCos + l_fCos * z2, 0),
      create(0, 0, 0, 1)
   );
}


#define GET_3x3_ELEM(i, j) (_get##j(m.r[i]))

#define SQR(x) ((x)*(x))


float _get0(float4 v)
{
   return XMVectorGetX(v);
}


float _get1(float4 v)
{
   return XMVectorGetY(v);
}


float _get2(float4 v)
{
   return XMVectorGetZ(v);
}


float3 MakeRotationVector(const float3x3& m)
{
   float3 v;
   float thetha;
   float3 sp = create(GET_3x3_ELEM(2, 1) - GET_3x3_ELEM(1, 2),
      GET_3x3_ELEM(0, 2) - GET_3x3_ELEM(2, 0),
      GET_3x3_ELEM(1, 0) - GET_3x3_ELEM(0, 1));


   if (my_abs(getx(sp)) < EPSILON && my_abs(gety(sp)) < EPSILON && my_abs(getz(sp)) < EPSILON)
   {
      //identity matrix
      if (my_abs(GET_3x3_ELEM(1, 0) + GET_3x3_ELEM(0, 1)) < EPSILON &&
         my_abs(GET_3x3_ELEM(0, 2) + GET_3x3_ELEM(2, 0)) < EPSILON &&
         my_abs(GET_3x3_ELEM(1, 2) + GET_3x3_ELEM(2, 1)) < EPSILON)
         return create(0.0f, 0.0f, 0.0f);
      else
      {
         thetha = (float)PI;
         float xx = (GET_3x3_ELEM(0, 0) + 1.0f) / 2.0f;
         float yy = (GET_3x3_ELEM(1, 1) + 1.0f) / 2.0f;
         float zz = (GET_3x3_ELEM(2, 2) + 1.0f) / 2.0f;

         float xy = (GET_3x3_ELEM(0, 1) + GET_3x3_ELEM(1, 0)) / 4.0f;
         float xz = (GET_3x3_ELEM(0, 2) + GET_3x3_ELEM(2, 0)) / 4.0f;
         float yz = (GET_3x3_ELEM(1, 2) + GET_3x3_ELEM(2, 1)) / 4.0f;

         if ((xx > yy) && (xx > zz))
         {
            if (xx < EPSILON)
            {
               v = create(0.0f, 0.7071f, 0.7071f);
               return v * thetha;
            }
            else
            {
               v = create(xx, xy, xz) / sqrtf(xx);
               return v * thetha;
            }
         }
         else if (yy > zz)
         {
            if (yy < EPSILON)
            {
               v = create(0.7071f, 0.0f, 0.7071f);
               return v * thetha;
            }
            else
            {
               v = create(yy, xy, yz) / sqrtf(yy);
               return v * thetha;
            }
         }
         else
         {
            if (zz < EPSILON)
            {
               v = create(0.7071f, 0.7071f, 0.0f);
               return v * thetha;
            }
            else
            {
               v = create(zz, xz, yz) / sqrtf(zz);
               return v * thetha;
            }
         }
      }
   }

   //non singular
   float spp = sqrtf(SQR(getx(sp)) +
      SQR(gety(sp)) +
      SQR(getz(sp)));

   if (spp < EPSILON)
      spp = 1.0f;

   v = sp / spp;
   float cosAngle = (GET_3x3_ELEM(0, 0) + GET_3x3_ELEM(1, 1) + GET_3x3_ELEM(2, 2) - 1.0f) / 2.0f;

   if (cosAngle > 1.0f) cosAngle = 1.0f;
   if (cosAngle < -1.0) cosAngle = -1.0f;

   thetha = acos(cosAngle);

   return thetha * v;
}


float3x3 MakeIdentityMatrix(void)
{
   float3x3 i(
      create(1, 0, 0, 0),
      create(0, 1, 0, 0),
      create(0, 0, 1, 0),
      create(0, 0, 0, 1)
   );

   return i;
}