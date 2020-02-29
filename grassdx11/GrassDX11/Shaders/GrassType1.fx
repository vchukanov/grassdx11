//--------------------------------------------------------------------------------------
// Constant buffers 
//--------------------------------------------------------------------------------------
cbuffer cRarely
{
    float g_fMostDetailedDist;
    float g_fLastDetailedDist;
    float g_fGrassRadius;    
    float g_fTerrRadius;
    float g_fGrassLod0Dist;
    float g_fMaxQuality;// = 0.7;
    float3 vLightDir;
    float3 g_vDiffuse = float3(0.6, 1.0, 0.5);
    float4 g_vFogColor;// = float4(0.0, 0.3, 0.8, 1.0);
    float3 g_vTerrRGB;
}

cbuffer cEveryFrame
{
    float4x4 g_mWorld;
    float4x4 g_mViewProj;
    float4x4 g_mLightViewProj;
    float4x4 g_mInvCamView;
    float4x4 g_mView;
    float    g_fTime;
    /* pos and radius of B-spheres of the meshes */
    float4 g_vMeshSpheres[20];
};

cbuffer cUserControlled
{
    float g_fGrassAmbient;
    float g_fGrassDiffuse;
    float g_fGrassLodBias;
    float g_fGrassSubScatterGamma;
    float g_fWindStrength;
    float g_fWindTexTile;
    float g_fMass;
    float g_fHeightScale;
    float g_fQuality;// in range 0..1
    float g_fHardness;
    float3 g_vTerrSpec = float3(202.0/255.0, 218.0/255.0, 50.0/255.0);
};

cbuffer cImmutable
{
    float2 g_vPixSize = float2(0.0039, 0.0039);
    /* grass subscatter parameter for lighting */
    float3 g_vSubScatter = float3(1.0, 1.0, 0.0);
    /* alpha borders for lods */
    float g_fLod0Offset = 0.0;
    float g_fLod2Offset = 0.67;
};

struct GrassSubType
{ 
    float3 vHardnessSegment; //float3
    float3 vMassSegment;  //float3
    float2 vSizes; //x = segment width, y = segment height //float2
    float4 vColor; //float4
    uint uTexIndex; //uint
    
    float3 pad0;
};

cbuffer cGrassSubTypes
{                  
    GrassSubType SubTypes[10];                      
};

//--------------------------------------------------------------------------------------
// Texture and samplers
//--------------------------------------------------------------------------------------
Texture2DArray   g_txGrassDiffuseArray;
Texture2DArray   g_txWindTex;
Texture2D        g_txAxesFanFlow;
Texture2D        g_txIndexMap;
Texture2D        g_txSeatingMap;
Texture2D        g_txNoise;
Texture2D        g_txHeightMap;
Texture2D        g_txShadowMap;
Texture2D        g_txTerrainLightMap;
Texture2D        g_txGrassColor;

#include "Samplers.fx"
#include "States.fx"


//--------------------------------------------------------------------------------------
// Input and output structures 
//--------------------------------------------------------------------------------------

/* Grass input structures */
#include "VSIn.fx"

/* Grass input structures */
struct GSIn
{
    float3 vPos0                  : TEXCOORD0;
    float3 vOffs0                 : OFFSET0;
    float3 vPos1                  : TEXCOORD1;
    float3 vOffs1                 : OFFSET1;
    float3 vPos2                  : TEXCOORD2;
    float3 vOffs2                 : OFFSET2;
    float3 vPos3                  : TEXCOORD3;
    float3 vOffs3                 : OFFSET3;
    float4 vPackedData           : AlphaSegwidthTexindXtexcoord;
    //float  fTransparency          : TRANSPARENCY;
    float  fDissolve              : DISSOLVE;
    /* from vs to gs */
    uint   uStartSegment          : STARTSEGMENT;
    uint   uNumVertices           : NUMVERTS;
    uint   uTypeIndex             : TYPEINDEX; 
    float3 vColor                 : GCOLOR;
    float  fNoise              : NOISECOLOR;   
    float3 vBladeColor            : BLADECOLOR;
};

