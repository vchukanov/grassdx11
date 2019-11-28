#include "GrassPatch.h"
#include "PhysMath.h"

/* GrassPatch */
GrassPatch::GrassPatch (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, float a_fPatchSize, DWORD a_dwBladesPerSide)
{
	m_fPatchSize = a_fPatchSize;
	/* m_dwBladesPerSide % 4 must be 0 */
	m_dwBladesPerSide = a_dwBladesPerSide;
	m_pD3DDevice = a_pD3DDevice;
	m_pD3DDeviceCtx = a_pD3DDeviceCtx;
	m_dwVertexStride = sizeof(GrassVertex);
	m_dwVertexOffset = 0;
	m_dwVerticesCount = m_dwBladesPerSide * m_dwBladesPerSide;
	m_pVertices = new GrassVertex[m_dwVerticesCount];
	m_pVertexBuffer = NULL;
}

GrassPatch::GrassPatch (GrassPatch& a_Patch)
{
	m_fPatchSize = a_Patch.m_fPatchSize;
	m_pD3DDevice = a_Patch.m_pD3DDevice;
	m_pD3DDeviceCtx = a_Patch.m_pD3DDeviceCtx;
	m_dwVertexStride = sizeof(GrassVertex);
	m_dwVertexOffset = 0;
	m_dwVerticesCount = a_Patch.m_dwBladesPerSide * a_Patch.m_dwBladesPerSide;
	m_pVertices = new GrassVertex[m_dwVerticesCount];
	memcpy_s(m_pVertices, m_dwVerticesCount * sizeof(GrassVertex), a_Patch.m_pVertices, m_dwVerticesCount * sizeof(GrassVertex));
	GenerateBuffers();
}

GrassPatch& GrassPatch::operator = (GrassPatch& a_Patch)
{
	if (m_pVertices)
		delete[] m_pVertices;
	SAFE_RELEASE(m_pVertexBuffer);
	m_fPatchSize = a_Patch.m_fPatchSize;
	m_pD3DDevice = a_Patch.m_pD3DDevice;
	m_pD3DDeviceCtx = a_Patch.m_pD3DDeviceCtx;
	m_dwVertexStride = sizeof(GrassVertex);
	m_dwVertexOffset = 0;
	m_dwVerticesCount = a_Patch.m_dwBladesPerSide * a_Patch.m_dwBladesPerSide;
	m_pVertices = new GrassVertex[m_dwVerticesCount];
	memcpy_s(m_pVertices, m_dwVerticesCount * sizeof(GrassVertex), a_Patch.m_pVertices, m_dwVerticesCount * sizeof(GrassVertex));
	GenerateBuffers();
	return *this;
}

DWORD GrassPatch::VerticesCount()
{
	return m_dwVerticesCount;
}

float GrassPatch::GetPatchSize()
{
	return m_fPatchSize;
}

ID3D11Device* GrassPatch::GetD3DDevicePtr()
{
	return m_pD3DDevice;
}

ID3D11DeviceContext* GrassPatch::GetD3DDeviceCtxPtr()
{
	return m_pD3DDeviceCtx;
}

void GrassPatch::GeneratePatch()
{

}

void GrassPatch::GenerateBuffers()
{
	D3D11_BUFFER_DESC bufferDesc =
	{
		m_dwVerticesCount * sizeof(GrassVertex),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_VERTEX_BUFFER,
		0, 0
	};
	D3D11_SUBRESOURCE_DATA vbInitData;
	ZeroMemory(&vbInitData, sizeof(D3D11_SUBRESOURCE_DATA));
	vbInitData.pSysMem = m_pVertices;
	m_pD3DDevice->CreateBuffer(&bufferDesc, &vbInitData, &m_pVertexBuffer);
}

void GrassPatch::IASetVertexBuffer0()
{
	m_pD3DDeviceCtx->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_dwVertexStride, &m_dwVertexOffset);
}

GrassPatch::~GrassPatch()
{
	delete[] m_pVertices;
	SAFE_RELEASE(m_pVertexBuffer);
}

/* GrassPatchLod0 */

