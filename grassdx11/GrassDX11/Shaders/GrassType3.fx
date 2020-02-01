//--------------------------------------------------------------------------------------
// Constant buffers 
//--------------------------------------------------------------------------------------
cbuffer cRarely
{
    float g_fMostDetailedDist;
    float g_fLastDetailedDist;
    float g_fTerrRadius;
    float g_fGrassRadius;         
    float g_fGrassLod0Dist;
    float g_fMaxQuality;// = 0.7;
    float3 vLightDir;
    float4 g_vFogColor;
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
    uint   uTexIndex;
    uint   uTopTexIndex;
    
    float2 pad0;
};

cbuffer cGrassSubTypes
{                  
    GrassSubType SubTypes[10];                        
};

//--------------------------------------------------------------------------------------
// Texture and samplers
//--------------------------------------------------------------------------------------
Texture2DArray g_txGrassDiffuseArray;
Texture2DArray g_txTopDiffuseArray;
Texture2DArray g_txWindTex;
Texture2D      g_txAxesFanFlow;
Texture2D      g_txSeatingMap;
Texture2D      g_txIndexMap;
Texture2D      g_txNoise;
Texture2D      g_txHeightMap;
Texture2D      g_txShadowMap;
Texture2D      g_txTerrainLightMap;
Texture2D      g_txGrassColor;

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
    float3 vPos1                  : TEXCOORD1;
    float3 vPos2                  : TEXCOORD2;
    float3 vPos3                  : TEXCOORD3;
    float  fTransparency          : TRANSPARENCY;
    float  fDissolve              : DISSOLVE;
    /* from vs to gs */
    uint   uNumVertices           : NUMVERTS;
    uint   uTypeIndex             : TYPEINDEX;    
};

struct PSIn
{
    float4 vPos                       : SV_Position;
    //float4 vShadowPos                 : TEXCOORD0;
    float4 vTexCoord                  : TEXCOORD1;
    nointerpolation float2 vWorldTC   : TEXCOORD2;
    float  fLightParam                : NORMALY;
    float2 vTerrSpec                  : TERRSPEC;
    nointerpolation float fDissolve   : DISSOLVE;
    nointerpolation bool  bIsTop      : ISTOP;
    nointerpolation uint  uIndex      : TYPEINDEX;
};

/* Grass shaders */
#include "EnableSeating.fx"
#include "EnableSubTypes.fx"
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
    float  fSegLength       = SubTypes[uIndex].vSizes.y;
    float3 vAxe             = float3(0.0, SubTypes[uIndex].vSizes.y * 0.5, 0.0);
    float3 vGrav            = float3(0.0, -10.0, 0.0);
    float3 vF, vLocalF, vAbsWind;
    float fH, fFL, fLenG, fSinBetha, fBetha, fPhi;

    float3x3 mMYrot  = MakeRotateMtx(a_vYRotAxe); 
    
    float3x3 mMStart = mul(mMYrot, MakeRotateMtx(a_vRotAxe));

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
//			sum = g * vMassSegment[j-1] * mM_T[j][1].y;
			sum = g * vMassSegment[j-1];
            localSum = mul(sum, mM_T[j]);
			float3 G;
            G = cross(halfAxis, localSum);
            mM_R[j] = MakeRotateMtx(G / vHardnessSeg[j - 1]);
            mM_T[j] = mul(mM_T[j - 1], mM_R[j]);
		}

        w_ = CalcWind(a_vBladePos, j - 1);
//		sum = g * vMassSegment[j-1] * mM_T[j][1].y;
		sum = g * vMassSegment[j-1];
        localSum = mul(sum, mM_T[j]);
		float3 G;
        G = cross(halfAxis, localSum);
        w = mul(w_, mM_T[j]);
		G += w;
        mM_R[j] = MakeRotateMtx(G / vHardnessSeg[j - 1]);
        mM_T[j] = mul(mM_T[j-1], mM_R[j]);
	}