struct PSIn
{
    float4 vPos                        : SV_Position;
    float4 vTexCoord                   : TEXCOORD1;
    float  fLightParam                 : NORMALY;
    float2 vTerrSpec                   : TERRSPEC;
    nointerpolation float3 vColor      : GCOLOR;
    nointerpolation float2 vWorldTC    : TEXCOORD2;
    nointerpolation float fDissolve    : DISSOLVE;
    nointerpolation uint  uIndex       : TYPEINDEX;
    nointerpolation float  fNoise      : NOISECOLOR;
    nointerpolation float3 vBladeColor : BLADECOLOR;
};

/* Grass shaders */
#include "DisableSeating.fx"
#include "DisableSubTypes.fx"
#include "Lod.fx"

float3x3 MatrixIdentity()
{
   return (float3x3( 1.0, 0.0, 0.0,
                     0.0, 1.0, 0.0,
                     0.0, 0.0, 1.0
                    ));
}
float3x3 MakeRotateMtx( float3 a_vAxe )
{    
    float fAngle =  length(a_vAxe);
    if (abs(fAngle) < 0.0001)
       return (float3x3 (1.0, 0.0, 0.0,
                    0.0, 1.0, 0.0,
                    0.0, 0.0, 1.0
                    ));
    float3 vAxe = normalize(a_vAxe);
    float fCos = cos(fAngle);
    float l_fCos = 1 - fCos;
    float fSin = sin(fAngle);
    
    float x2 = vAxe.x * vAxe.x;
    float y2 = vAxe.y * vAxe.y;
    float z2 = vAxe.z * vAxe.z;
    
    float xy = vAxe.x * vAxe.y;
    float xz = vAxe.x * vAxe.z;    
    float yz = vAxe.y * vAxe.z;
    

    return (float3x3( fCos + l_fCos * x2       , l_fCos * xy - fSin * vAxe.z, l_fCos * xz + fSin * vAxe.y,
                    l_fCos * xy + fSin * vAxe.z, fCos + l_fCos * y2         , l_fCos * yz - fSin * vAxe.x,
                    l_fCos * xz - fSin * vAxe.y, l_fCos * yz + fSin * vAxe.x, fCos + l_fCos * z2
                    ));

}

