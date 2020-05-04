#include "plane.h"

#include <xmmintrin.h>
#include <cmath>
#include <DDSTextureLoader.h>


Plane::Plane (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect* a_pEffect, XMFLOAT4& a_vPosAndRadius,
   float a_fWidth, float a_fHeight, float a_fWheelBeg, float a_fWheelEnd) :
   m_fWidth(a_fWidth), m_fHeight(a_fHeight)
{
   m_pD3DDevice    = a_pD3DDevice;
   m_pD3DDeviceCtx = a_pD3DDeviceCtx;
   m_vPosAndRadius = a_vPosAndRadius;
   m_uVertexStride = sizeof(MeshVertex);
   m_uIndexStride  = sizeof(UINT32);
   m_uVertexOffset = 0;
   m_uIndexOffset  = 0;
   m_pIndexBuffer  = NULL;
   m_fWheelBeg = a_fWheelBeg;
   m_fWheelEnd = a_fWheelEnd;

   // Render if we need 
   ID3DX11EffectTechnique* pTechnique = a_pEffect->GetTechniqueByIndex(0);
   m_pPass = pTechnique->GetPassByName("RenderMeshPassDbg");

   m_pTransformEMV = a_pEffect->GetVariableByName("g_mWorld")->AsMatrix();
  // m_pTextureESRV = a_pEffect->GetVariableByName("g_txMeshDiffuse")->AsShaderResource();
  // CreateDDSTextureFromFile(m_pD3DDevice, L"resources/stone.dds", nullptr, &m_pTextureSRV);
   
  // D3D11_SHADER_RESOURCE_VIEW_DESC desc;
  // m_pTextureSRV->GetDesc(&desc);

  // m_pTextureESRV->SetResource(m_pTextureSRV);


   XMMATRIX newTransform = XMMatrixTranslation(a_vPosAndRadius.x, a_vPosAndRadius.y, a_vPosAndRadius.z);
   XMStoreFloat4x4(&m_mTransform, newTransform);

   CreateVertexBuffer();
   CreateInputLayout();

   XMStoreFloat4x4(&m_mTranslation, XMMatrixIdentity());
   XMStoreFloat4x4(&m_mRotation,    XMMatrixIdentity());
}

void Plane::CreateVertexBuffer (void)
{
   MeshVertex Vertices[4];

   Vertices[3].vPos = XMFLOAT3(-m_fWidth, -m_fHeight, 0.0f);
   Vertices[3].vTexCoord = XMFLOAT2(0.0f, 1.0f);
   Vertices[2].vPos = XMFLOAT3(m_fWidth, -m_fHeight, 0.0f);
   Vertices[2].vTexCoord = XMFLOAT2(1.0f, 1.0f);
   Vertices[1].vPos = XMFLOAT3(-m_fWidth, m_fHeight, 0.0f);
   Vertices[1].vTexCoord = XMFLOAT2(0.0f, 0.0f);
   Vertices[0].vPos = XMFLOAT3(m_fWidth, m_fHeight, 0.0f);
   Vertices[0].vTexCoord = XMFLOAT2(1.0f, 0.0f);

   D3D11_BUFFER_DESC BufferDesc =
   {
      4 * sizeof(MeshVertex),
      D3D11_USAGE_DEFAULT,
      D3D11_BIND_VERTEX_BUFFER,
      0, 0
   };
   D3D11_SUBRESOURCE_DATA BufferInitData;
   BufferInitData.pSysMem = Vertices;
   BufferInitData.SysMemPitch = 0;
   BufferInitData.SysMemSlicePitch = 0;
   m_pD3DDevice->CreateBuffer(&BufferDesc, &BufferInitData, &m_pVertexBuffer);
}

void Plane::CreateInputLayout (void)
{
   D3D11_INPUT_ELEMENT_DESC layout[] =
   {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0},
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
      { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
   };

   // Create the input layout
   D3DX11_PASS_DESC PassDesc;
   m_pPass->GetDesc(&PassDesc);
   m_pD3DDevice->CreateInputLayout(layout, 3, PassDesc.pIAInputSignature,
      PassDesc.IAInputSignatureSize, &m_pInputLayout);
}

void Plane::Render(void)
{
  m_pTransformEMV->SetMatrix((float*)&m_mTransform);
  
  m_pD3DDeviceCtx->IASetInputLayout(m_pInputLayout);
  m_pD3DDeviceCtx->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
  m_pD3DDeviceCtx->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  
  m_pPass->Apply(0, m_pD3DDeviceCtx);
  m_pD3DDeviceCtx->Draw(4, 0);
}


XMFLOAT4 Plane::GetPosAndRadius (void)
{
   if (m_fWidth > m_fHeight)
      return XMFLOAT4(m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z, m_fWidth);
   else
      return XMFLOAT4(m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z, m_fHeight);
}


void Plane::SetPosAndRadius(XMFLOAT4& a_vPosAndRadius)
{
   m_vPosAndRadius = a_vPosAndRadius;
}


void Plane::SetHeight(float a_fH)
{
   ;
}


float Plane::GetDist(XMVECTOR& Pnt, bool* IsUnderWheel)
{
   XMVECTOR loc_coord;
   XM_TO_M(m_mInvTransform, invTr);
   loc_coord = XMVector3Transform(Pnt, invTr);
   
   if (fabs(getx(loc_coord)) > m_fWidth)
      return INVALID_DIST;

   if (IsUnderWheel != NULL)
      if (fabs(getx(loc_coord)) >= m_fWheelBeg &&
         fabs(getx(loc_coord)) <= m_fWheelEnd)
         * IsUnderWheel = true;

   return getz(loc_coord);
}

