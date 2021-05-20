#pragma once

#include "mesh.h"
#include "plane.h"
#include "terrain.h"
#include "ModelLoader.h"

class Car : public Mesh
{
public:
   float param = 0;

   Car (ID3D11Device* dev, ID3D11DeviceContext* devcon, ID3DX11Effect* a_pEffect, XMVECTOR a_vPosAndRadius,
      Terrain* const a_pTerrain, float a_fHeightScale, float a_fGrassRadius,
      float a_fCarWidth, float a_fCarHeight, float a_fCarLength, float a_fAmplAngle);

   virtual ~Car (void);


   virtual void SetPosAndRadius (XMFLOAT4& a_vPosAndRadius);

   virtual void SetHeight(float a_fH);

   virtual XMFLOAT4 GetPosAndRadius (void);

   virtual void Render (void);


   virtual bool CheckCollision (XMVECTOR Beg, XMVECTOR End, float* Dist);

   virtual bool Collide (XMVECTOR* Ret, XMVECTOR& Beg, XMVECTOR& End,
      PhysPatch::BladePhysData* a_pBladePhysData, int a_iSegmentIndex);

   virtual float GetDist (XMVECTOR& Pnt, bool* IsUnderWheel);

   virtual void RotateToEdge (XMVECTOR* Ret, XMVECTOR& Beg, XMVECTOR& End)
   {
      ;
   }

   virtual int IsBottom(XMVECTOR& Pnt, XMVECTOR& vNormal);

private:
   // Car sizes
   float m_fCarLength;
   float m_fCarHeight;
   float m_fCarWidth;
   float m_fAmplAngle;
   float m_fCarBackWidth;

   // Plane sizes
   float m_fPlaneLength;
   float m_fPlaneWidth;
   float m_fPlaneHeight;

   // Inverse Matrix of bottom plane  
   XMFLOAT4X4 m_mInvBottom;

   // Use to convert normals to world space in shader
   XMMATRIX m_mNormalMatrix;
   //ID3DX11EffectMatrixVariable* m_pNormalMatrixEMV;

   // Planes for physics
   Plane* m_pPlanes[4];
   UINT m_uNumPlanes;

   // Car Mesh
   //CMeshLoader10 m_Mesh;
   ModelLoader* carModel;

   // Terrain height data
   Terrain* m_pTerrain;
   float m_fHeightScale, m_fGrassRadius;

   ID3DX11EffectShaderResourceVariable* m_pTexESRV;

   void CreateInputLayout(void);
};