//    /* Computing moments as in the paper */
//    float3 vG;
//    float3x3 mM[3];
//    
//    /* First segment */
//    vAbsWind = CalcWind(a_vBladePos, 0);
////    vF = g_fMass * vMassSegment.x * vGrav + vAbsWind; 
//    vF =  (vMassSegment.x * vGrav * mMStart[1].y + vAbsWind); 
//    vLocalF = mul(vF, (mMStart));
//    fH = g_fHardness * vHardnessSeg.x;
//    vG = cross(vAxe, vLocalF);
//    fFL = (SubTypes[uIndex].vSizes.y * 0.5 * length(vLocalF));
//	fLenG = length(vG);
//	fSinBetha = clamp(fLenG / fFL, -1, 1);
//	if (abs(fSinBetha) < 0.0001)
//		mM[0] = MatrixIdentity();
//	else
//	{
//		fBetha = asin(fSinBetha);
//		fPhi = fFL * fSinBetha / (1 - fFL * cos(fBetha));
//		fPhi = fFL*sin(fBetha+fPhi) / fH;
//		fPhi = fFL*sin(fBetha+fPhi) / fH;
//		fPhi = fFL*sin(fBetha+fPhi) / fH;
//		mM[0] = mul(mMStart, MakeRotateMtx(fPhi * vG / fLenG));
//    };
//	
//    /* Second segment */
//    vAbsWind = CalcWind(a_vBladePos, 1);
////    vF = g_fMass * vMassSegment.y * vGrav + vAbsWind; 
//    vF =  (vMassSegment.y * vGrav * (mM[0][1].y - 0.1) + vAbsWind); 
//    vLocalF = mul(vF, (mM[0]));
//    fH = g_fHardness * vHardnessSeg.y;
//    vG = cross(vAxe, vLocalF);
//    fFL = (SubTypes[uIndex].vSizes.y * 0.5 * length(vLocalF));
//	fLenG = length(vG);
//	fSinBetha = clamp(fLenG / fFL, -1, 1);
//	if (abs(fSinBetha) < 0.0001)
//		mM[1] = MatrixIdentity();
//	else
//	{
//		fBetha = asin(fSinBetha);
//		fPhi = fFL * fSinBetha / (1 - fFL * cos(fBetha));
//		fPhi = fFL*sin(fBetha+fPhi) / fH;
//		fPhi = fFL*sin(fBetha+fPhi) / fH;
//		fPhi = fFL*sin(fBetha+fPhi) / fH;
//		mM[1] = mul(mM[0], MakeRotateMtx(fPhi * vG / fLenG));
//	};
//
//    /* Third segment */
//    vAbsWind = CalcWind(a_vBladePos, 2);
////    vF = g_fMass * vMassSegment.z * vGrav + vAbsWind; 
//    vF = vMassSegment.z * vGrav * (mM[1][1].y - 0.2) + vAbsWind; 
//    vLocalF = mul(vF, (mM[1]));
//    fH = g_fHardness * vHardnessSeg.z;
//    vG = cross(vAxe, vLocalF);
//    fFL = (SubTypes[uIndex].vSizes.y * 0.5 * length(vLocalF));
//	fLenG = length(vG);
//	fSinBetha = clamp(fLenG / fFL, -1, 1);
//	if (abs(fSinBetha) < 0.0001)
//		mM[2] = MatrixIdentity();
//	else
//	{
//		fBetha = asin(fSinBetha);
//		fPhi = fFL * fSinBetha / (1 - fFL * cos(fBetha));
//		fPhi = fFL*sin(fBetha+fPhi) / fH;
//		fPhi = fFL*sin(fBetha+fPhi) / fH;
//		fPhi = fFL*sin(fBetha+fPhi) / fH;
//		mM[2] = mul(mM[1], MakeRotateMtx(fPhi * vG / fLenG));
//	};

    /* world coord of grass blade start pt */
    Output.vPos0 = a_vBladePos;
    
	float fDist = length(a_vBladePos - g_mInvCamView[3].xyz);
	float fEnd = g_fGrassRadius - 50.0;
    float fScale = 0.5*(2.0*fEnd - fDist)/fEnd;
	
    if (fScale < 0.5) 
    {
		fScale = 0.5*(g_fGrassRadius - fDist)/50.0;
    }  
    
  //  fSegLength *= (0.75 + 0.5 * vColorAndH.a); 
