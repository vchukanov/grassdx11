#include "GrassLod.h"

/* GrassLod */
GrassLod::GrassLod(GrassPatch* a_pPatch)
{
	m_TransformsOffset = 0;
	m_TransformsStride = sizeof(XMMATRIXEXT);
	m_pGrassPatch = a_pPatch;
	m_pD3DDevice = m_pGrassPatch->GetD3DDevicePtr();
	m_pD3DDeviceCtx = m_pGrassPatch->GetD3DDeviceCtxPtr();
	m_pTransformsBuffer = NULL;
}

GrassLod::~GrassLod()
{
	delete m_pGrassPatch;//unsafe
	m_Transforms.clear();
	SAFE_RELEASE(m_pTransformsBuffer);
}

void GrassLod::AddTransform(XMFLOAT4X4& a_mTransform, float a_fCamDist, bool a_bIsCornerPatch, int a_iIndex /* = -1 */)
{
	if (a_iIndex == -1)
		m_Transforms.push_back(XMMATRIXEXT(a_mTransform, a_fCamDist));
	else
	{
		m_Transforms[a_iIndex].mValue = a_mTransform;
		m_Transforms[a_iIndex].fCamDist = a_fCamDist;
		m_Transforms[a_iIndex].uOnEdge = UINT(a_bIsCornerPatch);
	}
}

void GrassLod::SetTransformsCount(DWORD a_dwCount)
{
	//m_Transforms.clear();
	m_Transforms.resize(a_dwCount);
}

DWORD GrassLod::GetTransformsCount()
{
	return m_Transforms.size();
}

DWORD GrassLod::VerticesCount()
{
	return m_pGrassPatch->VerticesCount();
}

XMFLOAT4X4& GrassLod::GetTransform(DWORD a_dwIndex)
{
	return m_Transforms[a_dwIndex].mValue;
}

void GrassLod::IASetVertexBuffers()
{
	m_pGrassPatch->IASetVertexBuffer0();
	m_pD3DDeviceCtx->IASetVertexBuffers(1, 1, &m_pTransformsBuffer, &m_TransformsStride, &m_TransformsOffset);
}

void GrassLod::GenTransformBuffer()
{
	if (m_Transforms.empty())
		return;
	SAFE_RELEASE(m_pTransformsBuffer);

	/*Create buffer as dynamic*/
	D3D11_BUFFER_DESC bufferDesc =
	{
		m_Transforms.size() * sizeof(XMMATRIXEXT),
		D3D11_USAGE_DYNAMIC,
		D3D11_BIND_VERTEX_BUFFER,
		D3D11_CPU_ACCESS_WRITE,
		0
	};

	m_pD3DDevice->CreateBuffer(&bufferDesc, NULL, &m_pTransformsBuffer);

	D3D11_MAPPED_SUBRESOURCE pMatrices;
	
	m_pD3DDeviceCtx->Map(m_pTransformsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pMatrices);
	memcpy(pMatrices.pData, &m_Transforms[0], m_Transforms.size() * sizeof(XMMATRIXEXT));
	m_pD3DDeviceCtx->Unmap(m_pTransformsBuffer, 0);
}

bool CompareFunc(XMMATRIXEXT& a, XMMATRIXEXT& b)
{
	return a.fCamDist < b.fCamDist;
}

void GrassLod::UpdateTransformBuffer()
{
	if (m_Transforms.empty())
		return;

	D3D11_MAPPED_SUBRESOURCE pMatrices;
	m_pD3DDeviceCtx->Map(m_pTransformsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pMatrices);
	memcpy(pMatrices.pData, &m_Transforms[0], m_Transforms.size() * sizeof(XMMATRIXEXT));
	m_pD3DDeviceCtx->Unmap(m_pTransformsBuffer, 0);
}