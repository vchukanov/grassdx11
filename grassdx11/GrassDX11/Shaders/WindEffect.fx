//--------------------------------------------------------------------------------------
// Constant buffers 
//--------------------------------------------------------------------------------------
cbuffer cEveryFrame
{
    float g_fTime;
};

cbuffer cRarely
{    
    float2 g_vPixSize;
    float4 g_vRotate45  = float4(0.7071, 0.7071, -0.7071, 0.7071);
    float4 g_vRotate315 = float4(0.7071, -0.7071, 0.7071, 0.7071);
};

cbuffer cUserControlled
{
    float g_fWindSpeed;
    float g_fWindBias;
    float g_fWindScale;
};

//--------------------------------------------------------------------------------------
// Texture and samplers
//--------------------------------------------------------------------------------------
Texture2D g_txHeightTex;
Texture2D g_txHeightMap;
Texture2D g_txWindMap;

SamplerState g_samLinear
{
    Filter   = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

//--------------------------------------------------------------------------------------
// State structures
//--------------------------------------------------------------------------------------
DepthStencilState EnableDepthTestWrite
{
    DepthEnable    = TRUE;
    DepthWriteMask = ALL;
};

RasterizerState RasterState
{
    CullMode          = NONE;
    FillMode          = SOLID;
    MultisampleEnable = TRUE;
};
//--------------------------------------------------------------------------------------
// Input and output structures 
//--------------------------------------------------------------------------------------
struct WindVSIn
{
    float3 vPos      : POSITION;
    float2 vTexCoord : TEXCOORD0;
};

struct WindMapPSIn
{
	float4 vPos      : SV_Position;
    float2 vTexCoord : TEXCOORD0;
};

struct WindTexPSIn
{
	float4 vPos       : SV_Position;
    float2 vTexCoord0 : TEXCOORD0;
    float2 vTexCoord1 : TEXCOORD1;
    float2 vTexCoord2 : TEXCOORD2;
};

//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------

float HeightTexLookUp( float2 vTexCoord )
{
	float2 dU = float2(g_vPixSize.x, 0.0);
	float2 dV = float2(0.0, g_vPixSize.y);
	float  fTexel = 0.0;//float3(0.0, 0.0, 0.0);
	int i, j;
	[unroll]for (i = -2; i <= 2; i++)
	  [unroll]for (j = -2; j <= 2; j++)
	  {
		  fTexel += g_txHeightTex.Sample(g_samLinear, vTexCoord + i * dU + j * dV).r;	  
	  }
	
	return fTexel * 0.04; // * 1/25
}

/* Height map from height tex*/
WindMapPSIn HeightMapVS( WindVSIn In )
{
	WindMapPSIn Out;
	Out.vPos = float4(In.vPos, 1.0);
	Out.vTexCoord = In.vTexCoord;

    return Out;
}

float4 HeightMapPS( WindMapPSIn In ): SV_Target
{
	return g_txHeightTex.Sample(g_samLinear, In.vTexCoord).xxxx;//HeightTexLookUp(In.vTexCoord).xxxx;
}

/* Normal map (wind map) from height map*/
WindMapPSIn WindMapVS( WindVSIn In )
{
	WindMapPSIn Out;
	Out.vPos = float4(In.vPos, 1.0);
	Out.vTexCoord = In.vTexCoord;

    return Out;
}

float4 WindMapPS( WindMapPSIn In ): SV_Target
{
    const float2 dU = float2(g_vPixSize.x, 0);
    const float2 dV = float2(0, g_vPixSize.y);
  
    float height  = g_txHeightMap.Sample(g_samLinear, In.vTexCoord).r;
    float heightL = g_txHeightMap.Sample(g_samLinear, In.vTexCoord - dU).r;
    float heightR = g_txHeightMap.Sample(g_samLinear, In.vTexCoord + dU).r;
    float heightT = g_txHeightMap.Sample(g_samLinear, In.vTexCoord - dV).r;
    float heightB = g_txHeightMap.Sample(g_samLinear, In.vTexCoord + dV).r;
    float heightC = height + 1.;
	
	float3 vRes = float3(1.0, 0.0, 0.0) * g_fWindScale * heightC + float3(16.0 * (heightR - heightL), 0.0, 16.0 * (heightB - heightT))*g_fWindBias;	
	
    return float4(vRes, height);//float4(1.25 + 16.0 * (heightR - heightL), 0.0, 0.5 + 16.0 * (heightB - heightT), 1.0);    
}

/* Wind Texture from wind map */

float2 Transform(float2 vec, float4 rot, float off)
{
    return float2(dot(vec, rot.xy), dot(vec, rot.zw)) + rot.xy * off;
}

WindTexPSIn WindTexVS( WindVSIn In )
{
	WindTexPSIn Out;
	Out.vPos = float4(In.vPos, 1.0);
	Out.vTexCoord0 = In.vTexCoord + float2(-g_fTime * 0.02 * g_fWindSpeed, 0.0);
    Out.vTexCoord1 = 0.707 * Transform(In.vTexCoord, g_vRotate45 , g_fTime * 0.04 * g_fWindSpeed);
    Out.vTexCoord2 = 0.707 * Transform(In.vTexCoord, g_vRotate315, g_fTime * 0.08 * g_fWindSpeed);

    return Out;
}

float4 WindTexPS( WindTexPSIn In ): SV_Target
{
/* Weights: 1, 0.5, 0.25 after normalizing (* 1/(1+0.5+0.25)) */
    return /*0.5714 **/ g_txWindMap.Sample(g_samLinear, In.vTexCoord0);/* + 
           0.2857 * g_txWindMap.Sample(g_samLinear, In.vTexCoord1) + 
           0.1428 * g_txWindMap.Sample(g_samLinear, In.vTexCoord2);      */
}

//--------------------------------------------------------------------------------------
// Techniques and passes
//--------------------------------------------------------------------------------------
technique10 RenderWind
{
	pass HeighttexToHeightmap
    {
        SetVertexShader( CompileShader( vs_4_0, HeightMapVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, HeightMapPS() ) );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
        SetRasterizerState( RasterState );
    }

    pass HeightmapToWindmap
    {
        SetVertexShader( CompileShader( vs_4_0, WindMapVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, WindMapPS() ) );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
        SetRasterizerState( RasterState );
    }
    
    pass WindmapToWindtex
    {
        SetVertexShader( CompileShader( vs_4_0, WindTexVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, WindTexPS() ) );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
        SetRasterizerState( RasterState );
    }
}