//	fSegLength *= 1.15;
    a_vBladePos += transpose(mM_T[1])[1] * fScale*fSegLength;
    Output.vPos1 = a_vBladePos;
    
    a_vBladePos += transpose(mM_T[2])[1] * fScale*fSegLength;    
    Output.vPos2 = a_vBladePos;    
    
    a_vBladePos += transpose(mM_T[3])[1] * fScale*fSegLength;
    Output.vPos3 = a_vBladePos;
    
    return Output;
}

GSIn InstVSMain( InstVSIn Input )
{
    float4 vPos = mul(float4(Input.vPos, 1.0), Input.mTransform);
    float fY = g_txHeightMap.SampleLevel(g_samLinear, (vPos.xz / g_fTerrRadius) * 0.5 + 0.5, 0).a * g_fHeightScale;    
    vPos.y = fY;
    
    GSIn Output = CalcWindAnimation(vPos.xyz, Input.vRotAxe * 0.01745, Input.vYRotAxe * 0.01745);

    /******************************/    
    
    Output.fTransparency = CalcTransparency(Input.fTransparency, vPos, Output.fDissolve, Output.uNumVertices).x;   
    ScreenClip(float4(Output.vPos0, 1.0), float4(Output.vPos3, 1.0), Output.fTransparency);
    return Output;
}

GSIn AnimVSMain ( InstVSIn Input )
{                               
    float fY = g_txHeightMap.SampleLevel(g_samLinear, (Input.vPos.xz / g_fTerrRadius) * 0.5 + 0.5, 0).a * g_fHeightScale;    
    Input.vPos.y = fY;
    
    GSIn Output = CalcWindAnimation(Input.vPos.xyz, Input.vRotAxe * 0.01745, Input.vYRotAxe * 0.01745);
    /******************************/    
    
	Output.fTransparency = CalcTransparency(Input.fTransparency, float4(Input.vPos, 1.0), Output.fDissolve, Output.uNumVertices).x;
	ScreenClip(float4(Output.vPos0, 1.0), float4(Output.vPos3, 1.0), Output.fTransparency);
    return Output;
}

GSIn PhysVSMain( PhysVSIn Input )
{
    float4 vPos = float4(Input.vPos, 1.0); //mul(float4(Input.vPos, 1.0), g_mWorld);
    float fY = g_txHeightMap.SampleLevel(g_samLinear, (vPos.xz / g_fTerrRadius) * 0.5 + 0.5, 0).a * g_fHeightScale;    
    vPos.y = fY;
    
    GSIn Output;
    
    float2 vUV              = (vPos.xz / g_fTerrRadius) * 0.5 + 0.5;
    uint uIndex             = GetTypeIndex(vUV);
    Output.uTypeIndex       = uIndex;
    float fSeating		    = GetSeatingInfo(vUV);
    float  fSegLength       = SubTypes[uIndex].vSizes.y;
    float  fSegWidth        = SubTypes[uIndex].vSizes.x;
    
    /* fuck... */
    /*Output.vOffs1 = Output.vOffs0 = Input.fSegmentWidth * Input.mR0[0].xyz;
    Output.vOffs2 = Input.fSegmentWidth * Input.mR1[0].xyz;
    Output.vOffs3 = Input.fSegmentWidth * Input.mR2[0].xyz;*/
    
	float fDist = length(Input.vPos.xyz - g_mInvCamView[3].xyz);
	float fEnd = g_fGrassRadius - 50.0;
    float fScale = 0.5*(2.0*fEnd - fDist)/fEnd;
	
    if (fScale < 0.5) 
    {
		fScale = 0.5*(g_fGrassRadius - fDist)/50.0;
    }
    
    Output.vPos0 = vPos.xyz; vPos += transpose(Input.mR0)[1] * fSegLength*fScale;
    Output.vPos1 = vPos.xyz; vPos += transpose(Input.mR1)[1] * fSegLength*fScale;
    Output.vPos2 = vPos.xyz; vPos += transpose(Input.mR2)[1] * fSegLength*fScale;
//    Output.vPos0 = vPos.xyz; vPos += transpose(Input.mR0)[1] * Input.fSegmentHeight *1.15;
//    Output.vPos1 = vPos.xyz; vPos += transpose(Input.mR1)[1] * Input.fSegmentHeight *1.15;
//    Output.vPos2 = vPos.xyz; vPos += transpose(Input.mR2)[1] * Input.fSegmentHeight *1.15;
    Output.vPos3 = vPos.xyz;
 /*   
    if (Output.vPos0.y <= fY) Output.vPos0.y = fY + 0.02;
    if (Output.vPos1.y <= fY) Output.vPos1.y = fY + 0.02;
    if (Output.vPos2.y <= fY) Output.vPos2.y = fY + 0.02;
    if (Output.vPos3.y <= fY) Output.vPos3.y = fY + 0.02;
 */   
    /******************************/    
    float fBladeDist;
    Output.fTransparency = CalcTransparency(Input.fTransparency, float4(Input.vPos, 1.0), Output.fDissolve, Output.uNumVertices).x;
    ScreenClip(float4(Output.vPos0, 1.0), float4(Output.vPos3, 1.0), Output.fTransparency);
    return Output;
}

