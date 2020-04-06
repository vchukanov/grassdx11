#pragma once

#include "includes.h"


class VelocityMap {
public:
   VelocityMap  (ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceCtx);
   ~VelocityMap (void);

   void SetRenderTarget   (ID3D11DepthStencilView* pDSV);
   void ClearRenderTarget (ID3D11DepthStencilView* pDSV);
   
   ID3D11ShaderResourceView* GetShaderResourceView (void);

   void                      BeginVelocityMap (void);
   ID3D11ShaderResourceView* EndVelocityMap   (void);

private: 
   ID3D11Device          *m_pD3DDevice;
   ID3D11DeviceContext   *m_pD3DDeviceCtx;

   ID3D11Texture2D           *m_renderTargetTexture;
   ID3D11RenderTargetView    *m_renderTargetView;
   ID3D11ShaderResourceView  *m_shaderResourceView;

   ID3D11RenderTargetView* m_pOrigRT;
   ID3D11DepthStencilView* m_pOrigDS;
   D3D11_VIEWPORT          m_OrigViewPort[1];
   D3D11_VIEWPORT          m_ViewPort[1];
};
