#include <DDSTextureLoader.h>
#include <DirectXTex.h>

#include "GrassManager.h"
#include "StateManager.h"
#include "AxesFanFlow.h"


bool GrassInitState::ReadFromFile(const char* a_pFileName)
{
   ifstream ioInput;
   ioInput.open(a_pFileName);
   if (!ioInput.is_open())
      return false;
   ioInput >> dwBladesPerPatchSide;
   ioInput >> dwPatchesPerSide;
   ioInput >> fMostDetailedDist;
   ioInput >> fLastDetailedDist;
   ioInput >> fGrassRadius;
   ioInput.close();
   return true;
}

void GrassManager::LoadIndexData(void)
{
   /* Creating texture for binding to shader */
   ID3D11Resource* pRes = NULL;
   m_pIndexMapData.pData = NULL;
   m_pIndexTexSRV = NULL;
   ID3D11Texture2D* pStagingTex;
   ID3D11Texture2D* pSRTex;

   HRESULT hr;
   hr = CreateDDSTextureFromFile(m_GrassState.pD3DDevice, m_GrassState.sIndexTexPath.c_str(), &pRes, nullptr);
   if (hr != S_OK)
      return;

   D3D11_TEXTURE2D_DESC SRTexDesc;
   D3D11_SHADER_RESOURCE_VIEW_DESC IndexMapSRVDesc;
   pRes->QueryInterface(__uuidof(ID3D11Texture2D), (LPVOID*)& pSRTex);
   pSRTex->GetDesc(&SRTexDesc);
   ZeroMemory(&IndexMapSRVDesc, sizeof(IndexMapSRVDesc));
   IndexMapSRVDesc.Format = SRTexDesc.Format;
   IndexMapSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
   IndexMapSRVDesc.Texture2D.MostDetailedMip = 0;
   IndexMapSRVDesc.Texture2D.MipLevels = 1;
   V(m_GrassState.pD3DDevice->CreateShaderResourceView(pSRTex, &IndexMapSRVDesc, &m_pIndexTexSRV));
   //pESRV->SetResource(m_pHeightMapSRV);
   /* Creating texture for reading on CPU */
   SRTexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
   SRTexDesc.BindFlags = 0;
   SRTexDesc.Usage = D3D11_USAGE_STAGING;
   V(m_GrassState.pD3DDevice->CreateTexture2D(&SRTexDesc, 0, &pStagingTex));
   /* Copying data from one texture to another */
   m_GrassState.pD3DDeviceCtx->CopyResource(pStagingTex, pSRTex);
   D3D11_MAPPED_SUBRESOURCE MappedTexture;
   m_GrassState.pD3DDeviceCtx->Map(pStagingTex, D3D11CalcSubresource(0, 0, 1), D3D11_MAP_READ, 0, &MappedTexture);
   {
      UCHAR* pTexels = (UCHAR*)MappedTexture.pData;
      m_pIndexMapData.pData = new UCHAR[SRTexDesc.Height * SRTexDesc.Width];
      m_pIndexMapData.uHeight = SRTexDesc.Height;
      m_pIndexMapData.uWidth = SRTexDesc.Width;

      for (UINT row = 0; row < SRTexDesc.Height; row++)
      {
         UINT rowStart = row * MappedTexture.RowPitch;
         for (UINT col = 0; col < SRTexDesc.Width; col++)
         {
            UINT colStart = col;// * 3;//RGB
            m_pIndexMapData.pData[row * SRTexDesc.Width + col] = UCHAR((float)pTexels[rowStart + colStart] / 255.f * 9.0f);
         }
      }
   }
   m_GrassState.pD3DDeviceCtx->Unmap(pStagingTex, D3D11CalcSubresource(0, 0, 1));
   SAFE_RELEASE(pRes);
   SAFE_RELEASE(pSRTex);
   SAFE_RELEASE(pStagingTex);
}