GSIn CalcWindAnimation( float3 a_vBladePos, float3 a_vRotAxe, float3 a_vYRotAxe )
{
    GSIn Output;
    float2 vUV              = (a_vBladePos.xz / g_fTerrRadius) * 0.5 + 0.5;
    uint uIndex             = GetTypeIndex(vUV);
    Output.uTypeIndex       = uIndex;
    Output.vPackedData.x    = GetSeatingInfo(vUV); 
    float3 vHardnessSeg     = SubTypes[uIndex].vHardnessSegment;
    float3 vMassSegment     = SubTypes[uIndex].vMassSegment;
    float4 vColorAndH       = g_txGrassColor.SampleLevel(g_samLinear, vUV, 0);
    float  fSegLength       = 0.75;
    Output.vColor           = vColorAndH.xyz;
    float3 vAxe             = float3(0.0, fSegLength * 0.5, 0.0);

    float3 vGrav            = float3(0.0, -9.8, 0.0);
    float3 vF, vLocalF, vAbsWind;
    float fH, fFL, fLenG, fSinBetha, fBetha, fPhi;
   
    float fT_ = 1000001.0 * vUV.x * vUV.y;
    int uT_ = int(fT_) % 1011; 
    Output.fNoise           = float(uT_)/1011.0;
 
    float3x3 mMYrot = MakeRotateMtx(a_vYRotAxe); 
    
    float3x3 mMStart = mul(mMYrot, MakeRotateMtx(a_vRotAxe));

    
    float fDist = length(a_vBladePos - g_mInvCamView[3].xyz);
    float fMass = g_fMass - 0.2*clamp((fDist - 7.0)/25.0, 0.0, 1.0); 
    
    /* Computing moments */
    float3x3 mM_T[4], mM_R[4];
    mM_T[0] = mMStart;

   float3 g = float3(0.0f, -9.8f, 0.0f);
   float3 halfAxis = vAxe;
    float3 axis = float3(0.0, fSegLength, 0.0);
    float3 sum = float3(0.0f, 0.0f, 0.0f), localSum;
    float3 w, w_;

   [unroll]for (int j = 1; j < 4; j++)
   {      
        mM_T[j] = mM_T[j - 1];
      [unroll]for (int k = 1; k < 4; k++)
      {
//         sum = g * vMassSegment[j-1] * mM_T[j][1].y;
         sum = g * vMassSegment[j-1];
            localSum = mul(sum, mM_T[j]);
         float3 G;
            G = cross(halfAxis, localSum);
            mM_R[j] = MakeRotateMtx(G / vHardnessSeg[j - 1]);
            mM_T[j] = mul(mM_T[j - 1], mM_R[j]);
      }

        w_ = CalcWind(a_vBladePos, j - 1);
//      sum = g * vMassSegment[j-1] * mM_T[j][1].y;
      sum = g * vMassSegment[j-1];
        localSum = mul(sum, mM_T[j]);
      float3 G;
        G = cross(halfAxis, localSum);
        w = mul(w_, mM_T[j]);
      G += w;
        mM_R[j] = MakeRotateMtx(G / vHardnessSeg[j - 1]);
        mM_T[j] = mul(mM_T[j-1], mM_R[j]);
   }


    Output.vOffs1 = Output.vOffs0 = SubTypes[uIndex].vSizes.x * mM_T[1][0];
    Output.vOffs2 = SubTypes[uIndex].vSizes.x * mM_T[2][0];
    Output.vOffs3 = SubTypes[uIndex].vSizes.x * mM_T[3][0];
    /* world coord of grass blade start pt */
    Output.vPos0 = a_vBladePos;    
  
    float fEnd = g_fGrassRadius - 50.0;

    float fScale = 0.5*(2.0*fEnd - fDist)/fEnd;
    if (fScale < 0.5) 
    {
      fScale = 0.5*(g_fGrassRadius - fDist)/50.0;
    } 
 //   fSegLength *= (0.75 + 0.5 * vColorAndH.a);
    a_vBladePos += transpose(mM_T[1])[1] * fScale*fSegLength;
    Output.vPos1 = a_vBladePos;
    
    a_vBladePos += transpose(mM_T[2])[1] * fScale*fSegLength;    
    Output.vPos2 = a_vBladePos;    
    
    a_vBladePos += transpose(mM_T[3])[1] * fScale*fSegLength;
    Output.vPos3 = a_vBladePos;
//    if (Output.vPos3.y < Output.vPos0.y) Output.vPos3.y += 1.5; 
    
    float fY = g_txHeightMap.SampleLevel(g_samLinear, (Output.vPos3.xz / g_fTerrRadius) * 0.5 + 0.5, 0).a * g_fHeightScale;    
    if (Output.vPos3.y <= fY)
      Output.vPos3.y = fY + 0.01;

    return Output;
}

GSIn InstVSMain( InstVSIn Input )
{
    float4 vPos = mul(float4(Input.vPos, 1.0), Input.mTransform);
    float fY = g_txHeightMap.SampleLevel(g_samLinear, (vPos.xz / g_fTerrRadius) * 0.5 + 0.5, 0).a * g_fHeightScale;    
    vPos.y = fY;
     
    GSIn Output = CalcWindAnimation(vPos.xyz, Input.vRotAxe * 0.01745, Input.vYRotAxe * 0.01745);
    Output.vBladeColor = Input.vColor;
    /******************************/    
    Output.vPackedData = CalcTransparency(Input.fTransparency, vPos, Output.fDissolve, Output.uNumVertices);
   // Output.vPackedData.x = Output.fNoise - 0.9;
    ScreenClip(float4(Output.vPos0, 1.0), float4(Output.vPos3, 1.0), Output.vPackedData.x);
    return Output;
}


GSIn AnimVSMain ( InstVSIn Input )
{                               
   
    float fY = g_txHeightMap.SampleLevel(g_samLinear, (Input.vPos.xz / g_fTerrRadius) * 0.5 + 0.5, 0).a * g_fHeightScale;    
    Input.vPos.y = fY;
    
    GSIn Output = CalcWindAnimation(Input.vPos.xyz, Input.vRotAxe * 0.01745, Input.vYRotAxe * 0.01745);
    Output.vBladeColor = Input.vColor;
 
    Output.vPackedData = CalcTransparency(Input.fTransparency, float4(Input.vPos, 1.0), Output.fDissolve, Output.uNumVertices);
   // Output.vPackedData.x = Output.fNoise - 0.9;
    ScreenClip(float4(Output.vPos0, 1.0), float4(Output.vPos3, 1.0), Output.vPackedData.x);
    return Output;
   
}

