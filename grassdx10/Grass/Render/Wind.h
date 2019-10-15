#pragma once

#include "includes.h"

struct QuadVertex
{
    D3DXVECTOR3 vPos;
    D3DXVECTOR2 vTexCoord;
};

struct WindData
{
    D3DXVECTOR3  *pData;//wind for CPU
	D3DXVECTOR3  vWindData[64*64];//wind for CPU
    D3DXVECTOR4  *pWindMapData;//gradient map
    UINT          uHeight;
    UINT          uWidth;
    float         fHeight;
    float         fWidth;
    float*        pTime;//pointer to Wind member m_fTime. Unsafe, but fast :)
    float*        pWindSpeed;//pointer to Wind member m_fWindSpeed. Unsafe, but fast :)
    D3DXVECTOR3   GetValue      ( const D3DXVECTOR2 &a_vTexCoord, const float a_fWindTexTile ) const;
    D3DXVECTOR3   GetValueA      ( const D3DXVECTOR2 &a_vTexCoord, const float a_fWindTexTile,
                                   int a_iSegmentIndex ) const;
    float	      BiLinear      ( const D3DXVECTOR2 &a_vTexCoord );
    D3DXVECTOR3   GetWindValue  ( const D3DXVECTOR2 &a_vTexCoord, const float a_fWindTexTile, const float a_fWindStrength ) const;
    D3DXVECTOR3   GetWindValueA  ( const D3DXVECTOR2 &a_vTexCoord, const float a_fWindTexTile, const float a_fWindStrength,
                                   int a_iSegmentIndex ) const;
    //void          Update        ( );
    void          UpdateWindTex ( D3D10_MAPPED_TEXTURE2D &a_MappedTex, float a_fElapsed, D3DXVECTOR3 a_vCamDir );
    void          ConvertFrom   ( const D3D10_MAPPED_TEXTURE2D &a_MappedTex, const D3D10_TEXTURE2D_DESC &a_TexDesc );
    WindData                    ( );
    ~WindData                   ( );

    void WindCopy( ID3D10Texture2D *a_pDestTex, ID3D10Device *a_pDevice );
};

class Wind
{
private:
    ID3D10Device                       *m_pD3DDevice;
    UINT                                m_uViewPortWidth;
    UINT                                m_uViewPortHeight;

    ID3D10ShaderResourceView           *m_pHeightTexSRV;
	ID3D10EffectShaderResourceVariable *m_pHeightTexESRV;
	
	ID3D10Texture2D                    *m_pHeightMap;
	ID3D10RenderTargetView             *m_pHeightMapRTV;
	ID3D10ShaderResourceView           *m_pHeightMapSRV;
    ID3D10EffectShaderResourceVariable *m_pHeightMapESRV;

    ID3D10Texture2D                    *m_pWindMap;
    ID3D10RenderTargetView             *m_pWindMapRTV;
    ID3D10ShaderResourceView           *m_pWindMapSRV;
    ID3D10EffectShaderResourceVariable *m_pWindMapESRV;

    ID3D10Texture2D                    *m_pWindTex;
    D3D10_TEXTURE2D_DESC                m_WindTexStagingDesc;
    ID3D10Texture2D                    *m_pWindTexStaging;//special resource to read on GPU
    //ID3D10RenderTargetView             *m_pWindTexRTV;
    ID3D10ShaderResourceView           *m_pWindTexSRV;
    WindData                            m_WindData;

    ID3D10Texture2D                    *m_pDepthTex;
    ID3D10DepthStencilView             *m_pDSV;

    /* This variable corresponds to the grass shader, where wind texture will be used */
    //ID3D10EffectShaderResourceVariable *m_pWindTexESRV;

    /* performs texture update */
    ID3D10Effect                       *m_pWindEffect;
	ID3D10EffectPass                   *m_pHeightMapPass;
    ID3D10EffectPass                   *m_pWindMapPass;
    ID3D10EffectPass                   *m_pWindTexPass;
    ID3D10EffectScalarVariable         *m_pTimeESV;
    ID3D10EffectScalarVariable         *m_pWindBias;
    ID3D10EffectScalarVariable         *m_pWindScale;
    float                               m_fTime;
    float                               m_fWindSpeed;

    ID3D10EffectScalarVariable         *m_pWindSpeedESV;

    ID3D10InputLayout                  *m_pInputLayout;
    ID3D10Buffer                       *m_pVertexBuffer;
    UINT                                m_uVertexStride;
    UINT                                m_uVertexOffset;

    void CreateVertexBuffer            ( );
    void CreateInputLayout             ( );
	void MakeHeightMap                 ( );
    void MakeWindMap                   ( );
    void MakeWindTex                   ( float a_fElapsed, D3DXVECTOR3 a_vCamDir );
    void UpdateWindData                ( );

public:
    Wind                               ( ID3D10Device *a_pD3DDevice );
    ~Wind                              ( );

    void SetWindBias                   ( float a_fBias );
    void SetWindScale                  ( float a_fScale );
    void SetWindSpeed                  ( float a_fWindSpeed );
    void Update                        ( float a_fElapsed, D3DXVECTOR3 a_vCamDir);
    const WindData *WindDataPtr        ( );
    ID3D10ShaderResourceView *GetMap   ( );

};