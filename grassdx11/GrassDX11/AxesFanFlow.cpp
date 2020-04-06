#include "AxesFanFlow.h"


AxesFanFlow::AxesFanFlow (ID3D11Device * pD3DDevice, ID3D11DeviceContext * pD3DDeviceCtx, int textureWidth, int textureHeight, float a_fTerrRadius)
{
   D3D11_TEXTURE2D_DESC            textureDesc;
   D3D11_RENDER_TARGET_VIEW_DESC   renderTargetViewDesc;
   D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
   
   m_pD3DDevice = pD3DDevice;
   m_pD3DDeviceCtx = pD3DDeviceCtx;
   m_width = textureWidth;
   m_height = textureHeight;
   m_fTerrRadius = a_fTerrRadius;

   // Initialize the render target texture description.
   ZeroMemory(&textureDesc, sizeof(textureDesc));

   // Setup the render target texture description.
   textureDesc.Width = textureWidth;
   textureDesc.Height = textureHeight;
   textureDesc.MipLevels = 1;
   textureDesc.ArraySize = HISTORY_TEX_CNT;
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
   shaderResourceViewDesc.Texture2DArray.ArraySize = HISTORY_TEX_CNT;
   shaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;

   // Create the shader resource view.
   m_pD3DDevice->CreateShaderResourceView(m_renderTargetTexture, &shaderResourceViewDesc, &m_shaderResourceView);
   
   /* Loading effect */
   ID3DBlob* pErrorBlob = nullptr;
   D3DX11CompileEffectFromFile(L"Shaders/AxesFanFlow.fx",
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
   m_pPass = m_pEffect->GetTechniqueByIndex(0)->GetPassByName("AxesFanFlowPass");
   m_vPositionESV = m_pEffect->GetVariableByName("g_vAxesFanPosOnTex")->AsVector();
   m_vDirectionESV = m_pEffect->GetVariableByName("g_vDir")->AsVector();
   m_pRingsNumber = m_pEffect->GetVariableByName("g_uRingNumber")->AsScalar();
   m_pTime = m_pEffect->GetVariableByName("g_fTime")->AsScalar();
   m_pNoiseSRV = m_pEffect->GetVariableByName("g_txNoise")->AsShaderResource();
   m_pHeightMapSRV = m_pEffect->GetVariableByName("g_txHeightMap")->AsShaderResource();

   m_pHeightScale = m_pEffect->GetVariableByName("g_fHeightScale")->AsScalar();
   
   m_pMaxFlowStrengthESV = m_pEffect->GetVariableByName("g_fMaxFlowStrength")->AsScalar();;
   m_pFanRadiusESV = m_pEffect->GetVariableByName("g_fFanRadius")->AsScalar();;
   m_pDeltaSlicesESV = m_pEffect->GetVariableByName("g_fDeltaSlices")->AsScalar();;
   m_pShiftESV = m_pEffect->GetVariableByName("g_fShift")->AsScalar();
   m_pAngleSpeedESV = m_pEffect->GetVariableByName("g_fAngleSpeed")->AsScalar();

   m_uVertexStride = sizeof(AxesFanFlowVertex);
   m_uVertexOffset = 0;

   CreateVertexBuffer();
   CreateInputLayout();
}


AxesFanFlow::~AxesFanFlow (void)
{
   SAFE_RELEASE(m_shaderResourceView);
   SAFE_RELEASE(m_renderTargetView);
   m_renderTargetTexture->Release();   //HACK:  m_renderTargetTexture refcount is 2??
   SAFE_RELEASE(m_renderTargetTexture);
   SAFE_RELEASE(m_pPass);
   SAFE_RELEASE(m_pEffect);
   SAFE_RELEASE(m_pInputLayout);
   SAFE_RELEASE(m_pVertexBuffer);
}


void AxesFanFlow::SetRenderTarget(ID3D11DepthStencilView* depthStencilView)
{
   // Bind the render target view and depth stencil buffer to the output render pipeline.
   m_pD3DDeviceCtx->OMSetRenderTargets(1, &m_renderTargetView, depthStencilView);

   return;
}

void AxesFanFlow::ClearRenderTarget(ID3D11DepthStencilView* depthStencilView)
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


ID3D11ShaderResourceView* AxesFanFlow::GetShaderResourceView(void)
{
   return m_shaderResourceView;
}


void AxesFanFlow::Update(void)
{
   MakeFlowTexture();
}


void AxesFanFlow::MakeTextHistory (void)
{
   for (int i = (HISTORY_TEX_CNT - 2); i >= 0; i--) {
      m_pD3DDeviceCtx->CopySubresourceRegion(
         m_renderTargetTexture,
         D3D11CalcSubresource(0, i + 1, 1),
         0, 0, 0, 
         m_renderTargetTexture,
         D3D11CalcSubresource(0, i, 1), 
         NULL
      );
   }
}


void AxesFanFlow::MakeFlowTexture(void)
{
   // Saving render targets
   ID3D11RenderTargetView* pOrigRT;
   ID3D11DepthStencilView* pOrigDS;
   D3D11_VIEWPORT          OrigViewPort[1];
   D3D11_VIEWPORT          ViewPort[1];
   UINT                    NumV = 1;

   m_pD3DDeviceCtx->RSGetViewports(&NumV, OrigViewPort);

   ViewPort[0] = OrigViewPort[0];
   ViewPort[0].Height = m_width;
   ViewPort[0].Width = m_height;

   m_pD3DDeviceCtx->RSSetViewports(1, ViewPort);

   m_pD3DDeviceCtx->OMGetRenderTargets(1, &pOrigRT, &pOrigDS);

   SetRenderTarget(NULL);
   ClearRenderTarget(NULL);

   /* Executing rendering */
   m_pD3DDeviceCtx->IASetInputLayout(m_pInputLayout);
   m_pD3DDeviceCtx->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
   m_pD3DDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
   m_pPass->Apply(0, m_pD3DDeviceCtx);
   m_pD3DDeviceCtx->Draw(4, 0);

   /* Reverting changes */
   m_pD3DDeviceCtx->OMSetRenderTargets(1, &pOrigRT, pOrigDS);
   m_pD3DDeviceCtx->RSSetViewports(NumV, OrigViewPort);

   SAFE_RELEASE(pOrigRT);
   SAFE_RELEASE(pOrigDS);

   MakeTextHistory();
}


void AxesFanFlow::CreateVertexBuffer(void)
{
   /* Initializing vertices */
   AxesFanFlowVertex Vertices[4];
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
      4 * sizeof(AxesFanFlowVertex),
      D3D11_USAGE_DEFAULT,
      D3D11_BIND_VERTEX_BUFFER,
      0, 0
   };
   D3D11_SUBRESOURCE_DATA BufferInitData;
   BufferInitData.pSysMem = Vertices;
   m_pD3DDevice->CreateBuffer(&BufferDesc, &BufferInitData, &m_pVertexBuffer);
}