#include "GSFunc.fx"
//CreateVertex( float3 a_vPos, float2 a_vTexCoord, float a_fLightParam, uint a_uTexIndex, float a_fDissolve, float2 a_vWorldTC, inout PSIn Vertex)
void MakeTop( float3 vCenter, float3 vX, float3 vY, float3 vZ, uint uTypeIndex, float a_fDissolve, inout TriangleStream< PSIn > TriStream )
{
    uint uTexIndex = SubTypes[uTypeIndex].uTopTexIndex;
    PSIn Vertex;
    Vertex.bIsTop = true;
    float2 T = float2(0.0, 0.0);
    CreateVertex(vCenter + vX + vY, float2( 1.0,  1.0), vZ.y, uTexIndex, a_fDissolve, T, Vertex);
    TriStream.Append(Vertex);
    CreateVertex(vCenter - vX + vY, float2( 0.0,  1.0), vZ.y, uTexIndex, a_fDissolve, T, Vertex);
    TriStream.Append(Vertex);
    CreateVertex(vCenter + vX - vY, float2( 1.0,  0.0), vZ.y, uTexIndex, a_fDissolve, T, Vertex);
    TriStream.Append(Vertex);
    CreateVertex(vCenter - vX - vY, float2( 0.0,  0.0), vZ.y, uTexIndex, a_fDissolve, T, Vertex);
    TriStream.Append(Vertex);
    TriStream.RestartStrip(); 
}        

inline void PtTo2Vertex(float3 vPt, float3 vOffs, float vTCy, float vNy, uint uInd, float2 a_vWorldTC, float a_fDissolve, 
	inout TriangleStream< PSIn > TriStream)
{
	PSIn Vertex;
	Vertex.bIsTop = false;
	float2 vTexCoord;
	vTexCoord.y = vTCy;
    vTexCoord.x = 0.0;
    CreateVertex(vPt + vOffs, vTexCoord, vNy, uInd, a_fDissolve, a_vWorldTC, Vertex);
    TriStream.Append(Vertex);    
    vTexCoord.x = 1.0;
    CreateVertex(vPt - vOffs, vTexCoord, vNy, uInd, a_fDissolve, a_vWorldTC, Vertex);
    TriStream.Append(Vertex);
}

inline void Make4Pts( GSIn In, inout TriangleStream< PSIn > TriStream )
{
	float3 vDir = normalize(In.vPos3 - In.vPos2);
	float3 vCamDir = g_mInvCamView[2].xyz;
	vCamDir.y = 0.0;
	float3 vOffs = normalize(cross(vCamDir, vDir)) * SubTypes[In.uTypeIndex].vSizes.x;
	//float fNormalY = normalize(cross(vDir, vOffs)).y;	
	float fNormalY = vDir.y;
	float2 vWorldTC = (In.vPos0.xz) / g_fTerrRadius * 0.5 + 0.5;
    PtTo2Vertex(In.vPos3, vOffs, 1.0, fNormalY, In.uTypeIndex, vWorldTC, In.fDissolve, TriStream);
    /* Update offset */
    vDir = (In.vPos2 - In.vPos1);
    vOffs = normalize(cross(vCamDir, vDir)) * SubTypes[In.uTypeIndex].vSizes.x;
    PtTo2Vertex(In.vPos2, vOffs, 0.66, fNormalY, In.uTypeIndex, vWorldTC, In.fDissolve, TriStream);
    /* Update offset */
    vDir = (In.vPos1 - In.vPos0);
    vOffs = normalize(cross(vCamDir, vDir)) * SubTypes[In.uTypeIndex].vSizes.x;
    PtTo2Vertex(In.vPos1, vOffs, 0.33, fNormalY, In.uTypeIndex, vWorldTC, In.fDissolve, TriStream);
    /* Update offset */
    vDir = float3(0.0, 1.0, 0.0);
    vOffs = normalize(cross(vCamDir, vDir)) * SubTypes[In.uTypeIndex].vSizes.x;
    PtTo2Vertex(In.vPos0, vOffs, 0.0, fNormalY, In.uTypeIndex, vWorldTC, In.fDissolve, TriStream);
    
	TriStream.RestartStrip();    
}

