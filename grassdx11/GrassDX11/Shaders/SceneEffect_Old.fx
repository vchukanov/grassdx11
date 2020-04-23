//--------------------------------------------------------------------------------------
// Constant buffers 
//--------------------------------------------------------------------------------------
cbuffer cEveryFrame
{
    float4x4 g_mWorld;
    float4x4 g_mViewProj;
    float4x4 g_mInvCamView;
    float4x4 g_mLightViewProj;
    float    g_fTime;
};

cbuffer cUserControlled
{
    float3 g_vTerrRGB;
    float4 g_vFogColor;// = float4(0.0, 0.3, 0.8, 1.0);
    float  g_fTerrTile;
    float  g_fHeightScale;
    float3 g_vTerrSpec = float3(202.0/255.0, 218.0/255.0, 50.0/255.0);
};

cbuffer cRarely
{                         
    float g_fTerrRadius;
    float g_fGrassRadius;
    float2 g_vPixSize = float2(0.0039, 0.0039);
};

cbuffer cImmutable
{
    float3 vLightDir;// = float3( 0.0, 0.9174, -0.2752 ); 
};

//--------------------------------------------------------------------------------------
// Texture and samplers
//--------------------------------------------------------------------------------------
Texture2D g_txShadowMap;
Texture2D g_txGrassDiffuse;
Texture2D g_txSandDiffuse;
Texture2D g_txMeshDiffuse;
Texture2D g_txSeatingT1;
Texture2D g_txSeatingT2;
Texture2D g_txSeatingT3;
Texture2D g_txHeightMap;
Texture2D g_txLightMap;
Texture2D g_txSkyBox;
Texture2D g_txGrassColor;

#include "Samplers.fx"
#include "Fog.fx"
#include "States.fx"


//--------------------------------------------------------------------------------------
// Input and output structures 
//--------------------------------------------------------------------------------------

/* Terrain input structures */

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

/* Sky box input */

struct VSSceneIn
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float2 tex : TEXTURE0;
};

struct PSSceneIn
{
    float4 pos : SV_Position;
    float3 tex : TEXTURE0;
};

struct PSCarIn
{
    float4 pos : SV_Position;
    float3 norm : NORMAL;
    float2 tex : TEXTURE0;
};

//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Sky vertex shader
//--------------------------------------------------------------------------------------
PSSceneIn VSSkymain(VSSceneIn Input)
{
    PSSceneIn Output;
    float3 vWorldPos = Input.pos * 0.25;
    Output.pos = mul(float4(vWorldPos, 1.0), g_mViewProj);
    Output.tex.xy = 1.0 - Input.tex;
    float fY = vWorldPos.y;
    Output.tex.z = FogValue(350 - fY);
    return Output;
}

float4 PSSkymain(PSSceneIn Input): SV_Target0
{
	return lerp(g_txSkyBox.Sample(g_samLinear, Input.tex.xy), g_vFogColor, Input.tex.z);
}

PSCarIn VSCarmain(VSSceneIn Input)
{
    PSCarIn Output;
    float4 vWorldPos = mul(float4(Input.pos, 1.0), g_mWorld);
    //vWorldPos.y += 0.2;
    Output.pos = mul(vWorldPos, g_mViewProj);
    Output.tex = Input.tex;
    Output.norm = Input.norm;
    return Output;
}

float4 PSCarmain0(PSCarIn Input): SV_Target0
{
	return g_txMeshDiffuse.Sample(g_samLinear, Input.tex) * max(dot(-vLightDir, Input.norm), 0.2);
}


/* Mesh shaders */
TerrPSIn MeshVSMain( TerrVSIn Input )
{
    TerrPSIn Output;
    Output.vPos         = mul(mul(float4(Input.vPos, 1.0), g_mWorld), g_mViewProj);
    Output.vTexCoord.xy = Input.vTexCoord;
    return Output;    
}

float4 MeshPSMain( TerrPSIn Input ): SV_Target
{
    float4 vTexel = g_txMeshDiffuse.Sample(g_samLinear, Input.vTexCoord.xy );
    return vTexel;
}


/* Terrain shaders */

//float3 CalcNormal(float2 a_vTexCoord)
//{
//    const float2 dU = float2(g_vPixSize.x, 0);
//    const float2 dV = float2(0, g_vPixSize.y);
//  
//    float heightC = g_txHeightMap.SampleLevel(g_samLinear, a_vTexCoord, 0).r * g_fHeightScale;
//    float heightL = g_txHeightMap.SampleLevel(g_samLinear, a_vTexCoord - dU, 0).r * g_fHeightScale;
//    float heightR = g_txHeightMap.SampleLevel(g_samLinear, a_vTexCoord + dU, 0).r * g_fHeightScale;
//    float heightT = g_txHeightMap.SampleLevel(g_samLinear, a_vTexCoord - dV, 0).r * g_fHeightScale;
//    float heightB = g_txHeightMap.SampleLevel(g_samLinear, a_vTexCoord + dV, 0).r * g_fHeightScale;
//	
//    return normalize(float3(heightR - heightL, 1.0, heightB - heightT));
//}


