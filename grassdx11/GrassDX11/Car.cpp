#include "car.h"
#include "PhysPatch.h"
#include "StateManager.h"
#include "aabb.h"

XMMATRIX GetNormalizedMt (AABB vBBox);

Car::Car (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect* a_pEffect, XMVECTOR a_vPosAndRadius,
   Terrain* const a_pTerrain, float a_fHeightScale, float a_fGrassRadius,
   float a_fCarWidth, float a_fCarHeight, float a_fCarLength, float a_fAmplAngle)
{
   m_pD3DDevice    = a_pD3DDevice;
   m_pD3DDeviceCtx = a_pD3DDeviceCtx;
   XMStoreFloat4(&m_vPosAndRadius, a_vPosAndRadius);
   m_pTerrain      = a_pTerrain;
   m_fHeightScale  = a_fHeightScale;
   m_fGrassRadius  = a_fGrassRadius;
   m_pIndexBuffer  = NULL;
   m_pVertexBuffer = NULL;
   m_pInputLayout  = NULL;
   m_pTextureSRV   = NULL;


   // Set car sizes
   m_fCarWidth = a_fCarWidth;
   m_fCarHeight = a_fCarHeight;
   m_fCarLength = a_fCarLength;
   m_fAmplAngle = a_fAmplAngle;
   m_fCarBackWidth = m_fCarWidth + m_fCarLength * tanf(m_fAmplAngle);

   // Set plane sizes
   m_fPlaneLength = m_fCarLength;
   m_fPlaneWidth = m_fCarWidth;
   m_fPlaneHeight = m_fCarHeight;

   /* just one technique in effect */
   ID3DX11EffectTechnique* pTechnique = a_pEffect->GetTechniqueByIndex(0);
   m_pPass = pTechnique->GetPassByName("RenderMeshPass");

   m_pTransformEMV = a_pEffect->GetVariableByName("g_mWorld")->AsMatrix();
   m_pTexESRV = a_pEffect->GetVariableByName("g_txMeshDiffuse")->AsShaderResource();

   CreateInputLayout();


   m_uNumPlanes = 4;
   // Front
   m_pPlanes[0] = new Plane(a_pD3DDevice, a_pD3DDeviceCtx, a_pEffect, m_vPosAndRadius, m_fPlaneWidth, m_fPlaneHeight,
      m_fPlaneWidth - 0.5f, m_fPlaneWidth - 0.01f);
   // Right
   m_pPlanes[1] = new Plane(a_pD3DDevice, a_pD3DDeviceCtx, a_pEffect, m_vPosAndRadius, m_fPlaneLength, m_fPlaneHeight, INVALID_DIST, INVALID_DIST);
   // Left
   m_pPlanes[2] = new Plane(a_pD3DDevice, a_pD3DDeviceCtx, a_pEffect, m_vPosAndRadius, m_fPlaneLength, m_fPlaneHeight, INVALID_DIST, INVALID_DIST);
   // Back
   m_pPlanes[3] = new Plane(a_pD3DDevice, a_pD3DDeviceCtx, a_pEffect, m_vPosAndRadius, m_fPlaneWidth, m_fPlaneHeight, INVALID_DIST, INVALID_DIST);

   carModel = new ModelLoader;
   //if (!carModel->Load(0, m_pD3DDevice, m_pD3DDeviceCtx, "resources/SuperCar/obj/carFordtest.obj"))
   //   assert(false);
   if (!carModel->Load(0, m_pD3DDevice, m_pD3DDeviceCtx, "resources/Car/car_low/source/car_low.fbx"))
      assert(false);
}

Car::~Car(void)
{
   delete m_pPlanes[0];
   delete m_pPlanes[1];
   delete m_pPlanes[2];
   delete m_pPlanes[3];
   carModel->Close();
}


