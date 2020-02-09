#include "States.fx"

cbuffer cEveryFrame
{
    float4x4 g_mWorld;
    float4x4 g_mViewProj;
}

struct TerrVSIn
{
    float3 vPos      : POSITION;
    float2 vTexCoord : TEXCOORD0;
};

struct TerrPSIn
{
    float4 vPos      : SV_Position;
    //float4 vShadowPos: TEXCOORD0;
    float4 vTexCoord : TEXCOORD1;
    //float3 vNormal   : NORMAL;
};

TerrPSIn MeshVSMain( TerrVSIn Input )
{
    TerrPSIn Output;
    Output.vPos         = mul(mul(float4(Input.vPos, 1.0), g_mWorld), g_mViewProj);
    Output.vTexCoord.xy = Input.vTexCoord;
    return Output;    
}

float4 MeshPSMain( TerrPSIn Input ): SV_Target
{
    return float4(1, 0, 0, 1);
}

technique10 AxesFan
{
    pass RenderAxesFan
    {
       SetVertexShader( CompileShader( vs_5_0, MeshVSMain() ) );
       SetGeometryShader( NULL );
       SetPixelShader( CompileShader( ps_4_0, MeshPSMain() ) ); 
       SetBlendState( NonAlphaState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );   
    }
}
