#pragma once

#include "includes.h"
#include "common.h"
#include "ObjArray.h"

namespace maths
{
   inline XMFLOAT3 operator * (XMFLOAT3 vVec, XMFLOAT4X4& mtx)
   {
      XMFLOAT3 output;
      float* v = (float*)& vVec;
      float* m = (float*)& mtx;
      float x = m[0] * v[0] + m[4] * v[1] + m[8] * v[2] + m[12];
      float y = m[1] * v[0] + m[5] * v[1] + m[9] * v[2] + m[13];
      float z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14];
      float w = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15];

      output.x = x / w;
      output.y = y / w;
      output.z = z / w;
      return output;
   }

   /*start point & direction*/
   class Line
   {
   protected:
      XMFLOAT3 m_Dir;
      XMFLOAT3 m_Point;
   public:
      Line (void);
      Line (XMFLOAT3& a_Dir, XMFLOAT3& a_Point);

      XMFLOAT3& Dir   (void);
      XMFLOAT3& Point (void);
   };

   /*(n, d) - two values to determine a plane*/
   class Plane
   {
   protected:
      XMFLOAT4 m_NandD;

   public:
      Plane (void);
      Plane (XMFLOAT3 a_Norm, float a_D);
                 
      void     SetD (float a_D);
      void     SetN (XMFLOAT3 a_Norm);
      void     SetN (float x, float y, float z);
      XMFLOAT3 GetN (void);
      float    GetD (void);
   };

   class PointArray;
   /*axis-aligned box, defined by two extreme points*/
   class AABox
   {
   protected:
      XMFLOAT3 m_Min;
      XMFLOAT3 m_Max;

   public:
      AABox (void);
      AABox (XMFLOAT3 a_Min, XMFLOAT3 a_Max);
      
      XMFLOAT3& Min(void);
      XMFLOAT3& Max(void);
      
      void GetPoints (PointArray* a_Res);
      bool Collide   (AABox& a_Collider);
      /*gets last intersection point of line with this BBox*/
      bool LastLineISect (XMFLOAT3* a_Res, Line a_Line);
   };

   const int MaxPointsNum = 150;
   class PointArray : public ObjArray<XMFLOAT3, MaxPointsNum>
   {
   public:
      void Transform  (XMMATRIX& a_Mtx);
      void CalcAABBox (AABox* a_Res);
   };

   /*geometry object, which contains some polygons as point arrays*/
   const int MaxPolygonsNum = 32;
   class PolyPointArray : public ObjArray<PointArray, MaxPolygonsNum>
   {
   public:
      void ToPointArray (PointArray* a_Res);
   };

   inline int alikeVector3 (const XMFLOAT3 a, const XMFLOAT3 b, const float epsilon)
   {
      return
         com::alike(a.x, b.x, epsilon) &&
         com::alike(a.y, b.y, epsilon) &&
         com::alike(a.z, b.z, epsilon);
   }
}