void Car::CreateInputLayout(void)
{
   const D3D11_INPUT_ELEMENT_DESC InputDesc[] =
   {
       { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
       { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
       { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
   };
   D3DX11_PASS_DESC PassDesc;
   m_pPass->GetDesc(&PassDesc);
   int InputElementsCount = sizeof(InputDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);
   HRESULT hr = m_pD3DDevice->CreateInputLayout(InputDesc, InputElementsCount,
      PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize,
      &m_pInputLayout);
}

void Car::Render(void)
{
   //m_pNormalMatrixEMV->SetMatrix((float*)& m_mNormalMatrix);
   //if (GetGlobalStateManager().UseWireframe())
   //   GetGlobalStateManager().SetRasterizerState("EnableMSAACulling_Wire");
   //else
   //   GetGlobalStateManager().SetRasterizerState("EnableMSAACulling");

   m_pD3DDeviceCtx->IASetInputLayout(m_pInputLayout);
   m_pD3DDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   XMVECTOR trnsl, quatr, scale;
   XMMATRIX tr = XMLoadFloat4x4(&m_mTransform);
   XMMatrixDecompose(&scale, &quatr, &trnsl, tr);

   tr = XMMatrixScaling(4.43, 4.43, 4.43) *
      XMMatrixRotationX(PI / 2) *
      XMMatrixRotationY(PI) *
      XMMatrixTranslation(0, -1.4050, 0) *
      XMMatrixTranslation(0, 0, -m_fCarLength) *
      XMMatrixRotationQuaternion(quatr) *
      XMMatrixTranslationFromVector(trnsl) *
      XMMatrixIdentity(); 

   /*tr = XMMatrixScaling(1, 1, 1) *
      //XMMatrixRotationX(PI / 2) *
      //XMMatrixRotationY(PI) *
      XMMatrixRotationQuaternion(quatr) *
      XMMatrixTranslationFromVector(trnsl) *
      //XMMatrixTranslation(0, -1.4050, 0) *
      //XMMatrixTranslation(0, 0, -m_fCarLength) *
      XMMatrixIdentity();*/

   //XMMATRIX initialTr = ;
   //initialTr *= XMMatrixScaling(0.1, 0.1, 0.01);
   m_pTransformEMV->SetMatrix((float*)& tr);

   m_pPass->Apply(0, m_pD3DDeviceCtx);
   carModel->Draw(m_pD3DDeviceCtx, m_pPass, m_pTexESRV);

   // Debug
   //for (auto* plane : m_pPlanes) {
   //   plane->Render();
   //}
}


static int StartFlag = 1;
void Car::SetPosAndRadius (XMFLOAT4& a_vPosAndRadius)
{
   float3 vY = create(0, 1, 0), dir, plane_cur_pos, plane_prev_pos, cur_pos, offset;
   XMMATRIX translate, scale, rotate, plane_coords, model_coords;

   // Calculate move direction
   XMVECTOR prevPos = create(m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z);
   XMStoreFloat3(&m_vPrevPos, prevPos);

   cur_pos = create(a_vPosAndRadius.x, a_vPosAndRadius.y, a_vPosAndRadius.z);
   m_vPosAndRadius = a_vPosAndRadius;

   m_vPrevPos.y = 0;
   if (StartFlag)
   {
      dir = create(0.0, 0.0, 1.0);
      StartFlag = 0;
   }
   else
      dir = cur_pos - create(m_vPrevPos.x, m_vPrevPos.y, m_vPrevPos.z);

   dir = XMVector3Normalize(dir);

   m_vMoveDir = { getx(dir), gety(dir), getz(dir) };

   // Calculate car height
   TerrainHeightData* pHD = m_pTerrain->HeightDataPtr();
   float2 vTexCoord;
   float fHeightOffset = 1.5f;
   float g_fCarLength = 1.0;

   // Calculate up dir
   XMVECTOR up;
   XMVECTOR norm_dir, v1, v2, p0, p1, p2, p3;

   norm_dir = XMVector3Cross(dir, vY);
   norm_dir = XMVector3Normalize(norm_dir);
   norm_dir *= m_fCarWidth;
   dir *= m_fCarLength;

   p0 = cur_pos - dir;
   p1 = p0 + norm_dir;
   p2 = p0 - norm_dir;
   p3 = cur_pos + dir;

   vTexCoord = create(getx(p0) / m_fGrassRadius * 0.5f + 0.5f,
      getz(p0) / m_fGrassRadius * 0.5f + 0.5f);
   sety(p0, pHD->GetHeight(getx(vTexCoord), gety(vTexCoord)) * m_fHeightScale + fHeightOffset);

   vTexCoord = create(getx(p1) / m_fGrassRadius * 0.5f + 0.5f,
      getz(p1) / m_fGrassRadius * 0.5f + 0.5f);
 
   sety(p1, pHD->GetHeight(getx(vTexCoord), gety(vTexCoord)) * m_fHeightScale + fHeightOffset);
   
   vTexCoord = create(getx(p2) / m_fGrassRadius * 0.5f + 0.5f,
      getz(p2) / m_fGrassRadius * 0.5f + 0.5f);
 
   sety(p2, pHD->GetHeight(getx(vTexCoord), gety(vTexCoord)) * m_fHeightScale + fHeightOffset);
  
   vTexCoord = create(getx(p3) / m_fGrassRadius * 0.5f + 0.5f,
      getz(p3) / m_fGrassRadius * 0.5f + 0.5f);
   sety(p3, pHD->GetHeight(getx(vTexCoord), gety(vTexCoord)) * m_fHeightScale + fHeightOffset);

   v1 = p3 - p1;
   v2 = p3 - p2;
   v1 = XMVector3Normalize(v1);
   v2 = XMVector3Normalize(v2);
   up = XMVector3Cross(v2, v1);
   up = XMVector3Normalize(up);
   //	cur_pos.y = ((p1.y + p2.y)/2.0 + p3.y) / 2.0;
   cur_pos = ((p1 + p2) / 2.0 + p3) / 2.0;

   // Calculate TBN matrix
   XMMATRIX tmp;

   dir = p3 - cur_pos;
   model_coords = XMMatrixLookAtLH(cur_pos, cur_pos + dir, up);
   XMStoreFloat4x4(&m_mMatr, XMMatrixInverse(NULL, model_coords));
   
   XMStoreFloat4x4(&m_mInvBottom, model_coords);
   // Scale
   scale = XMMatrixScaling(1.0f / m_fCarWidth, 1.0f / m_fCarHeight, 1.0f / m_fCarLength);
   //   D3DXMatrixScaling(&scale, 1.0f , 1.0f , 1.0f);
   XMMATRIX transform = XMMatrixMultiply(model_coords, scale);
  
   // Set base mesh transform
   transform = XMMatrixInverse(NULL, transform);
   transform = XMMatrixMultiply(GetNormalizedMt(carModel->GetBoundBoxes()[0]), transform);

   // Use this to convert normals to world space in shader
   m_mNormalMatrix = XMMatrixInverse(NULL, transform);
   XMStoreFloat4x4(&m_mTransform, transform);

   // Place front plane
   offset = create(0.0f, 0.0f, -m_fPlaneLength);

   translate = XMMatrixTranslation(getx(offset), gety(offset), getz(offset));
   plane_coords = XMMatrixMultiply(model_coords, translate);

   XMFLOAT4X4 planeCoords;
   XMStoreFloat4x4(&planeCoords, plane_coords);
   m_pPlanes[0]->SetInvTransform(planeCoords);

   XMFLOAT4 v(getx(cur_pos) + getx(offset), gety(cur_pos) + gety(offset), getz(cur_pos) + getz(offset), 0);
   m_pPlanes[0]->SetPosAndRadius(v);

   // Place right plane
   offset = create(0.0f, 0.0f, -m_fPlaneWidth);

   rotate = XMMatrixRotationAxis(vY, 3.0f * (float)M_PI / 2.0f);
   plane_coords = mul(model_coords, rotate);
   
   translate = XMMatrixTranslationFromVector(offset);
   plane_coords = mul(plane_coords, translate);
   
   translate = XMMatrixTranslation(m_fPlaneLength, 0, 0);
   plane_coords = mul(plane_coords, translate);
  
   rotate = XMMatrixRotationY(m_fAmplAngle);
   plane_coords = mul(plane_coords, rotate);

   translate = XMMatrixTranslation(-m_fPlaneLength, 0, 0);
   plane_coords = mul(plane_coords, translate);

   M_TO_XM(plane_coords, invTr);
   m_pPlanes[1]->SetInvTransform(invTr);
   v = XMFLOAT4(getx(cur_pos) + getx(offset), gety(cur_pos) + gety(offset), getz(cur_pos) + getz(offset), 0);
   m_pPlanes[1]->SetPosAndRadius(
      v
   );

   // Place left plane
   offset = create(0.0f, 0.0f, -m_fPlaneWidth);

   rotate = XMMatrixRotationAxis(vY, (float)M_PI / 2.0f);
   plane_coords = mul(model_coords, rotate);

   translate = XMMatrixTranslationFromVector(offset);
   plane_coords = mul(plane_coords, translate);

   translate = XMMatrixTranslation(-m_fPlaneLength, 0, 0);
   plane_coords = mul(plane_coords, translate);

   rotate = XMMatrixRotationY(-m_fAmplAngle);
   plane_coords = mul(plane_coords, rotate);

   translate = XMMatrixTranslation(m_fPlaneLength, 0, 0);
   plane_coords = mul(plane_coords, translate);

   XMStoreFloat4x4(&planeCoords, plane_coords);
   m_pPlanes[2]->SetInvTransform(planeCoords);
   v = XMFLOAT4(getx(cur_pos) + getx(offset), gety(cur_pos) + gety(offset), getz(cur_pos) + getz(offset), 0);
   m_pPlanes[2]->SetPosAndRadius(v);

   m_vPosAndRadius = XMFLOAT4(getx(cur_pos), gety(cur_pos), getz(cur_pos), 0.0f);

   // Place back plane
   offset = create(0.0f, 0.0f, m_fPlaneLength);

   translate = XMMatrixTranslationFromVector(offset);
   plane_coords = mul(model_coords, translate);

   XMStoreFloat4x4(&planeCoords, plane_coords);
   m_pPlanes[3]->SetInvTransform(planeCoords);
   v = XMFLOAT4(getx(cur_pos) + getx(offset), gety(cur_pos) + gety(offset), getz(cur_pos) + getz(offset), 0);
   m_pPlanes[3]->SetPosAndRadius(v);
}


XMFLOAT4 Car::GetPosAndRadius(void)
{
   //float r = 1.5f * sqrt(m_fCarLength * m_fCarLength + m_fCarHeight * m_fCarHeight);
   float r = m_fCarWidth;
   return XMFLOAT4(m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z, r);
   //        max(max(m_fCarBackWidth, m_fCarHeight), m_fCarLength));
}


void Car::SetHeight(float a_fH)
{

}

bool Car::CheckCollision (XMVECTOR Beg, XMVECTOR End, float* Dist)
{
   float treshold = -1.5f;

   for (UINT k = 1; k < m_uNumPlanes; k++)
   {
      float cur_dist;
      bool collision_flag;

      collision_flag =
         m_pPlanes[k]->CheckCollision(Beg, End, &cur_dist);
      if (collision_flag ||
         (cur_dist < 0 && cur_dist > treshold))
      {
         if (Dist != NULL)
            * Dist = cur_dist;
         return true;
      }
   }

   if (Dist != NULL)
      * Dist = INVALID_DIST;
   return false;
}


bool Car::Collide(XMVECTOR* Ret, XMVECTOR& Beg, XMVECTOR& End,
   PhysPatch::BladePhysData* a_pBladePhysData, int a_iSegmentIndex)
{
   bool is_grabbed = false, is_collided = false;

   *Ret = create(0, 0, 0);


   for (UINT k = 1; k < m_uNumPlanes; k++)
   {
      XMVECTOR psi;
      bool collision = m_pPlanes[k]->Collide(&psi, Beg, End, a_pBladePhysData);
      
      if (collision)
      {
         *Ret += psi;
         is_collided = true;
      }
   }

   return is_collided;
}

float Car::GetDist(XMVECTOR& Pnt, bool* IsUnderWheel)
{
   float min_dist = INVALID_DIST;

   if (IsUnderWheel != NULL)
      * IsUnderWheel = false;

   for (UINT i = 0; i < m_uNumPlanes; i++)
   {
      float dist = fabs(m_pPlanes[i]->GetDist(Pnt, IsUnderWheel));
      if (dist < min_dist)
         min_dist = dist;
   }

   return min_dist;
}


int Car::IsBottom(XMVECTOR& Pnt, XMVECTOR& vNormal)
{
   XMVECTOR loc_coord;
   float2 vTexCoord;
   TerrainHeightData* pHD = m_pTerrain->HeightDataPtr();

   XM_TO_M(m_mInvBottom, invB);
   loc_coord = XMVector3Transform(Pnt, invB);
   if (fabs(getx(loc_coord) <= m_fCarWidth && fabs(getz(loc_coord)) <= m_fCarLength))
   {
      vTexCoord = create(getx(Pnt) / m_fGrassRadius * 0.5f + 0.5f, getz(Pnt) / m_fGrassRadius * 0.5f + 0.5f);
      setx(vNormal, max((m_fCarWidth - fabs(getx(loc_coord))) / 0.75f, 0.3f));
      if (getx(vNormal) > 1.0f) 
         setx(vNormal, 1.0f);
      if (fabs(getx(loc_coord)) > m_fCarWidth - 0.75f)
      {
         if (getx(loc_coord) > 0.0f) 
            return 2;
         else 
            return 3;
      }
      else 
         return 1;

   }
   else
      return 0;
}


XMMATRIX GetNormalizedMt(AABB vBBox)
{
   XMMATRIX m;
   XMFLOAT3 vBBoxMax, vBBoxMin;
   vBBox.Get(vBBoxMin, vBBoxMax);

   float* output = (float*)(&m);
   output[0] = 2.0f / (vBBoxMax.x - vBBoxMin.x);
   output[4] = 0.0;
   output[8] = 0.0;
   output[12] = -(vBBoxMax.x + vBBoxMin.x) / (vBBoxMax.x - vBBoxMin.x);

   output[1] = 0.0;
   output[5] = 2.0f / (vBBoxMax.y - vBBoxMin.y);
   output[9] = 0.0;
   output[13] = -(vBBoxMax.y + vBBoxMin.y) / (vBBoxMax.y - vBBoxMin.y);


   output[2] = 0.0;
   output[6] = 0.0;
   output[10] = 2.0f / (vBBoxMax.z - vBBoxMin.z);
   output[14] = -(vBBoxMax.z + vBBoxMin.z) / (vBBoxMax.z - vBBoxMin.z);


   output[3] = 0.0;
   output[7] = 0.0;
   output[11] = 0.0;
   output[15] = 1.0;

   return m;
}