#include "mesh.h"

#include <DDSTextureLoader.h>
#include <DirectXMath.h>

Mesh::Mesh (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect *a_pEffect, XMFLOAT4& a_vPosAndRadius)
{
	m_pD3DDevice    = a_pD3DDevice;
	m_pD3DDeviceCtx = a_pD3DDeviceCtx;
	m_vPosAndRadius = a_vPosAndRadius;
	m_uVertexStride = sizeof(MeshVertex);
	m_uIndexStride  = sizeof(UINT32);
	m_uVertexOffset = 0;
	m_uIndexOffset  = 0;

	///* just one technique in effect */
	ID3DX11EffectTechnique *pTechnique = a_pEffect->GetTechniqueByIndex(0);
	m_pPass = pTechnique->GetPassByName("RenderMeshPass");
	m_pTextureESRV = a_pEffect->GetVariableByName("g_txMeshDiffuse")->AsShaderResource();
	CreateDDSTextureFromFile(m_pD3DDevice, L"resources/test.dds", nullptr, &m_pTextureSRV);
	m_pTextureESRV->SetResource(m_pTextureSRV);

	CreateVertexBuffer();

	XMMATRIX newTransform = XMMatrixTranslation(a_vPosAndRadius.x, a_vPosAndRadius.y, a_vPosAndRadius.z);
	XMStoreFloat4x4(&m_mTransform, newTransform);

	m_pTransformEMV = a_pEffect->GetVariableByName("g_mWorld")->AsMatrix();
	CreateInputLayout();

	XMStoreFloat4x4(&m_mRotation, XMMatrixIdentity());
	m_Angle = 0;
}

Mesh::~Mesh()
{
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pTextureSRV);
}

/*zero-centered sphere*/
void Mesh::CreateVertexBuffer()
{
	/* Initializing vertices */
	int i, j, ind;
	int NumStacks = 20;
	int NumSlices = 20;
	float _2PI = 2.0f * (float)M_PI;
	m_uVertexCount = NumSlices * (NumStacks + 1);
	m_uIndexCount = 6 * NumSlices * NumStacks;
	MeshVertex* Vertices = new MeshVertex[m_uVertexCount];
	UINT32* Indices = new UINT32[m_uIndexCount];

	for (i = 0; i < NumStacks; i++)
	{
		for (j = -NumSlices / 2; j < NumSlices / 2; j++)
		{
			ind = (i)* NumSlices + j + NumSlices / 2;
			Vertices[ind].vPos.y = m_vPosAndRadius.w * cos(float(i) / NumStacks * (float)M_PI);
			Vertices[ind].vPos.x = m_vPosAndRadius.w * cos(float(j) / NumSlices * (float)_2PI) * sin(float(i) / NumStacks * (float)M_PI);
			Vertices[ind].vPos.z = m_vPosAndRadius.w * sin(float(j) / NumSlices * (float)_2PI) * sin(float(i) / NumStacks * (float)M_PI);
			Vertices[ind].vTexCoord.y = float(j + NumSlices / 2) / NumSlices;
			Vertices[ind].vTexCoord.x = float(i) / NumStacks;
		}
	}

	ind = 0;
	for (i = 1; i <= NumStacks; i++)
	{
		for (j = 1; j < NumSlices; j++)
		{
			Indices[ind++] = i * NumSlices + j;
			Indices[ind++] = (i - 1) * NumSlices + j;
			Indices[ind++] = (i - 1) * NumSlices + (j - 1);

			Indices[ind++] = i * NumSlices + j;
			Indices[ind++] = (i - 1) * NumSlices + (j - 1);
			Indices[ind++] = i * NumSlices + (j - 1);
		}

		Indices[ind++] = i * NumSlices;
		Indices[ind++] = (i - 1) * NumSlices;
		Indices[ind++] = i * NumSlices - 1;

		Indices[ind++] = i * NumSlices;
		Indices[ind++] = i * NumSlices - 1;
		Indices[ind++] = (i + 1) * NumSlices - 1;
	}


	/* Initializing buffer */
	D3D11_BUFFER_DESC VBufferDesc =
	{
		m_uVertexCount * sizeof(MeshVertex),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_VERTEX_BUFFER,
		0, 0
	};
	D3D11_SUBRESOURCE_DATA VBufferInitData;
	VBufferInitData.pSysMem = Vertices;
	m_pD3DDevice->CreateBuffer(&VBufferDesc, &VBufferInitData, &m_pVertexBuffer);

	D3D11_BUFFER_DESC IBufferDesc =
	{
		m_uIndexCount * sizeof(UINT32),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_INDEX_BUFFER,
		0, 0
	};
	D3D11_SUBRESOURCE_DATA IBufferInitData;
	IBufferInitData.pSysMem = Indices;
	m_pD3DDevice->CreateBuffer(&IBufferDesc, &IBufferInitData, &m_pIndexBuffer);
	delete[] Indices;
	delete[] Vertices;
}

