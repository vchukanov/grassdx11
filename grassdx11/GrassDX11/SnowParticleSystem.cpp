#include "SnowParticleSystem.h"
#include <iostream>
#include <WICTextureLoader.h>
#include "ParticleShader.h"
#include "SimplexNoise.h"

SnowParticleSystem::SnowParticleSystem()
{
	m_Texture = nullptr;
	m_TextureView = nullptr;
	m_particleList = nullptr;
	m_instance = nullptr;
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_instanceBuffer = nullptr;
	m_uav = nullptr;
	m_uavTex = nullptr;
	m_snowCover = new float* [256];
	for (int i = 0; i < 256; ++i)
		m_snowCover[i] = new float[256];
	for (int i = 0; i < 256; ++i)
		for (int j = 0; j < 256; ++j)
			m_snowCover[i][j] = 0.f;
}

SnowParticleSystem::SnowParticleSystem(const SnowParticleSystem& other)
{
}

SnowParticleSystem::~SnowParticleSystem()
{
	for (int i = 0; i < 256; ++i) {
		delete[] m_snowCover[i];
	}
	delete[] m_snowCover;

	SAFE_DELETE(m_instance);
	SAFE_RELEASE(m_indexBuffer);
	SAFE_RELEASE(m_vertexBuffer);

	SAFE_DELETE_ARRAY(m_particleList);
	SAFE_RELEASE(m_Texture);
	SAFE_RELEASE(m_TextureView);

	SAFE_RELEASE(m_instanceBuffer);
	SAFE_RELEASE(m_uav);
	SAFE_RELEASE(m_uavTex);
}

bool SnowParticleSystem::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const WCHAR* textureFileName, int maxParticles)
{
	bool results;
	results = LoadTexture(device, deviceContext, textureFileName);
	if (!results)
		return false;

	results = InitializeParticleSystem(maxParticles);
	if (!results)
		return false;

	results = InitializeBuffers(device, deviceContext);
	if (!results)
		return false;

	return true;
}

bool SnowParticleSystem::Frame(float delta, ID3D11DeviceContext* deviceContext)
{
	bool results;

	EmitParticles(delta);

	UpdateParticles(delta);

	results = UpdateBuffers(deviceContext);
	if (!results)
		return false;
	return true;
}

void SnowParticleSystem::Render(ID3D11DeviceContext* deviceContext)
{
	RenderBuffers(deviceContext);
}

bool SnowParticleSystem::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const WCHAR* filename)
{
	HRESULT hr = DirectX::CreateWICTextureFromFile(device, deviceContext, filename, &m_Texture, &m_TextureView);
	return SUCCEEDED(hr);
}

bool SnowParticleSystem::InitializeParticleSystem(int maxParticles)
{
	//������� ����
	//m_particleDeviationX = 400.0f;
	//m_particleDeviationZ = 400.0f;
	m_particleDeviationX = 100.0f;
	m_particleDeviationZ = 100.0f;
	m_particleDeviationY = 0.0f;

	m_particleVeclocity = 1.0f;
	m_particleVelocityVariation = 0.0f;

	m_particleSize = 0.02f;

	m_particlePerSecond = 2000.f;

	m_maxParticles = m_instanceCount = maxParticles; // 5000 instances of one particle

	m_particleList = new ParticleType[m_maxParticles];
	if (!m_particleList)
		return false;

	m_currentParticleCount = 0;
	m_accumulatedTime = 0.f;

	return true;
}

