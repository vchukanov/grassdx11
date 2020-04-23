#include "VelocityMap.h"
//#include "main.h"


VelocityMap::VelocityMap (ID3D11Device * pD3DDevice, ID3D11DeviceContext * pD3DDeviceCtx)
{
   D3D11_TEXTURE2D_DESC            textureDesc;
   D3D11_RENDER_TARGET_VIEW_DESC   renderTargetViewDesc;
   D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
   
   m_pD3DDevice = pD3DDevice;
   m_pD3DDeviceCtx = pD3DDeviceCtx;

   // Initialize the render target texture description.
   ZeroMemory(&textureDesc, sizeof(textureDesc));

   // Setup the render target texture description.
   textureDesc.Width = 1600; // same as screen
   textureDesc.Height = 900; // same as screen

   textureDesc.MipLevels = 1;
   textureDesc.ArraySize = 1;
   textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
   textureDesc.SampleDesc.Count = 1;
   textureDesc.Usage = D3D11_USAGE_DEFAULT;
   textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
   textureDesc.CPUAccessFlags = 0;
   textureDesc.MiscFlags = 0;

   // Create the render target texture.
   m_pD3DDevice->CreateTexture2D(&textureDesc, NULL, &m_renderTargetTexture);

   // Setup the description of the render target view.
   renderTargetViewDesc.Format = textureDesc.Format;
   renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
   renderTargetViewDesc.Texture2D.MipSlice = 0;
   renderTargetViewDesc.Texture2DArray.ArraySize = 1;

   // Create the render targets view.
   m_pD3DDevice->CreateRenderTargetView(m_renderTargetTexture, &renderTargetViewDesc, &m_renderTargetView);

   // Setup the description of the shader resource view.
   shaderResourceViewDesc.Format = textureDesc.Format;
   shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
   shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
   shaderResourceViewDesc.Texture2D.MipLevels = 1;
   shaderResourceViewDesc.Texture2DArray.ArraySize = 1;
   shaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;

   // Create the shader resource view.
   m_pD3DDevice->CreateShaderResourceView(m_renderTargetTexture, &shaderResourceViewDesc, &m_shaderResourceView);
}


VelocityMap::~VelocityMap(void)
{
   SAFE_RELEASE(m_shaderResourceView);
   SAFE_RELEASE(m_renderTargetView);
   m_renderTargetTexture->Release();   //HACK:  m_renderTargetTexture refcount is 2??
   SAFE_RELEASE(m_renderTargetTexture);
}


void VelocityMap::SetRenderTarget (ID3D11DepthStencilView* depthStencilView)
{
   // Bind the render target view and depth stencil buffer to the output render pipeline.
   m_pD3DDeviceCtx->OMSetRenderTargets(1, &m_renderTargetView, depthStencilView);

   return;
}

void VelocityMap::ClearRenderTarget (ID3D11DepthStencilView* depthStencilView)
{
   float color[4];

   // Setup the color to clear the buffer to.
   color[0] = 0.0f;
   color[1] = 0.0f;
   color[2] = 0.0f;
   color[3] = 0.0f;

   // Clear the back buffer.
   m_pD3DDeviceCtx->ClearRenderTargetView(m_renderTargetView, color);
   // Clear the depth buffer.
   //m_pD3DDeviceCtx->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

   return;
}


ID3D11ShaderResourceView* VelocityMap::GetShaderResourceView (void)
{
   return m_shaderResourceView;
}



void VelocityMap::BeginVelocityMap (void)
{
   // Saving render targets
   UINT                    NumV = 1;

   m_pD3DDeviceCtx->RSGetViewports(&NumV, m_OrigViewPort);

   m_ViewPort[0] = m_OrigViewPort[0];
   m_ViewPort[0].Width = 1600;
   m_ViewPort[0].Height = 900;
   
   m_pD3DDeviceCtx->RSSetViewports(1, m_ViewPort);

   m_pD3DDeviceCtx->OMGetRenderTargets(1, &m_pOrigRT, &m_pOrigDS);

   SetRenderTarget(NULL);
   ClearRenderTarget(NULL);
}


ID3D11ShaderResourceView* VelocityMap::EndVelocityMap (void)
{
   /* Reverting changes */
   m_pD3DDeviceCtx->OMSetRenderTargets(1, &m_pOrigRT, m_pOrigDS);
   m_pD3DDeviceCtx->RSSetViewports(1, m_OrigViewPort);

   SAFE_RELEASE(m_pOrigRT);
   SAFE_RELEASE(m_pOrigDS);

   return m_shaderResourceView;
}