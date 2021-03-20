#include "Terrain.h"

#include <DDSTextureLoader.h>
#include <DirectXTex.h>

#include "StateManager.h"
#include "PhysMath.h"


static float ClearColor[4] = { 0.0f, 0.0f, 1.0f, 0.0f };


TerrainHeightData::TerrainHeightData(void)
{
   pData = NULL;
   pNormals = NULL;
}


TerrainHeightData::~TerrainHeightData(void)
{
   if (pData)
      delete[] pData;
   if (pNormals)
      delete[] pNormals;
}


float TerrainHeightData::GetHeight(float a_fX, float a_fY) const
{
   float fX = a_fX * (float)(uWidth - 1);
   float fY = a_fY * (float)(uHeight - 1);
   UINT uLX = (UINT)floor(fX);
   UINT uHX = uLX + 1;
   UINT uLY = (UINT)floor(fY);
   UINT uHY = uLY + 1;
   float fFracX = (fX)-floor(fX);
   float fFracY = (fY)-floor(fY);
   if (uHX > uWidth - 1)
      uHX = uWidth - 1;
   if (uHY > uHeight - 1)
      uHY = uHeight - 1;
   float fLL = pData[uWidth * uLY + uLX];
   float fHL = pData[uWidth * uHY + uLX];
   float fLR = pData[uWidth * uLY + uHX];
   float fHR = pData[uWidth * uHY + uHX];
   return ((1.0f - fFracX) * fLL + fFracX * fLR) * (1.0f - fFracY) +
      fFracY * ((1.0f - fFracX) * fHL + fFracX * fHR);
}

XMFLOAT3 TerrainHeightData::GetNormal(float a_fX, float a_fY) const
{
   float fX = a_fX * (float)(uWidth - 1);
   float fY = a_fY * (float)(uHeight - 1);
   UINT uLX = (UINT)floor(fX);
   UINT uHX = uLX + 1;
   UINT uLY = (UINT)floor(fY);
   UINT uHY = uLY + 1;
   float fFracX = (fX)-floor(fX);
   float fFracY = (fY)-floor(fY);
   if (uHX > uWidth - 1)
      uHX = uWidth - 1;
   if (uHY > uHeight - 1)
      uHY = uHeight - 1;
   XMFLOAT3 xmf_fLL = pNormals[uWidth * uLY + uLX];
   XMFLOAT3 xmf_fHL = pNormals[uWidth * uHY + uLX];
   XMFLOAT3 xmf_fLR = pNormals[uWidth * uLY + uHX];
   XMFLOAT3 xmf_fHR = pNormals[uWidth * uHY + uHX];

   XMVECTOR fLL, fHL, fLR, fHR;
   fLL = XMLoadFloat3(&xmf_fLL);
   fHL = XMLoadFloat3(&xmf_fHL);
   fLR = XMLoadFloat3(&xmf_fLR);
   fHR = XMLoadFloat3(&xmf_fHR);

   XMVECTOR normal = ((1.0f - fFracX) * fLL + fFracX * fLR) * (1.0f - fFracY) +
      fFracY * ((1.0f - fFracX) * fHL + fFracX * fHR);

   XMFLOAT3 xmN;
   XMStoreFloat3(&xmN, normal);
   return xmN;
}

float TerrainHeightData::GetHeight3x3(float a_fX, float a_fY) const
{
   float dU = 1.0f / (fWidth);
   float dV = 1.0f / (fWidth);
   float h0 = GetHeight(a_fX, a_fY);
   float h1 = GetHeight(a_fX - dU, a_fY - dV);
   float h2 = GetHeight(a_fX, a_fY - dV);
   float h3 = GetHeight(a_fX - dU, a_fY);

   return (h0 + h1 + h2 + h3) * 0.25f;
}