GSIn PhysVSMain ( PhysVSIn Input )
{
    //float4 vPos = mul(float4(Input.vPos, 1.0), g_mWorld);
    float4 vPos = float4(Input.vPos, 1.0); //mul(float4(Input.vPos, 1.0), g_mWorld);
    float fY = g_txHeightMap.SampleLevel(g_samLinear, (vPos.xz / g_fTerrRadius) * 0.5 + 0.5, 0).a * g_fHeightScale;    
    vPos.y = fY;
       
    
    GSIn Output;
    Output.vPackedData = CalcTransparency(Input.fTransparency, vPos, Output.fDissolve, Output.uNumVertices);
   // Output.vPackedData.x = Output.fNoise - 0.5;
     
    float2 vUV              = (vPos.xz / g_fTerrRadius) * 0.5 + 0.5;
    uint uIndex             = GetTypeIndex(vUV);
    Output.uTypeIndex       = uIndex;
    float fSeating          = GetSeatingInfo(vUV);

    float4 vColorAndH       = g_txGrassColor.SampleLevel(g_samLinear, vUV, 0);
    Output.vColor = vColorAndH.xyz;
    Output.vBladeColor = Input.vColor;
//    float  fSegLength       = SubTypes[uIndex].vSizes.y * (0.75 + 0.5 * vColorAndH.a);
    float  fSegLength       = SubTypes[uIndex].vSizes.y;
    float  fSegWidth        = SubTypes[uIndex].vSizes.x;
//    Output.fNoise           = g_txNoise.SampleLevel(g_samPoint, 10000.0*vUV, 0).r;

   float fT = 1000001.0 * vUV.x * vUV.y;
   int uT = int(fT) % 1011; 
    Output.fNoise = float(uT)/1011.0;
    
    /* SubTypes[uIndex].vSizes.x */
    Output.vOffs1 = Output.vOffs0 = fSegWidth * (Input.mR0)[0].xyz;
    Output.vOffs2 = fSegWidth * (Input.mR1)[0].xyz;
    Output.vOffs3 = fSegWidth * (Input.mR2)[0].xyz;
   
//   float fSegLength = Input.fSegmentHeight*(0.75 + 0.5 * vColorAndH.a);
    Output.vPos0 = vPos.xyz; vPos += transpose(Input.mR0)[1] * fSegLength;
    Output.vPos1 = vPos.xyz; vPos += transpose(Input.mR1)[1] * fSegLength;
    Output.vPos2 = vPos.xyz; vPos += transpose(Input.mR2)[1] * fSegLength;
  //  Output.vPos0 = vPos.xyz; vPos += transpose(Input.mR0)[1] * Input.fSegmentHeight;
  //  Output.vPos1 = vPos.xyz; vPos += transpose(Input.mR1)[1] * Input.fSegmentHeight;
  //  Output.vPos2 = vPos.xyz; vPos += transpose(Input.mR2)[1] * Input.fSegmentHeight;
    Output.vPos3 = vPos.xyz;
    
    fY = g_txHeightMap.SampleLevel(g_samLinear, (Output.vPos3.xz / g_fTerrRadius) * 0.5 + 0.5, 0).a * g_fHeightScale;    
    if (Output.vPos3.y <= fY)
      Output.vPos3.y = fY + 0.01;
   
    
    /******************************/        
    ScreenClip(float4(Output.vPos0, 1.0), float4(Output.vPos3, 1.0), Output.vPackedData.x);   
    return Output;
}

#include "LowGrass.fx"
#include "GSFunc.fx"
#include "GST1.fx"


[maxvertexcount(14)]
void GSGrassMain( point GSIn Input[1], inout TriangleStream< PSIn > TriStream )
{
    if (Input[0].vPackedData.x <= 0.0)
      return;
  
    if (Input[0].uNumVertices == 7)
    {
      Make7Pts(Input[0], TriStream);
      return;    
    }    
   
    if (Input[0].uNumVertices == 4)
    {
      Make4Pts(Input[0], TriStream);
      return;    
    }
   
    if (Input[0].uNumVertices == 3)
    {
      Make3Pts(Input[0], TriStream);
      return;    
    }
  
    if (Input[0].uNumVertices == 2)
    {
      Make2Pts(Input[0], TriStream);
      return;    
    }
    
}

