//--------------------------------------------------------------------------------------
// Constant buffers 
//--------------------------------------------------------------------------------------
cbuffer cRarely
{
    float g_fMostDetailedDist;
    float g_fLastDetailedDist;
    float g_fGrassRadius;        
    float g_fGrassLod0Dist;
    float g_fMaxQuality;// = 0.7;
    float3 vLightDir;
}

cbuffer cEveryFrame
{
    float4x4 g_mWorld;
    float4x4 g_mLightViewProj;
    float4x4 g_mViewProj;
    float4x4 g_mInvCamView;
    float4x4 g_mView;
    float4x4 g_mProj;
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
    float3 vHardnessSegment;
    float3 vMassSegment; 
    float2 vSizes; //x = segment width, y = segment height
    float4 vColor;
    uint   uTexIndex;
};

cbuffer cGrassSubTypes
{                  
    GrassSubType SubTypes[10];                        
}

//--------------------------------------------------------------------------------------
// Texture and samplers
//--------------------------------------------------------------------------------------
Texture2DArray g_txGrassDiffuseArray;
Texture2DArray g_txWindTex;
Texture2D      g_txSeatingMap;
Texture2D      g_txIndexMap;
Texture2D      g_txNoise;
Texture2D      g_txHeightMap;
Texture2D      g_txShadowMap;

#include "Shaders/Samplers.fx"
#include "Shaders/States.fx"


//--------------------------------------------------------------------------------------
// Input and output structures 
//--------------------------------------------------------------------------------------

/* Grass input structures */
#include "Shaders/VSIn.fx"

/* Grass input structures */
struct GSIn
{
    float3 vPos0                  : TEXCOORD0;
    float3 vPos1                  : TEXCOORD1;
    float3 vPos2                  : TEXCOORD2;
    float3 vPos3                  : TEXCOORD3;
    float  fTransparency          : TRANSPARENCY;
    float  fDissolve              : DISSOLVE;
    /* from vs to gs */
    int    iNumVertices           : NUMVERTS;
    uint   uTypeIndex             : TYPEINDEX;    
};

struct PSIn
{
    float4 vPos                       : SV_Position;
    float4 vShadowPos                 : TEXCOORD0;
    float2 vTexCoord                  : TEXCOORD1;
    float  fLightParam                : NORMALY;
    float2 vTerrSpec                  : TERRSPEC;
    nointerpolation float fDissolve   : DISSOLVE;
    nointerpolation uint  uIndex      : TYPEINDEX;    
};

/* Grass shaders */
uint GetTypeIndex( float2 vUV )
{
    return uint(g_txIndexMap.SampleLevel(g_samPoint, vUV, 0).r * 9.0);        
}

float GetSeatingInfo( float2 vUV )
{              
    return g_txSeatingMap.SampleLevel(g_samLinear, vUV, 0).r;
}

float4 CalcTerrNormal(float2 a_vTexCoord)
{
    return float4(g_txHeightMap.SampleLevel(g_samLinear, a_vTexCoord, 0).rgb * 2.0 - 1.0, 0.0);
}

inline float LodAlphaOffset(float4 a_vWorldPt, out float a_fBladeDist)
{
	float3 vViewPos     = mul(a_vWorldPt, g_mView).xyz;
    a_fBladeDist        = length(vViewPos);
    float fLerpCoef		= sqrt((a_fBladeDist - g_fMostDetailedDist) / (g_fGrassRadius - g_fMostDetailedDist));
    
    /*float2 vTexCoord    = ((a_vWorldPt.xz / g_fTerrRadius) * 0.5 + 0.5 );
    float3 vNormal = mul(CalcTerrNormal(vTexCoord), g_mView).xyz;*/
    
    return 0.8f * (fLerpCoef) + g_fGrassLodBias * (1.0f - (fLerpCoef));        
}

