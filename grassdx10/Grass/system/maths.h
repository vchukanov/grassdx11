#pragma once

#include <d3d10.h>
#include "common.h"
#include "ObjArray.h"
#include "mtxfrustum.h"


namespace maths
{
    inline D3DXVECTOR3 operator * (D3DXVECTOR3 vVec, D3DXMATRIX& mtx)
    {
        D3DXVECTOR3 output;
        float *v = (float*)&vVec;
        float *m = (float*)&mtx;
        float x = m[0]*v[0] + m[4]*v[1] + m[ 8]*v[2] + m[12];
        float y = m[1]*v[0] + m[5]*v[1] + m[ 9]*v[2] + m[13];
        float z = m[2]*v[0] + m[6]*v[1] + m[10]*v[2] + m[14];
        float w = m[3]*v[0] + m[7]*v[1] + m[11]*v[2] + m[15];

        output.x = x / w;
        output.y = y / w;
        output.z = z / w;
        return output;
    }

    /*start point & direction*/
    class Line
    {
    protected:
        D3DXVECTOR3 m_Dir;
        D3DXVECTOR3 m_Point;
    public:
        Line              ();
        Line              (D3DXVECTOR3 &a_Dir, D3DXVECTOR3 &a_Point);
        D3DXVECTOR3& Dir     ();
        D3DXVECTOR3& Point   ();
    };

    /*(n, d) - two values to determine a plane*/
    class Plane
    {
    protected:
        D3DXVECTOR4 m_NandD;

    public:
        Plane         ();
        Plane         (D3DXVECTOR3 a_Norm, float a_D);
        void    SetD (float a_D);
        void    SetN (D3DXVECTOR3 a_Norm);
        void    SetN (float x, float y, float z);
        D3DXVECTOR3 GetN ();
        float   GetD ();
    };

    class PointArray;
    /*axis-aligned box, defined by two extreme points*/
    class AABox
    {
    protected:
        D3DXVECTOR3 m_Min;
        D3DXVECTOR3 m_Max;

    public:
        AABox               ();
        AABox               (D3DXVECTOR3 a_Min, D3DXVECTOR3 a_Max);
        D3DXVECTOR3& Min       ();
        D3DXVECTOR3& Max       ();
        void GetPoints     (PointArray *a_Res);
        bool Collide       (AABox &a_Collider);
        /*gets last intersection point of line with this BBox*/
        bool LastLineISect (D3DXVECTOR3 *a_Res, Line a_Line);
    };

    const int MaxPointsNum = 50;
    class PointArray: public ObjArray<D3DXVECTOR3, MaxPointsNum> 
    {
    public:
        void Transform(D3DXMATRIX &a_Mtx);
        void CalcAABBox(AABox *a_Res);
    };

    /*geometry object, which contains some polygons as point arrays*/
    const int MaxPolygonsNum = 32;
    class PolyPointArray: public ObjArray<PointArray, MaxPolygonsNum>
    {
    public:
        void ToPointArray(PointArray *a_Res);
    };

    inline int alikeVector3(const D3DXVECTOR3 a, const D3DXVECTOR3 b, const float epsilon) 
    {
        return 
            com::alike(a[0],b[0],epsilon) &&
            com::alike(a[1],b[1],epsilon) &&
            com::alike(a[2],b[2],epsilon);
    }

}