void Mesh::CreateInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC InputDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	D3DX11_PASS_DESC PassDesc;
	m_pPass->GetDesc(&PassDesc);
	int InputElementsCount = sizeof(InputDesc) / sizeof(D3D10_INPUT_ELEMENT_DESC);
	m_pD3DDevice->CreateInputLayout(InputDesc, InputElementsCount,
		PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize,
		&m_pInputLayout);
}

XMFLOAT4 Mesh::GetPosAndRadius()
{
	return m_vPosAndRadius;
}

void Mesh::SetPosAndRadius(XMFLOAT4& a_vPosAndRadius)
{
	XMVECTOR prevPos = XMLoadFloat3((XMFLOAT3*)& m_vPosAndRadius);
	XMStoreFloat3(&m_vPrevPos, prevPos);
	m_vPosAndRadius = a_vPosAndRadius;
	XMVECTOR newPos = XMLoadFloat3((XMFLOAT3*)& m_vPosAndRadius);

	XMVECTOR delta = XMVectorSubtract(newPos, prevPos);
	delta = XMVectorSet(XMVectorGetX(delta), 0, XMVectorGetY(delta), XMVectorGetW(delta));

	static XMFLOAT3 float3Y{ 0.0f, 1.0f, 0.0f };
	static XMVECTOR vY = XMLoadFloat3((XMFLOAT3*)& float3Y);

	XMVECTOR vRotAxis;
	vRotAxis = XMVector3Cross(vY, delta);
	vRotAxis = XMVector3Normalize(vRotAxis);

	XMMATRIX newTranslation = XMMatrixTranslation(m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z);
	XMStoreFloat4x4(&m_mTranslation, newTranslation);

	m_Angle += XMVectorGetX(XMVector3Length(delta)) / m_vPosAndRadius.w;
	XMMATRIX newRot = XMMatrixRotationAxis(vRotAxis, m_Angle);
	XMStoreFloat4x4(&m_mRotation, newRot);

	XMMATRIX newTransform = XMMatrixMultiply(newRot, newTranslation);

	delta = XMVector3Normalize(delta);
	XMStoreFloat3(&m_vMoveDir, delta);
}

XMVECTOR Mesh::GetMoveDir (void)
{
	return XMLoadFloat3(&m_vMoveDir);
}

XMFLOAT4X4 Mesh::GetMatr (void)
{
	return m_mMatr;
}

void Mesh::SetHeight(float a_fH)
{
	m_vPosAndRadius.y = a_fH;
	XMMATRIX newTranslation = XMMatrixTranslation(m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z);
	XMStoreFloat4x4(&m_mTranslation, newTranslation);

	XMMATRIX rot = XMLoadFloat4x4(&m_mRotation);
	XMMATRIX newTransform = XMMatrixMultiply(rot, newTranslation);

	XMStoreFloat4x4(&m_mTransform, newTransform);
}

void Mesh::Render()
{
	//m_pTransformEMV->SetMatrix((float *)&m_mTransform);
	m_pD3DDeviceCtx->IASetInputLayout(m_pInputLayout);
	m_pD3DDeviceCtx->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
	m_pD3DDeviceCtx->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pD3DDeviceCtx->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pPass->Apply(0, m_pD3DDeviceCtx);
	m_pD3DDeviceCtx->DrawIndexed(m_uIndexCount, 0, 0);
}

void Mesh::SetTransform(XMFLOAT4X4& a_mTransform)
{
	m_mTransform = a_mTransform;
}

void Mesh::SetInvTransform(XMFLOAT4X4& a_mInvTransform)
{
	XMMATRIX in = XMLoadFloat4x4(&a_mInvTransform);
	XMMATRIX out = XMMatrixInverse(NULL, in);
	XMStoreFloat4x4(&m_mTransform, out);
}