GrassPatchLod0::GrassPatchLod0 (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, float a_fPatchSize, DWORD a_dwBladesPerSide)
	: GrassPatch(a_pD3DDevice, a_pD3DDeviceCtx, a_fPatchSize, a_dwBladesPerSide - a_dwBladesPerSide % 4 + 4)
{
	GeneratePatch();
	GenerateBuffers();
}

GrassPatchLod0::~GrassPatchLod0()
{

}


void GrassPatchLod0::GeneratePatch()
{
	DWORD dw4x4BlockX;
	DWORD dw4x4BlockZ;
	DWORD dwTheChoosenOneX;
	DWORD dwTheChoosenOneZ;
	/* Transparency values for 4x4 grass blade block */
	float f4x4BlockAlpha[16];
	float f4LowAlpha[4];
	DWORD i, j;
	DWORD dwStartVertIndex = 0;
	XMFLOAT3 vPivotPt = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float fBlockSize = m_fPatchSize / m_dwBladesPerSide;

	for (dw4x4BlockZ = 0; dw4x4BlockZ < m_dwBladesPerSide; dw4x4BlockZ += 4)
	{
		for (dw4x4BlockX = 0; dw4x4BlockX < m_dwBladesPerSide; dw4x4BlockX += 4)
		{
			for (i = 0; i < 4; ++i)
				f4LowAlpha[i] = fRand(0.0825f) + i * 0.0825f;// 0.0825 = 1/4 * 0.33
			for (i = 0; i < 4; ++i)
				for (j = 0; j < 4; ++j)
				{
					f4x4BlockAlpha[i * 4 + j] = f4LowAlpha[rand() % 4];//fRand(0.33f);
				}
			/* Choosing the most non-transparent blade */
			dwTheChoosenOneX = rand() % 4;
			dwTheChoosenOneZ = rand() % 4;
			f4x4BlockAlpha[dwTheChoosenOneZ * 4 + dwTheChoosenOneX] = fRand(0.66f, 1.0f);
			/* Choosing the most non-transparent blade in each 2x2 block */
			if (dwTheChoosenOneZ > 2 || dwTheChoosenOneX > 2)
			{
				f4x4BlockAlpha[(rand() % 2) * 4 + (rand() % 2)] = f4LowAlpha[rand() % 4] + 0.33f;//fRand(0.33f, 0.66f);
			}

			if (dwTheChoosenOneZ < 2 || dwTheChoosenOneX > 2)
			{
				f4x4BlockAlpha[(rand() % 2 + 2) * 4 + (rand() % 2)] = f4LowAlpha[rand() % 4] + 0.33f;//fRand(0.33f, 0.66f);
			}

			if (dwTheChoosenOneZ > 2 || dwTheChoosenOneX < 2)
			{
				f4x4BlockAlpha[(rand() % 2) * 4 + (rand() % 2 + 2)] = f4LowAlpha[rand() % 4] + 0.33f;//fRand(0.33f, 0.66f);
			}

			if (dwTheChoosenOneZ < 2 || dwTheChoosenOneX < 2)
			{
				f4x4BlockAlpha[(rand() % 2 + 2) * 4 + (rand() % 2 + 2)] = f4LowAlpha[rand() % 4] + 0.33f;//fRand(0.33f, 0.66f);
			}


			/* zero-centered patch */
			for (i = 0; i < 4; ++i)
				for (j = 0; j < 4; ++j)
				{
					vPivotPt.x = (float(i + dw4x4BlockX) + fRand(1.0f)) * fBlockSize - m_fPatchSize / 2.0f;
					vPivotPt.z = (float(j + dw4x4BlockZ) + fRand(1.0f)) * fBlockSize - m_fPatchSize / 2.0f;
					vPivotPt.y = 0.0f;
					GenerateBlade(&dwStartVertIndex, &vPivotPt, f4x4BlockAlpha[i * 4 + j]);
				}
		}
	}
}

