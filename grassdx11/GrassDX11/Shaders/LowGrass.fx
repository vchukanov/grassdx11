#ifndef _LOW_GRASS_H_
#define _LOW_GRASS_H_

//cos, sin, -sin, cos
float4 g_vRotate120 = float4(-0.5, 0.8660, -0.8660, -0.5);
//cos sin -sin cos
float4 g_vRotate90  = float4(0.0, 1.0, -1.0, 0.0);

Texture2D g_txLowGrassDiffuse;
float4    g_vLowGrassDiffuse;

struct LowGrassPSIn
{
    float4 vPos                       : SV_Position;
    float4 vShadowPos                 : TEXCOORD0;
    float2 vTexCoord                  : TEXCOORD1;    
};

LowGrassPSIn LowGrassVertex( float3 a_vPos, float2 a_vTexCoord)
{
    LowGrassPSIn Vertex;
    Vertex.vShadowPos = mul( float4(a_vPos, 1.0), g_mLightViewProj);    
    Vertex.vPos       = mul( float4(a_vPos, 1.0), g_mViewProj);    
    Vertex.vTexCoord  = a_vTexCoord;
    return Vertex;
}

void CreateLowGrass( float3 vStart, float3 vX, float3 vY, inout TriangleStream < LowGrassPSIn > TriStream )
{    
    TriStream.Append(LowGrassVertex(vStart + vX + vY, float2( 1.0,  0.0)));
    TriStream.Append(LowGrassVertex(vStart - vX + vY, float2( 0.0,  0.0)));
    TriStream.Append(LowGrassVertex(vStart + vX, float2( 1.0,  1.0)));    
    TriStream.Append(LowGrassVertex(vStart - vX, float2( 0.0,  1.0)));
    TriStream.RestartStrip(); 
}

float2 RotateVec2(float2 vec, float4 rot)
{
    return float2(dot(vec, rot.xy), dot(vec, rot.zw));// + rot.xy * off;
}

/* Low Grass Geometry Shader */
[maxvertexcount(12)]
void GSLowGrassMain( point GSIn In[1], inout TriangleStream< LowGrassPSIn > TriStream )
{
    if (In[0].vPackedData.x <= 0.55)
		return;
    float4 vViewPos = mul(float4(In[0].vPos0, 1.0), g_mView);
    if (length(vViewPos) > 25)
        return;

    float3 vDelta = In[0].vPos3 - In[0].vPos0;
    float3 vX = float3(1.2, 0.0, 0.0);
    float3 vY = float3(vDelta.x, 1.5, vDelta.z);
    float3 vZ = normalize(cross(vX, vY));
    CreateLowGrass(In[0].vPos0, vX, vY, TriStream);

    //vX.xz = RotateVec2(vX.xz, g_vRotate90);
    vX = float3(0.0, 0.0, 0.8);
    vZ = normalize(cross(vX, vY));
    CreateLowGrass(In[0].vPos0, vX, vY, TriStream);
    /*vX.xz = RotateVec2(vX.xz, g_vRotate120);
    vZ = normalize(cross(vX, vY));
    CreateLowGrass(In[0].vPos0, vX, vY, TriStream);

    vX.xz = RotateVec2(vX.xz, g_vRotate120);
    vZ = normalize(cross(vX, vY));
    CreateLowGrass(In[0].vPos0, vX, vY, TriStream);*/
}

/* Low Grass Pixel Shader */
float4 PSLowGrassMain( LowGrassPSIn In ): SV_Target
{
    //return float4(1, 0, 0, 1);
    /*float fNoise = g_txNoise.Sample(g_samLinear, In.vTexCoord).r;
    if (In.fDissolve < 1.0)
        clip(fNoise - In.fDissolve);*/

    float4 vTexel = g_txLowGrassDiffuse.Sample(g_samLinear, In.vTexCoord) * g_vLowGrassDiffuse;    
    float fNdotL  = max(-vLightDir.y, 0.1)  + g_fGrassAmbient;
    return float4(fNdotL * vTexel.xyz, vTexel.a);
}

float4 PSLowGrassShadowMain( LowGrassPSIn In, out float fDepth : SV_Depth ): SV_Target
{
    float fAlpha = g_txLowGrassDiffuse.Sample(g_samAniso, In.vTexCoord).r;
    clip(fAlpha - 0.001);
    fDepth = In.vShadowPos.z / In.vShadowPos.w * 0.5 + 0.5;
    return float4(0.0, 0.0, 0.0, 0.0);
}

technique10 RenderLowGrass
{
    pass RenderLowGrass
    {
        SetVertexShader( CompileShader( vs_4_0, InstVSMain() ) );//extern vertex shader
        SetGeometryShader( CompileShader( gs_4_0, GSLowGrassMain() ) );
        SetPixelShader( CompileShader( ps_4_0, PSLowGrassMain() ) );

        SetBlendState( AlphaBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        //SetRasterizerState( EnableMSAA );
    }

    pass PhysLowGrass
    {
        SetVertexShader( CompileShader( vs_4_0, PhysVSMain() ) );//extern vertex shader
        SetGeometryShader( CompileShader( gs_4_0, GSLowGrassMain() ) );
        SetPixelShader( CompileShader( ps_4_0, PSLowGrassMain() ) );

        SetBlendState( AlphaBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        //SetRasterizerState( EnableMSAA );
    }

    pass AnimLowGrass
    {
        SetVertexShader( CompileShader( vs_4_0, AnimVSMain() ) );//extern vertex shader
        SetGeometryShader( CompileShader( gs_4_0, GSLowGrassMain() ) );
        SetPixelShader( CompileShader( ps_4_0, PSLowGrassMain() ) );

        SetBlendState( AlphaBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        //SetRasterizerState( EnableMSAA );
    }

    pass ShadowLowGrass
    {
        SetVertexShader( CompileShader( vs_4_0, InstVSMain() ) );//extern vertex shader
        SetGeometryShader( CompileShader( gs_4_0, GSLowGrassMain() ) );
        SetPixelShader( CompileShader( ps_4_0, PSLowGrassShadowMain() ) );
    }

    pass ShadowLowPhysGrass
    {
        SetVertexShader( CompileShader( vs_4_0, PhysVSMain() ) );//extern vertex shader
        SetGeometryShader( CompileShader( gs_4_0, GSLowGrassMain() ) );
        SetPixelShader( CompileShader( ps_4_0, PSLowGrassMain() ) );

        SetBlendState( AlphaBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        //SetRasterizerState( EnableMSAA );
    }

    pass ShadowLowAnimGrass
    {
        SetVertexShader( CompileShader( vs_4_0, AnimVSMain() ) );//extern vertex shader
        SetGeometryShader( CompileShader( gs_4_0, GSLowGrassMain() ) );
        SetPixelShader( CompileShader( ps_4_0, PSLowGrassMain() ) );

        SetBlendState( AlphaBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        //SetRasterizerState( EnableMSAA );
    }
}

#endif