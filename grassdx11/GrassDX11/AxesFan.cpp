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

   /* Loading effect */
   /*ID3DBlob* pErrorBlob = nullptr;
   D3DX11CompileEffectFromFile(L"Shaders/AxesFan.fx",
      0,
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      0,
      0,
      m_pD3DDevice,
      &m_pEffect,
      &pErrorBlob);
   */
  /* ID3DBlob* pErrorBlob = nullptr;
   HRESULT hr = D3DX11CompileEffectFromFile(L"Shaders/AxesFan.fx", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
      D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_SKIP_OPTIMIZATION, 0, m_pD3DDevice, &m_pEffect, &pErrorBlob);

   if (pErrorBlob)
   {
      OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
      pErrorBlob->Release();
   }*/

   ID3DX11EffectTechnique* pTechnique = a_pEffect->GetTechniqueByIndex(0);
   m_pPass = pTechnique->GetPassByName("RenderMeshPassBlured");
   m_pVelPass = pTechnique->GetPassByName("RenderVelocityPass");
   //m_pPass = pTechnique->GetPassByName("RenderAxesFan");

   CreateVertexBuffer();

   m_pTransformEMV = a_pEffect->GetVariableByName("g_mWorld")->AsMatrix();
   m_pPrevTransformEMV = a_pEffect->GetVariableByName("g_mPrevWorld")->AsMatrix();

   CreateInputLayout();

   XMStoreFloat4x4(&m_mRotation, XMMatrixIdentity());
   XMStoreFloat4x4(&m_mRot,XMMatrixIdentity());
}

AxesFan::~AxesFan()
{
   SAFE_RELEASE(m_pInputLayout);
   SAFE_RELEASE(m_pVertexBuffer);
}


void AxesFan::CreateVertexBuffer (void)
{
   SAFE_RELEASE(m_pVertexBuffer);

   VertexType* Vertices = new VertexType[m_uVertexCount];
   
   Vertices[0].vPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
   Vertices[1].vPos = XMFLOAT3(m_fBladeSize, -0.5f, 0.5f);
   Vertices[2].vPos = XMFLOAT3(m_fBladeSize, 0.5f, -0.5f);

   Vertices[5].vPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
   Vertices[4].vPos = XMFLOAT3(-m_fBladeSize, -0.5f, -0.5f);
   Vertices[3].vPos = XMFLOAT3(-m_fBladeSize, 0.5f, 0.5f);

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
}


void AxesFan::Render(bool isVelPass)
{
   XM_TO_M(m_mRot, startRot);
   XM_TO_M(m_mPrevRot, prevStartRot);

   for (int i = 0; i < m_iBladesNum; i++) {
      XMMATRIX mTr = XMLoadFloat4x4(&m_mTransform);

      float delta = PI / m_iBladesNum;

      XMMATRIX bladeM = XMMatrixIdentity();
      bladeM *= startRot;
      bladeM *= XMMatrixRotationY(delta * i);;
      bladeM *= mTr;

      XMMATRIX prevBladeM = XMMatrixIdentity();
      prevBladeM *= prevStartRot;
      prevBladeM *= XMMatrixRotationY(delta * i);;
      prevBladeM *= mTr;

      M_TO_XM(bladeM, bladeTr);
      M_TO_XM(prevBladeM, prevBladeTr);

      m_pTransformEMV->SetMatrix((float*)& bladeTr);
      m_pPrevTransformEMV->SetMatrix((float*)& prevBladeTr);

      m_pD3DDeviceCtx->IASetInputLayout(m_pInputLayout);
      m_pD3DDeviceCtx->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
      m_pD3DDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

      if (isVelPass) {
         m_pVelPass->Apply(0, m_pD3DDeviceCtx);
      } else {
         m_pPass->Apply(0, m_pD3DDeviceCtx);
      }

      m_pD3DDeviceCtx->Draw(m_uVertexCount, 0);
   }
}


void AxesFan::Update (float a_fElapsedTime)
{
   XM_TO_M(m_mRot, startRot);
   
   m_mPrevRot = m_mRot;
   startRot = mul(XMMatrixRotationY(a_fElapsedTime * m_fAngleVel), startRot);
   
   XMStoreFloat4x4(&m_mRot, startRot);
}


void AxesFan::SetR (float a_fR)
{
   m_fBladeSize = a_fR;
   CreateVertexBuffer();
}