inline void Make7Pts( GSIn In, inout TriangleStream< PSIn > TriStream )
{
	float3 vDir = float3(0.0, 1.0, 0.0);
	float3 vCamDir = g_mInvCamView[2].xyz;
	vCamDir.y = 0.0;
	
	float3 vZ = float3(0.0, 0.0, 1.0);
	float2 vTexCoord = float2(0.0, 0.0);
	
        
    float3 vPos[4];
    int i;
    InitFramePts(In, vPos);    
    float3 vOffsBase[4];
    
    /* Building normals by 4 pts */
    [unroll]for (i = 0; i < 4; i++)
    {
		if (i > 0)
	    {
	        vDir = vPos[i] - vPos[0];
	    }
	    
        vOffsBase[i] = normalize(cross(vCamDir, vDir)) * SubTypes[In.uTypeIndex].vSizes.x;
    }
    //float fNy = normalize(cross(vPos[3] - vPos[2], vOffsBase[3])).y;
	float fNy = normalize(vPos[3] - vPos[2]).y;
    float3 vOffs[7] = {vOffsBase[0],
                       vOffsBase[1],
                       vOffsBase[1],
                       vOffsBase[2],
                       vOffsBase[2],
                       vOffsBase[3],
                       vOffsBase[3],
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
    float2 vWorldTC = (In.vPos0.xz) / g_fTerrRadius * 0.5 + 0.5;
    PSIn Vertex;  
    /* Building vertices */
    [unroll]for (i = 0; i < 7; i++)
	{
	    PtTo2Vertex(vPts[i], vOffs[i], i * fInvNum, fNy, In.uTypeIndex, vWorldTC, In.fDissolve, TriStream);	    
	}   
	
	TriStream.RestartStrip();    
}

[maxvertexcount(18)]
void GSGrassMain( point GSIn Input[1], inout TriangleStream< PSIn > TriStream )
{
    if (Input[0].fTransparency <= 0.0)
		return;
		
	float3 vX, vY, vZ;
	float fLen;
	vZ = (Input[0].vPos3 - Input[0].vPos2);
	fLen = length(vZ) * 0.3;
	vX = cross(float3(vZ.x, vZ.y, vZ.z - 1.0), vZ);
	vX = normalize(vX);
	vZ = vZ * 0.3;//length = fLen * 0.3 / 0.3
	vY = cross(vZ, vX);//length = 1.0 * fLen
	vX = vX * fLen;
	
	MakeTop(Input[0].vPos3, vX, vY, vZ, Input[0].uTypeIndex, Input[0].fDissolve, TriStream);   

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
   
}
/*
float4 InstPSMain( PSIn Input ) : SV_Target
{        
  	//float fLimDist = (Input.vTexCoord.w - 75.f)/10.f;
	float fNoise = g_txNoise.Sample(g_samLinear, Input.vTexCoord.xy).r;
	
	if (fLimDist>0.0)
	{
		clip(fNoise - fLimDist);
	}
	else
	{
		if (Input.fDissolve < 1.0)
			clip(fNoise - Input.fDissolve);
	}
    float fL = max(0.017, (0.5 + 5.0 * Input.vTerrSpec.y)*0.6);
    float3 vT = g_vTerrRGB * max(0.8, (2.0 + 5.0f* Input.vTerrSpec.y)*0.5);
	float fY = Input.vTexCoord.y;
	fY *= fY;
	    
    float4 vC4;
    float3 vC;
    if (Input.bIsTop)
    {
        vC4 = g_txTopDiffuseArray.Sample(g_samLinear, float3(Input.vTexCoord.xy, Input.uIndex));    
        vC=vC4.xyz* fL;
    }
    else
    {
        vC4 = g_txGrassDiffuseArray.Sample(g_samLinear, float3(Input.vTexCoord.xy, Input.uIndex));
		vC = vT + fY*vC4.xyz* fL;
    }
	float fLimDist1 = (Input.vTexCoord.w - 50.f)/35.f;
	if (fLimDist1>0.0)
	{
		vC = (1.0 - fLimDist1)*vC +  fLimDist1*vT;
	}	
    
    return lerp(float4(vC, vC4.a), g_vFogColor, Input.vTexCoord.z);
 
}
*/
float4 InstPSMain( PSIn Input ) : SV_Target
{        
  if (Input.fDissolve < 0.0) clip(-1);
  /*	float fLimDist = (Input.vTexCoord.w - 75.f)/10.f;
    float fNoise = g_txNoise.Sample(g_samLinear, Input.vTexCoord.xy).r;
	if (fLimDist>0.0)
	{
		clip(fNoise - fLimDist);
	}
	else
	{
		if (Input.fDissolve < 1.0)
			clip(Input.fDissolve - fNoise);
	}
	*/
	float fL = max(0.17, (1.0 + 5.0 * Input.vTerrSpec.y)*0.6);
    float3 vT = float3(0.04, 0.1, 0.01) * max(0.8, (2.0 + 5.0f* Input.vTerrSpec.y)*0.5);
//	float3 vT = g_vTerrRGB * max(0.8, (2.0 + 5.0f* Input.vTerrSpec.y)*0.5);
	float fY = Input.vTexCoord.y;
	fY *= fY;
	
    float4 vTexel;
	float4 vC;
	float3 vC3;

	if (Input.bIsTop)
    {
        vTexel = g_txTopDiffuseArray.Sample(g_samLinear, float3(Input.vTexCoord.xy, Input.uIndex));    
        vC3=vTexel.xyz* fL;
    }
    else
    {
        vTexel = g_txGrassDiffuseArray.Sample(g_samLinear, float3(Input.vTexCoord.xy, Input.uIndex));
    //    vTexel.xyz = float3(0.1, 0.2, 0.03);
		vC3 = vT + fY*vTexel.xyz* fL;
    }
    
	float fLimDist1 = (Input.vTexCoord.w - 50.f)/35.f;
	if (fLimDist1>0.0)
	{
		vC3 = (1.0 - fLimDist1)*vC3 +  fLimDist1*vT;
	}	
  //  return float4(vC3, vTexel.a);
   return lerp(float4(vC3, vTexel.a * Input.fDissolve), g_vFogColor, Input.vTexCoord.z);
     
}
//float4 ShadowPSMain( PSIn Input, out float fDepth: SV_Depth ) : SV_Target
//{   
//    float fAlpha;
//    if (Input.bIsTop)
//        fAlpha = g_txTopDiffuseArray.Sample(g_samLinear, float3(Input.vTexCoord.xy, Input.uIndex)).a;    
//    else
//        fAlpha = g_txGrassDiffuseArray.Sample(g_samLinear, float3(Input.vTexCoord.xy, Input.uIndex)).a;
//    clip(fAlpha - 0.001);
//    fDepth = Input.vShadowPos.z / Input.vShadowPos.w * 0.5 + 0.5;
//    return float4(0.0, 0.0, 0.0, 1.0);
//}

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
        
        SetBlendState( Alpha2CovBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
        //SetRasterizerState( EnableMSAA );
    }

    pass RenderAnimPass
    {
        SetVertexShader( CompileShader( vs_4_0, AnimVSMain() ) );
        SetGeometryShader( CompileShader( gs_4_0, GSGrassMain() ) );
        SetPixelShader( CompileShader( ps_4_0, InstPSMain() ) );
        
        SetBlendState( Alpha2CovBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepthTestWrite, 0 );
        //SetRasterizerState( EnableMSAA );
    }

    /* Shadow passes */
    /*pass ShadowPass
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
    }*/
}