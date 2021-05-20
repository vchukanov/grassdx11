#pragma once

#include "includes.h"

struct TexturesMixerVertex
{
   XMFLOAT3 vPos;
   XMFLOAT2 vTexCoord;
};

class TexturesMixer {
public:
   TexturesMixer  (ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceCtx, int txW1, int txH1, int txW2, int txH2);
   ~TexturesMixer (void);
   
   void MixTextures (ID3D11ShaderResourceView* wind, ID3D11ShaderResourceView* flow);

   ID3D11ShaderResourceView* GetShaderResourceView (void);

   void SetWindStrength (float strength);

private:
   void SetRenderTarget    (ID3D11DepthStencilView* pRT);
   void ClearRenderTarget  (ID3D11DepthStencilView* pDSV);

   void CreateVertexBuffer(void);
   void CreateInputLayout(void);

public:
   ID3D11Device        *m_pD3DDevice;
   ID3D11DeviceContext *m_pD3DDeviceCtx;

   // mix
   ID3D11Texture2D         *m_renderTargetsTexture;
   ID3D11RenderTargetView  *m_renderTargetsView[NUM_SEGMENTS - 1];

   ID3D11ShaderResourceView *m_shaderResourceView;
   
   ID3D11InputLayout *m_pInputLayout;
   ID3D11Buffer      *m_pVertexBuffer;
   UINT               m_uVertexStride;
   UINT               m_uVertexOffset;

   ID3DX11Effect     *m_pEffect;
   ID3DX11EffectPass *m_pPass;

   ID3DX11EffectShaderResourceVariable *m_pTex1; // wind
   ID3DX11EffectShaderResourceVariable *m_pTex2; // flow
   ID3DX11EffectScalarVariable         *m_pStrength;

   int m_maxW;
   int m_maxH;
};

