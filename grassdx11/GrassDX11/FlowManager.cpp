#include "FlowManager.h"


FlowManager::FlowManager (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect* a_Effect, float a_fTerrRadius, ID3D11ShaderResourceView* a_NoiseSRV)
   : m_pD3DDevice(a_pD3DDevice)
   , m_pD3DDeviceCtx(a_pD3DDeviceCtx)
   , m_fTerrRadius(a_fTerrRadius)
   , m_Effect(a_Effect)
   , m_NoiseSRV(a_NoiseSRV)
{
}

FlowManager::~FlowManager(void)
{
   SAFE_DELETE(m_pAxesFan);
   SAFE_DELETE(m_pAxesFanFlow);
}


void FlowManager::CreateAxesFan (const XMFLOAT3& a_vPosition)
{
   float angleVel = 0.0f;
   float bladeSize = 10.0f;
   int   bladesNum = 2;

   m_pAxesFanFlow = new AxesFanFlow(m_pD3DDevice, m_pD3DDeviceCtx, m_uFlowTexSize, m_uFlowTexSize, m_fTerrRadius);
   m_pAxesFanFlow->SetNoise(m_NoiseSRV);
   m_pAxesFanFlow->SetFanRadius(bladeSize / m_fTerrRadius);
   m_pAxesFanFlow->SetRingsNumber(16);
   XM_TO_V(a_vPosition, pos, 3);
   
   m_pAxesFan = new AxesFan(m_pD3DDevice, m_pD3DDeviceCtx, m_Effect, bladesNum, bladeSize, angleVel);
   
   SetDirection(create(0, -1, 0));
   SetTransform(pos);
}


void FlowManager::Update (float a_fElapsedTime, float a_fTime)
{
   m_pAxesFanFlow->SetTime(a_fTime);
   m_pAxesFanFlow->Update();
   m_pAxesFan->Update(a_fElapsedTime);
}

void FlowManager::RenderFan (void)
{
   m_pAxesFan->Render();
}


void FlowManager::SetFanRadius (float a_fR)
{
   m_pAxesFanFlow->SetFanRadius(a_fR / m_fTerrRadius);
   m_pAxesFan->SetR(a_fR);
}


void FlowManager::SetDeltaSlices(float a_fDeltaSlices)
{
   m_pAxesFanFlow->SetDeltaSlices(a_fDeltaSlices); 
}


void FlowManager::SetShift(float a_fShift) 
{
   m_pAxesFanFlow->SetShift(a_fShift); 
}


void FlowManager::SetDirection(const float3& a_vDir)
{
   m_pAxesFanFlow->SetDirection(a_vDir);
   m_pAxesFan->SetDirection(a_vDir);
}

void FlowManager::SetTransform(const float3& a_vPos)
{
   m_vPosition = a_vPos;
   m_pAxesFanFlow->SetPosition(a_vPos);
   m_pAxesFan->SetPosition(a_vPos);
}

XMVECTOR FlowManager::GetPosition (void)
{
   return m_vPosition;
}

void FlowManager::SetAngleSpeed(float a_fS)
{
   m_pAxesFanFlow->SetAngleSpeed(a_fS);
   m_pAxesFan->SetAngleSpeed(a_fS);
}