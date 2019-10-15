#include "GrassLod.h"

/* GrassLod */
GrassLod::GrassLod( GrassPatch *a_pPatch )
{
    m_TransformsOffset  = 0;
    m_TransformsStride  = sizeof(D3DMATRIXEXT);
    m_pGrassPatch       = a_pPatch;
    m_pD3DDevice        = m_pGrassPatch->GetD3DDevicePtr();
    m_pTransformsBuffer = NULL;
}

GrassLod::~GrassLod( )
{
    delete m_pGrassPatch;//unsafe
    m_Transforms.clear();
    SAFE_RELEASE(m_pTransformsBuffer);
}

void GrassLod::AddTransform( D3DXMATRIX &a_mTransform, float a_fCamDist, bool a_bIsCornerPatch, int a_iIndex /* = -1 */ )
{
    if (a_iIndex == -1)
		m_Transforms.push_back(D3DMATRIXEXT(a_mTransform, a_fCamDist));
    else
	{
        m_Transforms[a_iIndex].mValue = a_mTransform;
		m_Transforms[a_iIndex].fCamDist = a_fCamDist;
        m_Transforms[a_iIndex].uOnEdge = UINT(a_bIsCornerPatch);
	}
}

void GrassLod::SetTransformsCount( DWORD a_dwCount )
{
    //m_Transforms.clear();
    m_Transforms.resize(a_dwCount);
}

DWORD GrassLod::GetTransformsCount( )
{
    return m_Transforms.size();
}

DWORD GrassLod::VerticesCount( )
{
    return m_pGrassPatch->VerticesCount();
}

D3DXMATRIX &GrassLod::GetTransform( DWORD a_dwIndex )
{
    return m_Transforms[a_dwIndex].mValue;
}

void GrassLod::IASetVertexBuffers( )
{
    m_pGrassPatch->IASetVertexBuffer0(); 
    m_pD3DDevice->IASetVertexBuffers(1, 1, &m_pTransformsBuffer, &m_TransformsStride, &m_TransformsOffset);
}

void GrassLod::GenTransformBuffer( )
{
    if (m_Transforms.empty())
        return;
    SAFE_RELEASE(m_pTransformsBuffer);
    /*Create buffer as static*/
    /*D3D10_BUFFER_DESC BufferDesc =
    {
        m_Transforms.size() * sizeof( D3DXMATRIX ),
        D3D10_USAGE_DEFAULT,
        D3D10_BIND_VERTEX_BUFFER,
        0, 0
    };
    D3D10_SUBRESOURCE_DATA BufInitData;
    ZeroMemory(&BufInitData, sizeof(D3D10_SUBRESOURCE_DATA));

    BufInitData.pSysMem = &m_Transforms[0];
    m_pD3DDevice->CreateBuffer( &BufferDesc, &BufInitData, &m_pTransformsBuffer );*/
    /*Create buffer as dynamic*/
    D3D10_BUFFER_DESC bufferDesc =
    {
        m_Transforms.size() * sizeof( D3DMATRIXEXT ),
        D3D10_USAGE_DYNAMIC,
        D3D10_BIND_VERTEX_BUFFER,
        D3D10_CPU_ACCESS_WRITE,
        0
    };

    m_pD3DDevice->CreateBuffer( &bufferDesc, NULL, &m_pTransformsBuffer );

    D3DMATRIXEXT* pMatrices = NULL;
    m_pTransformsBuffer->Map( D3D10_MAP_WRITE_DISCARD, NULL, ( void** )&pMatrices );

    memcpy( pMatrices, &m_Transforms[0], m_Transforms.size() * sizeof( D3DMATRIXEXT ) );

    m_pTransformsBuffer->Unmap();
}

bool CompareFunc (D3DMATRIXEXT &a, D3DMATRIXEXT &b)
{
	return a.fCamDist < b.fCamDist;
}

void GrassLod::UpdateTransformBuffer( )
{
    if (m_Transforms.empty())
        return;

	//std::sort(m_Transforms.begin(), m_Transforms.end(), CompareFunc);
    D3DMATRIXEXT* pMatrices = NULL;
    m_pTransformsBuffer->Map( D3D10_MAP_WRITE_DISCARD, NULL, ( void** )&pMatrices );

    memcpy( pMatrices, &m_Transforms[0], m_Transforms.size() * sizeof( D3DMATRIXEXT ) );

    m_pTransformsBuffer->Unmap();
}