#pragma once
#include "includes.h"
 
struct GrassVertex
{
    /* Start vertex position */
    D3DXVECTOR3 vPos;
    D3DXVECTOR3 vRotAxe;
    D3DXVECTOR3 vYRotAxe;
    D3DXVECTOR3 vColor;
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
    /* directx10 variables */
    ID3D10Buffer             *m_pVertexBuffer;
    ID3D10Device             *m_pD3DDevice;

    void          GenerateBuffers              ( );
    virtual void  GeneratePatch                ( );

public:
    GrassPatch                                ( ID3D10Device *a_pD3DDevice, float a_fPatchSize, DWORD a_dwBladesPerSide );
    GrassPatch                                ( GrassPatch& a_Patch );
    virtual ~GrassPatch                       ( );

    virtual void  IASetVertexBuffer0          ( );
    DWORD         VerticesCount               ( );
    float         GetPatchSize                ( );
    ID3D10Device *GetD3DDevicePtr             ( );
    GrassPatch& operator = ( GrassPatch &a_Patch );
};

class GrassPatchLod0: public GrassPatch
{
private:
    void GeneratePatch         ( );
    void GenerateBlade         ( DWORD *a_pCurVerticesIndex, D3DXVECTOR3 *a_pPivotPt, float a_fTransparency );

public:
    GrassPatchLod0             ( ID3D10Device *a_pD3DDevice, float a_fPatchSize, DWORD a_dwBladesPerSide );
    ~GrassPatchLod0            ( );
};

/* GrassPatchLod1 
 * is exactly the same as the GrassPatchLod0, 
 * except 2 functions: c-tor and GeneratePatch() 
 */
class GrassPatchLod1: public GrassPatch
{
private:
    void GeneratePatch ( GrassPatch *a_pBasePatch );

public:
    GrassPatchLod1             ( GrassPatch *a_pBasePatch );
};

class GrassPatchLod2: public GrassPatch
{
private:
    void GeneratePatch ( GrassPatch *a_pBasePatch );

public:
    GrassPatchLod2             ( GrassPatch *a_pBasePatch );
};