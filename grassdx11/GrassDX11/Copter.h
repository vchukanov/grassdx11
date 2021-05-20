#pragma once
#ifndef __COPTER_H__
#define __COPTER_H__

#include "includes.h"
#include "FlowManager.h"

class Copter {

private:
   struct VertexType
   {
      XMFLOAT3 vPos;
      XMFLOAT2 textCoord;
   };

public:
   Copter (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect* a_pEffect, FlowManager* flowManager);

   ~Copter (void);

   void Render (void);
   
   void UpdateFromTransform (const XMMATRIX& transform);

private:
   void CreateInputLayout(void);
   void CreateVertexBuffer(void);

public:
   ID3D11Device* m_pD3DDevice;
   ID3D11DeviceContext* m_pD3DDeviceCtx;

   ID3DX11EffectMatrixVariable* m_pTransformEMV;
   ID3DX11EffectMatrixVariable* m_pPrevTransformEMV;
 
   XMFLOAT4X4 m_mTransform;

   ID3DX11EffectShaderResourceVariable* m_pTexESRV;

   ID3D11InputLayout* m_pInputLayout;

   ID3DX11Effect* m_pEffect;
   ID3DX11EffectPass* m_pPass;
   
   ID3D11Buffer* m_pVertexBuffer = nullptr;
   ID3D11Buffer* m_pIndexBuffer  = nullptr;

   UINT m_uVertexCount;
   UINT m_uIndexCount;
   UINT m_uVertexStride;
   UINT m_uVertexOffset;

   std::vector<int> fanIds;

   FlowManager* flowManager;
};

#endif