#pragma once

#include "GrassManager.h"
#include "GrassProperties.h"
#include "Terrain.h"
#include "Wind.h"
#include "ShadowMapping.h"
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
    float          fHeightScale;
    float          fTerrRadius;
};

class GrassFieldManager
{
private:
    Terrain                    *m_pTerrain;
	float					    m_fHeightScale;
	float						m_fTerrRadius;

    Wind                       *m_pWind;
    LiSPSM                     *m_pShadowMapping;
    GrassManager               *m_pGrassTypes[GrassTypeNum];
    GrassPropertiesT1          *m_pT1SubTypes;
    GrassPropertiesT2          *m_pT2SubTypes;
    GrassPropertiesT3          *m_pT3SubTypes;
    ID3D10Effect               *m_pSceneEffect;
    /* links to variables, for all GrassManagers and one for scene effect */
    ID3D10EffectMatrixVariable *m_pViewProjEMV[GrassTypeNum + 1];
    ID3D10EffectMatrixVariable *m_pViewEMV[GrassTypeNum];
    ID3D10EffectMatrixVariable *m_pInvView[3];//only 1, 2 grass types and terrain
    ID3D10EffectScalarVariable *m_pTerrRadiusESV[3];//only 1, 2 grass types and terrain
    ID3D10EffectScalarVariable *m_pHeightScale[GrassTypeNum + 1];
    ID3D10EffectMatrixVariable *m_pLightViewProjEMV[GrassTypeNum + 1];
    ID3D10EffectShaderResourceVariable *m_pShadowMapESRV[GrassTypeNum + 1];
    ID3D10EffectVectorVariable *m_pLightDirEVV[GrassTypeNum + 1];
	ID3D10EffectVectorVariable *m_pTerrRGBEVV[3];
    ID3D10EffectVectorVariable *m_pFogColorEVV[3];
    
                                
    ID3D10ShaderResourceView   *m_pNoiseESV;
    ID3D10ShaderResourceView   *m_pGrassColorESV;
    ID3D10ShaderResourceView   *m_pSeatingMapESV[GrassTypeNum];
    ID3D10EffectVectorVariable *m_pMeshesEVV[GrassTypeNum];
    ID3D10EffectScalarVariable *m_pSegMass[GrassTypeNum];
    ID3D10EffectScalarVariable *m_pHardness[GrassTypeNum];
    ID3D10EffectScalarVariable *m_pWindMapTile[GrassTypeNum];
    ID3D10EffectScalarVariable *m_pTime;
    ID3D10EffectScalarVariable *m_pGrassTime[GrassTypeNum];
    D3DXMATRIX                 *m_pView;
    D3DXMATRIX                 *m_pProj;
    D3DXMATRIX                  m_mInvView;
    D3DXMATRIX                 *m_pViewProj;
    D3DXVECTOR3                m_vCamDir;
    D3DXVECTOR3                m_vCamPos;
    GrassTracker               *m_pGrassTracker;

    void SetHeightScale       ( float a_fHeightScale );

public:
    GrassFieldManager         ( GrassFieldState &a_InitState );
    ~GrassFieldManager        ( );
    void Reinit               ( GrassFieldState &a_InitState );
    void SetGrassLodBias      ( float a_fGrassLodBias );
    void SetSubScatterGamma   ( float a_fGrassSubScatterGamma );
    void SetGrassAmbient      ( float a_fGrassAmbient );
    void SetWindStrength      ( float a_fWindStrength );
    void SetWindSpeed         ( float a_fWindSpeed );
    void SetProjMtx           ( D3DXMATRIX &a_mProj  );
    void SetViewMtx           ( D3DXMATRIX &a_mView  );
    void SetViewProjMtx       ( D3DXMATRIX &a_mViewProj );
    void SetTime              ( float a_fTime );
    void SetHardness          ( float a_fHardness );
    void SetSegMass           ( float a_fSegMass );
    
    void SetWindMapTile       ( float a_fWindMapTile );
    void SetQuality           ( float a_fQuality );
    void SetLowGrassDiffuse   ( D3DXVECTOR4 &a_vValue );
    void SetWindScale         ( float a_fScale );
    void SetWindBias          ( float a_fBias );
	void SetTerrRGB			  ( D3DXVECTOR3 &a_vValue );
    void SetFogColor          ( D3DXVECTOR4 &a_vColor );

    void ClearGrassPools      ( void );

	Terrain * const GetTerrain ( float *a_fHeightScale, float *a_fGrassRadius );

    void Render               ( );
    void Update               ( D3DXVECTOR3 a_vCamDir, D3DXVECTOR3 a_vCamPos, Mesh *a_pMeshes[], UINT a_uNumMeshes, float a_fElapsedTime );
    ID3D10Effect *SceneEffect ( );
};