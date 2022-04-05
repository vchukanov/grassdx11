#include "TexturesMixer.h"

#include "includes.h"

TexturesMixer::TexturesMixer (ID3D11Device * pD3DDevice, ID3D11DeviceContext * pD3DDeviceCtx, int txW1, int txH1, int txW2, int txH2)
{
   D3D11_TEXTURE2D_DESC            textureDesc;
   D3D11_RENDER_TARGET_VIEW_DESC   renderTargetsViewDesc;
   D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
   
   m_pD3DDevice = pD3DDevice;
   m_pD3DDeviceCtx = pD3DDeviceCtx;
   m_maxW = std::max(txW1, txW2);
   m_maxH = std::max(txH1, txH2);
   
   // Initialize the render target texture description.
   ZeroMemory(&textureDesc, sizeof(textureDesc));

   // Setup the render target texture description.
   textureDesc.Width = m_maxW;
   textureDesc.Height = m_maxH;
   textureDesc.MipLevels = 1;
   textureDesc.ArraySize = 3;
   textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
   textureDesc.SampleDesc.Count = 1;
   textureDesc.Usage = D3D11_USAGE_DEFAULT;
   textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
   textureDesc.CPUAccessFlags = 0;
   textureDesc.MiscFlags = 0;

   // Create the render target textures.
   m_pD3DDevice->CreateTexture2D(&textureDesc, NULL, &m_renderTargetsTexture);

   // Setup the description of the render target view.
   renderTargetsViewDesc.Format = textureDesc.Format;
   renderTargetsViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
   renderTargetsViewDesc.Texture2D.MipSlice = 0;
   renderTargetsViewDesc.Texture2DArray.ArraySize = 1;

   // Create the render targets view.
      // Create the render targets view.
   for (int i = 0; i < NUM_SEGMENTS - 1; i++) {
      renderTargetsViewDesc.Texture2DArray.FirstArraySlice = D3D11CalcSubresource(0, i, 1);
      m_pD3DDevice->CreateRenderTargetView(m_renderTargetsTexture, &renderTargetsViewDesc, &m_renderTargetsView[i]);
   }

   // Setup the description of the shader resource view.
   shaderResourceViewDesc.Format = textureDesc.Format;
   shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
   shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
   shaderResourceViewDesc.Texture2D.MipLevels = 1;
   shaderResourceViewDesc.Texture2DArray.ArraySize = NUM_SEGMENTS - 1;
   shaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;

   // Create the shader resource view.
   m_pD3DDevice->CreateShaderResourceView(m_renderTargetsTexture, &shaderResourceViewDesc, &m_shaderResourceView);

   /* Loading effect */
   ID3DBlob* pErrorBlob = nullptr;
   D3DX11CompileEffectFromFile(L"Shaders/TexturesMixer.fx",
      0,
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      0,
      0,
      m_pD3DDevice,
      &m_pEffect,
      &pErrorBlob);

   if (pErrorBlob)
   {
      OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
      pErrorBlob->Release();
   }
   m_pPass = m_pEffect->GetTechniqueByIndex(0)->GetPassByName("MixTexturesPass");

   m_pTex1     = m_pEffect->GetVariableByName("g_txWind")->AsShaderResource();
   m_pTex2     = m_pEffect->GetVariableByName("g_txFlow")->AsShaderResource();
   m_pStrength = m_pEffect->GetVariableByName("g_fWindStrength")->AsScalar();

   m_uVertexStride = sizeof(TexturesMixerVertex);
   m_uVertexOffset = 0;

   CreateVertexBuffer();
   CreateInputLayout();
}


TexturesMixer::~TexturesMixer (void)
{
   SAFE_RELEASE(m_shaderResourceView);
   SAFE_RELEASE(m_renderTargetsTexture);
 //  m_renderTargetTexture->Release();   //HACK:  m_renderTargetTexture refcount is 2??
   for (int i = 0; i < NUM_SEGMENTS; i++) {
      SAFE_RELEASE(m_renderTargetsView[i]);
   }
   SAFE_RELEASE(m_pPass);
   SAFE_RELEASE(m_pEffect);
   SAFE_RELEASE(m_pInputLayout);
   SAFE_RELEASE(m_pVertexBuffer);
}


void TexturesMixer::SetRenderTarget(ID3D11DepthStencilView* depthStencilView)
{
   // Bind the render target view and depth stencil buffer to the output render pipeline.
   m_pD3DDeviceCtx->OMSetRenderTargets(NUM_SEGMENTS - 1, &m_renderTargetsView[0], depthStencilView);

   return;
}