TerrPSIn TerrainVSMain( TerrVSIn Input )
{
    TerrPSIn Output;
    float fY			= g_txHeightMap.SampleLevel(g_samLinear, Input.vTexCoord, 0).a * g_fHeightScale;
    float4 vWorldPos	= float4(Input.vPos + float3(0.0, fY, 0.0), 1.0);
    Output.vPos         = mul(vWorldPos, g_mViewProj);
    Output.vTexCoord.xy = Input.vTexCoord;
    Output.vTexCoord.z  = FogValue(length(vWorldPos - g_mInvCamView[3].xyz));
    Output.vTexCoord.w  = length(vWorldPos - g_mInvCamView[3].xyz);
    
    return Output;
}

float GetAlphaCoef(float2 vTexCoord)
{
    float3 vAlpha =  float3(g_txSeatingT1.Sample(g_samLinear, vTexCoord).r,
                            g_txSeatingT2.Sample(g_samLinear, vTexCoord).r,
                            g_txSeatingT3.Sample(g_samLinear, vTexCoord).r);
    return clamp(length(vAlpha), 0.0, 1.0);                            
}

float4 TerrainPSMain( TerrPSIn Input ): SV_Target
{
    float2 fDot = g_txLightMap.Sample(g_samLinear, Input.vTexCoord.xy).rg;
    float3 vTexel = g_txGrassDiffuse.Sample(g_samLinear, Input.vTexCoord.xy * 0.5*g_fTerrTile).xyz;
    vTexel *=  g_vTerrRGB;
    float3 vGrassColor = g_txGrassColor.Sample(g_samLinear, Input.vTexCoord.xy).xyz;

	float fTexDist = min(Input.vTexCoord.w/140.f, 1.0);
	
	//float3(0.04, 0.1, 0.01) - previous color	
    float3 vL = lerp(vGrassColor, vTexel, fTexDist) * max(0.8, (2.0 + 5.0 * fDot.y) * 0.5);
//    float3 vL = lerp(vGrassColor, vTexel, fTexDist);

	float fLimDist = clamp((Input.vTexCoord.w - 140.0) / 20.0, 0.0, 1.0);
    return lerp(float4(fDot.x * fLimDist* g_vTerrSpec + (1.0 - fDot.x * fLimDist) * vL, 1.0), g_vFogColor, Input.vTexCoord.z);

}

/* Light Map Shaders */

TerrPSIn LightMapVSMain( TerrVSIn Input )
{
    TerrPSIn Output;
    Output.vPos          = float4(Input.vPos, 1.0);
    Output.vTexCoord.xy  = Input.vTexCoord;
    //Output.vTexCoord.x = 1.0 - Output.vTexCoord.x;
    return Output;
}

float4 LightMapPSMain( TerrPSIn Input ): SV_Target
{
    float2 vTransformedTC = (Input.vTexCoord * 2.0 - 1.0) * g_fTerrRadius;
    float4 vHeightData    = g_txHeightMap.Sample(g_samLinear, Input.vTexCoord.xy);
    vHeightData.xyz = vHeightData.xyz * 2.0 - 1.0;
    float3 vTerrPos = float3(vTransformedTC.x, vHeightData.a * g_fHeightScale, vTransformedTC.y);
    float fDot = 1.0 - dot(normalize(g_mInvCamView[3].xyz - vTerrPos), vHeightData.xyz);
    float fLightDot = -dot(vLightDir, vHeightData.xyz);
    fDot *= fDot;  
	fDot *= fDot;  
    fDot *= fDot;   
    fDot *= fDot;  
    return float4(fDot, fLightDot, 0.0, 0.0);
}

technique10 Render
{    
    pass RenderTerrainPass
    {
        SetVertexShader( CompileShader( vs_4_0, TerrainVSMain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, TerrainPSMain() ) ); 
        SetBlendState( NonAlphaState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
        SetRasterizerState( EnableMSAACulling );
    }  

    pass RenderMeshPass
    {
        SetVertexShader( CompileShader( vs_4_0, MeshVSMain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, MeshPSMain() ) ); 
        SetBlendState( NonAlphaState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );   
        SetRasterizerState( EnableMSAA );
    }

    pass RenderLightMapPass
    {
        SetVertexShader( CompileShader( vs_4_0, LightMapVSMain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, LightMapPSMain() ) ); 
        SetBlendState( NonAlphaState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );   
        SetRasterizerState( EnableMSAA );
    }
}

technique10 RenderSkyBox
{
	pass RenderPass
    {
        SetVertexShader( CompileShader( vs_4_0, VSSkymain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSSkymain() ) ); 
        SetBlendState( NonAlphaState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );   
    }	
}

technique10 RenderCar
{
	pass RenderPass
    {
        SetVertexShader( CompileShader( vs_4_0, VSCarmain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSCarmain0() ) ); 
        SetBlendState( NonAlphaState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );   
    }	
}
