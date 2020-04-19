#include "Copter.h"

#include <DDSTextureLoader.h>
#include <DirectXMath.h>

#define COPTER_SIZE 3.0f

Copter::Copter (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect* a_pEffect, FlowManager* flowManager_)
{
   m_pD3DDevice = a_pD3DDevice;
   m_pD3DDeviceCtx = a_pD3DDeviceCtx;
   flowManager = flowManager_;

   XMStoreFloat4x4(&m_mTransform, XMMatrixIdentity());
   
   m_uVertexStride = sizeof(VertexType);
   m_uVertexOffset = 0;
   m_uVertexCount = 6;

   ID3DX11EffectTechnique* pTechnique = a_pEffect->GetTechniqueByIndex(0);
   m_pPass = pTechnique->GetPassByName("RenderMeshPass");
   
   CreateVertexBuffer();

   m_pTransformEMV = a_pEffect->GetVariableByName("g_mWorld")->AsMatrix();
   m_pPrevTransformEMV = a_pEffect->GetVariableByName("g_mPrevWorld")->AsMatrix();

   CreateInputLayout();

   fanId = flowManager->CreateAxesFan();
}


Copter::~Copter ()
{
   SAFE_RELEASE(m_pInputLayout);
   SAFE_RELEASE(m_pVertexBuffer);
}



void Copter::UpdateFromTransform (const XMMATRIX& transform)
{
   XMStoreFloat4x4(&m_mTransform, transform);
   flowManager->fans[fanId].UpdateFromTransform(transform);
}


void Copter::CreateVertexBuffer (void)
{
   SAFE_RELEASE(m_pVertexBuffer);

   VertexType* Vertices = new VertexType[m_uVertexCount];
   
   Vertices[0].vPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
   Vertices[1].vPos = XMFLOAT3(COPTER_SIZE, -COPTER_SIZE, COPTER_SIZE);
   Vertices[2].vPos = XMFLOAT3(COPTER_SIZE, -COPTER_SIZE, -COPTER_SIZE);

   Vertices[5].vPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
   Vertices[4].vPos = XMFLOAT3(-COPTER_SIZE, -COPTER_SIZE, -COPTER_SIZE);
   Vertices[3].vPos = XMFLOAT3(-COPTER_SIZE, -COPTER_SIZE, COPTER_SIZE);

   D3D11_BUFFER_DESC VBufferDesc =
   {
      m_uVertexCount * sizeof(VertexType),
      D3D11_USAGE_DEFAULT,
      D3D11_BIND_VERTEX_BUFFER,
      0, 0
   };
   D3D11_SUBRESOURCE_DATA VBufferInitData;
   VBufferInitData.pSysMem = Vertices;
   m_pD3DDevice->CreateBuffer(&VBufferDesc, &VBufferInitData, &m_pVertexBuffer);

   delete[] Vertices;
}

void Copter::CreateInputLayout (void)
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


void Copter::Render(void)
{
   m_pTransformEMV->SetMatrix((float*)& m_mTransform);
   
   m_pD3DDeviceCtx->IASetInputLayout(m_pInputLayout);
   m_pD3DDeviceCtx->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
   m_pD3DDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   m_pPass->Apply(0, m_pD3DDeviceCtx);
   m_pD3DDeviceCtx->Draw(m_uVertexCount, 0);
}


void Copter::Update (float a_fElapsedTime)
{
  // XM_TO_M(m_mRot, startRot);
  // 
  // m_mPrevRot = m_mRot;
  // startRot = mul(XMMatrixRotationY(a_fElapsedTime * m_fAngleVel), startRot);
  // 
  // XMStoreFloat4x4(&m_mRot, startRot);
}