void TerrainHeightData::ConvertFrom(const ScratchImage* a_image, const TexMetadata* a_info)
{
   UCHAR* pTexels = (UCHAR*)a_image->GetPixels();
   if (pData)
      return;
   pData = new float[a_info->height * a_info->width];
   pNormals = new XMFLOAT3[a_info->height * a_info->width];
   uHeight = a_info->height;
   uWidth = a_info->width;
   fHeight = (float)uHeight;
   fWidth = (float)uWidth;
   for (UINT row = 0; row < a_info->height; row++)
   {
      UINT rowStart = row * a_image->GetImage(0, 0, 0)->rowPitch;
      for (UINT col = 0; col < a_info->width; col++)
      {
         UINT colStart = col;// * 4;//RGBA
         pData[row * a_info->width + col] = ((float)pTexels[rowStart + colStart + 0]) / 255.0f;
      }
   }
}


void TerrainHeightData::CalcNormals(float a_fHeightScale, float a_fDistBtwVertices)
{
   enum DIR
   {
      LEFT = 0,
      RIGHT,
      UP,
      DOWN
   };
   XMFLOAT3 vToVertex[4]; //vector from current vertex to another one
   vToVertex[LEFT].x = -a_fDistBtwVertices;
   vToVertex[LEFT].z = 0.0f;
   vToVertex[RIGHT].x = a_fDistBtwVertices;
   vToVertex[RIGHT].z = 0.0f;
   vToVertex[UP].x = 0.0f;
   vToVertex[UP].z = a_fDistBtwVertices;
   vToVertex[DOWN].x = 0.0f;
   vToVertex[DOWN].z = -a_fDistBtwVertices;

   for (UINT row = 0; row < uHeight; row++)
   {
      for (UINT col = 0; col < uWidth; col++)
      {
         if (col == 0 || row == 0 || col == uWidth - 1 || row == uHeight - 1)
         {
            pNormals[row * uWidth + col].x = 0.0f;
            pNormals[row * uWidth + col].y = 1.0f;
            pNormals[row * uWidth + col].z = 0.0f;
            continue;
         }
         float fH = pData[row * uWidth + col];
         vToVertex[LEFT].y = pData[row * uWidth + col - 1] * a_fHeightScale - fH * a_fHeightScale;
         vToVertex[RIGHT].y = pData[row * uWidth + col + 1] * a_fHeightScale - fH * a_fHeightScale;
         vToVertex[DOWN].y = pData[row * uWidth + col - uWidth] * a_fHeightScale - fH * a_fHeightScale;
         vToVertex[UP].y = pData[row * uWidth + col + uWidth] * a_fHeightScale - fH * a_fHeightScale;
         pNormals[row * uWidth + col].x =
            pNormals[row * uWidth + col].y =
            pNormals[row * uWidth + col].z = 0.0f;

         XMVECTOR down = XMLoadFloat3(&vToVertex[DOWN]);
         XMVECTOR up = XMLoadFloat3(&vToVertex[UP]);
         XMVECTOR left = XMLoadFloat3(&vToVertex[LEFT]);
         XMVECTOR right = XMLoadFloat3(&vToVertex[RIGHT]);

         auto normalCalc = [](XMVECTOR& vec1, XMVECTOR& vec2, XMFLOAT3* xmf3Normal)
         {
            XMVECTOR vCrossProduct = XMVector3Cross(vec1, vec2);
            vCrossProduct = XMVector3Normalize(vCrossProduct);
            XMVECTOR normal = XMLoadFloat3(xmf3Normal);
            normal += vCrossProduct;
            XMStoreFloat3(xmf3Normal, normal);
         };

         normalCalc(down, left, &pNormals[row * uWidth + col]);
         normalCalc(right, down, &pNormals[row * uWidth + col]);
         normalCalc(up, right, &pNormals[row * uWidth + col]);
         normalCalc(left, up, &pNormals[row * uWidth + col]);

         XMVECTOR normal = XMLoadFloat3(&pNormals[row * uWidth + col]);
         normal = XMVector3Normalize(normal);
         XMStoreFloat3(&pNormals[row * uWidth + col], normal);
      }
   }

}