bool SnowParticleSystem::InitializeBuffers(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	unsigned long* indices;
	VertexType* vertices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc, instanceBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData, instanceData;
	HRESULT result;

	m_vertexCount = 1;
	m_indexCount = m_vertexCount;

	vertices = new VertexType[m_vertexCount];
	if (!vertices)
		return false;

	indices = new unsigned long[m_indexCount];
	if (!indices)
		return false;

	memset(vertices, 0, sizeof(VertexType) * m_vertexCount);
	for (int i = 0; i < m_indexCount; ++i)
		indices[i] = i;

	int index = 0;
	vertices[index].position = XMFLOAT3(0, 0, 0);
	vertices[index].texture = XMFLOAT2(0, 0);
	vertices[index].color = XMFLOAT4(0, 0, 0, 0);
	++index;

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
		return false;

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
		return false;

	SAFE_DELETE_ARRAY(indices);
	SAFE_DELETE_ARRAY(vertices);

	//Begining of instanceing

	m_instance = new InstanceType[m_instanceCount];
	if (!m_instance)
		return false;

	instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	instanceBufferDesc.ByteWidth = sizeof(InstanceType) * m_instanceCount;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;

	instanceData.pSysMem = m_instance;
	instanceData.SysMemPitch = 0;
	instanceData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&instanceBufferDesc, &instanceData, &m_instanceBuffer);
	if (FAILED(result))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC srv;
	ZeroMemory(&srv, sizeof(srv));
	m_TextureView->GetDesc(&srv);

	memset(m_instance, 0, (sizeof(InstanceType) * m_instanceCount));

	return true;
}

void SnowParticleSystem::EmitParticles(float delta)
{
	if (m_particlePerSecond <= 0)
		return;
	m_accumulatedTime += delta;

	if (m_currentParticleCount != 0)
		UpdateCloudPosition();

	while (m_accumulatedTime > (1.0f / m_particlePerSecond))
	{
		m_accumulatedTime -= 1.0f / m_particlePerSecond;
		SpawnParticle();
	}

}

void SnowParticleSystem::SpawnParticle()
{
	bool found;
	float positionX, positionY, positionZ, velocity, red, green, blue;
	int index, i, j;
	if (m_currentParticleCount < m_maxParticles - 1)
	{
		positionX = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationX + m_cloudPos.x;
		positionY = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationY + m_cloudPos.y;
		positionZ = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationZ + m_cloudPos.z;

		m_particleList[m_currentParticleCount].initialPos = XMFLOAT3(positionX, positionY, positionZ);
		m_particleList[m_currentParticleCount].curPos = XMFLOAT3(positionX, positionY, positionZ);
		m_particleList[m_currentParticleCount].age = 0.0f;
		m_particleList[m_currentParticleCount].offset = (float)rand() / RAND_MAX;
		m_instance[m_currentParticleCount].position = m_particleList[m_currentParticleCount].initialPos;
		++m_currentParticleCount;
	}
}

