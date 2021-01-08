#pragma once
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>
#include "SnowParticleSystem.h"
#include "DXUTcamera.h"

using namespace DirectX;

class ParticleShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct CameraBufferType
	{
		XMFLOAT3 cameraPosition;
		float padding;
	};

public:
	ParticleShader();
	ParticleShader(const ParticleShader&);
	~ParticleShader();

	bool Initialize(ID3D11Device* device);
	bool Render(ID3D11DeviceContext* deviceContext, SnowParticleSystem* particlesystem, CFirstPersonCamera* camera);

private:
	bool InitializeShader(ID3D11Device* device, const WCHAR* vsFilename, const WCHAR* psFilename, const WCHAR* gsFilename);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, CFirstPersonCamera* cameraPos, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture);
	void RenderShader(ID3D11DeviceContext* deviceContext, int vertexCount, int instanceCount, int indexCount);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11GeometryShader* m_geometryShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_cameraBuffer;
	ID3D11SamplerState* m_sampleState;
};
