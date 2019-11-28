#pragma once

#include "GrassLod.h"
#include "GrassPool.h"
//#include "GrassCollideStatic.h"
#include "Terrain.h"
#include "Wind.h"
#include "mesh.h"
#include "ConvexVolume.h"
#include <fstream>

using std::ifstream;

#include "GrassTrack.h"

class GrassTracker;

struct GrassInitState
{
    ID3D11Device	    *pD3DDevice;
	ID3D11DeviceContext *pD3DDeviceCtx;
    float				 fGrassRadius;
    float				 fTerrRadius;
    float				 fMostDetailedDist;
    float				 fLastDetailedDist;
    float				 fMaxQuality;
    DWORD				 dwPatchesPerSide;
    DWORD				 dwBladesPerPatchSide;
    UINT				 uNumCollidedPatchesPerMesh;
    UINT				 uMaxColliders;
    float				 fCameraMeshDist;
    
	std::vector<std::wstring> sTexPaths;
    
	/* Textures for flowers "tops" */
    std::vector<std::wstring>   sTopTexPaths;
    std::wstring                sLowGrassTexPath;
    std::wstring                sEffectPath;
    std::wstring                sSeatingTexPath;
    std::wstring                sIndexTexPath;
    std::wstring                sSubTypesPath;

    bool ReadFromFile (const char *a_pFileName);
};

enum 
{
    GrassLodsCount = 3
};

struct IndexMapData
{
    UCHAR *pData;
    UINT uHeight;
    UINT uWidth;
    ~IndexMapData ( )
    {
        if (pData)
            delete [] pData;
    }
};

class GrassManager
{
private:
    GrassInitState                      m_GrassState;
    const TerrainHeightData            *m_pHeightData;
    float                               m_fHeightScale;
    float                               m_fGrassLod0Dist;
    float                               m_fGrassLod1Dist;
    float                               m_fPatchSize;
    DWORD                              *m_pRotatePattern;
    GrassLod                           *m_GrassLod[GrassLodsCount];
    GrassPool                          *m_GrassPool[GrassLodsCount - 1];//last lod don't need physics
    const WindData                     *m_pWindData;
    std::vector< GrassPropsUnified >    m_SubTypeProps;
    IndexMapData                        m_pIndexMapData;
    //GrassCollideStatic                 *m_pGrassCollideStatic;
    ID3D11InputLayout                  *m_pInputLayout;
    ID3DX11Effect                       *m_pEffect;
    ID3DX11EffectPass                   *m_pRenderPass;
    ID3DX11EffectPass                   *m_pLowGrassPass;
    bool                                 m_bUseLowGrass;
    ID3DX11EffectPass                   *m_pShadowPass;
    ID3D11Texture2D                     *m_pDiffuseTex;
    ID3D11ShaderResourceView            *m_pLowGrassTexSRV;
    ID3D11ShaderResourceView            *m_pDiffuseTexSRV;

    ID3D11Texture2D                     *m_pTopDiffuseTex;
    ID3D11ShaderResourceView            *m_pTopDiffuseTexSRV;
    /* Seating texture, A8 */
    ID3D11ShaderResourceView            *m_pSeatingTexSRV;
    /* Texture with grass subtypes indices, L8 */
    ID3D11ShaderResourceView            *m_pIndexTexSRV;
    float                                m_fGrassLodBias;
    ID3DX11EffectScalarVariable         *m_pGrassLodBiasESV;
    float                                m_fGrassSubScatterGamma;
    ID3DX11EffectScalarVariable         *m_pGrassSubScatterGammaESV;
    float                                m_fGrassAmbient;
    ID3DX11EffectScalarVariable         *m_pGrassAmbientESV;
    float                                m_fWindStrength;
    ID3DX11EffectScalarVariable         *m_pWindStrengthESV;
    ID3DX11EffectScalarVariable         *m_pMostDetailedDistESV;
    ID3DX11EffectScalarVariable         *m_pLastDetailedDistESV;
    ID3DX11EffectScalarVariable         *m_pGrassRadiusESV;
    float                                m_fQuality;
    ID3DX11EffectScalarVariable         *m_pQualityESV;  
    ID3DX11EffectVectorVariable         *m_pLowGrassDiffuseEVV;
    GrassTracker                        *m_pGrassTracker;
    ID3DX11EffectMatrixVariable         *m_pTrackViewProjEMV;
    ID3DX11EffectShaderResourceVariable *m_pTrackMapSRV;
	UINT								 m_uNumPatchesPerTerrSide;
    //DWORD                                m_dwLodTransformsIndex[GrassLodsCount];
    
    //void UpdateEven         ( ConvexVolume &a_cvFrustum, XMFLOAT4X4 &a_mViewProj, XMFLOAT3 a_vCamPos, Mesh *a_pMeshes[], UINT a_uNumMeshes, float a_fElapsedTime );
    //void UpdateOdd          ( ConvexVolume &a_cvFrustum, XMFLOAT4X4 &a_mViewProj, XMFLOAT3 a_vCamPos, Mesh *a_pMeshes[], UINT a_uNumMeshes, float a_fElapsedTime );

    void ReInitLods         (void);
    void Init               (void);
    void GenerateTransforms (float a_fMaxDist, float a_fPatchSize);
    void LoadIndexData      (void);
	bool IsPatchVisible		(ConvexVolume& a_cvFrustum, XMVECTOR& a_vPatchPos);
    float GetPatchHeight    (UINT a_uX, UINT a_uY );
    //void  GetPatchNormal   ( UINT a_uX, UINT a_uY, XMFLOAT3 *a_vNormal );
	float LodAlphaOffset    (const XMVECTOR &a_vCamPos, const XMVECTOR&a_vPatchPos, const float a_fDist, const float a_fIsCorner );

public:
    GrassManager  (GrassInitState &a_pInitState, GrassTracker *a_pGrassTracker );
    ~GrassManager (void);

    void Reinit             (GrassInitState &a_pInitState);
    void SetHeightScale     (float a_fHeightScale);
    void SetHeightDataPtr   (const TerrainHeightData *a_pHeightData);
    void SetWindDataPtr     (const WindData *a_pWindData);
    void SetGrassLodBias    (float a_fGrassLodBias);
    void SetSubScatterGamma (float a_fGrassSubScatterGamma);
    void SetGrassAmbient    (float a_fGrassAmbient);
    void SetWindStrength    (float a_fWindStrength);
    void SetWindSpeed       (float a_fWindSpeed);
    void SetQuality         (float a_fQuality);
    void SetLowGrassDiffuse (float4 &a_vValue);

    void ClearGrassPools    (void);

    void AddSubType         (const GrassPropsUnified &a_SubTypeData);
    void ClearSubTypes      (void);

	ID3DX11Effect* GetEffect (void);
    void Render              (bool a_bShadowPass);
    void Update              (float4x4 &a_mViewProj, float3 a_vCamPos, Mesh *a_pMeshes[], UINT a_uNumMeshes, float a_fElapsedTime);
};