void SnowParticleSystem::UpdateParticles(float delta)
{
	for (int i = 0; i < m_currentParticleCount; ++i)
	{
		m_particleList[i].age += delta;
		m_particleList[i].curPos = m_instance[i].position;
		if (m_instance[i].inTornado) {
			m_particleList[i].curPos.x += m_deltaTorandoPos.x;
			m_particleList[i].curPos.z += m_deltaTorandoPos.z;
		}
		if (m_instance[i].position.y <= 0.0f)
		//if (m_particleList[i].age > 80.0f)
		{
			if (isSnowCoverActive) {
				auto intCoord = GetIntCoord(XMFLOAT2(m_instance[i].position.x, m_instance[i].position.z));
				/*auto value = m_snowCover[intCoord.y][intCoord.x];
				if (value < 0.6f) {
					//absolute uniform
					float kernel = 0.f;
					kernel += 0.002f * 0.001f / (value > 0.001f ? value : 0.001f);
					kernel += m_snowCover[intCoord.y - 1 < 0 ? 0 : intCoord.y - 1][intCoord.x - 1 < 0 ? 0 : intCoord.x - 1];
					kernel += m_snowCover[intCoord.y - 1 < 0 ? 0 : intCoord.y - 1][intCoord.x];
					kernel += m_snowCover[intCoord.y - 1 < 0 ? 0 : intCoord.y - 1][intCoord.x + 1 > 255 ? 255 : intCoord.x + 1];
					kernel += m_snowCover[intCoord.y][intCoord.x + 1 > 255 ? 255 : intCoord.x + 1];
					kernel += m_snowCover[intCoord.y + 1 > 255 ? 255 : intCoord.y + 1][intCoord.x + 1 > 255 ? 255 : intCoord.x + 1];
					kernel += m_snowCover[intCoord.y + 1 > 255 ? 255 : intCoord.y + 1][intCoord.x];
					kernel += m_snowCover[intCoord.y + 1 > 255 ? 255 : intCoord.y + 1][intCoord.x - 1 < 0 ? 0 : intCoord.x - 1];
					kernel += m_snowCover[intCoord.y][intCoord.x - 1 < 0 ? 0 : intCoord.x - 1];

					float averageVal = kernel / 9.f;
				
					m_snowCover[intCoord.y - 1 < 0 ? 0 : intCoord.y - 1][intCoord.x - 1 < 0 ? 0 : intCoord.x - 1] = averageVal;
					m_snowCover[intCoord.y - 1 < 0 ? 0 : intCoord.y - 1][intCoord.x] = averageVal;
					m_snowCover[intCoord.y - 1 < 0 ? 0 : intCoord.y - 1][intCoord.x + 1 > 255 ? 255 : intCoord.x + 1] = averageVal;
					m_snowCover[intCoord.y][intCoord.x + 1 > 255 ? 255 : intCoord.x + 1] = averageVal;
					m_snowCover[intCoord.y + 1 > 255 ? 255 : intCoord.y + 1][intCoord.x + 1 > 255 ? 255 : intCoord.x + 1] = averageVal;
					m_snowCover[intCoord.y + 1 > 255 ? 255 : intCoord.y + 1][intCoord.x] = averageVal;
					m_snowCover[intCoord.y + 1 > 255 ? 255 : intCoord.y + 1][intCoord.x - 1 < 0 ? 0 : intCoord.x - 1] = averageVal;
					m_snowCover[intCoord.y][intCoord.x - 1 < 0 ? 0 : intCoord.x - 1] = averageVal;
					m_snowCover[intCoord.y][intCoord.x] = averageVal;\
			}*/

				//find least way
				float least = 1.f;
				int indY = 0, indX = 0;
				for (int j = intCoord.y - 1 < 0 ? 0 : intCoord.y - 1; j <= (intCoord.y + 1 > 255 ? 255 : intCoord.y + 1); ++j) {
					for (int k = intCoord.x - 1 < 0 ? 0 : intCoord.x - 1; k <= (intCoord.x + 1 > 255 ? 255 : intCoord.x + 1); ++k) {
						if (m_snowCover[j][k] < least) {
							least = m_snowCover[j][k];
							indY = j;
							indX = k;
						}
					}
				}
					
				float value = m_snowCover[indY][indX];
				if (value < 0.30f) {
					float addValue = 0.015f * 0.06f / (value > 0.06f ? value : 0.06f);
					m_snowCover[indY][indX] += addValue;
				}
				
			}

			++_reachGround;
			m_particlePerSecond = floorf((_reachGround * m_particlePerSecond + m_particleList[i].age) / _reachGround);

			std::swap(m_particleList[i], m_particleList[m_currentParticleCount - 1]);
			std::swap(m_instance[i], m_instance[m_currentParticleCount - 1]);
			--m_currentParticleCount;
		}
	}
	m_deltaTorandoPos = { 0.f,0.f,0.f };
}
void SnowParticleSystem::CalculateInstancePositions(int begin, int end)
{
	float x, y, z, offset, yAmplitude, age, yVelocity, angle, length;
	XMFLOAT3 initialPos, curPos;
	angle = SimplexNoise::turbulence(m_cloudPos.x / 50, m_cloudPos.z / 50, m_cloudPos.y, 0., 2) * PI * 2;
	length = SimplexNoise::turbulence(m_cloudPos.x / 10 + 4000, m_cloudPos.z / 10 + 4000, m_cloudPos.y, 0., 1);
	m_cloudPos.x += length * cos(angle);
	m_cloudPos.z += length * sin(angle);

	for (int i = begin; i < end; ++i)
	{
		offset = m_particleList[i].offset;
		yAmplitude = 0.5f;
		age = m_particleList[i].age;
		initialPos = m_particleList[i].initialPos;
		curPos = m_instance[i].position;
		yVelocity = -0.5f; //temp
		angle = SimplexNoise::turbulence(curPos.x / 50, curPos.z / 50, curPos.y, (double)age, 2) * PI * 2;
		length = SimplexNoise::turbulence(curPos.x / 10 + 4000, curPos.z / 10 + 4000, curPos.y, (double)age, 1);
		
		x = curPos.x + length * cos(angle);
		z = curPos.z + length * sin(angle);

		y = yAmplitude * sin(age * 0.5f * offset);
		y += yAmplitude * sin(age * 0.66f * offset);
		y += age * yVelocity + initialPos.y;


		m_instance[i].position = XMFLOAT3(x, y, z);
		m_instance[i].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void SnowParticleSystem::UpdateCloudPosition()
{
	long meanX = 0;
	long meanZ = 0;
	long countParticle = m_currentParticleCount;
	for (int i = 0; i < m_currentParticleCount; ++i)
	{
		if (isnan(m_instance[i].position.x) || isnan(m_instance[i].position.z)) {
			--countParticle;
		}
		else
		{
			meanX += m_instance[i].position.x;
			meanZ += m_instance[i].position.z;
		}
	}

	if (isCloudMovementActive) {
		m_cloudPos.x = meanX / countParticle;
		m_cloudPos.z = meanZ / countParticle;
	}

	if (!_tornadoActive || !_manualTornadoContral) {
		m_tornadoPos.x = m_cloudPos.x;
		m_tornadoPos.z = m_cloudPos.z;
	}
}

XMUINT2 SnowParticleSystem::GetIntCoord(XMFLOAT2 pos)
{
	static const float scaleFactor = 3.125f;
	static const XMINT2 shift = {128, 128};
	XMFLOAT2 scaledPos = { pos.x / scaleFactor, pos.y / scaleFactor };
	XMINT2 intPos = { (int)roundf(scaledPos.x), (int)roundf(scaledPos.y) };
	auto ans = XMUINT2(iclamp(intPos.x + shift.x, 0, 255), iclamp(intPos.y + shift.y, 0, 255));
	return ans;
}

bool SnowParticleSystem::UpdateBuffers(ID3D11DeviceContext* deviceContext)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InstanceType* instancePtr;

	/*int totalThreads = 8;

	float calculationsPerThread = (float)m_currentParticleCount / (float)totalThreads;

	for (int i = 0; i < totalThreads; ++i)
	{
		m_threads.push_back(std::thread(&SnowParticleSystem::CalculateInstancePositions, this, (int)((float)i * calculationsPerThread), (int)((float)(i + 1) * calculationsPerThread)));
	}
	for (int i = 0; i < m_threads.size(); ++i)
	{
		m_threads[i].join();
	}
	m_threads.clear();*/
	//CalculateInstancePositions(0, m_currentParticleCount);
	//m_pFlowManager->CreateFlowTextureOnZeroLevel();
	m_pParticleShader->CalculateInstancePositions(deviceContext/*, m_pFlowManager->m_pAxesFanFlow->GetAxesFanFlowTexture()*/);

	result = deviceContext->Map(m_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	instancePtr = (InstanceType*)mappedResource.pData;
	memcpy(instancePtr, (void*)m_instance, (sizeof(InstanceType) * m_instanceCount));
	deviceContext->Unmap(m_instanceBuffer, 0);

	return true;
}

void SnowParticleSystem::FillConstantDataBuffer(ID3D11DeviceContext* deviceContext, ID3D11Buffer* inputBuffer)
{
	HRESULT hr = S_OK;
	if (!m_currentParticleCount)
		return;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = deviceContext->Map(inputBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	if (SUCCEEDED(hr))
	{
		memcpy(mappedResource.pData, m_particleList, sizeof(ParticleType) * m_instanceCount);
		deviceContext->Unmap(inputBuffer, 0);
	}
}

void SnowParticleSystem::UpdatePosition(InstanceType* dataView)
{
	memcpy(m_instance, dataView, sizeof(InstanceType) * m_instanceCount);
}

void SnowParticleSystem::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int strides[2];
	unsigned int offsets[2];
	ID3D11Buffer* bufferPointers[2];

	strides[0] = sizeof(VertexType);
	strides[1] = sizeof(InstanceType);

	offsets[0] = 0;
	offsets[1] = 0;

	bufferPointers[0] = m_vertexBuffer;
	bufferPointers[1] = m_instanceBuffer;

	deviceContext->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
}