#include "Copter.h"

#include <DDSTextureLoader.h>
#include <DirectXMath.h>

#define COPTER_SIZE 5.0f
#define COPTER_HEIGHT 1.0f
#define WING_HEIGHT 5.0f
#define WING_OFFSET 1.0f
#define BLADE_R 1


Copter::Copter (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect* a_pEffect, FlowManager* flowManager_)
{
   m_pD3DDevice = a_pD3DDevice;
   m_pD3DDeviceCtx = a_pD3DDeviceCtx;
   flowManager = flowManager_;

   XMStoreFloat4x4(&m_mTransform, XMMatrixIdentity());

   ID3DX11EffectTechnique* pTechnique = a_pEffect->GetTechniqueByIndex(0);
   m_pPass = pTechnique->GetPassByName("RenderMeshPass");

   m_pTransformEMV     = a_pEffect->GetVariableByName("g_mWorld")->AsMatrix();
   m_pPrevTransformEMV = a_pEffect->GetVariableByName("g_mPrevWorld")->AsMatrix();
   m_pTexESRV          = a_pEffect->GetVariableByName("g_txMeshDiffuse")->AsShaderResource();
  
   CreateInputLayout();
   
   fanIds.push_back(flowManager->CreateAxesFan());
   fanIds.push_back(flowManager->CreateAxesFan());
   fanIds.push_back(flowManager->CreateAxesFan());
   fanIds.push_back(flowManager->CreateAxesFan());

   UpdateScale(1.0f);
   
   copterModel = new ModelLoader;
   if (!copterModel->Load(0, m_pD3DDevice, m_pD3DDeviceCtx, "resources/Copter/quadrocopter.fbx"))
      assert (false);
}


Copter::~Copter (void)
{
   copterModel->Close();
   SAFE_RELEASE(m_pInputLayout);
}


void Copter::UpdateFromTransform (const XMMATRIX& transform)
{
   // copter model up is not like y axes
   XMMATRIX initialTr = XMMatrixRotationX(PI / 2);
   initialTr *= XMMatrixScaling(scale, scale, scale);
   initialTr *= transform;

   XMStoreFloat4x4(&m_mTransform, initialTr);

   for (auto& fanId : fanIds) {
      flowManager->fans[fanId].UpdateFromTransform(transform/* * XMMatrixTranslation(x, y, z)*/);
   }
}


void Copter::CreateInputLayout (void)
{
   D3D11_INPUT_ELEMENT_DESC InputDesc[] =
   {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
      { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
   };
   D3DX11_PASS_DESC PassDesc;
   m_pPass->GetDesc(&PassDesc);
   int InputElementsCount = sizeof(InputDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);
   m_pD3DDevice->CreateInputLayout(InputDesc, InputElementsCount,
      PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize,
      &m_pInputLayout);
}


void Copter::Render (void)
{
   m_pTransformEMV->SetMatrix((float*)& m_mTransform);
   
   m_pD3DDeviceCtx->IASetInputLayout(m_pInputLayout);
   m_pD3DDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   m_pPass->Apply(0, m_pD3DDeviceCtx);
   copterModel->Draw(m_pD3DDeviceCtx, m_pPass, m_pTexESRV);
}


void Copter::UpdateScale (float newScale)
{
   scale = newScale;

   XMVECTOR fan1Tranlation = create(-2 * scale, 0.7 * scale, -2 * scale);
   XMVECTOR fan2Tranlation = create(-2 * scale, 0.7 * scale,  2 * scale);;
   XMVECTOR fan3Tranlation = create( 2 * scale, 0.7 * scale, -2 * scale);;
   XMVECTOR fan4Tranlation = create( 2 * scale, 0.7 * scale,  2 * scale);;
                                       
   XMMATRIX ini = XMMatrixRotationX(PI / 2);
   ini *= XMMatrixScaling(scale, scale, scale);

   XMMATRIX iniTr[4];
   iniTr[0] = ini * XMMatrixTranslationFromVector(fan1Tranlation);
   iniTr[1] = ini * XMMatrixRotationY(PI / 2) * XMMatrixTranslationFromVector(fan2Tranlation);
   iniTr[2] = ini * XMMatrixTranslationFromVector(fan3Tranlation);
   iniTr[3] = ini * XMMatrixRotationY(PI / 2) * XMMatrixTranslationFromVector(fan4Tranlation);

   int i = 0;
   for (auto& fanId : fanIds) {
      flowManager->fans[fanId].radius = BLADE_R * scale;
      XMStoreFloat4x4(&flowManager->fans[fanId].initialTransform, iniTr[i++]);
   }
}