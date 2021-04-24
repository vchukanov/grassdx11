#include "ParticleShader.h"


ParticleShader::ParticleShader()
{
	m_vertexShader = nullptr;
	m_pixelShader = nullptr;
	m_layout = nullptr;
	m_matrixBuffer = nullptr;
	m_cameraBuffer = nullptr;
	m_sampleState = nullptr;

	mComputeShader = nullptr;

	// Read Only
	mInputBuffer = nullptr;
	mInputView = nullptr;

	// Read/Write
	mRWBuffer = nullptr;
	//mRWInputBuffer = nullptr;
	mRWOutputBuffer = nullptr;
	mRWUAV = nullptr;
}

ParticleShader::ParticleShader(const ParticleShader& other)
{
}

ParticleShader::~ParticleShader()
{
	SAFE_RELEASE(m_sampleState);
	SAFE_RELEASE(m_matrixBuffer);
	SAFE_RELEASE(m_cameraBuffer);
	SAFE_RELEASE(m_layout);
	SAFE_RELEASE(m_pixelShader);
	SAFE_RELEASE(m_vertexShader);
	SAFE_RELEASE(m_pSnowCoverMapSRV);
	SAFE_RELEASE(m_pSnowCoverMap);
	//===== Compute Shader Components =====
	SAFE_RELEASE(mComputeShader);

	// Read Only
	SAFE_RELEASE(mInputBuffer);
	SAFE_RELEASE(mInputView);

	// Read/Write
	SAFE_RELEASE(mRWBuffer);
	//SAFE_RELEASE(mRWInputBuffer);
	SAFE_RELEASE(mRWOutputBuffer);
	SAFE_RELEASE(mRWUAV);
}

bool ParticleShader::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, GrassFieldManager* grassFieldManager)
{
	bool result;
	m_pGrassFieldManager = grassFieldManager;
	
	result = InitializeShader(device, deviceContext, L"Shaders/SnowParticleVS.hlsl", L"Shaders/SnowParticlePS.hlsl", L"Shaders/SnowParticleGS.hlsl", L"Shaders/SnowParticleCS.hlsl");
	if (!result)
		return false;

	return true;
}

bool ParticleShader::Render(ID3D11DeviceContext* direct, SnowParticleSystem* particlesystem, CFirstPersonCamera* camera)
{
	bool result;

	result = SetShaderParameters(direct, camera, XMMatrixIdentity(), camera->GetViewMatrix(), camera->GetProjMatrix(), particlesystem->GetTexture());
	if (!result)
		return false;

	if (++m_frame % 60 == 0) {
		SetSnowCoverTexture(direct);
	}


	RenderShader(direct, particlesystem->GetVertexCount(), particlesystem->GetInstaceCount(), particlesystem->GetIndexCount());
	return true;
}