void TexturesMixer::ClearRenderTarget(ID3D11DepthStencilView* depthStencilView)
{
   float color[4];

   // Setup the color to clear the buffer to.
   color[0] = 0.0f;
   color[1] = 0.0f;
   color[2] = 0.0f;
   color[3] = 0.0f;

   // Clear the back buffer.
   for (int i = 0; i < NUM_SEGMENTS - 1; i++) {
      m_pD3DDeviceCtx->ClearRenderTargetView(m_renderTargetsView[i], color);
   }

   return;
}


ID3D11ShaderResourceView* TexturesMixer::GetShaderResourceView(void)
{
   return m_shaderResourceView;
}



void TexturesMixer::SetWindStrength (float strength)
{
   m_pStrength->SetFloat(strength);
}


void TexturesMixer::MixTextures (ID3D11ShaderResourceView* wind, ID3D11ShaderResourceView* flow)
{
   m_pTex1->SetResource(wind);
   m_pTex2->SetResource(flow);

   D3D11_VIEWPORT          m_ViewPort;
   ID3D11RenderTargetView *m_pOrigRTV;
   ID3D11DepthStencilView *m_pOrigDSV;
   ID3D11RasterizerState  *m_pOrigRS;
   D3D11_VIEWPORT          m_OrigVP;
   // Saving render targets
   UINT NumV = 1;

   m_pD3DDeviceCtx->RSGetViewports(&NumV, &m_OrigVP);

   m_ViewPort = m_OrigVP;
   m_ViewPort.Height = m_maxW;
   m_ViewPort.Width = m_maxH;

   m_pD3DDeviceCtx->RSSetViewports(1, &m_ViewPort);
   m_pD3DDeviceCtx->OMGetRenderTargets(1, &m_pOrigRTV, &m_pOrigDSV);
   m_pD3DDeviceCtx->RSGetState(&m_pOrigRS);

   SetRenderTarget(NULL);
   ClearRenderTarget(NULL);

   m_pD3DDeviceCtx->IASetInputLayout(m_pInputLayout);
   m_pD3DDeviceCtx->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
   m_pD3DDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
   m_pPass->Apply(0, m_pD3DDeviceCtx);
   m_pD3DDeviceCtx->Draw(4, 0);

   m_pPass->Apply(0, m_pD3DDeviceCtx);

   /* Reverting changes */
   m_pD3DDeviceCtx->OMSetRenderTargets(1, &m_pOrigRTV, m_pOrigDSV);
   m_pD3DDeviceCtx->RSSetViewports(1, &m_OrigVP);

   SAFE_RELEASE(m_pOrigRTV);
   SAFE_RELEASE(m_pOrigDSV);
   SAFE_RELEASE(m_pOrigRS);

   m_pTex1->SetResource(NULL);
   m_pTex2->SetResource(NULL);
}


void TexturesMixer::CreateVertexBuffer(void)
{
   /* Initializing vertices */
   TexturesMixerVertex Vertices[4];
   Vertices[0].vPos = XMFLOAT3(-1.0f, -1.0f, 0.1f);
   Vertices[0].vTexCoord = XMFLOAT2(0.0f, 0.0f);

   Vertices[1].vPos = XMFLOAT3(1.0f, -1.0f, 0.1f);
   Vertices[1].vTexCoord = XMFLOAT2(1.0f, 0.0f);

   Vertices[2].vPos = XMFLOAT3(-1.0f, 1.0f, 0.1f);
   Vertices[2].vTexCoord = XMFLOAT2(0.0f, 1.0f);

   Vertices[3].vPos = XMFLOAT3(1.0f, 1.0f, 0.1f);
   Vertices[3].vTexCoord = XMFLOAT2(1.0f, 1.0f);

   /* Initializing buffer */
   D3D11_BUFFER_DESC BufferDesc =
   {
      4 * sizeof(TexturesMixerVertex),
      D3D11_USAGE_DEFAULT,
      D3D11_BIND_VERTEX_BUFFER,
      0, 0
   };
   D3D11_SUBRESOURCE_DATA BufferInitData;
   BufferInitData.pSysMem = Vertices;
   m_pD3DDevice->CreateBuffer(&BufferDesc, &BufferInitData, &m_pVertexBuffer);
}


void TexturesMixer::CreateInputLayout(void)
{
   D3D11_INPUT_ELEMENT_DESC InputDesc[] =
   {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0},
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
   };
   D3DX11_PASS_DESC PassDesc;
   m_pPass->GetDesc(&PassDesc);
   int InputElementsCount = sizeof(InputDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);
   m_pD3DDevice->CreateInputLayout(InputDesc, InputElementsCount,
      PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize,
      &m_pInputLayout);
}

