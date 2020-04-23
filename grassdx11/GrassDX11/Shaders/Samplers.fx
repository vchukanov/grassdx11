#ifndef _SAMPLERS_H_
#define _SAMPLERS_H_

SamplerState g_samShadow
{
    Filter   = MIN_MAG_LINEAR_MIP_POINT;
    AddressU = Border;
    AddressV = Border;
    BorderColor = float4(1, 1, 1, 1);
};

SamplerState g_samAniso
{
    Filter   = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState g_samLinear
{
    Filter   = MIN_MAG_LINEAR_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
    MIPLODBIAS = -1.0;
};

SamplerState g_samPoint
{
    Filter   = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

float ShadowCoef(float4 vShadowTexCoord)
{
    vShadowTexCoord    = vShadowTexCoord / vShadowTexCoord.w;
    if (abs(vShadowTexCoord.z) > 1.0)
        return 0.7;
    vShadowTexCoord.y  = -vShadowTexCoord.y;
    vShadowTexCoord.xy = vShadowTexCoord.xy * 0.5 + float2(0.5, 0.5);
    float fDepth = g_txShadowMap.Sample(g_samShadow, vShadowTexCoord.xy).r;
    float fPixelZ = vShadowTexCoord.z * 0.5 + 0.5;
    if (fDepth + 0.0005 < fPixelZ)
    {
        return 0.2;
    }
    
    return 0.7;
}

#endif