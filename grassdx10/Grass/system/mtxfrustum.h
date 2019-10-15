#pragma once

#include <d3dx10math.h>

inline void OrthoMtx(D3DXMATRIX *a_Out, const D3DXVECTOR3 &a_Min, const D3DXVECTOR3 &a_Max)
{
    float *output = (float*)(a_Out);
    output[ 0] = 2.0f/(a_Max[0]-a_Min[0]);
    output[ 4] = 0.0;
    output[ 8] = 0.0;
    output[12] = -(a_Max[0]+a_Min[0])/(a_Max[0]-a_Min[0]);

    output[ 1] = 0.0;
    output[ 5] = 2.0f/(a_Max[1]-a_Min[1]);
    output[ 9] = 0.0;
    output[13] = -(a_Max[1]+a_Min[1])/(a_Max[1]-a_Min[1]);

    
    output[ 2] = 0.0;
    output[ 6] = 0.0;
    output[10] = 1.0f/(a_Max[2]-a_Min[2]);//2.0f/(a_Max[2]-a_Min[2]);
    //output[10] = -2.0f/(a_Max[2]-a_Min[2]);
    output[14] = -a_Min[2]/(a_Max[2]-a_Min[2]);//-(a_Max[2]+a_Min[2])/(a_Max[2]-a_Min[2]);
    //output[14] = (a_Max[2]+a_Min[2])/(a_Max[2]-a_Min[2]);

    output[ 3] = 0.0;
    output[ 7] = 0.0;
    output[11] = 0.0;
    output[15] = 1.0;
}

inline void ModelViewMtx(D3DXMATRIX *a_Out,  D3DXVECTOR3 a_Pos,  D3DXVECTOR3 a_Dir,  D3DXVECTOR3 a_Up)
{
    D3DXVECTOR3 upN;
    D3DXVECTOR3 lftN;
    float *output = (float*)a_Out;

    //lftN = a_Dir ^ a_Up;
    D3DXVec3Cross(&lftN, &a_Dir, &a_Up);
    //lftN.Normalize();
    D3DXVec3Normalize(&lftN, &lftN);

    //upN = lftN ^ a_Dir;
    D3DXVec3Cross(&upN, &lftN, &a_Dir);
    //upN.Normalize();
    D3DXVec3Normalize(&upN, &upN);
    //a_Dir.Normalize();
    D3DXVec3Normalize(&a_Dir, &a_Dir);

    output[ 0] = lftN[0];
    output[ 1] = upN[0];
    output[ 2] = a_Dir[0];
    output[ 3] = 0.0;

    output[ 4] = lftN[1];
    output[ 5] = upN[1];
    output[ 6] = a_Dir[1];
    output[ 7] = 0.0;

    output[ 8] = lftN[2];
    output[ 9] = upN[2];
    output[10] = a_Dir[2];
    output[11] = 0.0;


    output[12] = -D3DXVec3Dot(&lftN, &a_Pos);//(lftN * a_Pos);
    output[13] = -D3DXVec3Dot(&upN, &a_Pos);//(upN * a_Pos);
    output[14] = -D3DXVec3Dot(&a_Dir, &a_Pos);
    output[15] =  1.0;
}