void Plane::RotateToEdge(XMVECTOR* Ret, XMVECTOR& Beg, XMVECTOR& End)
{
   XMVECTOR loc_beg, loc_end, loc_pos;

   /* Convert to model coordinate system */
   XM_TO_M(m_mInvTransform, invTr);
   loc_beg = XMVector3TransformCoord(Beg, invTr);
   loc_end = XMVector3TransformCoord(End, invTr);
   
   /* Calculate vectors */
   XMVECTOR dir = loc_end - loc_beg, dest_dir;

   dest_dir = create(getx(loc_beg), -m_fHeight, 0) - loc_beg;

   dir = XMVector3Normalize(dir);
   dest_dir = XMVector3Normalize(dest_dir);

   /* Calculate angle */
   float angle = getx(XMVector3Dot(dir, dest_dir));

   if (angle > 1)
      angle = 1;
   if (angle < -1)
      angle = -1;
   angle = acosf(angle);

   /* Calculate rotation axis */
   XMVECTOR psi;
   psi = XMVector3Cross(dir, dest_dir);
   psi = XMVector3Normalize(psi);

   /* Convert to back world space */
   XMVECTOR beg1 = create(0, 0, 0), beg2 = psi;

   XM_TO_M(m_mTransform, tr);
   beg1 = XMVector3TransformCoord(beg1, tr);
   beg2 = XMVector3TransformCoord(beg2, tr);
   *Ret = beg2 - beg1;

   *Ret = XMVector3Normalize(*Ret);
   *Ret = (*Ret) * angle;
}


void Plane::SetTransform (XMFLOAT4X4& a_mTransform)
{
   XM_TO_M(a_mTransform, tr);
   m_mTransform = a_mTransform;
   tr = XMMatrixInverse(NULL, tr);
   XMStoreFloat4x4(&m_mInvTransform, tr);
}


void Plane::SetInvTransform (XMFLOAT4X4& a_mInvTransform)
{
   XM_TO_M(a_mInvTransform, tr);
   m_mInvTransform = a_mInvTransform;
   tr = XMMatrixInverse(NULL, tr);
   XMStoreFloat4x4(&m_mTransform, tr);
}

bool Plane::CheckCollision(XMVECTOR& Beg, XMVECTOR& End, float* Dist)
{
   XMVECTOR loc_beg, loc_end;

   if (Dist != NULL)
      * Dist = INVALID_DIST;

   // In model coordinate system
   XM_TO_M(m_mInvTransform, m);
   loc_beg = XMVector3TransformCoord(Beg, m);
   loc_end = XMVector3TransformCoord(End, m);
   /*
      if (loc_end.z > 0.0) return true;
      else return false;
   */
   // Count plane interval intersection 
   float t = -1;
   XMVECTOR dir = loc_end - loc_beg;
   float x = 0, y = 0;
   XMVECTOR intersection;

   if (getz(dir) != 0)
   {
      t = -getz(loc_beg) / getz(dir);
      x = getx(loc_beg) + t * getx(dir);
      y = gety(loc_beg) + t * gety(dir);
   }

   if (t < 0 || t > 1 ||
      fabs(x) > m_fWidth)
   {
      if (fabs(getx(loc_beg)) < m_fWidth)
         if (Dist != NULL)
            *Dist = getz(loc_beg);

      return false;
   }

   return true;
}


bool Plane::Collide (XMVECTOR* Ret, XMVECTOR& Beg, XMVECTOR& End,
   PhysPatch::BladePhysData* a_pBladePhysData)
{
   XMVECTOR loc_beg, loc_end, loc_pos;
   XMVECTOR psi;
   float gamma;
   bool ret_code = true, ret_code1 = true;

   // In model coordinate system
   XM_TO_M(m_mInvTransform, invTr);
   loc_beg = XMVector3TransformCoord(Beg, invTr);
   loc_end = XMVector3TransformCoord(End, invTr);

   // Count plane interval intersection 
   float t;
   XMVECTOR dir = loc_end - loc_beg;
   float x, y;
   XMVECTOR intersection;

   if (getz(loc_end) * getz(loc_beg) < 0.0)
      //	if (dir.z != 0)    
   {
      t = -getz(loc_beg) / getz(dir);
      x = getx(loc_beg) + t * getx(dir);
      y = gety(loc_beg) + t * gety(dir);
   }
   else
      return false;

   intersection = create(x, y, 0);

   if (t < 0 || t > 1)
      return false;

   if (fabs(x) > m_fWidth || fabs(y) > m_fHeight)
      return false;

   // Count rotation angle
   XMVECTOR proj = create(getx(dir), gety(dir), 0);
   float a = length(loc_beg - intersection),
      b = length(dir), alpha, beta;

   dir = normalize(dir);
   //SuperNormalize(&dir, &dir);
   proj = normalize(proj);

   float angle = dot(proj, dir);

   if (angle < -1)
      angle = -1;
   else if (angle > 1)
      angle = 1;
   alpha = (float)M_PI - acosf(angle);
   beta = a * sinf(alpha) / b;
   beta = sqrt(1 - beta * beta);
   if (beta < -1)
      beta = -1;
   else if (beta > 1)
      beta = 1;
   beta = acosf(beta);
   gamma = (float)M_PI - beta - alpha;

   // Count rotation axis
   psi = cross(dir, proj);
   psi = normalize(psi);


   XMVECTOR beg1 = create(0, 0, 0), beg2 = psi;

   XM_TO_M(m_mTransform, tr);
   beg1 = XMVector3TransformCoord(beg1, tr);
   beg2 = XMVector3TransformCoord(beg2, tr);
   *Ret = beg2 - beg1;
   *Ret = normalize(*Ret);
   *Ret = (*Ret) * gamma * 1.1f;

   return true;
}