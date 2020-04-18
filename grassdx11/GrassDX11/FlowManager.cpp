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
   for (auto& fan : fans) {
      SAFE_DELETE(fan.pAxesFan);
      SAFE_DELETE(fan.pAxesFanFlow);
   }
}


int FlowManager::CreateAxesFan (const XMFLOAT3& a_vPosition)
{
   const float bladeSize = 10.0f;
   const int   bladesNum = 2;

   AxesFanDesc fan;
   fan.pFlowManager = this;
   fan.pAxesFanFlow = m_pAxesFanFlow;
   fan.pAxesFan = new AxesFan(m_pD3DDevice, m_pD3DDeviceCtx, m_Effect, &fan);

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


void FlowManager::RenderFans (bool isVelPass)
{
   for (auto& fan : fans) {
      fan.Setup();
      fan.pAxesFan->Render(isVelPass);
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


void AxesFanDesc::Setup (void)
{
   // setup position
   XM_TO_V(position, pos, 3);
   pAxesFanFlow->SetPosition(pos);
   pAxesFan->SetPosition(pos);

   // setup direction
   XM_TO_V(direction, dir, 3);
   pAxesFanFlow->SetDirection(dir);
   pAxesFan->SetDirection(dir);

   // setup radius
   pAxesFanFlow->SetFanRadius(radius);
   pAxesFan->SetR(radius);

   // setup angle speed
   pAxesFanFlow->SetAngleSpeed(angleSpeed);
   pAxesFan->SetAngleSpeed(angleSpeed);
}