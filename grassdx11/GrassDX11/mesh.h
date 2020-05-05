#pragma once

#include "includes.h"
#include "PhysPatch.h"

struct MeshVertex
{   
    XMFLOAT3 vPos;
    XMFLOAT2 vTexCoord;
    XMFLOAT3 vNormal;
};


class Mesh
{
protected:
   ///* Just a simple textured sphere */
   ID3DX11EffectShaderResourceVariable *m_pTextureESRV  = NULL;
   ID3D11ShaderResourceView            *m_pTextureSRV   = NULL;
   ID3DX11EffectMatrixVariable         *m_pTransformEMV = NULL;
   ID3DX11EffectPass                   *m_pPass         = NULL;
   ID3D11Device                        *m_pD3DDevice    = NULL;
   ID3D11DeviceContext                 *m_pD3DDeviceCtx = NULL;
   ID3D11InputLayout                   *m_pInputLayout;
   ID3D11Buffer                        *m_pVertexBuffer = NULL;
   UINT                                 m_uVertexCount;
   UINT                                 m_uVertexStride;
   UINT                                 m_uVertexOffset;
   ID3D11Buffer                        *m_pIndexBuffer  = NULL;
   UINT                                 m_uIndexCount;
   UINT                                 m_uIndexStride;
   UINT                                 m_uIndexOffset;
   XMFLOAT4X4                           m_mTransform;
   XMFLOAT4X4                           m_mMatr;
   XMFLOAT4X4                           m_mRotation;
   XMFLOAT4X4                           m_mTranslation;
   XMFLOAT4                             m_vPosAndRadius;
   XMFLOAT3                             m_vPrevPos;
   XMFLOAT3                             m_vMoveDir;

   float                                m_Angle;

   void CreateVertexBuffer();
   void CreateInputLayout();
public:
   Mesh (void)
   {}

   Mesh (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect *a_pEffect, XMFLOAT4& a_vPosAndRadius);
   virtual ~Mesh ();

   virtual void        SetPosAndRadius (XMFLOAT4& a_vPosAndRadius);
   virtual void        SetHeight       (float a_fH);
   virtual void        SetTransform    (XMFLOAT4X4& a_mTransform);
   virtual void        SetInvTransform (XMFLOAT4X4& a_mInvTransform);
   virtual XMFLOAT4    GetPosAndRadius (void);
   virtual XMVECTOR    GetMoveDir      (void);
   virtual XMFLOAT4X4  GetMatr         (void);
   virtual void        Render          (void);


   virtual bool CheckCollision (XMVECTOR &Beg, XMVECTOR &End, float* Dist) { return false; }

   virtual bool Collide(XMVECTOR* Ret, XMVECTOR& Beg, XMVECTOR& End,
      PhysPatch::BladePhysData *a_pBladePhysData, int a_iSegmentIndex) 
   { return false; }

   virtual float GetDist      (XMVECTOR& Pnt, bool* IsUnderWheel) { return 0; }
   virtual void  RotateToEdge (XMVECTOR* Ret, XMVECTOR& Beg, XMVECTOR& End) {}
   virtual int   IsBottom     (XMVECTOR& Pnt, XMVECTOR& vNormal) { return 0; }
};