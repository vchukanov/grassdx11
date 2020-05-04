#pragma once

#include "mesh.h"

class Plane : public Mesh {
public:
   enum MOVE_TYPE
   {
      MOVE_TANGENT = 0,
      MOVE_NORMAl
   };

   Plane (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect* a_pEffect, XMFLOAT4& a_vPosAndRadius,
      float a_fWidth, float a_fHeight, float a_fWheelBeg, float a_fWheelEnd);


   virtual void     SetPosAndRadius (XMFLOAT4& a_vPosAndRadius) override;
   virtual void     SetHeight        (float a_fH) override;
   virtual XMFLOAT4 GetPosAndRadius (void) override;
   virtual void     Render           (void) override;

   virtual void     SetTransform    (XMFLOAT4X4& a_mTransform) override;
   virtual void     SetInvTransform (XMFLOAT4X4& a_mInvTransform) override;

   virtual bool  CheckCollision (XMVECTOR& Beg, XMVECTOR& End, float* Dist);
   virtual bool  Collide        (XMVECTOR* Ret, XMVECTOR& Beg, XMVECTOR& End, PhysPatch::BladePhysData* a_pBladePhysData);
   virtual float GetDist        (XMVECTOR& Pnt, bool* IsUnderWheel);
   virtual void  RotateToEdge   (XMVECTOR* Ret, XMVECTOR& Beg, XMVECTOR& End);

protected:
   virtual void CreateVertexBuffer(void);
   virtual void CreateInputLayout(void);

private:
   XMFLOAT4X4 m_mInvTransform;
   float m_fWidth;
   float m_fHeight;
   float m_fWheelBeg, m_fWheelEnd;

   ID3D11SamplerState* m_pSamplerLinear;
};