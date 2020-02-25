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
   float angleVel = 100.0f;
   float bladeSize = 10.0f;
   int   bladesNum = 2;

   m_pAxesFanFlow = new AxesFanFlow();
   m_pAxesFanFlow->Initialize(m_pD3DDevice, m_pD3DDeviceCtx, m_uFlowTexSize, m_uFlowTexSize, m_fTerrRadius);
   
   m_pAxesFanFlow->SetNoise(m_NoiseSRV);
   m_pAxesFanFlow->SetR(bladeSize);
   m_pAxesFanFlow->SetRingsNumber(16);
   m_pAxesFanFlow->SetDirection(create(0, -1, 0));
   XM_TO_V(a_vPosition, pos, 3);
   m_pAxesFanFlow->SetPosition(pos);

   m_pAxesFan = new AxesFan(m_pD3DDevice, m_pD3DDeviceCtx, m_Effect, bladesNum, bladeSize, angleVel);
   m_pAxesFan->SetPosition(pos);
   m_pAxesFan->SetDirection(create(0, -1, 0));
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