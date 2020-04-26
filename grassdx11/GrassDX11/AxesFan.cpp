#include "AxesFan.h"
#include "FlowManager.h"

#include <DDSTextureLoader.h>
#include <DirectXMath.h>


AxesFan::AxesFan(ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect* a_pEffect, AxesFanDesc* desc)
{
   m_pD3DDevice = a_pD3DDevice;
   m_pD3DDeviceCtx = a_pD3DDeviceCtx;
   XMStoreFloat4x4(&m_mTransform, XMMatrixIdentity());
   m_uVertexStride = sizeof(VertexType);
   m_uVertexOffset = 0;
   m_uVertexCount = 2 * 3;
   m_iBladesNum = desc->bladesNum;
   m_fBladeSize = desc->radius;
   m_fAngleVel  = desc->angleSpeed;

   ID3DX11EffectTechnique* pTechnique = a_pEffect->GetTechniqueByIndex(0);
   m_pPass1 = pTechnique->GetPassByName("RenderMeshPass");

   m_pVelPass = pTechnique->GetPassByName("RenderVelocityPass");
   m_pPass = pTechnique->GetPassByName("RenderMeshPassBlured");
   
   m_pTransformEMV = a_pEffect->GetVariableByName("g_mWorld")->AsMatrix();
   m_pPrevTransformEMV = a_pEffect->GetVariableByName("g_mPrevWorld")->AsMatrix();

   CreateInputLayout();

   XMStoreFloat4x4(&m_mRotation, XMMatrixIdentity());
   XMStoreFloat4x4(&m_mRot,XMMatrixIdentity());

   /*
      !!! IF SOME ONE WOULD WORK WITH THIS CODE!!!
      Look at ModelLoader.cpp code
      113 line: str = "Texture.png";

      I use online expoter of model (to get wing.fbx). Online exporter export model, and frite to fbx bad texture 
      I don't know why. So I hardcode this texture (because i load 2 models with 1 texture).
      If you want to load other models, clean this code frome this. 
      Sorry, i havn't enougth time to make it works :)

   */

   wingModel = new ModelLoader;
   if (!wingModel->Load(0, m_pD3DDevice, m_pD3DDeviceCtx, "resources/Copter/wing.fbx"))
      assert(false);
}

AxesFan::~AxesFan()
{
   wingModel->Close();
   SAFE_RELEASE(m_pInputLayout);
   SAFE_RELEASE(m_pVertexBuffer);
}


void AxesFan::CreateInputLayout (void)
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

   m_pPass1->GetDesc(&PassDesc);
   InputElementsCount = sizeof(InputDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);
   m_pD3DDevice->CreateInputLayout(InputDesc, InputElementsCount,
      PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize,
      &m_pInputLayout1);
}


void AxesFan::Render(bool isVelPass, bool isBlured)
{
   XM_TO_M(m_mRot, startRot);
   XM_TO_M(m_mPrevRot, prevStartRot);

   XMMATRIX mTr = XMLoadFloat4x4(&m_mTransform);

   float delta = PI / m_iBladesNum;

   XMMATRIX bladeM = XMMatrixIdentity();
   bladeM *= startRot;
   bladeM *= mTr;

   XMMATRIX prevBladeM = XMMatrixIdentity();
   prevBladeM *= prevStartRot;
   prevBladeM *= mTr;

   M_TO_XM(bladeM, bladeTr);
   M_TO_XM(prevBladeM, prevBladeTr);

   m_pTransformEMV->SetMatrix((float*)& bladeTr);
   m_pPrevTransformEMV->SetMatrix((float*)& prevBladeTr);

   m_pD3DDeviceCtx->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
   m_pD3DDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   if (isVelPass) {
      m_pVelPass->Apply(0, m_pD3DDeviceCtx);
   } else if (isBlured) {
      m_pD3DDeviceCtx->IASetInputLayout(m_pInputLayout);
      m_pPass->Apply(0, m_pD3DDeviceCtx);
   } else {
      m_pD3DDeviceCtx->IASetInputLayout(m_pInputLayout1);
      m_pPass1->Apply(0, m_pD3DDeviceCtx);
   }

   wingModel->Draw(m_pD3DDeviceCtx);
}


void AxesFan::Update (float a_fElapsedTime)
{
   XM_TO_M(m_mRot, startRot);
   
   m_mPrevRot = m_mRot;
   startRot = mul(XMMatrixRotationZ(a_fElapsedTime * m_fAngleVel), startRot);
   
   XMStoreFloat4x4(&m_mRot, startRot);
}


void AxesFan::SetR (float a_fR)
{
   m_fBladeSize = a_fR;
}