/* Pixel shaders */
/* Input:
 * Input.vTerrSpec.x - terrain specular (dot(VertexToCamera, Normal))
 * Input.vTerrSpec.y - terrain lighting (dot(Lightdir, Normal))
 * Input.vTexCoord.w - distance from camera to vertex
 * Input.vTexCoord.z - fog
 * Input.fLightParam - Ny
 *
 */
float4 InstPSMain( PSIn Input ) : SV_Target
{
    float fNoise;
    if (Input.fDissolve < 0.0) clip(-1);
	/*
	if ((Input.vTexCoord.w < 40.0) && (Input.fDissolve < 1.0))
	{
			fNoise = g_txNoise.Sample(g_samLinear, Input.vTexCoord.xy).r;
			clip(Input.fDissolve - fNoise);
	}
*/
    uint uTexIndex   = Input.uIndex;//0;//SubTypes[Input.uIndex].uTexIndex;
    float fAlpha;
    float fL = max(0.17, (1.0 + 5.0 * Input.vTerrSpec.y)*0.6);
    fAlpha = Input.fDissolve;
    float4 vDiffuseTexel = g_txGrassDiffuseArray.Sample(g_samLinear, float3(Input.vTexCoord.xy, uTexIndex));
    if (Input.vTexCoord.w < 70.0)
    {
       float fTest = max((Input.vTexCoord.w - 40.0)/30.0, 0.0);
       fAlpha = vDiffuseTexel.a;//g_txGrassDiffuseArray.Sample(g_samLinear, float3(Input.vTexCoord.xy, uTexIndex)).a;
       fAlpha = (fTest + (1.0 - fTest)*fAlpha) * Input.fDissolve;
//	    if (Input.vTexCoord.x < 0.5)fL *= 0.862;
       if (Input.vTexCoord.x < 0.5)fL *= 0.7862;
    }	
    float3 vColor1 = float3(0.5, 0.5, 1.0);
    float3 vColor2 = float3(1.5, 1.5, 1.0);
    float fNoiseScale = lerp(vColor1, vColor2, Input.fNoise);
    float3 vA = Input.vBladeColor;
    float3 vD = vDiffuseTexel.xyz;//g_vLowGrassDiffuse.xyz;
    float3 vT = Input.vColor * max(0.8, (2.0 + 5.0f* Input.vTerrSpec.y)*0.5);
    float fDot = Input.vTerrSpec.x;
    float fT = 1.0 - abs(Input.fLightParam);
    float fT2 = fT*fT;
    fT2 = fT2*fT2;
    float fY = Input.vTexCoord.y;
    float fY2 = fY*fY;
    fY2 = fY2*fY2;
    fY2 = fY2*fY;
	  float3 vC = vT + fY*(vA + fY2*fT2*vD)*fL*fNoiseScale;
	  vC = fY2*fDot * g_vTerrSpec + (1.0 - fY2*fDot) * vC;
	
  	float fDd = clamp((Input.vTexCoord.w - 90.0)*0.02, 0.0, 1.0);
    float fDd2 = fDd*fDd;
    fDd = 1.0 - 3.0*fDd2 +2.0*fDd2*fDd;
   //vC = fDd*vC + (1.0-fDd)*vT;
    return lerp(float4(vC, fAlpha) , g_vFogColor, Input.vTexCoord.z);
}
 

technique10 RenderGrass
{
    pass RenderLod0
    {        
        SetVertexShader( CompileShader( vs_4_0, InstVSMain() ) );
        SetGeometryShader( CompileShader( gs_4_0, GSGrassMain() ) );
        SetPixelShader( CompileShader( ps_4_0, InstPSMain() ) );
        
        SetBlendState( AlphaBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
        //SetRasterizerState( EnableMSAA );
    }  

    pass RenderPhysPass
    {
        SetVertexShader( CompileShader( vs_4_0, PhysVSMain() ) );
        SetGeometryShader( CompileShader( gs_4_0, GSGrassMain() ) );
        SetPixelShader( CompileShader( ps_4_0, InstPSMain() ) );
        
        SetBlendState( AlphaBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
        //SetRasterizerState( EnableMSAA );
    }

    pass RenderAnimPass
    {
        SetVertexShader( CompileShader( vs_4_0, AnimVSMain() ) );
        SetGeometryShader( CompileShader( gs_4_0, GSGrassMain() ) );
        SetPixelShader( CompileShader( ps_4_0, InstPSMain() ) );
        
        SetBlendState( AlphaBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
    }
    
}
