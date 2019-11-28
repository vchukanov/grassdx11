#include "plane.h"

#include <xmmintrin.h>
#include <cmath>
#include <DDSTextureLoader.h>


Plane::Plane (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect* a_pEffect, XMFLOAT4& a_vPosAndRadius,
	float a_fWidth, float a_fHeight, float a_fWheelBeg, float a_fWheelEnd) :
	m_fWidth(a_fWidth), m_fHeight(a_fHeight)
{
	m_pD3DDevice    = a_pD3DDevice;
	m_pD3DDeviceCtx = a_pD3DDeviceCtx;
	m_vPosAndRadius = a_vPosAndRadius;
	m_uVertexStride = sizeof(MeshVertex);
	m_uIndexStride  = sizeof(UINT32);
	m_uVertexOffset = 0;
	m_uIndexOffset  = 0;
	m_pIndexBuffer  = NULL;
	//m_fWheelBeg = a_fWheelBeg;
	//m_fWheelEnd = a_fWheelEnd;

	// Render if we need 
	ID3DX11EffectTechnique* pTechnique = a_pEffect->GetTechniqueByIndex(0);
	m_pPass = pTechnique->GetPassByName("RenderMeshPass");

	m_pTextureESRV = a_pEffect->GetVariableByName("g_txMeshDiffuse")->AsShaderResource();
	CreateDDSTextureFromFile(m_pD3DDevice, L"resources/test.dds", nullptr, &m_pTextureSRV);
	
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	m_pTextureSRV->GetDesc(&desc);

	m_pTextureESRV->SetResource(m_pTextureSRV);


	XMMATRIX newTransform = XMMatrixTranslation(a_vPosAndRadius.x, a_vPosAndRadius.y, a_vPosAndRadius.z);
	XMStoreFloat4x4(&m_mTransform, newTransform);

	CreateVertexBuffer();
	CreateInputLayout();

	XMStoreFloat4x4(&m_mTranslation, XMMatrixIdentity());
	XMStoreFloat4x4(&m_mRotation,    XMMatrixIdentity());
}

void Plane::CreateVertexBuffer (void)
{
	MeshVertex Vertices[4];

	Vertices[3].vPos = XMFLOAT3(-m_fWidth, -m_fHeight, 0.0f);
	Vertices[3].vTexCoord = XMFLOAT2(0.0f, 1.0f);
	Vertices[2].vPos = XMFLOAT3(m_fWidth, -m_fHeight, 0.0f);
	Vertices[2].vTexCoord = XMFLOAT2(1.0f, 1.0f);
	Vertices[1].vPos = XMFLOAT3(-m_fWidth, m_fHeight, 0.0f);
	Vertices[1].vTexCoord = XMFLOAT2(0.0f, 0.0f);
	Vertices[0].vPos = XMFLOAT3(m_fWidth, m_fHeight, 0.0f);
	Vertices[0].vTexCoord = XMFLOAT2(1.0f, 0.0f);

	D3D11_BUFFER_DESC BufferDesc =
	{
		4 * sizeof(MeshVertex),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_VERTEX_BUFFER,
		0, 0
	};
	D3D11_SUBRESOURCE_DATA BufferInitData;
	BufferInitData.pSysMem = Vertices;
	BufferInitData.SysMemPitch = 0;
	BufferInitData.SysMemSlicePitch = 0;
	m_pD3DDevice->CreateBuffer(&BufferDesc, &BufferInitData, &m_pVertexBuffer);
}

void Plane::CreateInputLayout (void)
{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// Create the input layout
	D3DX11_PASS_DESC PassDesc;
	m_pPass->GetDesc(&PassDesc);
	m_pD3DDevice->CreateInputLayout(layout, 2, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &m_pInputLayout);
}

void Plane::Render(void)
{
	//D3DXMatrixTranslation(&m_mTransform, m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z);
	//D3DXMatrixInverse(&m_mInvTransform, NULL, &m_mTransform);

	//m_pTransformEMV->SetMatrix((FLOAT*)m_mTransform);

	m_pD3DDeviceCtx->IASetInputLayout(m_pInputLayout);
	m_pD3DDeviceCtx->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
	m_pD3DDeviceCtx->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//m_pD3DDeviceCtx->PSSetShaderResources(0, 1, &m_pTextureSRV);
	//m_pD3DDeviceCtx->PSSetSamplers(0, 1, &m_pSamplerLinear);
	
	m_pPass->Apply(0, m_pD3DDeviceCtx);
	m_pD3DDeviceCtx->Draw(4, 0);
}

/*
static void SuperNormalize( XMFLOAT3 *Out, XMFLOAT3 *In )
{
	__m128 v = _mm_set_ps(0.0f, In->z, In->y, In->x);
	__m128 sq_v;

	sq_v = _mm_mul_ps(v, v);

}
*/


XMFLOAT4 Plane::GetPosAndRadius(void)
{
	if (m_fWidth > m_fHeight)
		return XMFLOAT4(m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z, m_fWidth);
	else
		return XMFLOAT4(m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z, m_fHeight);
}

void Plane::SetPosAndRadius(XMFLOAT4& a_vPosAndRadius)
{
	m_vPosAndRadius = a_vPosAndRadius;
}

void Plane::SetHeight(float a_fH)
{
	;
}