bool ParticleShader::InitializeShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const WCHAR* vsFilename, const WCHAR* psFilename, const WCHAR* gsFilename, const WCHAR* csFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage = nullptr;
	ID3D10Blob* vertexShaderBuffer = nullptr;
	ID3D10Blob* pixelShaderBuffer = nullptr;
	ID3D10Blob* geometryShaderBuffer = nullptr;
	ID3D10Blob* computeShaderBuffer = nullptr;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[5];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

	// Compile Vertex Shader
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "VS_main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		return false;
	}
	// Compile Pixel Shader
	result = D3DCompileFromFile(psFilename, NULL, NULL, "PS_main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		return false;
	}
	// Compile Geometry Shader
	result = D3DCompileFromFile(gsFilename, NULL, NULL, "GS_main", "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &geometryShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		return false;
	}

	// Compile Compute Shader
	result = D3DCompileFromFile(csFilename, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CS_main", "cs_5_0", D3DCOMPILE_DEBUG, 0, &computeShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputDebugStringA((char*)errorMessage->GetBufferPointer());
			errorMessage->Release();
		}
		return false;
	}

	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
		return false;

	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
		return false;

	result = device->CreateGeometryShader(geometryShaderBuffer->GetBufferPointer(), geometryShaderBuffer->GetBufferSize(), NULL, &m_geometryShader);
	if (FAILED(result))
		return false;

	result = device->CreateComputeShader(computeShaderBuffer->GetBufferPointer(), computeShaderBuffer->GetBufferSize(), NULL, &mComputeShader);
	if (FAILED(result))
		return false;
	SAFE_RELEASE(computeShaderBuffer);

	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "COLOR";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	polygonLayout[3].SemanticName = "TEXCOORD";
	polygonLayout[3].SemanticIndex = 1;
	polygonLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[3].InputSlot = 1;
	polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
	polygonLayout[3].InstanceDataStepRate = 1;

	polygonLayout[4].SemanticName = "COLOR";
	polygonLayout[4].SemanticIndex = 1;
	polygonLayout[4].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[4].InputSlot = 1;
	polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
	polygonLayout[4].InstanceDataStepRate = 1;

	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
		return false;

	SAFE_RELEASE(vertexShaderBuffer);
	SAFE_RELEASE(pixelShaderBuffer);

	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
		return false;

	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);
	if (FAILED(result))
		return false;

	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 4;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
		return false;

	/* NEW */
	CD3D11_TEXTURE2D_DESC dstTexDesc;

	dstTexDesc.Width = 256;
	dstTexDesc.Height = 256;
	dstTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	dstTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	dstTexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	dstTexDesc.Usage = D3D11_USAGE_DYNAMIC;
	dstTexDesc.MipLevels = 1;
	dstTexDesc.ArraySize = 1;
	dstTexDesc.MiscFlags = 0;
	dstTexDesc.SampleDesc.Count = 1;
	dstTexDesc.SampleDesc.Quality = 0;

	result = device->CreateTexture2D(&dstTexDesc, 0, &m_pSnowCoverMap);
	if (FAILED(result))
		return false;

	D3D11_MAPPED_SUBRESOURCE MappedTexture;
	deviceContext->Map(m_pSnowCoverMap, D3D11CalcSubresource(0, 0, 1), D3D11_MAP_WRITE_DISCARD, 0, &MappedTexture);

	float* pTexels = (float*)MappedTexture.pData;

	for (UINT row = 0; row < dstTexDesc.Height; row++)
	{
		UINT rowStart = row * MappedTexture.RowPitch / sizeof(float);
		for (UINT col = 0; col < dstTexDesc.Width; col++)
		{
			UINT colStart = col * 4;
			pTexels[rowStart + colStart + 0] = 0.0f;
			pTexels[rowStart + colStart + 1] = 0.0f;
			pTexels[rowStart + colStart + 2] = 0.0f;
			pTexels[rowStart + colStart + 3] = 0.0f;
		}
	}
	deviceContext->Unmap(m_pSnowCoverMap, D3D11CalcSubresource(0, 0, 1));

	/* And creating SRV for it */
	D3D11_SHADER_RESOURCE_VIEW_DESC SnowCoverMapSRVDesc;
	ZeroMemory(&SnowCoverMapSRVDesc, sizeof(SnowCoverMapSRVDesc));
	SnowCoverMapSRVDesc.Format = dstTexDesc.Format;
	SnowCoverMapSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SnowCoverMapSRVDesc.Texture2D.MostDetailedMip = 0;
	SnowCoverMapSRVDesc.Texture2D.MipLevels = 1;
	result = device->CreateShaderResourceView(m_pSnowCoverMap, &SnowCoverMapSRVDesc, &m_pSnowCoverMapSRV);
	/******/

	m_pSnowCoverMapESRV = m_pGrassFieldManager->m_pSceneEffect->GetVariableByName("g_txSnowCover")->AsShaderResource();
	m_pSnowCoverMapESRV->SetResource(m_pSnowCoverMapSRV);

	m_pSnowCoverMapESRV = m_pGrassFieldManager->GetGrassTypeEffect(0)->GetVariableByName("g_txSnowCover")->AsShaderResource();
	m_pSnowCoverMapESRV->SetResource(m_pSnowCoverMapSRV);

	//m_pSnowCoverMapESRV = m_pGrassFieldManager->GetGrassTypeEffect(1)->GetVariableByName("g_txSnowCover")->AsShaderResource();
	//m_pSnowCoverMapESRV->SetResource(m_pSnowCoverMapSRV);


	//==============================================//
	//			Compute Shader Components			//
	//==============================================//

	// Create a buffer to be bound as Compute Shader input (D3D11_BIND_SHADER_RESOURCE).
	D3D11_BUFFER_DESC constantDataDesc;
	constantDataDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantDataDesc.ByteWidth = sizeof(ParticleType) * m_pParticleSystem->GetInstaceCount();
	constantDataDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	constantDataDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantDataDesc.StructureByteStride = sizeof(ParticleType);
	constantDataDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	result = device->CreateBuffer(&constantDataDesc, 0, &mInputBuffer);
	if (FAILED(result))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.BufferEx.Flags = 0;
	srvDesc.BufferEx.NumElements = m_pParticleSystem->GetInstaceCount();

	result = device->CreateShaderResourceView(mInputBuffer, &srvDesc, &mInputView);
	if (FAILED(result))
		return false;

	// Create a read-write buffer the compute shader can write to (D3D11_BIND_UNORDERED_ACCESS).
	D3D11_BUFFER_DESC outputDesc;
	outputDesc.Usage = D3D11_USAGE_DEFAULT;
	outputDesc.ByteWidth = sizeof(InstanceType) * m_pParticleSystem->GetInstaceCount();
	outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	outputDesc.CPUAccessFlags = 0;
	outputDesc.StructureByteStride = sizeof(InstanceType);
	outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	result = (device->CreateBuffer(&outputDesc, 0, &mRWBuffer));
	if (FAILED(result))
		return false;

	// Create a system memory version of the buffer to read the results back from.
	outputDesc.Usage = D3D11_USAGE_STAGING;
	outputDesc.BindFlags = 0;
	outputDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	result = (device->CreateBuffer(&outputDesc, 0, &mRWOutputBuffer));
	if (FAILED(result))
		return false;

	//outputDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//result = (device->CreateBuffer(&outputDesc, 0, &mRWInputBuffer));
	//if (FAILED(result))
	//	return false;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = 0;
	uavDesc.Buffer.NumElements = m_pParticleSystem->GetInstaceCount();
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

	result = device->CreateUnorderedAccessView(mRWBuffer, &uavDesc, &mRWUAV);
	if (FAILED(result))
		return false;

	return true;
}

