#pragma once

#include "includes.h"

inline void OrthoMtx(XMFLOAT4X4* a_Out, const XMFLOAT3& a_Min, const XMFLOAT3& a_Max)
{
   float* output = (float*)(a_Out);
   output[0] = 2.0f / (a_Max.x - a_Min.x);
   output[4] = 0.0;
   output[8] = 0.0;
   output[12] = -(a_Max.x + a_Min.x) / (a_Max.x - a_Min.x);

   output[1] = 0.0;
   output[5] = 2.0f / (a_Max.y - a_Min.y);
   output[9] = 0.0;
   output[13] = -(a_Max.y + a_Min.y) / (a_Max.y - a_Min.y);


   output[2] = 0.0;
   output[6] = 0.0;
   output[10] = 1.0f / (a_Max.z - a_Min.z);//2.0f/(a_Max.z-a_Min.z);
   //output[10] = -2.0f/(a_Max.z-a_Min.z);
   output[14] = -a_Min.z / (a_Max.z - a_Min.z);//-(a_Max.z+a_Min.z)/(a_Max.z-a_Min.z);
   //output[14] = (a_Max.z+a_Min.z)/(a_Max.z-a_Min.z);

   output[3] = 0.0;
   output[7] = 0.0;
   output[11] = 0.0;
   output[15] = 1.0;
}

inline void ModelViewMtx(XMFLOAT4X4* a_Out, XMFLOAT3 a_Pos, XMFLOAT3 a_Dir, XMFLOAT3 a_Up)
{
   float* output = (float*)a_Out;

   //lftN = a_Dir ^ a_Up;
   XM_TO_V(a_Pos, vPos, 3);
   XM_TO_V(a_Dir, vDir, 3);
   XM_TO_V(a_Up,  vUp,  3);

   XMVECTOR lftN = XMVector3Cross(vDir, vPos);
   lftN = XMVector3Normalize(lftN);

   lftN = XMVector3Cross(vDir, vUp);
   //lftN.Normalize();
   lftN = XMVector3Normalize(lftN);

   //upN = lftN ^ a_Dir;
   XMVECTOR upN = XMVector3Cross(lftN, vDir);
   //upN.Normalize();
   upN = XMVector3Normalize(upN);
   //a_Dir.Normalize();
   vDir = XMVector3Normalize(vDir);

   V_TO_XM(lftN, xLftN, 3);
   V_TO_XM(upN,  xUpN,  3);
   V_TO_XM(vDir, xDir,  3);

   output[0] = xLftN.x;
   output[1] = xUpN.x;
   output[2] = xDir.x;
   output[3] = 0.0;

   output[4] = xLftN.y;
   output[5] = xUpN.y;
   output[6] = xDir.y;
   output[7] = 0.0;

   output[8] = xLftN.z;
   output[9] = xUpN.z;
   output[10] = xDir.z;
   output[11] = 0.0;
   
   output[12] = XMVectorGetX(XMVectorScale(XMVector3Dot(lftN, vPos), -1));//(lftN * a_Pos);
   output[13] = XMVectorGetX(XMVectorScale(XMVector3Dot(upN, vPos) , -1));//(upN * a_Pos);
   output[14] = XMVectorGetX(XMVectorScale(XMVector3Dot(vDir, vPos), -1));
   output[15] = 1.0;
}
