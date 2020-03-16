Texture2D g_txShadowMap; // Hack

#include "Samplers.fx"

cbuffer cbPerObject
{
    float4x4 g_mWorld;
    float4x4 g_mOrtho;
    float    g_fScale;
    int      g_iArraySlice;
};


struct DebugWindowVSIn
{
    float3 vPos      : POSITION;
    float2 vTexCoord : TEXCOORD0;
};

struct DebugWindowPSIn
{
    float4 vPos      : SV_Position;
    float2 vTexCoord : TEXCOORD0;
};

Texture2DArray g_texture;

  
DebugWindowPSIn VS( DebugWindowVSIn In )
{
    DebugWindowPSIn Out;
    float4 vWorldPos = float4(In.vPos, 1.0);
    Out.vPos  = mul(vWorldPos, g_mOrtho);
    Out.vTexCoord = In.vTexCoord;
    return Out;
}

float4 PS( DebugWindowPSIn In ): SV_Target
{ 
   float4 color = g_texture.SampleLevel(g_samLinear, float3(In.vTexCoord, g_iArraySlice), 0) * 10;
   float gray = (abs(color.r) + abs(color.g) + abs(color.b)) / 3;
   //return float4(gray, gray, gray, 1);
   
   return g_texture.SampleLevel(g_samLinear, float3(In.vTexCoord, g_iArraySlice), 0) * 10;
}

technique10 Render
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, PS()));
    }
}
