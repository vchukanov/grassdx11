#include "Copter.h"

#include <DDSTextureLoader.h>
#include <DirectXMath.h>

#define COPTER_SIZE 5.0f
#define COPTER_HEIGHT 1.0f
#define WING_HEIGHT 5.0f
#define WING_OFFSET 1.0f
#define BLADE_R 6.5f

Copter::Copter (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect* a_pEffect, FlowManager* flowManager_)
{
   m_pD3DDevice = a_pD3DDevice;
   m_pD3DDeviceCtx = a_pD3DDeviceCtx;
   flowManager = flowManager_;

   XMStoreFloat4x4(&m_mTransform, XMMatrixIdentity());
   
   m_uVertexStride = sizeof(VertexType);
   m_uVertexOffset = 0;
   m_uVertexCount = 20;
   m_uIndexCount = 72;

   ID3DX11EffectTechnique* pTechnique = a_pEffect->GetTechniqueByIndex(0);
   m_pPass = pTechnique->GetPassByName("RenderMeshPass");
   
   CreateVertexBuffer();

   m_pTransformEMV = a_pEffect->GetVariableByName("g_mWorld")->AsMatrix();
   m_pPrevTransformEMV = a_pEffect->GetVariableByName("g_mPrevWorld")->AsMatrix();

   CreateInputLayout();

   XMVECTOR fan1Tranlation = create(-WING_HEIGHT,              (COPTER_HEIGHT + WING_HEIGHT) * 0.5, -WING_HEIGHT);
   XMVECTOR fan2Tranlation = create(-WING_HEIGHT,              (COPTER_HEIGHT + WING_HEIGHT) * 0.5, WING_HEIGHT + COPTER_SIZE);;
   XMVECTOR fan3Tranlation = create(WING_HEIGHT + COPTER_SIZE, (COPTER_HEIGHT + WING_HEIGHT) * 0.5, WING_HEIGHT + COPTER_SIZE);;
   XMVECTOR fan4Tranlation = create(WING_HEIGHT + COPTER_SIZE, (COPTER_HEIGHT + WING_HEIGHT) * 0.5, -WING_HEIGHT);;

   XMMATRIX iniTr1 = XMMatrixTranslationFromVector(fan1Tranlation);
   XMMATRIX iniTr2 = XMMatrixTranslationFromVector(fan2Tranlation);
   XMMATRIX iniTr3 = XMMatrixTranslationFromVector(fan3Tranlation);
   XMMATRIX iniTr4 = XMMatrixTranslationFromVector(fan4Tranlation);

   fanIds.push_back(flowManager->CreateAxesFan(iniTr1));
   fanIds.push_back(flowManager->CreateAxesFan(iniTr2));
   fanIds.push_back(flowManager->CreateAxesFan(iniTr3));
   fanIds.push_back(flowManager->CreateAxesFan(iniTr4));

   for (auto& fanId : fanIds) {
      flowManager->fans[fanId].radius = BLADE_R;
   }
}


Copter::~Copter ()
{
   SAFE_RELEASE(m_pInputLayout);
   SAFE_RELEASE(m_pVertexBuffer);
   SAFE_RELEASE(m_pIndexBuffer);
}


void Copter::UpdateFromTransform(const XMMATRIX& transform)
{
   for (auto& fanId : fanIds) {
      XMStoreFloat4x4(&m_mTransform, transform);
      flowManager->fans[fanId].UpdateFromTransform(transform);
   }
}


