#pragma once

#include <DirectXMath.h>
#include "PhysMath.h"

inline void OrthoMtx(XMMATRIX* a_Out, const XMVECTOR& a_Min, const XMVECTOR& a_Max)
{
   float* output = (float*)(a_Out);
   output[0] = 2.0f / (getx(a_Max) - getx(a_Min));
   output[4] = 0.0;
   output[8] = 0.0;
   output[12] = -(getx(a_Max) + getx(a_Min)) / (getx(a_Max) - getx(a_Min));

   output[1] = 0.0;
   output[5] = 2.0f / (gety(a_Max) - gety(a_Min));
   output[9] = 0.0;
   output[13] = -(gety(a_Max) + gety(a_Min)) / (gety(a_Max) - gety(a_Min));


   output[2] = 0.0;
   output[6] = 0.0;
   output[10] = 1.0f / (getz(a_Max) - getz(a_Min));//2.0f/(getz(a_Max)-getz(a_Min));
   //output[10] = -2.0f/(getz(a_Max)-getz(a_Min));
   output[14] = -getz(a_Min) / (getz(a_Max) - getz(a_Min));//-(getz(a_Max)+getz(a_Min))/(getz(a_Max)-getz(a_Min));
   //output[14] = (getz(a_Max)+getz(a_Min))/(getz(a_Max)-getz(a_Min));

   output[3] = 0.0;
   output[7] = 0.0;
   output[11] = 0.0;
   output[15] = 1.0;
}

inline void ModelViewMtx(XMMATRIX* a_Out, XMVECTOR a_Pos, XMVECTOR a_Dir, XMVECTOR a_Up)
{
   XMVECTOR upN;
   XMVECTOR lftN;
   float* output = (float*)a_Out;

   //lftN = a_Dir ^ a_Up;
   lftN = XMVector3Cross(a_Dir, a_Up);
   lftN = XMVector3Normalize(lftN);
   
   //upN = lftN ^ a_Dir;
   upN = XMVector3Cross(lftN, a_Dir);
   upN = XMVector3Normalize(upN);
   
   a_Dir = XMVector3Normalize(a_Dir);

   output[0] = getx(lftN);
   output[1] = getx(upN);
   output[2] = getx(a_Dir);
   output[3] = 0.0;

   output[4] = gety(lftN);
   output[5] = gety(upN);
   output[6] = gety(a_Dir);
   output[7] = 0.0;

   output[8] = getz(lftN);
   output[9] = getz(upN);
   output[10] = getz(a_Dir);
   output[11] = 0.0;

   output[12] = getx(-XMVector3Dot(lftN, a_Pos));//(lftN * a_Pos);
   output[13] = getx(-XMVector3Dot(upN, a_Pos));//(upN * a_Pos);
   output[14] = getx(-XMVector3Dot(a_Dir, a_Pos));
   output[15] = 1.0;
}
