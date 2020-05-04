
Texture2DArray g_txWind;
Texture2D      g_txFlow;

Texture2D g_txShadowMap; // Hack

cbuffer cUserControlled
{
    float g_fWindStrength;
};

#include "Samplers.fx"

struct MixVSIn
{
    float3 vPos      : POSITION;
    float2 vTexCoord : TEXCOORD0;
};

struct MixPSIn
{
    float4 vPos       : SV_Position;
    float2 vTexCoordW : TEXCOORD0;
    float2 vTexCoordF : TEXCOORD1;
};

struct MixPSOut
{
    float4 vOut0 : SV_Target0;
    float4 vOut1 : SV_Target1;
    float4 vOut2 : SV_Target2;
};
  
MixPSIn VSMix( MixVSIn In )
{
    MixPSIn Out;
    Out.vPos = float4(In.vPos, 1.0);
    Out.vTexCoordW = In.vTexCoord * 6; //g_fWindTexTile
    Out.vTexCoordF = float2(In.vTexCoord.x, -In.vTexCoord.y);
    return Out;
}


MixPSOut PSMix( MixPSIn In ) : SV_Target
{
    MixPSOut Out;

    float3 vValue1 = g_txWind.SampleLevel(g_samLinear, float3(In.vTexCoordW, 0), 0).rgb;
    float3 vWind1 = (vValue1) * g_fWindStrength;
    float3 vValue2 = g_txWind.SampleLevel(g_samLinear, float3(In.vTexCoordW, 1), 0).rgb;
    float3 vWind2 = (vValue2) * g_fWindStrength;
    float3 vValue3 = g_txWind.SampleLevel(g_samLinear, float3(In.vTexCoordW, 2), 0).rgb;
    float3 vWind3 = (vValue3) * g_fWindStrength;
    
    float3 vFlow = g_txFlow.SampleLevel(g_samLinear, float3(In.vTexCoordF, 0), 0).rgb;

    Out.vOut0 = float4(vWind1 + vFlow, 1.0f);
    Out.vOut1 = float4(vWind2 + vFlow, 1.0f);
    Out.vOut2 = float4(vWind3 + vFlow, 1.0f);

    return Out;
}


technique10 Render
{
    pass MixTexturesPass
    {
        SetVertexShader(CompileShader(vs_4_0, VSMix()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PSMix()));
    }
}
