#pragma once

#include "includes.h"

#include <DirectXTex.h>

struct TerrainVertex
{
    XMFLOAT3 vPos;
    XMFLOAT2 vTexCoord;
};

struct TerrainHeightData
{
    float        *pData;
   XMFLOAT3     *pNormals;
    UINT          uHeight;
    UINT          uWidth;
    float         fHeight;
    float         fWidth; 

    void     ConvertFrom        (const ScratchImage* a_image, const TexMetadata* a_info);
    void     CalcNormals        (float a_fHeightScale, float a_fDistBtwVertices);
    float    GetHeight          (float a_fX, float a_fY) const;
   XMFLOAT3 GetNormal          (float a_fX, float a_fY) const;
    float    GetHeight3x3       (float a_fX, float a_fY) const;
    
   TerrainHeightData        (void);
    ~TerrainHeightData       (void);
};

class Terrain
{
private:
    /* Just a simple textured grid */
    ID3D11ShaderResourceView            *m_pGrassSRV;
    ID3D11ShaderResourceView            *m_pSandSRV;
    ID3D11ShaderResourceView            *m_pHeightMapSRV;
    ID3DX11EffectShaderResourceVariable *m_pHeightMapESRV;
    /* Light Map */
    ID3DX11EffectShaderResourceVariable *m_pLightMapESRV;
    ID3D11Texture2D                     *m_pLightMap;
    ID3D11RenderTargetView              *m_pLightMapRTV;
    ID3D11ShaderResourceView            *m_pLightMapSRV;
    D3D11_VIEWPORT                       m_ViewPort;

    ID3DX11EffectPass                   *m_pPass;
    ID3DX11EffectPass                   *m_pLightMapPass;
    ID3D11Device                        *m_pD3DDevice;
    ID3D11DeviceContext                 *m_pD3DDeviceCtx;
    ID3D11InputLayout                   *m_pInputLayout;
    ID3D11Buffer                        *m_pVertexBuffer;
    ID3D11Buffer                        *m_pQuadVertexBuffer;
    UINT                                 m_uVertexStride;
    UINT                                 m_uVertexOffset;
    ID3D11Buffer                        *m_pIndexBuffer;
    UINT                                 m_uIndicesCount;
    TerrainHeightData                    m_HeightData;
    float                                m_fCellSize;   //distance between neighbour vertices

    void CreateBuffers                       ( float a_fSize );
    void CreateInputLayout                   ( );

public:
    Terrain                                  (ID3D11Device *a_pD3DDevice, ID3D11DeviceContext * a_pD3DDeviceCtx, ID3DX11Effect *a_pEffect, float a_fSize);
    ~Terrain                                 (void);
                                   
    void  BuildHeightMap                     (float a_fHeightScale );
    ID3D11ShaderResourceView  *LightMapSRV   (void);
    ID3D11ShaderResourceView  *HeightMapSRV  (void);
    TerrainHeightData         *HeightDataPtr (void);//unsafe!
    void UpdateLightMap                      (void);
    void Render                              (void);
};