/* a_fBaseAlpha - base alpha for grass blade (generated on cpu)
 * a_vFirstPt - grass blade FIRST point
 * a_vLastPt - grass blade LAST point
 * out: a_fBladeDist - distanse from blade to camera
 * out: a_fDissolve  - value for alpha dissolve
*/
float CalcTransparency( float a_fBaseAlpha, float4 a_vFirstPt, float4 a_vLastPt, out float a_fBladeDist, out float a_fDissolve)
{
    float fAlphaOffset;    

    fAlphaOffset = LodAlphaOffset(a_vFirstPt, a_fBladeDist);
    a_fDissolve = 1.0;
    return a_fBaseAlpha - fAlphaOffset;
}

float3x3 MakeRotateMtx( float3 a_vAxe )
{
    /*float fSign = 1.0;
    if (a_vAxe.y < 0.0)
        fSign = -1.0;*/
    float fAngle = length(a_vAxe);// * 0.01745; // PI/180
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
/*
float3 CalcWind( float3 a_vPos )
{
    float2 vTexCoord = ((a_vPos.xz / g_fGrassRadius) * 0.5 + 0.5 )  * g_fWindTexTile;
	float2 dU = float2(1 / 1024, 0.0);
	float2 dV = float2(0.0, 1 / 1024);
	float3 vValue = g_txWindTex.SampleLevel(g_samLinear, vTexCoord, 0).rgb; 
    float3 vWind = (vValue) * g_fWindStrength;// / 4.0; 

    return vWind;
}
*/
/* sub-function for calculating ONLY grass blade pts */
GSIn CalcWindAnimation( float3 a_vBladePos, float3 a_vRotAxe, float3 a_vYRotAxe )
{
    GSIn Output;
    float2 vUV              = (a_vBladePos.xz / g_fTerrRadius) * 0.5 + 0.5;
    uint uIndex             = GetTypeIndex(vUV);
    Output.uTypeIndex       = uIndex;
    Output.fTransparency    = GetSeatingInfo(vUV);
    float3 vHardnessSeg     = SubTypes[uIndex].vHardnessSegment;
    float3 vMassSegment     = SubTypes[uIndex].vMassSegment;
    float3 vAxe             = float3(0.0, SubTypes[uIndex].vSizes.y * 0.5, 0.0);
    float3 vGrav            = float3(0.0, -10.0, 0.0);
    float3 vF, vLocalF, vAbsWind;
    float fH, fFL, fLenG, fSinBetha, fBetha, fPhi;

    float3x3 mMYrot  = MakeRotateMtx(a_vYRotAxe); 
    
    float3x3 mMStart = mul(mMYrot, MakeRotateMtx(a_vRotAxe));
    //vAbsWind = CalcWind(a_vBladePos);
    
    /* Computing moments as in the paper */
    float3 vG;
    float3x3 mM[3];
    
    /* First segment */
    vAbsWind = CalcWind(a_vBladePos, 0);
    vF = g_fMass * vMassSegment.x * vGrav + vAbsWind; 
    vLocalF = mul(vF, (mMStart));
    fH = g_fHardness * vHardnessSeg.x;
    vG = cross(vAxe, vLocalF);
    fFL = (SubTypes[uIndex].vSizes.y * 0.5 * length(vLocalF));
	fLenG = length(vG);
	fSinBetha = clamp(fLenG / fFL, -1, 1);
	fBetha = asin(fSinBetha);
	//fPhi = fFL*fSinBetha / fH;
	fPhi = fFL * fSinBetha / (1 - fFL * cos(fBetha));
	fPhi = fFL*sin(fBetha+fPhi) / fH;
	fPhi = fFL*sin(fBetha+fPhi) / fH;
	fPhi = fFL*sin(fBetha+fPhi) / fH;
	//fPhi = fFL*sin(fBetha+fPhi) / fH;
	//fPhi = fFL*sin(fBetha+fPhi) / fH;
	//fPhi = fFL*sin(fBetha+fPhi) / fH;
	//fPhi = fFL*sin(fBetha+fPhi) / fH;
    mM[0] = mul(mMStart, MakeRotateMtx(fPhi * vG / fLenG));
    
    /* Second segment */
    vAbsWind = CalcWind(a_vBladePos, 1);
    vF = g_fMass * vMassSegment.y * vGrav + vAbsWind; 
    vLocalF = mul(vF, (mM[0]));
    fH = g_fHardness * vHardnessSeg.y;
    vG = cross(vAxe, vLocalF);
    fFL = (SubTypes[uIndex].vSizes.y * 0.5 * length(vLocalF));
	fLenG = length(vG);
	fSinBetha = clamp(fLenG / fFL, -1, 1);
	fBetha = asin(fSinBetha);
	fPhi = fFL * fSinBetha / (1 - fFL * cos(fBetha));
	fPhi = fFL*sin(fBetha+fPhi) / fH;
	fPhi = fFL*sin(fBetha+fPhi) / fH;
	fPhi = fFL*sin(fBetha+fPhi) / fH;
	//fPhi = fFL*sin(fBetha+fPhi) / fH;
	//fPhi = fFL*sin(fBetha+fPhi) / fH;
	//fPhi = fFL*sin(fBetha+fPhi) / fH;
	//fPhi = fFL*sin(fBetha+fPhi) / fH;
    mM[1] = mul(mM[0], MakeRotateMtx(fPhi * vG / fLenG));

    /* Third segment */
    vAbsWind = CalcWind(a_vBladePos, 2);
    vF = g_fMass * vMassSegment.z * vGrav + vAbsWind; 
    vLocalF = mul(vF, (mM[1]));
    fH = g_fHardness * vHardnessSeg.z;
    vG = cross(vAxe, vLocalF);
    fFL = (SubTypes[uIndex].vSizes.y * 0.5 * length(vLocalF));
	fLenG = length(vG);
	fSinBetha = clamp(fLenG / fFL, -1, 1);
	fBetha = asin(fSinBetha);
	fPhi = fFL * fSinBetha / (1 - fFL * cos(fBetha));
	fPhi = fFL*sin(fBetha+fPhi) / fH;
	fPhi = fFL*sin(fBetha+fPhi) / fH;
	fPhi = fFL*sin(fBetha+fPhi) / fH;
	//fPhi = fFL*sin(fBetha+fPhi) / fH;
	//fPhi = fFL*sin(fBetha+fPhi) / fH;
	//fPhi = fFL*sin(fBetha+fPhi) / fH;
	//fPhi = fFL*sin(fBetha+fPhi) / fH;
    mM[2] = mul(mM[1], MakeRotateMtx(fPhi * vG / fLenG));

    /* world coord of grass blade start pt */
    Output.vPos0 = a_vBladePos;
    
    a_vBladePos += transpose(mM[0])[1] * SubTypes[uIndex].vSizes.y;
    Output.vPos1 = a_vBladePos;
    
    a_vBladePos += transpose(mM[1])[1] * SubTypes[uIndex].vSizes.y;    
    Output.vPos2 = a_vBladePos;    
    
    a_vBladePos += transpose(mM[2])[1] * SubTypes[uIndex].vSizes.y;
    Output.vPos3 = a_vBladePos;
    
    return Output;
}

GSIn InstVSMain( InstVSIn Input )
{
    float4 vPos = mul(float4(Input.vPos, 1.0), Input.mTransform);
    float fY = g_txHeightMap.SampleLevel(g_samLinear, (vPos.xz / g_fTerrRadius) * 0.5 + 0.5, 0).r * g_fHeightScale;    
    vPos.y = fY;
    
    GSIn Output = CalcWindAnimation(vPos.xyz, Input.vRotAxe * 0.01745, Input.vYRotAxe * 0.01745);
    /******************************/    
    
    float fBladeDist;    
    /* Checking SeatingInfo */
    if (Output.fTransparency > 0.0)
        Output.fTransparency = CalcTransparency(Input.fTransparency, vPos, float4(Output.vPos3, 1.0), fBladeDist, Output.fDissolve);

    Output.iNumVertices = 4;
    if (fBladeDist < (g_fMostDetailedDist + g_fLastDetailedDist) * 0.5)
    {
        Output.iNumVertices = 7;
    }
    
    return Output;
}

GSIn StaticVSMain( StaticVSIn Input )
{
    float4 vPos = mul(float4(Input.vPos, 1.0), Input.mTransform);
    
    GSIn Output = CalcWindAnimation(vPos.xyz, Input.vRotAxe * 0.01745, Input.vYRotAxe * 0.01745);
    /******************************/    
    
    float fBladeDist;
    /* Correcting transparency if mesh collide current grass blade */
    [branch]if ( length(vPos.xz - g_vMeshSpheres[Input.uMeshIndex].xz) < g_vMeshSpheres[Input.uMeshIndex].w )
        Output.fTransparency = 0.0;
    else
        Output.fTransparency = CalcTransparency(Input.fTransparency, vPos, float4(Output.vPos3, 1.0), fBladeDist, Output.fDissolve);
    
    Output.iNumVertices = 4;
    if (fBladeDist < (g_fMostDetailedDist + g_fLastDetailedDist) * 0.5)
    {
        Output.iNumVertices = 7;
    }
    
    return Output;
}

GSIn PhysVSMain( PhysVSIn Input )
{
    float4 vPos = float4(Input.vPos, 1.0); //mul(float4(Input.vPos, 1.0), g_mWorld);
    float fY = g_txHeightMap.SampleLevel(g_samLinear, (vPos.xz / g_fTerrRadius) * 0.5 + 0.5, 0).r * g_fHeightScale;    
    vPos.y = fY;       
    
    GSIn Output;
    
    float2 vUV              = (vPos.xz / g_fTerrRadius) * 0.5 + 0.5;
    uint uIndex             = GetTypeIndex(vUV);
    Output.uTypeIndex       = uIndex;
    float fSeating		    = GetSeatingInfo(vUV);
    
    /* fuck... */
    /*Output.vOffs1 = Output.vOffs0 = Input.fSegmentWidth * Input.mR0[0].xyz;
    Output.vOffs2 = Input.fSegmentWidth * Input.mR1[0].xyz;
    Output.vOffs3 = Input.fSegmentWidth * Input.mR2[0].xyz;*/

    Output.vPos0 = vPos.xyz; vPos += transpose(Input.mR0)[1] * SubTypes[uIndex].vSizes.y;
    Output.vPos1 = vPos.xyz; vPos += transpose(Input.mR1)[1] * SubTypes[uIndex].vSizes.y;
    Output.vPos2 = vPos.xyz; vPos += transpose(Input.mR2)[1] * SubTypes[uIndex].vSizes.y;
    Output.vPos3 = vPos.xyz;
    
    /*if (Output.vPos0.y <= fY) Output.vPos0.y = fY + 0.2f;//what the fuck?????????????
    if (Output.vPos1.y <= fY) Output.vPos1.y = fY + 0.2f;
    if (Output.vPos2.y <= fY) Output.vPos2.y = fY + 0.2f;
    if (Output.vPos3.y <= fY) Output.vPos3.y = fY + 0.2f;*/
    
    /******************************/    
    
    float fBladeDist;    
    if (fSeating > 0.0)
        Output.fTransparency = CalcTransparency(Input.fTransparency, vPos, float4(Output.vPos3, 1.0), fBladeDist, Output.fDissolve);    
    else 
		Output.fTransparency = 0.0;
    //Output.uTypeIndex = 9;

    Output.iNumVertices = 4;
    if (fBladeDist < (g_fMostDetailedDist + g_fLastDetailedDist) * 0.5)
    {
        Output.iNumVertices = 7;
    }
    
    return Output;
}

#include "Shaders/GSFunc.fx"

void Make4Pts( GSIn In, inout TriangleStream< PSIn > TriStream )
{
	float3 vDir = mul(float4(0.0, 1.0, 0.0, 0.0), g_mView).xyz;
	float3 vNormal;
	float3 vZ = float3(0.0, 0.0, 1.0);
	float2 vTexCoord = float2(0.0, 0.0);
    int i;	
	float3 vPos[4];
    float3 vOffs;
    float2 vLightParam;
    InitFramePts(In, vPos);
    float fInvNum = 0.33;
    PSIn Vertex;
    for (i = 0; i < 4; i++)
	{
	    vOffs = mul(float4(cross(vDir, vZ) * SubTypes[In.uTypeIndex].vSizes.x, 0.0), g_mInvCamView).xyz;
	    
	    /* correcting offset vector */
	    //vPos[i]  = mul(float4(vPos[i], 1.0), g_mView).xyz;	    
	    if (i > 0)
	    {
	        vDir = normalize(vPos[i] - vPos[i-1]);
	    }
        if (vDir.y < 0.0)
	    {
	        //vOffs = -vOffs;
	        vDir = -vDir;	        
	    }
        vNormal = cross(vDir, vOffs);
        vNormal = normalize(vNormal);	    	
	    vTexCoord.y = i * fInvNum;
	    vTexCoord.x = 0.0;
	    CreateVertex(vPos[i] + vOffs, vTexCoord, vNormal.y, In.uTypeIndex, In.fDissolve, Vertex);
	    TriStream.Append(Vertex);    
	    vTexCoord.x = 1.0;
	    CreateVertex(vPos[i] - vOffs, vTexCoord, vNormal.y, In.uTypeIndex, In.fDissolve, Vertex);
	    TriStream.Append(Vertex);	    
	    /* Dir -> to view space */
	    vDir = mul(float4(vDir, 0.0), g_mView).xyz;
	}
	
	TriStream.RestartStrip();    
}

void Make7Pts( GSIn In, inout TriangleStream< PSIn > TriStream )
{
	float3 vDir = mul(float4(0.0, 1.0, 0.0, 0.0), g_mView).xyz;
	float3 vNormal[7];
	float3 vN;
	float3 vZ = float3(0.0, 0.0, 1.0);
	float2 vTexCoord = float2(0.0, 0.0);
        
    float3 vPos[4];
    int i;
    InitFramePts(In, vPos);    
    float3 vOffsBase[4];
    
    /* Building normals by 4 pts */
    vNormal[0] = vNormal[1] = cross(vOffsBase[0], vDir);
    for (i = 0; i < 4; i++)
    {
        vOffsBase[i] = mul(float4(cross(vDir, vZ) * SubTypes[In.uTypeIndex].vSizes.x, 0.0), g_mInvCamView).xyz;
	    
	    // vPos[i]  = mul(float4(vPos[i], 1.0), g_mView).xyz;	    
	    if (i > 0)
	    {
	        vDir = normalize(vPos[i] - vPos[i-1]);
	    }
        if (vDir.y < 0.0)
	    {
	        //vOffsBase[i] = -vOffsBase[i];
	        vDir = -vDir;	        
	    }
	    vN = cross(vDir, vOffsBase[i]);
	    vN = normalize(vN);
	    if (i < 3)
	        vNormal[i * 2] = vNormal[i * 2 + 1] = vN;
	    /* Dir -> to view space */
	    vDir = mul(float4(vDir, 0.0), g_mView).xyz;
    }
    vNormal[6] = vNormal[5];
    
    

    float3 vOffs[7] = {vOffsBase[0],//In.vOffs0, 
                       vOffsBase[1],//In.vOffs0,
                       vOffsBase[1],//In.vOffs1, 
                       vOffsBase[2],//In.vOffs1,
                       vOffsBase[2],//In.vOffs2,
                       vOffsBase[3],//In.vOffs2,
                       vOffsBase[3],//In.vOffs3
                       };

	float3 vPts[7];
    vPts[0] = vPos[0];
    vPts[1] = LerpPoint(vPos, 0.165);
    vPts[2] = LerpPoint(vPos, 0.33);
    vPts[3] = LerpPoint(vPos, 0.495);
    vPts[4] = LerpPoint(vPos, 0.66);
    vPts[5] = LerpPoint(vPos, 0.825);
    vPts[6] = vPos[3];

    float fInvNum = 0.166;  
    PSIn Vertex;  
    /* Building vertices */
    [unroll]for (i = 0; i < 7; i++)
	{
	    vTexCoord.y = i * fInvNum;
	    vTexCoord.x = 0.0;
	    CreateVertex(vPts[i] + vOffs[i], vTexCoord, vNormal[i].y, In.uTypeIndex, In.fDissolve, Vertex);
	    TriStream.Append(Vertex);    
	    vTexCoord.x = 1.0;
	    CreateVertex(vPts[i] - vOffs[i], vTexCoord, vNormal[i].y, In.uTypeIndex, In.fDissolve, Vertex);
	    TriStream.Append(Vertex);	    
	}
	
	TriStream.RestartStrip();    
}

[maxvertexcount(14)]
void GSGrassMain( point GSIn Input[1], inout TriangleStream< PSIn > TriStream )
{
    if (Input[0].fTransparency <= 0.0)
		return;

    if (Input[0].iNumVertices > 4)
        Make7Pts(Input[0], TriStream);
    else
        Make4Pts(Input[0], TriStream);
    
}

/* Pixel shaders */

float4 InstPSMain( PSIn Input ) : SV_Target
{
    float fNoise = g_txNoise.Sample(g_samLinear, Input.vTexCoord).r;
    if (Input.fDissolve < 1.0)
        clip(fNoise - Input.fDissolve);

    uint uTexIndex = SubTypes[Input.uIndex].uTexIndex;
    float4 vTexel = g_txGrassDiffuseArray.Sample(g_samAniso, float3(Input.vTexCoord, uTexIndex)) * SubTypes[Input.uIndex].vColor;

    //float fShadowCoef = ShadowCoef(Input.vShadowPos);
        
    float3 vL = mul(float4(-vLightDir, 0.0), g_mView).xyz;
    float fNdotL  = clamp(dot(Input.vNormal, vL), 0.15, 0.25) + g_fGrassAmbient;

    return float4(fNdotL * vTexel.xyz, vTexel.a);
}

float4 ShadowPSMain( PSIn Input, out float fDepth: SV_Depth ) : SV_Target
{   
    uint uTexIndex = SubTypes[Input.uIndex].uTexIndex;
    float fAlpha = g_txGrassDiffuseArray.Sample(g_samLinear, float3(Input.vTexCoord, uTexIndex)).a;
    clip(fAlpha - 0.001);
    fDepth = Input.vShadowPos.z / Input.vShadowPos.w * 0.5 + 0.5;
    return float4(0.0, 0.0, 0.0, 1.0);
}

#include "Shaders/LowGrass.fx"

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

    pass RenderPhysicsPass
    {
        SetVertexShader( CompileShader( vs_4_0, PhysVSMain() ) );
        SetGeometryShader( CompileShader( gs_4_0, GSGrassMain() ) );
        SetPixelShader( CompileShader( ps_4_0, InstPSMain() ) );
        
        SetBlendState( AlphaBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
        //SetRasterizerState( EnableMSAA );
    }

    pass StaticCollidePass
    {
        SetVertexShader( CompileShader( vs_4_0, StaticVSMain() ) );
        SetGeometryShader( CompileShader( gs_4_0, GSGrassMain() ) );
        SetPixelShader( CompileShader( ps_4_0, InstPSMain() ) );
        
        SetBlendState( AlphaBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
        //SetRasterizerState( EnableMSAA );
    }
    
    /* Shadow passes */
    pass ShadowPass
    {        
        SetVertexShader( CompileShader( vs_4_0, InstVSMain() ) );
        SetGeometryShader( CompileShader( gs_4_0, GSGrassMain() ) );
        SetPixelShader( CompileShader( ps_4_0, ShadowPSMain() ) );
    }  
    
    pass ShadowPhysicsPass
    {
        SetVertexShader( CompileShader( vs_4_0, PhysVSMain() ) );
        SetGeometryShader( CompileShader( gs_4_0, GSGrassMain() ) );
        SetPixelShader( CompileShader( ps_4_0, ShadowPSMain() ) );
    }
}