GrassManager::GrassManager(GrassInitState& a_pInitState, GrassTracker* a_pGrassTracker, FlowManager* a_pFlowManager)
{
   m_GrassState = a_pInitState;
   ID3DBlob* pErrors;
   m_pEffect = NULL;
   m_pDiffuseTexSRV = NULL;
   m_pDiffuseTex = NULL;
   m_pTopDiffuseTexSRV = NULL;
   m_pTopDiffuseTex = NULL;
   m_pHeightData = NULL;
   m_pWindData = NULL;
   m_pRotatePattern = NULL;
   m_pLowGrassTexSRV = NULL;
   m_fHeightScale = 0.0f;
   m_pGrassTracker = a_pGrassTracker;
   m_pFlowManager = a_pFlowManager;
   HRESULT hr;

   D3DXLoadTextureArray(m_GrassState.pD3DDevice, m_GrassState.pD3DDeviceCtx, m_GrassState.sTexPaths, &m_pDiffuseTex, &m_pDiffuseTexSRV);
   D3DXLoadTextureArray(m_GrassState.pD3DDevice, m_GrassState.pD3DDeviceCtx, m_GrassState.sTopTexPaths, &m_pTopDiffuseTex, &m_pTopDiffuseTexSRV);

   hr = CreateDDSTextureFromFile(m_GrassState.pD3DDevice, m_GrassState.sLowGrassTexPath.c_str(), NULL, &m_pLowGrassTexSRV);
   m_bUseLowGrass = (hr == S_OK);

   LoadIndexData();

   ID3DBlob* pErrorBlob = nullptr;
   D3DX11CompileEffectFromFile(a_pInitState.sEffectPath.c_str(),
      0,
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG,
      0,
      m_GrassState.pD3DDevice,
      &m_pEffect,
      &pErrorBlob);

   if (pErrorBlob)
   {
      OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
      pErrorBlob->Release();
   }

   const D3D11_INPUT_ELEMENT_DESC TransformLayout[] =
   {
      { "POSITION"     , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA  , 0 },
      { "TEXCOORD"     , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA  , 0 },
      { "TEXCOORD"     , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA  , 0 },
      { "TEXCOORD"     , 2, DXGI_FORMAT_R32G32B32_FLOAT   , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA  , 0 },
      { "TRANSPARENCY" , 0, DXGI_FORMAT_R32_FLOAT         , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA  , 0 },
      { "mTransform"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_INSTANCE_DATA, 1 },
      { "mTransform"   , 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
      { "mTransform"   , 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
      { "mTransform"   , 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
      //{ "uOnEdge"      , 0, DXGI_FORMAT_R32_UINT          , 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
   };
   int iNumElements = sizeof(TransformLayout) / sizeof(D3D11_INPUT_ELEMENT_DESC);

   ID3DX11EffectTechnique* pRenderLod0Technique = m_pEffect->GetTechniqueByName("RenderGrass");
   m_pRenderPass = pRenderLod0Technique->GetPassByName("RenderLod0");
   m_pShadowPass = pRenderLod0Technique->GetPassByName("ShadowPass");
   m_pLowGrassDiffuseEVV = m_pEffect->GetVariableByName("g_vLowGrassDiffuse")->AsVector();
   if (!m_pLowGrassDiffuseEVV->IsValid())
      m_pLowGrassDiffuseEVV = NULL;
   if (m_bUseLowGrass)
   {
      m_pLowGrassPass = m_pEffect->GetTechniqueByName("RenderLowGrass")->GetPassByIndex(0);
      m_pLowGrassDiffuseEVV = m_pEffect->GetVariableByName("g_vLowGrassDiffuse")->AsVector();
   }
   else
      m_pLowGrassPass = NULL;

   D3DX11_PASS_DESC RenderLod0PassDesc;
   m_pRenderPass->GetDesc(&RenderLod0PassDesc);
   m_GrassState.pD3DDevice->CreateInputLayout(TransformLayout, iNumElements,
      RenderLod0PassDesc.pIAInputSignature, RenderLod0PassDesc.IAInputSignatureSize,
      &m_pInputLayout);
   ID3DX11EffectShaderResourceVariable* pESRV = m_pEffect->GetVariableByName("g_txGrassDiffuseArray")->AsShaderResource();
   pESRV->SetResource(m_pDiffuseTexSRV);
   pESRV = m_pEffect->GetVariableByName("g_txLowGrassDiffuse")->AsShaderResource();
   pESRV->SetResource(m_pLowGrassTexSRV);
   pESRV = m_pEffect->GetVariableByName("g_txIndexMap")->AsShaderResource();
   pESRV->SetResource(m_pIndexTexSRV);
   /* if tops needed, then load them */
   if (m_pTopDiffuseTexSRV)
   {
      pESRV = m_pEffect->GetVariableByName("g_txTopDiffuseArray")->AsShaderResource();
      pESRV->SetResource(m_pTopDiffuseTexSRV);
   }
   m_pGrassLodBiasESV = m_pEffect->GetVariableByName("g_fGrassLodBias")->AsScalar();
   m_pGrassSubScatterGammaESV = m_pEffect->GetVariableByName("g_fGrassSubScatterGamma")->AsScalar();
   m_pGrassAmbientESV = m_pEffect->GetVariableByName("g_fGrassAmbient")->AsScalar();
   m_pWindStrengthESV = m_pEffect->GetVariableByName("g_fWindStrength")->AsScalar();
   /* Initializing Lod ESVs */
   m_pMostDetailedDistESV = m_pEffect->GetVariableByName("g_fMostDetailedDist")->AsScalar();
   m_pLastDetailedDistESV = m_pEffect->GetVariableByName("g_fLastDetailedDist")->AsScalar();
   m_pGrassRadiusESV = m_pEffect->GetVariableByName("g_fGrassRadius")->AsScalar();
   m_pQualityESV = m_pEffect->GetVariableByName("g_fQuality")->AsScalar();
   m_pTrackViewProjEMV = m_pEffect->GetVariableByName("g_mTrackViewProj")->AsMatrix();
   m_pTrackMapSRV = m_pEffect->GetVariableByName("g_txTrackMap")->AsShaderResource();
   m_pTrackMapSRV->SetResource(m_pGrassTracker->GetTrackSRV());

   m_pAxesFanFlowESRV = m_pEffect->GetVariableByName("g_txAxesFanFlow")->AsShaderResource();

   ID3DX11EffectScalarVariable* pESV = m_pEffect->GetVariableByName("g_fMaxQuality")->AsScalar();
   pESV->SetFloat(m_GrassState.fMaxQuality);

   /*************************/
   Init();
}

void GrassManager::ReInitLods()
{
   DWORD dwCount = m_GrassState.dwPatchesPerSide * m_GrassState.dwPatchesPerSide;
   m_GrassLod[0]->SetTransformsCount(dwCount);                              //WARNING!!: too big pool 
   m_GrassLod[1]->SetTransformsCount(dwCount);
   m_GrassLod[2]->SetTransformsCount(dwCount);

   for (DWORD i = 0; i < GrassLodsCount; ++i)
      m_GrassLod[i]->GenTransformBuffer();
}

void GrassManager::Init()
{
   /* initializing lod0 */
   m_fPatchSize = m_GrassState.fGrassRadius * 2.0f / m_GrassState.dwPatchesPerSide;
   /* These patches will be destroyed inside ~GrassLod() */
   GrassPatchLod0* pGrassPatchLod0 = new GrassPatchLod0(m_GrassState.pD3DDevice,
      m_GrassState.pD3DDeviceCtx,
      m_fPatchSize,
      m_GrassState.dwBladesPerPatchSide);
   GrassPatch* pGrassPatchLod1 = new GrassPatchLod1(pGrassPatchLod0);
   GrassPatch* pGrassPatchLod2 = new GrassPatchLod2(pGrassPatchLod1);
   m_GrassLod[0] = new GrassLod(pGrassPatchLod0);
   m_GrassLod[1] = new GrassLod(pGrassPatchLod1);
   m_GrassLod[2] = new GrassLod(pGrassPatchLod2);
   ReInitLods();
   /* pools for physics */
//    m_GrassPool[0] = new GrassPool(m_GrassState.pD3DDevice, m_pInputLayout, m_pEffect, pGrassPatchLod0, 25, m_bUseLowGrass);
   m_GrassPool[0] = new GrassPool(m_GrassState.pD3DDevice, m_GrassState.pD3DDeviceCtx, m_pInputLayout, m_pEffect, pGrassPatchLod0, 35, m_bUseLowGrass);
   //m_GrassPool[1] = new GrassPool(m_GrassState.pD3DDevice, m_pEffect, pGrassPatchLod1, 100);
   //PhysPatch::fGrassRadius = m_GrassState.fGrassRadius;
   PhysPatch::fTerrRadius = m_GrassState.fTerrRadius;

   /* patch orientation pattern */
   m_uNumPatchesPerTerrSide = UINT(m_GrassState.fTerrRadius / m_fPatchSize);
   DWORD dwPatchCount = m_uNumPatchesPerTerrSide * m_uNumPatchesPerTerrSide;
   if (m_pRotatePattern)
      delete[] m_pRotatePattern;
   m_pRotatePattern = new DWORD[dwPatchCount];
   for (DWORD i = 0; i < dwPatchCount; i++)
   {
      m_pRotatePattern[i] = rand() % 4;
   }

   m_pMostDetailedDistESV->SetFloat(m_GrassState.fMostDetailedDist);
   m_pLastDetailedDistESV->SetFloat(m_GrassState.fLastDetailedDist);
   m_pGrassRadiusESV->SetFloat(m_GrassState.fGrassRadius);
   /*m_pGrassCollideStatic = new GrassCollideStatic(*pGrassPatchLod1, m_pEffect);
   m_pGrassCollideStatic->SetNumInstanses(m_GrassState.uNumCollidedPatchesPerMesh * m_GrassState.uMaxColliders);
   m_pGrassCollideStatic->GenTransformBuffer();*/
}

void GrassManager::Reinit(GrassInitState& a_pInitState)
{
   /* Releasing memory */
   for (DWORD i = 0; i < GrassLodsCount; ++i)
   {
      delete m_GrassLod[i];
   }
   delete m_GrassPool[0];
   //delete m_GrassPool[1];
   //delete m_pGrassCollideStatic;
   m_GrassState = a_pInitState;
   Init();
}

void GrassManager::SetGrassLodBias(float a_fGrassLodBias)
{
   m_fGrassLodBias = a_fGrassLodBias;
   m_pGrassLodBiasESV->SetFloat(m_fGrassLodBias);
   /* rebuilding lods using new bias value */
   GrassInitState GrassState = m_GrassState;
   Reinit(GrassState);
}

void GrassManager::SetSubScatterGamma(float a_fGrassSubScatterGamma)
{
   m_fGrassSubScatterGamma = a_fGrassSubScatterGamma;
   m_pGrassSubScatterGammaESV->SetFloat(m_fGrassSubScatterGamma);
}

void GrassManager::SetQuality(float a_fQuality)
{
   m_pQualityESV->SetFloat(a_fQuality);
   m_fQuality = a_fQuality;
}

void GrassManager::SetGrassAmbient(float a_fGrassAmbient)
{
   m_fGrassAmbient = a_fGrassAmbient;
   m_pGrassAmbientESV->SetFloat(m_fGrassAmbient);
}

void GrassManager::SetLowGrassDiffuse(float4& a_vValue)
{
   if (m_pLowGrassDiffuseEVV)
      m_pLowGrassDiffuseEVV->SetFloatVector((float*)& a_vValue);
}

void GrassManager::SetHeightDataPtr(const TerrainHeightData* a_pHeightData)
{
   m_pHeightData = a_pHeightData;
   PhysPatch::pHeightData = a_pHeightData;
}

void GrassManager::SetWindDataPtr(const WindData* a_pWindData)
{
   m_pWindData = a_pWindData;
   PhysPatch::pWindData = m_pWindData;
}

void GrassManager::SetHeightScale(float a_fHeightScale)
{
   m_fHeightScale = a_fHeightScale;
}

void GrassManager::Render(bool a_bShadowPass)
{
   if (m_pFlowManager)
      m_pAxesFanFlowESRV->SetResource(m_pFlowManager->GetFlowSRV());

   if (GetGlobalStateManager().UseWireframe())
      GetGlobalStateManager().SetRasterizerState("EnableMSAA_Wire");
   else
      GetGlobalStateManager().SetRasterizerState("EnableMSAA");

   /* Grass lods rendering */
   m_GrassState.pD3DDeviceCtx->IASetInputLayout(m_pInputLayout);
   m_GrassState.pD3DDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
   if (a_bShadowPass)
   {
      m_pShadowPass->Apply(0, m_GrassState.pD3DDeviceCtx);
      m_GrassLod[0]->IASetVertexBuffers();
      m_GrassState.pD3DDeviceCtx->DrawInstanced(m_GrassLod[0]->VerticesCount(), m_GrassLod[0]->GetTransformsCount(), 0, 0);
      if (m_bUseLowGrass)
      {
         m_pLowGrassPass->Apply(0, m_GrassState.pD3DDeviceCtx);
         m_GrassState.pD3DDeviceCtx->DrawInstanced(m_GrassLod[0]->VerticesCount(), m_GrassLod[0]->GetTransformsCount(), 0, 0);
         m_pAxesFanFlowESRV->SetResource(NULL);
         m_pLowGrassPass->Apply(0, m_GrassState.pD3DDeviceCtx);
      }

      m_pAxesFanFlowESRV->SetResource(NULL);
      m_pShadowPass->Apply(0, m_GrassState.pD3DDeviceCtx);
   }
   else
   {
      m_GrassLod[0]->IASetVertexBuffers();
      if (m_bUseLowGrass)
      {
         m_pLowGrassPass->Apply(0, m_GrassState.pD3DDeviceCtx);
         m_GrassState.pD3DDeviceCtx->DrawInstanced(m_GrassLod[0]->VerticesCount(), m_GrassLod[0]->GetTransformsCount(), 0, 0);
      }

      m_pRenderPass->Apply(0, m_GrassState.pD3DDeviceCtx);

      m_GrassState.pD3DDeviceCtx->DrawInstanced(m_GrassLod[0]->VerticesCount(), m_GrassLod[0]->GetTransformsCount(), 0, 0);
      m_GrassLod[1]->IASetVertexBuffers();
      m_GrassState.pD3DDeviceCtx->DrawInstanced(m_GrassLod[1]->VerticesCount(), m_GrassLod[1]->GetTransformsCount(), 0, 0);
      m_GrassLod[2]->IASetVertexBuffers();
      m_GrassState.pD3DDeviceCtx->DrawInstanced(m_GrassLod[2]->VerticesCount(), m_GrassLod[2]->GetTransformsCount(), 0, 0);

      m_pAxesFanFlowESRV->SetResource(NULL);
      m_pRenderPass->Apply(0, m_GrassState.pD3DDeviceCtx);
   }

   /* Grass physics rendering */
   m_GrassPool[0]->Render(a_bShadowPass);
}

bool GrassManager::IsPatchVisible(ConvexVolume& a_cvFrustum, XMVECTOR& a_vPatchPos)
{
   static AABB AABbox;

   AABbox.Set(getx(a_vPatchPos) - m_fPatchSize, getx(a_vPatchPos) + m_fPatchSize,
      gety(a_vPatchPos) - m_fPatchSize * 2.0f, gety(a_vPatchPos) + m_fPatchSize * 2.0f,
      getz(a_vPatchPos) - m_fPatchSize, getz(a_vPatchPos) + m_fPatchSize);
   return a_cvFrustum.IntersectBox(AABbox);
}

float GrassManager::GetPatchHeight(UINT a_uX, UINT a_uY)
{
   if (m_pHeightData == NULL)
      return 0.0f;

   /* Getting UV-coordinates of patch mid-point (a_vPatchPos) */
   return m_pHeightData->pData[m_pHeightData->uWidth * a_uY + a_uX] * m_fHeightScale;
}

float GrassManager::LodAlphaOffset(const XMVECTOR& a_vCamPos, const XMVECTOR& a_vPatchPos, const float a_fDist, const float a_fIsCorner)
{
   float fLerpCoef1 = (a_fDist + 0.1f) / m_GrassState.fGrassRadius;
   fLerpCoef1 *= fLerpCoef1;
   UINT uX = (UINT)(((getx(a_vPatchPos) / m_GrassState.fTerrRadius) * 0.5f + 0.5f) * m_pHeightData->fWidth);
   UINT uY = (UINT)(((getz(a_vPatchPos) / m_GrassState.fTerrRadius) * 0.5f + 0.5f) * m_pHeightData->fHeight);
   XMFLOAT3 vNormal = m_pHeightData->pNormals[m_pHeightData->uWidth * uY + uX];

   XM_TO_V(vNormal, normal, 3);

   XMVECTOR vV = a_vCamPos - a_vPatchPos;

   float tmp = 0.43f; //= 0.44 + 0.1 * vV.y/(5.0 + abs(vV.y));
   if (gety(vV) < 0.0f)
      tmp += 0.1f * gety(vV) / (5.0f + abs(gety(vV)));
   //   float tmp = 0.44 + 0.1 * vV.y/(5.0 + fabs(vV.y));
   vV = normalize(vV);
   float fDot = dot(vV, normal);
   tmp = tmp - fDot;
   float t = 1.f - fDot;
   float h = gety(a_vCamPos);

   if (h < 17.0f)
      h = 0.0f;
   else
      h = (h - 17.0f) / 50.0f;

   if ((gety(a_vPatchPos) > 6.0f) && (t > 0.92f))
      return (0.2f + h);
   else
      return (tmp * (1.0f + 3.0f * fLerpCoef1) + h);
}

void GrassManager::Update(float4x4& a_mViewProj, float3 a_vCamPos, Mesh* a_pMeshes[], UINT a_uNumMeshes, float a_fElapsedTime)
{
   float physLodDst = m_GrassState.fGrassRadius * 0.2f;
   float fMaxDist = m_GrassState.fGrassRadius;
   static ConvexVolume cvFrustum;
   cvFrustum.BuildFrustum(a_mViewProj);
   float fHalfSize = m_fPatchSize * 0.5f;

   float fCamOffsetX = INT(getx(a_vCamPos) / m_fPatchSize) * m_fPatchSize;
   float fCamOffsetZ = INT(getz(a_vCamPos) / m_fPatchSize) * m_fPatchSize;
   XMVECTOR vPatchPos = create(-fMaxDist + fHalfSize + fCamOffsetX, 0.0f, -fMaxDist + fHalfSize + fCamOffsetZ);

   UINT uX = 0, uY = 0;
   float fX, fY;

   float fDX, fDY;
   fDX = (m_fPatchSize / m_GrassState.fTerrRadius) * 0.25f;
   fDY = (m_fPatchSize / m_GrassState.fTerrRadius) * 0.25f;
   int iDotSign[2] = { -1, 1 };
   bool bOnEdge = false;
   XMVECTOR vDist;

   XMVECTOR vSpherePos;
   float fRadius;
   float fDist;
   DWORD i = 0;
   DWORD j = 0;
   DWORD k = 0; // :) 
   DWORD dwRotMtxIndex = 0;
   INT32 dwLodIndex;
   DWORD dwStaticInstanseInd = -1;
   bool  bCollided = false;
   DWORD dwLodTransformsIndex[GrassLodsCount] = { -1, -1, -1 };
   /* Patch orientation matrices */
   XMMATRIX mRotationMatrices[4];
   XMMATRIX mTranslateMtx;
   XMMATRIX mCurTransform;
   float fAngle = 0.0f;

   /* setting up maximum capacity */
   DWORD dwCount = m_GrassState.dwPatchesPerSide * m_GrassState.dwPatchesPerSide;
   m_GrassLod[0]->SetTransformsCount(dwCount);
   m_GrassLod[1]->SetTransformsCount(dwCount);
   m_GrassLod[2]->SetTransformsCount(dwCount);
   /* clear dead patches */
   m_GrassPool[0]->ClearDeadPatches(a_fElapsedTime);
   XMFLOAT3 vCornerNormals[4];

   for (i = 0; i < 4; ++i)
   {
      mRotationMatrices[i] = XMMatrixRotationY(fAngle);
      fAngle += M_PI / 2.0f;
   }

   for (i = 0; i < m_GrassState.dwPatchesPerSide; ++i)
   {
      setz(vPatchPos, -fMaxDist + fHalfSize + fCamOffsetZ);
      if (getx(vPatchPos) > m_GrassState.fTerrRadius)
      {
         break;
      }
      if (-getx(vPatchPos) > m_GrassState.fTerrRadius)
      {
         setx(vPatchPos, getx(vPatchPos) + m_fPatchSize);
         continue;
      }
      for (j = 0; j < m_GrassState.dwPatchesPerSide; ++j)
      {
         if (getz(vPatchPos) > m_GrassState.fTerrRadius)
            break;
         if (-getz(vPatchPos) > m_GrassState.fTerrRadius)
         {
            setz(vPatchPos, getz(vPatchPos) + m_fPatchSize);
            continue;
         }

         if (!IsPatchVisible(cvFrustum, vPatchPos))
         {
            int ind = m_GrassPool[0]->GetPatchIndex(vPatchPos);
            if (ind != NO_VALUE)
               m_GrassPool[0]->FreePatch(ind);

            setz(vPatchPos, getz(vPatchPos) + m_fPatchSize);
            continue;
         }
         /* Height map coords */
         fX = (getx(vPatchPos) / m_GrassState.fTerrRadius) * 0.5f + 0.5f;
         fY = (getz(vPatchPos) / m_GrassState.fTerrRadius) * 0.5f + 0.5f;
         uX = (UINT)((fX)* m_pHeightData->fWidth);
         uY = (UINT)((fY)* m_pHeightData->fHeight);

         sety(vPatchPos, m_pHeightData->pData[m_pHeightData->uWidth * uY + uX] * m_fHeightScale);
         vDist = a_vCamPos - vPatchPos;
         fDist = length(vDist);

         uX = (UINT)(((getx(vPatchPos) / m_GrassState.fTerrRadius) * 0.5f + 0.5f) * (m_uNumPatchesPerTerrSide - 1));
         uY = (UINT)(((getz(vPatchPos) / m_GrassState.fTerrRadius) * 0.5f + 0.5f) * (m_uNumPatchesPerTerrSide - 1));
         dwRotMtxIndex = m_pRotatePattern[uY * m_uNumPatchesPerTerrSide + uX];
         /* Calculating matrix */
         mTranslateMtx = XMMatrixTranslation(getx(vPatchPos), 0.0f, getz(vPatchPos)); // Position on plane, not on a terrain!
         mCurTransform = XMMatrixMultiply(mRotationMatrices[dwRotMtxIndex], mTranslateMtx);

         /* Lods */
         dwLodIndex = 0;
         fDist -= fHalfSize;//farthest patch point
         float fAlphaOffs = LodAlphaOffset(a_vCamPos, vPatchPos, fDist, bOnEdge);
         if (fAlphaOffs > 0.34f)
            dwLodIndex++;
         if (fAlphaOffs > 0.66f)
            dwLodIndex++;

         /* Determining collision */
         sety(vPatchPos, 0.0f); //All collisions ON A PLANE, NOT ON A TERRAIN
         int iLodPatchInd = m_GrassPool[0]->GetPatchIndex(vPatchPos);

         if (fDist < physLodDst)
         {
            for (k = 0; k < a_uNumMeshes; k++)
            {
               vSpherePos = create(a_pMeshes[k]->GetPosAndRadius().x, 0.0f, a_pMeshes[k]->GetPosAndRadius().z);

               fRadius = a_pMeshes[k]->GetPosAndRadius().w;
               vDist = vPatchPos - vSpherePos;
               fDist = length(vDist);
               if (fDist < fRadius * 1.1f + m_fPatchSize / 2.0f)
               {
                  bCollided = true;
                  m_GrassPool[0]->TakePatch(mCurTransform, PhysPatch::maxBrokenTime * 4.0f, k);
                  iLodPatchInd = m_GrassPool[0]->GetPatchIndex(vPatchPos);
                  if (iLodPatchInd != NO_VALUE)
                     m_GrassPool[0]->SetPatchVisibility(iLodPatchInd, true);
                  break;
               }
            }
         }

         if (iLodPatchInd == NO_VALUE) {
            M_TO_XM(mCurTransform, m);
            m_GrassLod[dwLodIndex]->AddTransform(m, fDist, bOnEdge, ++dwLodTransformsIndex[dwLodIndex]);
         }
         /* Updating parameters */
         setz(vPatchPos, getz(vPatchPos) + m_fPatchSize);
      }
      setx(vPatchPos, getx(vPatchPos) + m_fPatchSize);
   }

   for (i = 0; i < GrassLodsCount; i++)
   {
      dwLodTransformsIndex[i]++;
      m_GrassLod[i]->SetTransformsCount(dwLodTransformsIndex[i]);
      m_GrassLod[i]->UpdateTransformBuffer();
   }

   /* updating physics */
   m_GrassPool[0]->Update(a_vCamPos, m_GrassState.fCameraMeshDist, a_fElapsedTime, a_pMeshes, a_uNumMeshes, m_SubTypeProps, m_pIndexMapData);

   m_pTrackViewProjEMV->SetMatrix((float*)& m_pGrassTracker->GetViewProjMatrix());
}


ID3DX11Effect* GrassManager::GetEffect(void)
{
   return m_pEffect;
}


void GrassManager::AddSubType(const GrassPropsUnified& a_SubTypeData)
{
   m_SubTypeProps.push_back(a_SubTypeData);
}


void GrassManager::ClearSubTypes(void)
{
   m_SubTypeProps.clear();
}


GrassManager::~GrassManager(void)
{
   delete[] m_pRotatePattern;
   for (DWORD i = 0; i < GrassLodsCount; ++i)
   {
      delete m_GrassLod[i];
   }
   delete m_GrassPool[0];

   SAFE_RELEASE(m_pInputLayout);
   SAFE_RELEASE(m_pEffect);
   SAFE_RELEASE(m_pDiffuseTex);
   SAFE_RELEASE(m_pLowGrassTexSRV);
   SAFE_RELEASE(m_pDiffuseTexSRV);
   SAFE_RELEASE(m_pTopDiffuseTex);
   SAFE_RELEASE(m_pTopDiffuseTexSRV);
   SAFE_RELEASE(m_pIndexTexSRV);
}


void GrassManager::ClearGrassPools(void)
{
   for (int i = 0; i < m_GrassPool[0]->GetPatchCount(); i++)
      m_GrassPool[0]->FreePatch(i);
}