Terrain::Terrain(ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect* a_pEffect, float a_fSize)
{
   m_pD3DDevice = a_pD3DDevice;
   m_pD3DDeviceCtx = a_pD3DDeviceCtx;
   m_uVertexStride = sizeof(TerrainVertex);
   m_uVertexOffset = 0;

   /* just one technique in effect */
   ID3DX11EffectTechnique* pTechnique = a_pEffect->GetTechniqueByIndex(0);
   m_pPass = pTechnique->GetPassByName("RenderTerrainPass");
   m_pSnowPass = pTechnique->GetPassByName("RenderShowPass");
   m_pLightMapPass = pTechnique->GetPassByName("RenderLightMapPass");
   m_pLightMapESRV = a_pEffect->GetVariableByName("g_txLightMap")->AsShaderResource();

   ID3DX11EffectShaderResourceVariable* pESRV;
   pESRV = a_pEffect->GetVariableByName("g_txGrassDiffuse")->AsShaderResource();
   HRESULT hr = CreateDDSTextureFromFile(m_pD3DDevice, L"resources/Grass.dds", nullptr, &m_pGrassSRV);
   pESRV->SetResource(m_pGrassSRV);

   pESRV = a_pEffect->GetVariableByName("g_txSandDiffuse")->AsShaderResource();
   hr = CreateDDSTextureFromFile(m_pD3DDevice, L"resources/Sand.dds", nullptr, &m_pSandSRV);
   pESRV->SetResource(m_pSandSRV);

   m_fCellSize = 0.0f;
   CreateInputLayout();
   CreateBuffers(a_fSize);  //initializing m_fCellSize

   m_pHeightMapESRV = a_pEffect->GetVariableByName("g_txHeightMap")->AsShaderResource();
   m_pHeightMapSRV = NULL;
}

Terrain::~Terrain(void)
{
   SAFE_RELEASE(m_pInputLayout);
   SAFE_RELEASE(m_pVertexBuffer);
   SAFE_RELEASE(m_pIndexBuffer);
   SAFE_RELEASE(m_pGrassSRV);
   SAFE_RELEASE(m_pHeightMapSRV);
   SAFE_RELEASE(m_pSandSRV);
   SAFE_RELEASE(m_pLightMapSRV);
   SAFE_RELEASE(m_pLightMapRTV);
   SAFE_RELEASE(m_pLightMap);
   SAFE_RELEASE(m_pQuadVertexBuffer);
}

TerrainHeightData* Terrain::HeightDataPtr(void)
{
   return &m_HeightData;
}