void Copter::CreateVertexBuffer (void)
{
   SAFE_RELEASE(m_pVertexBuffer);

   VertexType* Vertices = new VertexType[m_uVertexCount];
   
   // bottom
   Vertices[0].vPos = XMFLOAT3(0, 0, 0);
   Vertices[1].vPos = XMFLOAT3(0, 0, COPTER_SIZE);
   Vertices[2].vPos = XMFLOAT3(COPTER_SIZE, 0, COPTER_SIZE);
   Vertices[3].vPos = XMFLOAT3(COPTER_SIZE, 0, 0);

   // up
   Vertices[4].vPos = XMFLOAT3(0, COPTER_HEIGHT, 0);
   Vertices[5].vPos = XMFLOAT3(0, COPTER_HEIGHT, COPTER_SIZE);
   Vertices[6].vPos = XMFLOAT3(COPTER_SIZE, COPTER_HEIGHT, COPTER_SIZE);
   Vertices[7].vPos = XMFLOAT3(COPTER_SIZE, COPTER_HEIGHT, 0);

   //
   Vertices[8].vPos = XMFLOAT3(WING_OFFSET, COPTER_HEIGHT, 0);
   Vertices[9].vPos = XMFLOAT3(0, COPTER_HEIGHT, WING_OFFSET);
   
   Vertices[10].vPos = XMFLOAT3(0, COPTER_HEIGHT, COPTER_SIZE - WING_OFFSET);
   Vertices[11].vPos = XMFLOAT3(WING_OFFSET, COPTER_HEIGHT, COPTER_SIZE);

   Vertices[12].vPos = XMFLOAT3(COPTER_SIZE - WING_OFFSET, COPTER_HEIGHT, COPTER_SIZE);
   Vertices[13].vPos = XMFLOAT3(COPTER_SIZE, COPTER_HEIGHT, COPTER_SIZE - WING_OFFSET);

   Vertices[14].vPos = XMFLOAT3(COPTER_SIZE, COPTER_HEIGHT, WING_OFFSET);
   Vertices[15].vPos = XMFLOAT3(COPTER_SIZE - WING_OFFSET, COPTER_HEIGHT, 0);

   //
   Vertices[16].vPos = XMFLOAT3(-WING_HEIGHT,              (COPTER_HEIGHT + WING_HEIGHT) * 0.5, -WING_HEIGHT);
   Vertices[17].vPos = XMFLOAT3(-WING_HEIGHT,              (COPTER_HEIGHT + WING_HEIGHT) * 0.5, WING_HEIGHT + COPTER_SIZE);
   Vertices[18].vPos = XMFLOAT3(WING_HEIGHT + COPTER_SIZE, (COPTER_HEIGHT + WING_HEIGHT) * 0.5, WING_HEIGHT + COPTER_SIZE);
   Vertices[19].vPos = XMFLOAT3(WING_HEIGHT + COPTER_SIZE, (COPTER_HEIGHT + WING_HEIGHT) * 0.5, -WING_HEIGHT);


   UINT *indexes = new UINT[m_uIndexCount] {
      //body box
      0, 1, 2,
      0, 2, 3,
      4, 5, 6,
      4, 6, 7,
      0, 1, 5,
      0, 5, 4,
      1, 6, 5,
      1, 2, 6,
      2, 7, 3,
      2, 6, 7,
      1, 0, 7,
      0, 7, 4,

      4, 8, 16,
      4, 9, 16,
      8, 9, 16,

      5, 10, 17,
      5, 11, 17,
      11, 10, 17,

      6, 12, 18,
      6, 13, 18,
      12, 13, 18,

      7, 14, 19,
      7, 14, 19,
      14, 15, 19
   };


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

   D3D11_BUFFER_DESC IBufferDesc =
   {
      m_uIndexCount * sizeof(int),
      D3D11_USAGE_DEFAULT,
      D3D11_BIND_INDEX_BUFFER,
      0, 0
   };
   D3D11_SUBRESOURCE_DATA IBufferInitData;
   IBufferInitData.pSysMem = indexes;
   m_pD3DDevice->CreateBuffer(&IBufferDesc, &IBufferInitData, &m_pIndexBuffer);

   delete[] Vertices;
   delete[] indexes;
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
   m_pD3DDeviceCtx->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
   m_pD3DDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   m_pPass->Apply(0, m_pD3DDeviceCtx);
   m_pD3DDeviceCtx->DrawIndexed(m_uIndexCount, 0, 0);
}
