cbuffer cAxesFanSettings
{
    float3 g_vAxesFanPosOnTex;
    float  g_fTime;

    float  g_fMaxHorizFlow;
    float  g_fMaxVertFlow;
    float  g_fDampPower;
    float  g_fDistPower;
    float  g_fMaxFlowRadius;
    float  g_fShift;
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
    float2 vsPos     : POSITION;
    float2 vTexCoord : TEXCOORD0;
};

  
AxesFanFlowPSIn VS( AxesFanFlowVSIn In )
{
    AxesFanFlowPSIn Out;
	Out.vPos = float4(In.vPos, 1.0);
	Out.vTexCoord = In.vTexCoord;
    Out.vsPos = float2(In.vPos.x, -In.vPos.y);

    return Out;
}


float4 PS( AxesFanFlowPSIn In ): SV_Target
{
   //float2 flowSrc = g_vAxesFanPosOnTex.xz;
   float2 flowSrc = float2(0, 0);
   
   float2 vFlowDirection = normalize(In.vsPos - flowSrc);
   float fDist = length(In.vsPos - flowSrc); //0, 1

   float fMagnitude = g_fMaxHorizFlow / pow(pow(abs(fDist / g_fMaxFlowRadius), g_fDistPower) + g_fShift, g_fDampPower);

   float2 vFlow = vFlowDirection * (fMagnitude / 2 + sin(g_fTime) * fMagnitude / 2);
   
   return float4(vFlow.y, -g_fMaxVertFlow, -vFlow.x, 1);

   /*if (length(In.vsPos - g_vAxesFanPosOnTex.xz) < 0.1) {
       return float4(0, -0.04, 0, 1);
   }
   return float4(0, 0, 0, 1);*/
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
