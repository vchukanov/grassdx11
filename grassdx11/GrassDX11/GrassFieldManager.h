#pragma once

#include "includes.h"
#include "mesh.h"

#include "GrassManager.h"
#include "GrassProperties.h"
#include "Terrain.h"
#include "PhysMath.h"

//TODO:
#include "Wind.h"
//#include "ShadowMapping.h"
#include "GrassTrack.h"


enum
{
    GrassTypeNum = 3
};

struct GrassFieldState
{
    GrassInitState InitState[GrassTypeNum];
    std::wstring   sSceneEffectPath;
    std::wstring   sNoiseMapPath;
    std::wstring   sColorMapPath;
	std::wstring   sGrassOnTerrainTexturePath;
    float          fHeightScale;
    float          fTerrRadius;
};

class GrassFieldManager
{
private:
	bool					    isGrassRendering = true;

    Terrain                    *m_pTerrain;
	float					    m_fHeightScale;
	float						m_fTerrRadius;

	//TODO: Add wind and shadow mapping
    Wind                       *m_pWind;
    //LiSPSM                     *m_pShadowMapping;
    
	GrassManager               *m_pGrassTypes[GrassTypeNum];
    GrassPropertiesT1          *m_pT1SubTypes;
    GrassPropertiesT2          *m_pT2SubTypes;
    GrassPropertiesT3          *m_pT3SubTypes;
    ID3DX11Effect               *m_pSceneEffect;

    /* links to variables, for all GrassManagers and one for scene effect */
    ID3DX11EffectMatrixVariable *m_pViewProjEMV[GrassTypeNum + 1];
    ID3DX11EffectMatrixVariable *m_pViewEMV[GrassTypeNum];
    ID3DX11EffectMatrixVariable *m_pInvView[3];//only 1, 2 grass types and terrain
    ID3DX11EffectScalarVariable *m_pTerrRadiusESV[3];//only 1, 2 grass types and terrain
    ID3DX11EffectScalarVariable *m_pHeightScale[GrassTypeNum + 1];
    ID3DX11EffectMatrixVariable *m_pLightViewProjEMV[GrassTypeNum + 1];
    ID3DX11EffectShaderResourceVariable *m_pShadowMapESRV[GrassTypeNum + 1];
    ID3DX11EffectVectorVariable *m_pLightDirEVV[GrassTypeNum + 1];
	ID3DX11EffectVectorVariable *m_pTerrRGBEVV[3];
    ID3DX11EffectVectorVariable *m_pFogColorEVV[3];
    
                                
    ID3D11ShaderResourceView    *m_pNoiseESV;
	ID3D11ShaderResourceView    *m_pGrassColorESV;
    ID3D11ShaderResourceView    *m_pTerrGrassESV;
	ID3D11ShaderResourceView    *m_pSeatingMapESV[GrassTypeNum];
    ID3DX11EffectVectorVariable *m_pMeshesEVV[GrassTypeNum];
    ID3DX11EffectScalarVariable *m_pSegMass[GrassTypeNum];
    ID3DX11EffectScalarVariable *m_pHardness[GrassTypeNum];
    ID3DX11EffectScalarVariable *m_pWindMapTile[GrassTypeNum];
    ID3DX11EffectScalarVariable *m_pTime;
    ID3DX11EffectScalarVariable *m_pGrassTime[GrassTypeNum];
    float4x4                    *m_pView;
    float4x4                    *m_pProj;
    float4x4                     m_mInvView;
    float4x4                    *m_pViewProj;
    float3	                     m_vCamDir;
    float3	                     m_vCamPos;
    GrassTracker                *m_pGrassTracker;

    void SetHeightScale       (float a_fHeightScale);

public:
    GrassFieldManager         (GrassFieldState& a_InitState);
    ~GrassFieldManager        (void);
    void Reinit               (GrassFieldState &a_InitState);
    void SetGrassLodBias      (float a_fGrassLodBias);
    void SetSubScatterGamma   (float a_fGrassSubScatterGamma);
    void SetGrassAmbient      (float a_fGrassAmbient);
    void SetWindStrength      (float a_fWindStrength);
    void SetWindSpeed         (float a_fWindSpeed);
    void SetProjMtx           (float4x4 &a_mProj);
    void SetViewMtx           (float4x4 &a_mView);
    void SetViewProjMtx       (float4x4 &a_mViewProj);
    void SetTime              (float a_fTime);
    void SetHardness          (float a_fHardness);
    void SetSegMass           (float a_fSegMass);
    
    void SetWindMapTile       (float a_fWindMapTile);
    void SetQuality           (float a_fQuality);
    void SetLowGrassDiffuse   (float4 &a_vValue);
    void SetWindScale         (float a_fScale);
    void SetWindBias          (float a_fBias);
	void SetTerrRGB			  (float3 &a_vValue);
    void SetFogColor          (float4 &a_vColor);
	void ToggleRenderingGrass ();

    void ClearGrassPools      (void);

	Terrain * const GetTerrain (float *a_fHeightScale, float *a_fGrassRadius);

    void Render  (void);
    void Update  (float3 a_vCamDir, float3 a_vCamPos, Mesh *a_pMeshes[], UINT a_uNumMeshes, float a_fElapsedTime);
    
	ID3DX11Effect *SceneEffect (void);
};