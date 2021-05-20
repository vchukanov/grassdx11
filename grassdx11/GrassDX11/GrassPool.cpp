#include "GrassPool.h"
//#include <omp.h>

GrassPatchExt::GrassPatchExt (GrassPatch* a_pPatch)
   : Patch(a_pPatch)
{
   fLifeTime = 0.0f;
   pNext = NULL;
   pPrev = NULL;
   bIsDead = true;
}

GrassPool::GrassPool (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3D11InputLayout* a_pAnimInputLayout,
   ID3DX11Effect* a_pEffect, GrassPatch* a_pBasePatch, int a_iPatchCount, bool a_bUseLowGrass)
{
   m_pAnimInputLayout = a_pAnimInputLayout;
   m_pD3DDevice = a_pD3DDevice;
   m_pD3DDeviceCtx = a_pD3DDeviceCtx;
   m_bUseLowGrass = a_bUseLowGrass;
   m_pRenderAnimPass = a_pEffect->GetTechniqueByName("RenderGrass")->GetPassByName("RenderAnimPass");
   m_pRenderPhysPass = a_pEffect->GetTechniqueByName("RenderGrass")->GetPassByName("RenderPhysPass");
   m_pShadowPass = a_pEffect->GetTechniqueByName("RenderGrass")->GetPassByName("ShadowPhysicsPass");
   if (m_bUseLowGrass)
   {
      m_pLowGrassPhysPass = a_pEffect->GetTechniqueByName("RenderLowGrass")->GetPassByName("PhysLowGrass");
      m_pLowGrassAnimPass = a_pEffect->GetTechniqueByName("RenderLowGrass")->GetPassByName("AnimLowGrass");
   }

   const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc[] =
   {
      { "R0MTX"         , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0 ,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "R0MTX"         , 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "R0MTX"         , 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "R0MTX"         , 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "R1MTX"         , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "R1MTX"         , 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 80,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "R1MTX"         , 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 96,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "R1MTX"         , 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 112, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "R2MTX"         , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 128, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "R2MTX"         , 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 144, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "R2MTX"         , 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 160, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "R2MTX"         , 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 176, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "POSITION"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 192, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TRANSPARENCY"  , 0, DXGI_FORMAT_R32_FLOAT         , 0, 204, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "BLADECOLOR"    , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 208, D3D11_INPUT_PER_VERTEX_DATA, 0 },
   };
   int iNumElements = sizeof(InputLayoutDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);

   D3DX11_PASS_DESC PassDesc;
   m_pRenderPhysPass->GetDesc(&PassDesc);
   m_pD3DDevice->CreateInputLayout(InputLayoutDesc, iNumElements,
      PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize,
      &m_pPhysInputLayout);

   m_pPatches = new GrassPatchExt * [a_iPatchCount];
   m_iPatchCount = a_iPatchCount;
   /* list of free space */
   int i;
   for (i = 0; i < m_iPatchCount; i++)
      m_pPatches[i] = new GrassPatchExt(a_pBasePatch);

   for (i = 0; i < m_iPatchCount; i++)
   {
      if (i != 0)
      {
         m_pPatches[i]->pPrev = m_pPatches[i - 1];
      }
      else
      {
         m_pPatches[i]->pPrev = NULL;
      }
      if (i != m_iPatchCount - 1)
      {
         m_pPatches[i]->pNext = m_pPatches[i + 1];
      }
      else
      {
         m_pPatches[i]->pNext = NULL;
      }
   }
   m_pFirst = m_pPatches[0];
   m_pLast = m_pPatches[m_iPatchCount - 1];
}

GrassPool::~GrassPool()
{
   for (int i = 0; i < m_iPatchCount; i++)
   {
      delete m_pPatches[i];
   }
   delete[] m_pPatches;
   SAFE_RELEASE(m_pPhysInputLayout);
}

int GrassPool::GetPatchCount (void)
{
   return m_iPatchCount;
}

XMVECTOR GrassPool::GetPatchPos (int a_iPatchIndex)
{
   return create(getx(m_pPatches[a_iPatchIndex]->Transform.r[4]),
              gety(m_pPatches[a_iPatchIndex]->Transform.r[4]),
              getz(m_pPatches[a_iPatchIndex]->Transform.r[4]));
}

const PhysPatch& GrassPool::GetPatch (int a_iPatchIndex)
{
   return m_pPatches[a_iPatchIndex]->Patch;
}


float GrassPool::GetLifeTime (int a_iPatchIndex)
{
   return m_pPatches[a_iPatchIndex]->fLifeTime;
}

bool GrassPool::FreePatch (XMVECTOR a_vPatchPos)
{
   /* looking for a match */
   int i;
   for (i = 0; i < m_iPatchCount; i++)
   {
      /* skipping dead patches */
      //if (m_pPatches[i]->fLifeTime == 0.0f)
      //    continue;
      if (m_pPatches[i]->bIsDead)
         continue;
      if (XMVector3Alike(GetPatchPos(i), a_vPatchPos))
      {
         /* match found, returning true and killing patch */
         m_pPatches[i]->bIsDead = true;
         m_pPatches[i]->inFirstLod = false;
         return true;
      }
   }
   return false;
}

bool GrassPool::FreePatch (int a_iPatchIndex)
{
   if ((a_iPatchIndex > m_iPatchCount) || (a_iPatchIndex < 0))
      return false;
   m_pPatches[a_iPatchIndex]->bIsDead = true;
   m_pPatches[a_iPatchIndex]->inFirstLod = false;
   return true;
}

int GrassPool::GetPatchIndex (XMVECTOR a_vPatchPos)
{
   /* looking for a match */
   int i;
   for (i = 0; i < m_iPatchCount; i++)
   {
      /* skipping dead patches */
      if (m_pPatches[i]->bIsDead)
         continue;

      if (XMVector3Alike(GetPatchPos(i), a_vPatchPos))
      {
         /* match found */
         return i;
      }
   }
   return NO_VALUE;
}

void GrassPool::SetPatchVisibility (int a_iPatchIndex, bool isVisible)
{
   m_pPatches[a_iPatchIndex]->isVisible = isVisible;
}

UINT GrassPool::GetMeshIndex (int a_iPatchIndex)
{
   return m_pPatches[a_iPatchIndex]->uMeshIndex;
}

bool GrassPool::TakePatch (XMMATRIX a_mTransform, float a_fLifeTime, UINT a_uMeshIndex)
{
   float3 vNewPos = create(getx(a_mTransform.r[4]),
      gety(a_mTransform.r[4]),
      getz(a_mTransform.r[4]));

   /* looking for a match */
   int i;
   for (i = 0; i < m_iPatchCount; i++)
   {
      /* skipping dead patches */
      if (m_pPatches[i]->bIsDead)
         continue;
      if (XMVector3Alike(GetPatchPos(i), vNewPos))
      {
         /* match found, returning true */
         m_pPatches[i]->fLifeTime = a_fLifeTime;
         m_pPatches[i]->uMeshIndex = a_uMeshIndex;
         return true;
      }
   }
   /* looking for a dead patch otherwise*/
   GrassPatchExt* pPatch = m_pFirst;

   if (pPatch)
   {
      pPatch->Transform = a_mTransform;
      pPatch->Patch.SetTransform(&pPatch->Transform);
      pPatch->fLifeTime = a_fLifeTime;
      pPatch->uMeshIndex = a_uMeshIndex;
      pPatch->inFirstLod = false;
      pPatch->bIsDead = false;
      pPatch->Patch.Reinit();
      m_pFirst = m_pFirst->pNext;
   }
   else
      return false;


   return true;
}

bool GrassPool::TakePatch (XMMATRIX a_mTransform, bool a_bInFirstLod)
{
   float3 vNewPos = create(getx(a_mTransform.r[4]),
      gety(a_mTransform.r[4]),
      getz(a_mTransform.r[4]));

   /* looking for a match */
   int i;
   for (i = 0; i < m_iPatchCount; i++)
   {
      /* skipping dead patches */
      if (m_pPatches[i]->bIsDead)
         continue;
      if (XMVector3Alike(GetPatchPos(i), vNewPos))
      {
         /* match found, returning true */
         m_pPatches[i]->inFirstLod = a_bInFirstLod;
         if (!a_bInFirstLod && m_pPatches[i]->fLifeTime <= 0.0f)
         {
            m_pPatches[i]->bIsDead = true;
            return false;
         }
         return true;
      }
   }

   //not found and not in phys lod
   if (!a_bInFirstLod)
      return false;

   //not found and in phys lod and without collision
   GrassPatchExt* pPatch = m_pFirst;

   if (pPatch)
   {
      pPatch->Transform = a_mTransform;
      pPatch->Patch.SetTransform(&pPatch->Transform);
      pPatch->fLifeTime = 0.0f;
      pPatch->uMeshIndex = -1;
      pPatch->inFirstLod = a_bInFirstLod;
      pPatch->bIsDead = false;
      pPatch->Patch.Reinit();
      m_pFirst = m_pFirst->pNext;
      return true;
   }

   return false;
}

void GrassPool::ClearDeadPatches (float a_fElapsedTime)
{
   int i;
   m_pFirst = NULL;
   m_pLast = NULL;

   for (i = 0; i < m_iPatchCount; i++)
   {
      /* killing patches */
      if (!m_pPatches[i]->bIsDead)
      {
         if (((m_pPatches[i]->fLifeTime - a_fElapsedTime) <= 0.0f || m_pPatches[i]->uMeshIndex == -1) &&
            !m_pPatches[i]->inFirstLod)
         {
            m_pPatches[i]->bIsDead = true;
            m_pPatches[i]->fLifeTime = 0.0f;
            //m_pPatches[i]->Patch.Reinit();
            m_pPatches[i]->Patch.SetTransform(NULL);
         }
      }

      /* constructing list of dead patches */
      if (m_pPatches[i]->bIsDead)
      {
         if (m_pFirst == NULL)
         {
            m_pLast = m_pFirst = m_pPatches[i];
            m_pPatches[i]->pNext = NULL;
         }
         else
         {
            m_pLast->pNext = m_pPatches[i];
            m_pPatches[i]->pNext = NULL;
            m_pLast = m_pPatches[i];
         }
      }
   }
}

UINT PhysPatch::uTickCount = 0;
static int MaxPatchCount = 0;
static int NPatchs1[32];
static int NPatchs2[32];
static int j1_ = 0, j2_ = 0, m_ = 0;

void GrassPool::Update (const float3& viewPos, float physLodDst, float a_fElapsedTime, Mesh* a_pMeshes[], UINT a_uNumMeshes, const std::vector<GrassPropsUnified>& grassProps, const IndexMapData& indexMapData)
{
   j1_ = 0;
   j2_ = 0;
   m_ = 0;
   for (int i = 0; i < m_iPatchCount; i++)
   {
      if (!m_pPatches[i]->bIsDead)
      {
         if (m_ == 0)
         {
            NPatchs1[j1_] = i;
            j1_++;
            m_ = 1;
         }
         else
         {
            NPatchs2[j2_] = i;
            j2_++;
            m_ = 0;
         }
      }
   }

#pragma omp parallel sections num_threads(2)
   {
#pragma omp section
      {
         int i, PatchCount = 0;
         //         for (int i = 0; i < m_iPatchCount; i += 2)
         for (int k = 0; k < j1_; k++)
         {
            i = NPatchs1[k];
            if (m_pPatches[i]->bIsDead)
               continue;
            PatchCount++;
            if (PatchCount > MaxPatchCount)
               MaxPatchCount = PatchCount;
            //out << "Num=" << i << "\n";
            float3 sp;
            float sr = 0.0f;
            bool collision = (m_pPatches[i]->uMeshIndex != -1 && a_uNumMeshes > 0);
            if (collision)
            {
               XMFLOAT4 sphere = a_pMeshes[m_pPatches[i]->uMeshIndex]->GetPosAndRadius();
               sr = sphere.w;
               sp = create(sphere.x, sphere.y, sphere.z, sphere.w);
            }

            //
            if (m_pPatches[i]->isVisible)
               m_pPatches[i]->Patch.UpdatePhysics(viewPos, physLodDst, collision, sp, sr, a_fElapsedTime, grassProps, indexMapData, a_pMeshes, a_uNumMeshes);
            m_pPatches[i]->isVisible = true;

            m_pPatches[i]->fLifeTime -= a_fElapsedTime;
         }
      }

#pragma omp section
      {
         int i, PatchCount = 0;
         for (int k = 0; k < j2_; k++)
         {
            i = NPatchs2[k];
            if (m_pPatches[i]->bIsDead)
               continue;

            //out << "Num=" << i << "\n";
            float3 sp;
            float sr = 0.0f;
            bool collision = (m_pPatches[i]->uMeshIndex != -1 && a_uNumMeshes > 0);
            if (collision)
            {
               XMFLOAT4 sphere = a_pMeshes[m_pPatches[i]->uMeshIndex]->GetPosAndRadius();
               sr = sphere.w;
               sp = create(sphere.x, sphere.y, sphere.z, sphere.w);
            }

            //
            if (m_pPatches[i]->isVisible)
               m_pPatches[i]->Patch.UpdatePhysics(viewPos, physLodDst, collision, sp, sr, a_fElapsedTime, grassProps, indexMapData, a_pMeshes, a_uNumMeshes);
            m_pPatches[i]->isVisible = true;

            m_pPatches[i]->fLifeTime -= a_fElapsedTime;
         }
      }
   }

   //out.close();
/*__asm
{
   rdtsc
   mov     uEnd, EAX
}
PhysPatch::uTickCount = uEnd - uStart;*/
}

void GrassPool::Render(bool a_bShadowPass)
{
   int i;
   m_pD3DDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
   /* Only physics */
   m_pD3DDeviceCtx->IASetInputLayout(m_pPhysInputLayout);
   if (m_bUseLowGrass)
   {
      m_pLowGrassPhysPass->Apply(0, m_pD3DDeviceCtx);
      for (i = 0; i < m_iPatchCount; i++)
      {
         if (!m_pPatches[i]->bIsDead)
         {
            m_pPatches[i]->Patch.IASetPhysVertexBuffer0();
            m_pD3DDeviceCtx->Draw(m_pPatches[i]->Patch.PhysVerticesCount(), 0);
         }
      }
   }
   m_pRenderPhysPass->Apply(0, m_pD3DDeviceCtx);
   for (i = 0; i < m_iPatchCount; i++)
   {
      if (!m_pPatches[i]->bIsDead)
      {
         m_pPatches[i]->Patch.IASetPhysVertexBuffer0();
         m_pD3DDeviceCtx->Draw(m_pPatches[i]->Patch.PhysVerticesCount(), 0);
      }
   }
   /* Only animations */

   m_pD3DDeviceCtx->IASetInputLayout(m_pAnimInputLayout);
   if (m_bUseLowGrass)
   {
      m_pLowGrassAnimPass->Apply(0, m_pD3DDeviceCtx);
      for (i = 0; i < m_iPatchCount; i++)
      {
         if (!m_pPatches[i]->bIsDead)
         {
            m_pPatches[i]->Patch.IASetAnimVertexBuffer0();
            m_pD3DDeviceCtx->Draw(m_pPatches[i]->Patch.AnimVerticesCount(), 0);
         }
      }
   }

   m_pRenderAnimPass->Apply(0, m_pD3DDeviceCtx);
   for (i = 0; i < m_iPatchCount; i++)
   {
      if (!m_pPatches[i]->bIsDead)
      {
         m_pPatches[i]->Patch.IASetAnimVertexBuffer0();
         m_pD3DDeviceCtx->Draw(m_pPatches[i]->Patch.AnimVerticesCount(), 0);
      }
   }
}