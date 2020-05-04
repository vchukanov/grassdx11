#include "FlowManager.h"


FlowManager::FlowManager (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect* a_Effect, float a_fTerrRadius, ID3D11ShaderResourceView* a_NoiseSRV)
   : m_pD3DDevice(a_pD3DDevice)
   , m_pD3DDeviceCtx(a_pD3DDeviceCtx)
   , m_fTerrRadius(a_fTerrRadius)
   , m_Effect(a_Effect)
   , m_NoiseSRV(a_NoiseSRV)
{
   m_pAxesFanFlow = new AxesFanFlow(m_pD3DDevice, m_pD3DDeviceCtx, FLOW_TEX_SIZE, FLOW_TEX_SIZE, m_fTerrRadius);
   m_pAxesFanFlow->SetRingsNumber(16);
}


FlowManager::~FlowManager(void)
{
   if (fans.size() > 0) {
      SAFE_DELETE(fans[0].pAxesFanFlow);
      for (auto& fan : fans) {
         SAFE_DELETE(fan.pAxesFan);
      }
   }
}


int FlowManager::CreateAxesFan (const XMMATRIX& initialTransform_)
{
   const float bladeSize = 10.0f;
   const int   bladesNum = 2;

 
   AxesFanDesc fan;
   fan.pFlowManager = this;
   fan.pAxesFanFlow = m_pAxesFanFlow;
   fan.pAxesFan = new AxesFan(m_pD3DDevice, m_pD3DDeviceCtx, m_Effect, &fan);
   XMStoreFloat4x4(&fan.initialTransform, initialTransform_);

   fans.push_back(fan);
   return fans.size() - 1;
}


void FlowManager::Update (float a_fElapsedTime, float a_fTime)
{
   m_pAxesFanFlow->SetTime(a_fTime);

   for (auto& fan : fans) {
      fan.pAxesFan->Update(a_fElapsedTime);
   }

   m_pAxesFanFlow->BeginMakeFlowTexture();

   for (auto& fan : fans) {
      fan.Setup();
      m_pAxesFanFlow->CreateFlow();
   }

   m_pAxesFanFlow->EndMakeFlowTexture();
}


void FlowManager::RenderFans (bool isVelPass, bool isBlured)
{
   for (auto& fan : fans) {
      fan.Setup();
      fan.pAxesFan->Render(isVelPass, isBlured);
   }
}


ID3D11ShaderResourceView* FlowManager::GetFlowSRV (void)
{ 
   return m_pAxesFanFlow->GetShaderResourceView();
}



void FlowManager::SetMaxHorizFlow (float a_fMaxFlowStrength) 
{
   m_pAxesFanFlow->SetMaxFlowStrength(a_fMaxFlowStrength); 
}



void FlowManager::SetDeltaSlices (float a_fDeltaSlices)
{
   m_pAxesFanFlow->SetDeltaSlices(a_fDeltaSlices); 
}


void FlowManager::SetShift (float a_fShift) 
{
   m_pAxesFanFlow->SetShift(a_fShift); 
}

AxesFanDesc::AxesFanDesc (void)
{
   XMStoreFloat4x4(&transform, XMMatrixIdentity());
   XMStoreFloat4x4(&initialTransform, XMMatrixIdentity());
}


void AxesFanDesc::Setup (void)
{
   // setup position
   XM_TO_V(position, pos, 3);
   pAxesFanFlow->SetPosition(pos);

   // setup direction
   XM_TO_V(direction, dir, 3);
   pAxesFanFlow->SetDirection(dir);

   // setup radius
   pAxesFanFlow->SetFanRadius(radius);
   pAxesFan->SetR(radius);

   // setup angle speed
   pAxesFanFlow->SetAngleSpeed(angleSpeed);
   pAxesFan->SetAngleSpeed(angleSpeed);
}



void AxesFanDesc::UpdateFromTransform (const XMMATRIX& transform)
{
   XMMATRIX tr = XMLoadFloat4x4(&initialTransform);
   tr *= transform;

   XMVECTOR scale, pos, quatr;
   XMMatrixDecompose(&scale, &quatr, &pos, tr);
   
   XMMATRIX rot = XMMatrixRotationQuaternion(quatr);
   XMVECTOR down = create(0, -1, 0);
   
   XMVECTOR dir = XMVector3Transform(down, rot);
   XMStoreFloat3(&direction, dir);
   XMStoreFloat3(&position, pos);

   XMStoreFloat4x4(&pAxesFan->m_mTransform, tr);
}