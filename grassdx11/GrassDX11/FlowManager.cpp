#include "FlowManager.h"


FlowManager::FlowManager (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect* a_Effect, float a_fTerrRadius)
   : m_pD3DDevice(a_pD3DDevice)
   , m_pD3DDeviceCtx(a_pD3DDeviceCtx)
   , m_fTerrRadius(a_fTerrRadius)
   , m_Effect(a_Effect)
{
}

FlowManager::~FlowManager(void)
{
   SAFE_DELETE(m_pAxesFan);
   SAFE_DELETE(m_pAxesFanFlow);
}


void FlowManager::CreateAxesFan (const XMFLOAT3& a_vPosition)
{
   m_pAxesFanFlow = new AxesFanFlow();
   m_pAxesFanFlow->Initialize(m_pD3DDevice, m_pD3DDeviceCtx, m_uFlowTexSize, m_uFlowTexSize, m_fTerrRadius);
   
   auto mTransfrom = XMMatrixTranslation(a_vPosition.x, a_vPosition.y, a_vPosition.z);
   m_pAxesFan = new AxesFan(m_pD3DDevice, m_pD3DDeviceCtx, m_Effect, mTransfrom, 2, 10.0f, 10);
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