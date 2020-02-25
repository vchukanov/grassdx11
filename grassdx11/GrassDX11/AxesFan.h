#pragma once
#ifndef __AXES_FAN_H__
#define __AXES_FAN_H__

#include "includes.h"

class AxesFan {

private:
   struct VertexType
   {
      XMFLOAT3 vPos;
      XMFLOAT2 textCoord;
   };

public:
   AxesFan  (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect* a_pEffect,
         int a_iBladesNum, float a_fBladeSize, float angleVel = 1.0f);

   ~AxesFan (void);

   void Render (void);
   void Update (float a_fElapsedTime);
   
   void SetViewProjMtx (const float4x4& a_mViewProj);
   void SetPosition    (const float3& a_vPosition);
   void SetDirection   (const float3& a_vDirection);
   void SetR           (float a_fR);

private:
   void CreateInputLayout (void);
   void CreateVertexBuffer (void);

private:
   ID3D11Device        *m_pD3DDevice;
   ID3D11DeviceContext *m_pD3DDeviceCtx;
   
   ID3DX11EffectMatrixVariable* m_pTransformEMV;
   ID3DX11EffectMatrixVariable* m_pViewProjEMV;

   ID3DX11EffectMatrixVariable *m_pWorldEMV;

   XMFLOAT3                     m_vPosition;
   XMFLOAT3                     m_vDirection;

   XMFLOAT4X4                   m_mTransform;
   XMFLOAT4X4                   m_mRotation;
   XMFLOAT4X4                   m_mTranslation;

   
   ID3DX11EffectShaderResourceVariable* m_pTexESRV;

   ID3D11InputLayout *m_pInputLayout;

   ID3DX11Effect     *m_pEffect;
   ID3DX11EffectPass *m_pPass;
   ID3D11Buffer      *m_pVertexBuffer = nullptr;
   
   UINT m_uVertexCount;
   UINT m_uVertexStride;
   UINT m_uVertexOffset;

   UINT  m_iBladesNum;
   float m_fBladeSize;

   float m_fAngleVel;
   XMFLOAT4X4 m_mRot;

   ID3DX11EffectScalarVariable* m_fScaleESV;
};

#endif