void ParticleShader::CalculateInstancePositions(ID3D11DeviceContext* deviceContext)
{
	m_pParticleSystem->FillConstantDataBuffer(deviceContext, mInputBuffer);
	//deviceContext->CopyResource(mRWBuffer, mRWInputBuffer);

	// Enable Compute Shader
	deviceContext->CSSetShader(mComputeShader, nullptr, 0);

	deviceContext->CSSetShaderResources(0, 1, &mInputView);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &mRWUAV, 0);


	// Dispatch
	deviceContext->Dispatch(512, 1, 1);

	// Unbind the input textures from the CS for good housekeeping
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	deviceContext->CSSetShaderResources(0, 1, nullSRV);

	// Unbind output from compute shader ( we are going to use this output as an input in the next pass, 
	// and a resource cannot be both an output and input at the same time
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	deviceContext->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Disable Compute Shader
	deviceContext->CSSetShader(nullptr, nullptr, 0);


	// Copy result
	deviceContext->CopyResource(mRWOutputBuffer, mRWBuffer);


	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Update particle system data with output from Compute Shader
	HRESULT hr = deviceContext->Map(mRWOutputBuffer, 0, D3D11_MAP_READ, 0, &mappedResource);

	if (SUCCEEDED(hr))
	{
		InstanceType* dataView = reinterpret_cast<InstanceType*>(mappedResource.pData);

		// Update particle positions and velocities
		m_pParticleSystem->UpdatePosition(dataView);

		deviceContext->Unmap(mRWOutputBuffer, 0);
	}
}

bool ParticleShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, CFirstPersonCamera* camera, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Map matrix buffer
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;

	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	deviceContext->Unmap(m_matrixBuffer, 0);

	// Map camera buffer
	result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	CameraBufferType* dataPtr2 = (CameraBufferType*)mappedResource.pData;

	XMStoreFloat3(&(dataPtr2->cameraPosition), camera->GetEyePt());
	deviceContext->Unmap(m_cameraBuffer, 0);

	deviceContext->GSSetConstantBuffers(0, 1, &m_matrixBuffer);
	deviceContext->GSSetConstantBuffers(1, 1, &m_cameraBuffer);
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetShaderResources(1, 1, &texture);
	//deviceContext->OMSetRenderTargetsAndUnorderedAccessViews();
	return true;
}

void ParticleShader::SetSnowCoverTexture(ID3D11DeviceContext* deviceContext)
{
	D3D11_MAPPED_SUBRESOURCE MappedTexture;
	float** snowCover = m_pParticleSystem->GetSnowCover();
	deviceContext->Map(m_pSnowCoverMap, D3D11CalcSubresource(0, 0, 1), D3D11_MAP_WRITE_DISCARD, 0, &MappedTexture);

	float* pTexels = (float*)MappedTexture.pData;
	for (UINT row = 0; row < 256; row++)
	{
		UINT rowStart = row * MappedTexture.RowPitch / sizeof(float);
		for (UINT col = 0; col < 256; col++)
		{
			UINT colStart = col * 4;
			pTexels[rowStart + colStart] += snowCover[row][col];
		}
	}
	deviceContext->Unmap(m_pSnowCoverMap, D3D11CalcSubresource(0, 0, 1));
}

void ParticleShader::RenderShader(ID3D11DeviceContext* deviceContext, int vertexCount, int instanceCount, int indexCount)
{
	deviceContext->IASetInputLayout(m_layout);
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	deviceContext->GSSetShader(m_geometryShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);
	//deviceContext->DrawIndexed(indexCount, 0, 0);
	deviceContext->DrawInstanced(vertexCount, instanceCount, 0, 0);
}