void GrassPatchLod0::GenerateBlade (DWORD* a_pCurVerticesIndex, XMFLOAT3* a_pPivotPt, float a_fTransparency)
{
	DWORD& dwVertInd = *a_pCurVerticesIndex;
	m_pVertices[dwVertInd].vPos = *a_pPivotPt;
	m_pVertices[dwVertInd].fTransparency = a_fTransparency;
	m_pVertices[dwVertInd].vRotAxe.y = 0.0f;
	m_pVertices[dwVertInd].vRotAxe.x = SignedfRand(8.5f) + 1.0f;
	m_pVertices[dwVertInd].vRotAxe.z = SignedfRand(9.5f);

	XM_TO_V(m_pVertices[dwVertInd].vRotAxe, rotAxe, 3);
	float fLen = XMVectorGetX(XMVector3Length(rotAxe));
	rotAxe *= (fRand(8.f) / fLen);
	//    m_pVertices[dwVertInd].vRotAxe *= (fRand(10.f)/fLen);
	XMStoreFloat3(&m_pVertices[dwVertInd].vRotAxe, rotAxe);

	m_pVertices[dwVertInd].vYRotAxe.y = 1.0f;
	m_pVertices[dwVertInd].vYRotAxe.x = 0.0;
	m_pVertices[dwVertInd].vYRotAxe.z = 0.0;
	
	XM_TO_V(m_pVertices[dwVertInd].vYRotAxe, YrotAxe, 3);
	fLen = XMVectorGetX(XMVector3Length(YrotAxe));
	YrotAxe *= (fRand(90.f) / fLen);
	XMStoreFloat3(&m_pVertices[dwVertInd].vYRotAxe, YrotAxe);

	//m_pVertices[dwVertInd].vRotAxe.y = 0.0f;
	//m_pVertices[dwVertInd].vRotAxe.x = 0.0f;
	//m_pVertices[dwVertInd].vRotAxe.z = 0.0f;
	//m_pVertices[dwVertInd].vYRotAxe.y = 0.0;
	//m_pVertices[dwVertInd].vYRotAxe.x = 0.0;
	//m_pVertices[dwVertInd].vYRotAxe.z = 0.0;

	std::pair<float3, float> colors[4] = {
		std::pair<float3, float>(create(0.14f, 0.33f, 0.00f), 0.7f),
		std::pair<float3, float>(create(0.33f, 0.42f, 0.06f), 0.1f),
		std::pair<float3, float>(create(0.15f, 0.40f, 0.00f), 0.1f),
		std::pair<float3, float>(create(0.34f, 0.60f, 0.00f), 0.1f),
	};

	float rand_param = fRand(1);
	for (UINT i = 0; i < sizeof(colors) / sizeof(colors[0]); ++i)
		if (rand_param <= colors[i].second)
		{
			XMStoreFloat3(&m_pVertices[dwVertInd].vColor, colors[i].first);
			break;
		}
		else
			rand_param -= colors[i].second;

	dwVertInd++;
}

/* GrassPatchLod1 */

GrassPatchLod1::GrassPatchLod1(GrassPatch* a_pBasePatch)
	: GrassPatch(a_pBasePatch->m_pD3DDevice, a_pBasePatch->m_pD3DDeviceCtx, a_pBasePatch->m_fPatchSize, a_pBasePatch->m_dwBladesPerSide / 2)
{
	GeneratePatch(a_pBasePatch);
	GenerateBuffers();
}

