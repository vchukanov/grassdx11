cbuffer cAxesFanSettings
{
    float3 g_vAxesFanPosOnTex;
    float  g_fTime;
};

Texture2D g_txShadowMap; // Hack

#include "Samplers.fx"


struct AxesFanFlowVSIn
{
    float3 vPos      : POSITION;
    float2 vTexCoord : TEXCOORD0;
};

struct AxesFanFlowPSIn
{
    float4 vPos      : SV_Position;
    float3 vsPos     : POSITION;
    float2 vTexCoord : TEXCOORD0;
};

  
AxesFanFlowPSIn VS( AxesFanFlowVSIn In )
{
    AxesFanFlowPSIn Out;
	Out.vPos = float4(In.vPos, 1.0);
	Out.vTexCoord = In.vTexCoord;
    Out.vsPos = float3(In.vPos.x, 0, In.vPos.y);

    return Out;
}


float4 PS( AxesFanFlowPSIn In ): SV_Target
{
   float3 vFlowDirection = normalize(In.vsPos - float3(g_vAxesFanPosOnTex.x, 0, g_vAxesFanPosOnTex.z));
   float fDist = length(In.vsPos - float3(g_vAxesFanPosOnTex.x, 0, g_vAxesFanPosOnTex.z));
   float3 vFlow = vFlowDirection * (1 / (fDist + 1));
   vFlow *= 0.04; //* abs(sin(g_fTime * 2));
   // z, y, -x
   //return float4(vFlowDirection.z, vFlowDirection.y, -vFlowDirection.x, 1);
   return float4(vFlow.z, vFlow.y, vFlow.x, 1);
}


technique10 Render
{
    pass AxesFanFlowPass
    {
        SetVertexShader(CompileShader(vs_4_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, PS()));
    }
}