void Terrain::BuildHeightMap(float a_fHeightScale)
{
   /* Loading height map for future processing... */
   TexMetadata info;
   ScratchImage image;

   HRESULT hr = LoadFromDDSFile(L"resources/HeightMap.dds", DDS_FLAGS_NONE, &info, image);
   if (hr != S_OK)
      return;

   /* Calculating terrain heights */
   m_HeightData.ConvertFrom(&image, &info);

   /* Calculating normals with respect to HeightScale.
     * Note, that the m_fCellSize was precomputed in CreateBuffers().
     */
   m_HeightData.CalcNormals(a_fHeightScale, m_fCellSize);

   /* Now we need to create a texture to write heights and normals into */
   CD3D11_TEXTURE2D_DESC dstTexDesc;
   ID3D11Texture2D* pDstTexRes;
   D3D11_MAPPED_SUBRESOURCE MappedTexture;

   dstTexDesc.Width = info.width;
   dstTexDesc.Height = info.height;
   dstTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//DXGI_FORMAT_R8G8B8A8_UNORM;//
   dstTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
   dstTexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
   dstTexDesc.Usage = D3D11_USAGE_DYNAMIC;
   dstTexDesc.MipLevels = 1;
   dstTexDesc.ArraySize = 1;
   dstTexDesc.MiscFlags = 0;
   dstTexDesc.SampleDesc.Count = 1;
   dstTexDesc.SampleDesc.Quality = 0;

   V(m_pD3DDevice->CreateTexture2D(&dstTexDesc, 0, &pDstTexRes));

   m_pD3DDeviceCtx->Map(pDstTexRes, D3D11CalcSubresource(0, 0, 1), D3D11_MAP_WRITE_DISCARD, 0, &MappedTexture);

   float* pTexels = (float*)MappedTexture.pData;

   for (UINT row = 0; row < dstTexDesc.Height; row++)
   {
      UINT rowStart = row * MappedTexture.RowPitch / sizeof(float);
      for (UINT col = 0; col < dstTexDesc.Width; col++)
      {
         UINT colStart = col * 4;
         XMFLOAT3* pN = &m_HeightData.pNormals[row * dstTexDesc.Width + col];
         pTexels[rowStart + colStart + 0] = (pN->x * 0.5f + 0.5f);
         pTexels[rowStart + colStart + 1] = (pN->y * 0.5f + 0.5f);
         pTexels[rowStart + colStart + 2] = (pN->z * 0.5f + 0.5f);
         pTexels[rowStart + colStart + 3] = m_HeightData.pData[row * dstTexDesc.Width + col];
      }
   }
   m_pD3DDeviceCtx->Unmap(pDstTexRes, D3D11CalcSubresource(0, 0, 1));

   /* Ok, creating HeightMap texture finally */
   ID3D11Texture2D* pHeightMap;

   dstTexDesc.CPUAccessFlags = 0;
   dstTexDesc.Usage = D3D11_USAGE_DEFAULT;

   V(m_pD3DDevice->CreateTexture2D(&dstTexDesc, 0, &pHeightMap));

   /* Copying data from one texture to another */
   m_pD3DDeviceCtx->CopyResource(pHeightMap, pDstTexRes);
   SAFE_RELEASE(pDstTexRes);

   /* And creating SRV for it */
   D3D11_SHADER_RESOURCE_VIEW_DESC HeightMapSRVDesc;
   ZeroMemory(&HeightMapSRVDesc, sizeof(HeightMapSRVDesc));
   HeightMapSRVDesc.Format = dstTexDesc.Format;
   HeightMapSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
   HeightMapSRVDesc.Texture2D.MostDetailedMip = 0;
   HeightMapSRVDesc.Texture2D.MipLevels = 1;
   V(m_pD3DDevice->CreateShaderResourceView(pHeightMap, &HeightMapSRVDesc, &m_pHeightMapSRV));
   /******/
   SAFE_RELEASE(pHeightMap);
   m_pHeightMapESRV->SetResource(m_pHeightMapSRV);

#pragma region Light Map Creation  
   m_ViewPort.MaxDepth = 1.0f;
   m_ViewPort.MinDepth = 0.0f;
   m_ViewPort.TopLeftX = 0;
   m_ViewPort.TopLeftY = 0;
   m_ViewPort.Width = dstTexDesc.Width;
   m_ViewPort.Height = dstTexDesc.Height;

   dstTexDesc.MipLevels = 1;
   dstTexDesc.ArraySize = 1;
   dstTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
   dstTexDesc.SampleDesc.Count = 1;
   dstTexDesc.Usage = D3D11_USAGE_DEFAULT;
   dstTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
   dstTexDesc.CPUAccessFlags = 0;
   m_pD3DDevice->CreateTexture2D(&dstTexDesc, NULL, &m_pLightMap);

   /* Creating Render Target View for Height Map */
   D3D11_RENDER_TARGET_VIEW_DESC LightMapRTVDesc;
   ZeroMemory(&LightMapRTVDesc, sizeof(LightMapRTVDesc));
   LightMapRTVDesc.Format = dstTexDesc.Format;
   LightMapRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
   LightMapRTVDesc.Texture2D.MipSlice = 0;
   m_pD3DDevice->CreateRenderTargetView(m_pLightMap, &LightMapRTVDesc, &m_pLightMapRTV);

   /* Creating Shader Resource View for Height Map */
   D3D11_SHADER_RESOURCE_VIEW_DESC LightMapSRVDesc;
   ZeroMemory(&LightMapSRVDesc, sizeof(LightMapSRVDesc));
   LightMapSRVDesc.Format = LightMapRTVDesc.Format;
   LightMapSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
   LightMapSRVDesc.Texture2D.MostDetailedMip = 0;
   LightMapSRVDesc.Texture2D.MipLevels = 1;
   m_pD3DDevice->CreateShaderResourceView(m_pLightMap, &LightMapSRVDesc, &m_pLightMapSRV);
   SAFE_RELEASE(m_pLightMap);
#pragma endregion   
}

