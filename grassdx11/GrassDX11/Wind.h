#pragma once

#include "includes.h"


struct QuadVertex
{
   XMFLOAT3 vPos;
   XMFLOAT2 vTexCoord;
};

struct WindData
{
   XMFLOAT3     *pData;              //wind for CPU
   XMFLOAT3      vWindData[64 * 64]; //wind for CPU
   XMFLOAT4     *pWindMapData;       //gradient map
   UINT          uHeight;
   UINT          uWidth;
   float         fHeight;
   float         fWidth;
   float        *pTime;       // pointer to Wind member m_fTime. Unsafe, but fast :)
   float        *pWindSpeed;  // pointer to Wind member m_fWindSpeed. Unsafe, but fast :)

   XMVECTOR   GetValue      (const XMVECTOR& a_vTexCoord, const float a_fWindTexTile) const;
   XMVECTOR   GetValueA     (const XMVECTOR& a_vTexCoord, const float a_fWindTexTile, int a_iSegmentIndex) const;
   float      BiLinear      (const XMVECTOR& a_vTexCoord);
   XMVECTOR   GetWindValue  (const XMVECTOR& a_vTexCoord, const float a_fWindTexTile, const float a_fWindStrength) const;
   XMVECTOR   GetWindValueA (const XMVECTOR& a_vTexCoord, const float a_fWindTexTile, const float a_fWindStrength, int a_iSegmentIndex) const;

   //void          Update        ( );
   void          UpdateWindTex (float a_fElapsed, XMVECTOR a_vCamDir);
   void          ConvertFrom   (const D3D11_MAPPED_SUBRESOURCE& a_MappedTex, const D3D11_TEXTURE2D_DESC& a_TexDesc);

   WindData  (void);
   ~WindData (void);

   void WindCopy(ID3D11Texture2D* a_pDestTex, ID3D11DeviceContext* a_pDeviceCtx);
};

class Wind
{
private:
   ID3D11Device        *m_pD3DDevice;
   ID3D11DeviceContext *m_pD3DDeviceCtx;
   UINT                 m_uViewPortWidth;
   UINT                 m_uViewPortHeight;

   ID3D11ShaderResourceView            *m_pHeightTexSRV;
   ID3DX11EffectShaderResourceVariable *m_pHeightTexESRV;

   ID3D11Texture2D                     *m_pHeightMap;
   ID3D11RenderTargetView              *m_pHeightMapRTV;
   ID3D11ShaderResourceView            *m_pHeightMapSRV;
   ID3DX11EffectShaderResourceVariable *m_pHeightMapESRV;

   ID3D11Texture2D                     *m_pWindMap;
   ID3D11RenderTargetView              *m_pWindMapRTV;
   ID3D11ShaderResourceView            *m_pWindMapSRV;
   ID3DX11EffectShaderResourceVariable *m_pWindMapESRV;

   ID3D11Texture2D                    *m_pWindTex;
   D3D11_TEXTURE2D_DESC                m_WindTexStagingDesc;
   ID3D11Texture2D                    *m_pWindTexStaging;    //special resource to read on GPU
   ID3D11ShaderResourceView           *m_pWindTexSRV;
   WindData                            m_WindData;

   ID3D11Texture2D* m_pDepthTex;
   ID3D11DepthStencilView* m_pDSV;

   /* performs texture update */
   ID3DX11Effect               *m_pWindEffect;
   ID3DX11EffectPass           *m_pHeightMapPass;
   ID3DX11EffectPass           *m_pWindMapPass;
   ID3DX11EffectPass           *m_pWindTexPass;
   ID3DX11EffectScalarVariable *m_pTimeESV;
   ID3DX11EffectScalarVariable *m_pWindBias;
   ID3DX11EffectScalarVariable *m_pWindScale;
   float                        m_fTime;
   float                        m_fWindSpeed;

   ID3DX11EffectScalarVariable *m_pWindSpeedESV;

   ID3D11InputLayout *m_pInputLayout;
   ID3D11Buffer      *m_pVertexBuffer;
   UINT               m_uVertexStride;
   UINT               m_uVertexOffset;

   void CreateVertexBuffer (void);
   void CreateInputLayout  (void);
   void MakeHeightMap      (void);
   void MakeWindMap        (void);
   void MakeWindTex        (float a_fElapsed, XMVECTOR a_vCamDir);
   void UpdateWindData     (void);

public:
   Wind  (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx);
   ~Wind (void);

   void SetWindBias  (float a_fBias);
   void SetWindScale (float a_fScale);
   void SetWindSpeed (float a_fWindSpeed);
   void Update       (float a_fElapsed, XMVECTOR a_vCamDir);

   const WindData* WindDataPtr      (void);
   ID3D11ShaderResourceView* GetMap (void);
};