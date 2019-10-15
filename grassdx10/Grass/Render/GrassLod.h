#pragma once

#include <vector>
#include <algorithm>
#include "includes.h"
#include "GrassPatch.h"

struct D3DMATRIXEXT
{
	D3DXMATRIX mValue;
    UINT       uOnEdge;
	float      fCamDist;    
	D3DMATRIXEXT() {}
	D3DMATRIXEXT(const D3DXMATRIX& a_mValue, const float a_fCamDist)
	{
		mValue	       = a_mValue;
        uOnEdge = 0;
		fCamDist       = a_fCamDist;        
	}
};

class GrassLod
{
private:
    ID3D10Device             *m_pD3DDevice;
    GrassPatch               *m_pGrassPatch;
    std::vector< D3DMATRIXEXT > m_Transforms;
    UINT                      m_TransformsStride;
    UINT                      m_TransformsOffset;
    ID3D10Buffer             *m_pTransformsBuffer;

public:
    GrassLod                   ( GrassPatch *a_pPatch );
    ~GrassLod                  ( );
    void  SetTransformsCount   ( DWORD a_dwCount );
    DWORD GetTransformsCount   ( );
    /* Number of vertices in patch */
    DWORD VerticesCount        ( );
    D3DXMATRIX &GetTransform    ( DWORD a_dwIndex );
    void AddTransform          ( D3DXMATRIX &a_mTransform, float a_fCamDist, bool a_bIsCornerPatch, int a_iIndex = -1 );
    void GenTransformBuffer    ( );
    void UpdateTransformBuffer ( );
    /* Loading transforms into slot 1, patch into slot 0 */
    void IASetVertexBuffers    ( );
};