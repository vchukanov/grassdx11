#pragma once
#include "includes.h"

struct GrassVertex
{
   /* Start vertex position */
   XMFLOAT3    vPos;
   XMFLOAT3    vRotAxe;
   XMFLOAT3    vYRotAxe;
   XMFLOAT3    vColor;
   /* Transparency factor */
   float       fTransparency;
};

class GrassPatch
{
   friend class GrassPatchLod1;
   friend class GrassPatchLod2;
   friend class PhysPatch;

protected:
   GrassVertex              *m_pVertices;
   DWORD                     m_dwVerticesCount;
   UINT                      m_dwVertexStride;
   UINT                      m_dwVertexOffset;

   float                     m_fPatchSize;
   DWORD                     m_dwBladesPerSide;
   float                     m_fBladeWidth;
   float                     m_fBladeHeight;
   /* directx11 variables */
   ID3D11Buffer        *m_pVertexBuffer;
   ID3D11Device        *m_pD3DDevice;
   ID3D11DeviceContext *m_pD3DDeviceCtx;

   void          GenerateBuffers (void);
   virtual void  GeneratePatch   (void);

public:
   GrassPatch (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, float a_fPatchSize, DWORD a_dwBladesPerSide);
   GrassPatch (GrassPatch& a_Patch);
   
   virtual ~GrassPatch (void);

   virtual void  IASetVertexBuffer0 (void);
   DWORD         VerticesCount      (void);
   float         GetPatchSize       (void);

   ID3D11Device*        GetD3DDevicePtr    (void);
   ID3D11DeviceContext* GetD3DDeviceCtxPtr (void);

   GrassPatch& operator = (GrassPatch& a_Patch);
};

class GrassPatchLod0 : public GrassPatch
{
private:
   void GeneratePatch (void);
   void GenerateBlade (DWORD* a_pCurVerticesIndex, XMFLOAT3* a_pPivotPt, float a_fTransparency);

public:
   GrassPatchLod0  (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, float a_fPatchSize, DWORD a_dwBladesPerSide);
   ~GrassPatchLod0 (void);
};

/* GrassPatchLod1
 * is exactly the same as the GrassPatchLod0,
 * except 2 functions: c-tor and GeneratePatch()
 */
class GrassPatchLod1 : public GrassPatch
{
private:
   void GeneratePatch (GrassPatch* a_pBasePatch);

public:
   GrassPatchLod1 (GrassPatch* a_pBasePatch);
};

class GrassPatchLod2 : public GrassPatch
{
private:
   void GeneratePatch (GrassPatch* a_pBasePatch);

public:
   GrassPatchLod2 (GrassPatch* a_pBasePatch);
};