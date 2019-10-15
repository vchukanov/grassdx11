#pragma once

#include "includes.h"

struct TerrainVertex
{
    D3DXVECTOR3 vPos;
    D3DXVECTOR2 vTexCoord;
};

struct TerrainHeightData
{
    float        *pData;
    D3DXVECTOR3  *pNormals;
    UINT          uHeight;
    UINT          uWidth;
    float         fHeight;//float(uHeight)
    float         fWidth; //float(uWidth)
    void  ConvertFrom        ( const D3D10_MAPPED_TEXTURE2D &a_MappedTex, const D3D10_TEXTURE2D_DESC &a_TexDesc );
    void  CalcNormals        ( float a_fHeightScale, float a_fDistBtwVertices );
    float GetHeight          ( float a_fX, float a_fY ) const;
	D3DXVECTOR3 GetNormal	 ( float a_fX, float a_fY ) const;
    float GetHeight3x3       ( float a_fX, float a_fY ) const;
    TerrainHeightData        ( );
    ~TerrainHeightData       ( );
};

class Terrain
{
private:
    /* Just a simple textured grid */
    ID3D10ShaderResourceView           *m_pGrassSRV;
    ID3D10ShaderResourceView           *m_pSandSRV;
    ID3D10ShaderResourceView           *m_pHeightMapSRV;
    ID3D10EffectShaderResourceVariable *m_pHeightMapESRV;
    /* Light Map */
    ID3D10EffectShaderResourceVariable *m_pLightMapESRV;
    ID3D10Texture2D                    *m_pLightMap;
    ID3D10RenderTargetView             *m_pLightMapRTV;
    ID3D10ShaderResourceView           *m_pLightMapSRV;
    D3D10_VIEWPORT                      m_ViewPort;

    ID3D10EffectPass                   *m_pPass;
    ID3D10EffectPass                   *m_pLightMapPass;
    ID3D10Device                       *m_pD3DDevice;
    ID3D10InputLayout                  *m_pInputLayout;
    ID3D10Buffer                       *m_pVertexBuffer;
    ID3D10Buffer                       *m_pQuadVertexBuffer;
    UINT                                m_uVertexStride;
    UINT                                m_uVertexOffset;
    ID3D10Buffer                       *m_pIndexBuffer;
    UINT                                m_uIndicesCount;
    TerrainHeightData                   m_HeightData;
    float                               m_fCellSize;//distance between neighbour vertices

    void CreateBuffers                       ( float a_fSize );
    void CreateInputLayout                   ( );

public:
    Terrain                                  ( ID3D10Device *a_pD3DDevice, ID3D10Effect *a_pEffect, float a_fSize );
    ~Terrain                                 ( );

    void  BuildHeightMap                     ( float a_fHeightScale );
    ID3D10ShaderResourceView  *LightMapSRV   ( );
    ID3D10ShaderResourceView  *HeightMapSRV  ( );
    TerrainHeightData         *HeightDataPtr ( );//unsafe!
    void UpdateLightMap                      ( );
    void Render                              ( );
};