void AxesFanFlow::CreateInputLayout(void)
{
   D3D11_INPUT_ELEMENT_DESC InputDesc[] =
   {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0},
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
   };
   D3DX11_PASS_DESC PassDesc;
   m_pPass->GetDesc(&PassDesc);
   int InputElementsCount = sizeof(InputDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);
   m_pD3DDevice->CreateInputLayout(InputDesc, InputElementsCount,
      PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize,
      &m_pInputLayout);
}


void AxesFanFlow::SetPosition(const float3& a_vValue)
{
   XMVECTOR vAxesFanPosOnTex = scale(1.0 / m_fTerrRadius, a_vValue);
   m_vPositionESV->SetFloatVector((float*)& vAxesFanPosOnTex);
}


void AxesFanFlow::SetDirection(const float3& a_vValue)
{
   XMVECTOR vDir = normalize(a_vValue);
   m_vDirectionESV->SetFloatVector((float*)& vDir);
}


void AxesFanFlow::SetRingsNumber(int a_fValue)
{
   m_pRingsNumber->SetInt(a_fValue);
}


void AxesFanFlow::SetTime(float a_fTime)
{
   m_pTime->SetFloat(a_fTime);
}


void AxesFanFlow::SetNoise (ID3D11ShaderResourceView* a_pNoiseSRV)
{
   m_pNoiseSRV->SetResource(a_pNoiseSRV);
}


void AxesFanFlow::SetHeightMap (ID3D11ShaderResourceView* a_pHeightMapSRV)
{
   m_pHeightMapSRV->SetResource(a_pHeightMapSRV);
}


void AxesFanFlow::SetHeightScale(float a_fHeightScale)
{
   m_pHeightScale->SetFloat(a_fHeightScale / m_fTerrRadius);
}


void AxesFanFlow::SetMaxFlowStrength (float a_fValue)
{
   m_pMaxFlowStrengthESV->SetFloat(a_fValue);
}


void AxesFanFlow::SetFanRadius(float a_fValue)
{
   m_pFanRadiusESV->SetFloat(a_fValue);
}


void AxesFanFlow::SetDeltaSlices (float a_fValue)
{
   m_pDeltaSlicesESV->SetFloat(a_fValue);
}


void AxesFanFlow::SetShift (float a_fValue)
{
   m_pShiftESV->SetFloat(a_fValue);
}


void AxesFanFlow::SetAngleSpeed (float a_fValue)
{
   m_pAngleSpeedESV->SetFloat(a_fValue);
}