void GrassPatchLod1::GeneratePatch(GrassPatch* a_pBasePatch)
{
	DWORD i, j;
	DWORD dwBladeX;
	DWORD dwBladeZ;
	DWORD dwBaseVertIndex;
	DWORD dwStartVertIndex = 0;
	/* Each 16 vertices in a_pBasePatch representing 16 grass blades in 4x4 block */
	for (dwBaseVertIndex = 0; dwBaseVertIndex < a_pBasePatch->m_dwVerticesCount; dwBaseVertIndex += 16)
	{
		dwBladeZ = 0;
		dwBladeX = 0;
		for (i = 0; i < 2; ++i)
			for (j = 0; j < 2; ++j)
			{
				if (a_pBasePatch->m_pVertices[dwBaseVertIndex + i * 4 + j].fTransparency >
					a_pBasePatch->m_pVertices[dwBaseVertIndex + dwBladeZ * 4 + dwBladeX].fTransparency)
				{
					dwBladeZ = i;
					dwBladeX = j;
				}
			}
		m_pVertices[dwStartVertIndex++] = a_pBasePatch->m_pVertices[dwBaseVertIndex + dwBladeZ * 4 + dwBladeX];

		dwBladeZ = 2;
		dwBladeX = 0;
		for (i = 2; i < 4; ++i)
			for (j = 0; j < 2; ++j)
			{
				if (a_pBasePatch->m_pVertices[dwBaseVertIndex + i * 4 + j].fTransparency >
					a_pBasePatch->m_pVertices[dwBaseVertIndex + dwBladeZ * 4 + dwBladeX].fTransparency)
				{
					dwBladeZ = i;
					dwBladeX = j;
				}
			}
		m_pVertices[dwStartVertIndex++] = a_pBasePatch->m_pVertices[dwBaseVertIndex + dwBladeZ * 4 + dwBladeX];

		dwBladeZ = 0;
		dwBladeX = 2;
		for (i = 0; i < 2; ++i)
			for (j = 2; j < 4; ++j)
			{
				if (a_pBasePatch->m_pVertices[dwBaseVertIndex + i * 4 + j].fTransparency >
					a_pBasePatch->m_pVertices[dwBaseVertIndex + dwBladeZ * 4 + dwBladeX].fTransparency)
				{
					dwBladeZ = i;
					dwBladeX = j;
				}
			}
		m_pVertices[dwStartVertIndex++] = a_pBasePatch->m_pVertices[dwBaseVertIndex + dwBladeZ * 4 + dwBladeX];

		dwBladeZ = 2;
		dwBladeX = 2;
		for (i = 2; i < 4; ++i)
			for (j = 2; j < 4; ++j)
			{
				if (a_pBasePatch->m_pVertices[dwBaseVertIndex + i * 4 + j].fTransparency >
					a_pBasePatch->m_pVertices[dwBaseVertIndex + dwBladeZ * 4 + dwBladeX].fTransparency)
				{
					dwBladeZ = i;
					dwBladeX = j;
				}
			}
		m_pVertices[dwStartVertIndex++] = a_pBasePatch->m_pVertices[dwBaseVertIndex + dwBladeZ * 4 + dwBladeX];
	}
}

/* GrassPatchLod2 */

GrassPatchLod2::GrassPatchLod2(GrassPatch* a_pBasePatch)
	: GrassPatch(a_pBasePatch->m_pD3DDevice, a_pBasePatch->m_pD3DDeviceCtx, a_pBasePatch->m_fPatchSize, a_pBasePatch->m_dwBladesPerSide / 2)
{
	GeneratePatch(a_pBasePatch);
	GenerateBuffers();
}

void GrassPatchLod2::GeneratePatch(GrassPatch* a_pBasePatch)
{
	DWORD i, j;
	DWORD dwBladeX;
	DWORD dwBladeZ;
	DWORD dwBaseVertIndex;
	DWORD dwStartVertIndex = 0;
	/* Each 4 vertices in a_pBasePatch representing 4 grass blades in 2x2 block */
	for (dwBaseVertIndex = 0; dwBaseVertIndex < a_pBasePatch->m_dwVerticesCount; dwBaseVertIndex += 4)
	{
		dwBladeZ = 0;
		dwBladeX = 0;
		for (i = 0; i < 2; ++i)
			for (j = 0; j < 2; ++j)
			{
				if (a_pBasePatch->m_pVertices[dwBaseVertIndex + i * 2 + j].fTransparency >
					a_pBasePatch->m_pVertices[dwBaseVertIndex + dwBladeZ * 2 + dwBladeX].fTransparency)
				{
					dwBladeZ = i;
					dwBladeX = j;
				}
			}
		m_pVertices[dwStartVertIndex++] = a_pBasePatch->m_pVertices[dwBaseVertIndex + dwBladeZ * 2 + dwBladeX];
	}
}