void Terrain::UpdateLightMap(void)
{
   GetGlobalStateManager().SetRasterizerState("EnableMSAA");

   m_pLightMapESRV->SetResource(NULL);
   /* Saving render targets */
   ID3D11RenderTargetView* pOrigRT;
   ID3D11DepthStencilView* pOrigDS;
   D3D11_VIEWPORT         OrigViewPort[1];
   UINT                   NumV = 1;
   m_pD3DDeviceCtx->RSGetViewports(&NumV, OrigViewPort);
   m_pD3DDeviceCtx->RSSetViewports(1, &m_ViewPort);

   m_pD3DDeviceCtx->OMGetRenderTargets(1, &pOrigRT, &pOrigDS);
   /* Setting up WindTex and NULL as depth stencil */
   m_pD3DDeviceCtx->OMSetRenderTargets(1, &m_pLightMapRTV, NULL);
   m_pD3DDeviceCtx->ClearRenderTargetView(m_pLightMapRTV, ClearColor);

   /* Executing rendering */
   m_pD3DDeviceCtx->IASetInputLayout(m_pInputLayout);
   m_pD3DDeviceCtx->IASetVertexBuffers(0, 1, &m_pQuadVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
   m_pD3DDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
   m_pLightMapPass->Apply(0, m_pD3DDeviceCtx);
   m_pD3DDeviceCtx->Draw(4, 0);

   /* Reverting changes */
   m_pD3DDeviceCtx->OMSetRenderTargets(1, &pOrigRT, pOrigDS);
   m_pD3DDeviceCtx->RSSetViewports(NumV, OrigViewPort);

   SAFE_RELEASE(pOrigRT);
   SAFE_RELEASE(pOrigDS);
   m_pLightMapESRV->SetResource(m_pLightMapSRV);
}

void Terrain::CreateBuffers(float a_fSize)
{
   /* Initializing vertices; HeightMap dimension equal to 256 */
   // a number of vertices on one side
   UINT  uSideCount = 256 + 1;
   UINT  uVerticesCount = uSideCount * uSideCount;
   m_uIndicesCount = (uSideCount - 1) * (uSideCount - 1) * 6;
   m_fCellSize = 2.0f * a_fSize / (float)(uSideCount - 1);
   UINT i, j;

   XMVECTOR vStartPos = create(-a_fSize, 0.0f, -a_fSize);;

   TerrainVertex* pVertices = new TerrainVertex[uVerticesCount];
   UINT* pIndices = new UINT[m_uIndicesCount];
   UINT uStartInd = 0;
   for (i = 0; i < uSideCount; i++)
      for (j = 0; j < uSideCount; j++)
      {
         XMVECTOR res = vStartPos + create(m_fCellSize * i, 0.0f, m_fCellSize * j);
         XMStoreFloat3(&pVertices[i * uSideCount + j].vPos, res);

         pVertices[i * uSideCount + j].vTexCoord = XMFLOAT2((float)i / (float)(uSideCount - 1), (float)j / (float)(uSideCount - 1));
      }

   for (i = 0; i < uSideCount - 1; i++)
      for (j = 0; j < uSideCount - 1; j++)
      {
         pIndices[uStartInd] = i * uSideCount + j;
         uStartInd++;
         pIndices[uStartInd] = i * uSideCount + j + 1;
         uStartInd++;
         pIndices[uStartInd] = (i + 1) * uSideCount + j;
         uStartInd++;

         pIndices[uStartInd] = (i + 1) * uSideCount + j;
         uStartInd++;
         pIndices[uStartInd] = i * uSideCount + j + 1;
         uStartInd++;
         pIndices[uStartInd] = (i + 1) * uSideCount + j + 1;
         uStartInd++;
      }

   /* Initializing vertex buffer */
   D3D11_BUFFER_DESC VBufferDesc =
   {
       uVerticesCount * sizeof(TerrainVertex),
       D3D11_USAGE_DEFAULT,
       D3D11_BIND_VERTEX_BUFFER,
       0, 0
   };
   D3D11_SUBRESOURCE_DATA VBufferInitData;
   VBufferInitData.pSysMem = pVertices;
   m_pD3DDevice->CreateBuffer(&VBufferDesc, &VBufferInitData, &m_pVertexBuffer);
   delete[] pVertices;
   /* Initializing index buffer */
   D3D11_BUFFER_DESC IBufferDesc =
   {
       m_uIndicesCount * sizeof(UINT),
       D3D11_USAGE_DEFAULT,
       D3D11_BIND_INDEX_BUFFER,
       0, 0
   };
   D3D11_SUBRESOURCE_DATA IBufferInitData;
   IBufferInitData.pSysMem = pIndices;
   m_pD3DDevice->CreateBuffer(&IBufferDesc, &IBufferInitData, &m_pIndexBuffer);
   delete[] pIndices;

   /* Initializing vertices */
   TerrainVertex Vertices[4];
   Vertices[0].vPos = XMFLOAT3(-1.0f, -1.0f, 0.1f);
   Vertices[0].vTexCoord = XMFLOAT2(0.0f, 1.0f);

   Vertices[1].vPos = XMFLOAT3(1.0f, -1.0f, 0.1f);
   Vertices[1].vTexCoord = XMFLOAT2(1.0f, 1.0f);

   Vertices[2].vPos = XMFLOAT3(-1.0f, 1.0f, 0.1f);
   Vertices[2].vTexCoord = XMFLOAT2(0.0f, 0.0f);

   Vertices[3].vPos = XMFLOAT3(1.0f, 1.0f, 0.1f);
   Vertices[3].vTexCoord = XMFLOAT2(1.0f, 0.0f);
   /* Initializing buffer */
   D3D11_BUFFER_DESC BufferDesc =
   {
       4 * sizeof(TerrainVertex),
       D3D11_USAGE_DEFAULT,
       D3D11_BIND_VERTEX_BUFFER,
       0, 0
   };
   D3D11_SUBRESOURCE_DATA BufferInitData;
   BufferInitData.pSysMem = Vertices;
   m_pD3DDevice->CreateBuffer(&BufferDesc, &BufferInitData, &m_pQuadVertexBuffer);

}

void Terrain::CreateInputLayout(void)
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

ID3D11ShaderResourceView* Terrain::HeightMapSRV(void)
{
   return m_pHeightMapSRV;
}

ID3D11ShaderResourceView* Terrain::LightMapSRV(void)
{
   return m_pLightMapSRV;
}

void Terrain::Render(void)
{
   if (GetGlobalStateManager().UseWireframe())
      GetGlobalStateManager().SetRasterizerState("EnableMSAACulling_Wire");
   else
      GetGlobalStateManager().SetRasterizerState("EnableMSAACulling");

   m_pD3DDeviceCtx->IASetInputLayout(m_pInputLayout);
   m_pD3DDeviceCtx->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
   m_pD3DDeviceCtx->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
   m_pD3DDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
   m_pPass->Apply(0, m_pD3DDeviceCtx);
   m_pSnowPass->Apply(0, m_pD3DDeviceCtx);
   m_pD3DDeviceCtx->DrawIndexed(m_uIndicesCount, 0, 0);
}


void Terrain::ApplyPass (void)
{
   m_pPass->Apply(0, m_pD3DDeviceCtx);
   m_pSnowPass->Apply(0, m_pD3DDeviceCtx);
}