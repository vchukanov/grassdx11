#ifndef _GS_FUNC_H_
#define _GS_FUNC_H_

#include "Fog.fx"

void CreateVertex( float3 a_vPos, float2 a_vTexCoord, float a_fLightParam, uint a_uTexIndex, float a_fDissolve, float2 a_vWorldTC, inout PSIn Vertex)
{
    Vertex.vShadowPos   = mul( float4(a_vPos, 1.0), g_mLightViewProj);    
    Vertex.vPos         = mul( float4(a_vPos, 1.0), g_mViewProj);    
    Vertex.vTexCoord.xy = a_vTexCoord;
    Vertex.vTexCoord.z  = FogValue(length(Vertex.vPos.xyz));
    Vertex.vTexCoord.w  = length(a_vPos - g_mInvCamView[3].xyz);
    Vertex.vWorldTC     = a_vWorldTC;
    Vertex.fLightParam  = abs(a_fLightParam);
    Vertex.vTerrSpec	= g_txTerrainLightMap.SampleLevel(g_samLinear, a_vWorldTC, 0).rg;
    Vertex.uIndex       = a_uTexIndex;    
    Vertex.fDissolve    = a_fDissolve;
}

float3 LerpPoint( float3 vPos[4], float fLerpCoef )
{
    float InvLerp = 1.0 - fLerpCoef;
    return InvLerp * InvLerp * InvLerp * vPos[0] + 3.0 * fLerpCoef * InvLerp * InvLerp * vPos[1] +
            3.0 * InvLerp * fLerpCoef * fLerpCoef * vPos[2] + fLerpCoef * fLerpCoef * fLerpCoef * vPos[3];
}

void InitFramePts( GSIn In, out float3 vPos[4] )
{
    vPos[0] = In.vPos0;
    vPos[1] = In.vPos1;
    vPos[2] = In.vPos2;
    vPos[3] = In.vPos3;
}
#endif