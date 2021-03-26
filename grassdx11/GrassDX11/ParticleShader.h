#pragma once
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>
#include "SnowParticleSystem.h"
#include "includes.h"
#include "DXUTcamera.h"
#include "ComputeShaderStructs.h"

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

	void CalculateInstancePositions(ID3D11DeviceContext* deviceContext);

	~ParticleShader();

	void SetParticleSystem(SnowParticleSystem* system) { m_pParticleSystem = system; }
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3DX11Effect* sceneEffect);
	bool Render(ID3D11DeviceContext* deviceContext, SnowParticleSystem* particlesystem, CFirstPersonCamera* camera);

private:
	HRESULT UpdateSystemInfo(ID3D11DeviceContext* deviceContext);
	bool InitializeShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const WCHAR* vsFilename, const WCHAR* psFilename, const WCHAR* gsFilename, const WCHAR* csFilename);
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, CFirstPersonCamera* cameraPos, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture);
	void SetSnowCoverTexture(ID3D11DeviceContext* deviceContext);
	void RenderShader(ID3D11DeviceContext* deviceContext, int vertexCount, int instanceCount, int indexCount);

private:
	SnowParticleSystem* m_pParticleSystem;
	ID3DX11Effect* m_pSceneEffect;
	ID3D11Texture2D* m_pSnowCoverMap;
	ID3DX11EffectShaderResourceVariable* m_pSnowCoverMapESRV;
	ID3D11ShaderResourceView* m_pSnowCoverMapSRV;
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11GeometryShader* m_geometryShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_cameraBuffer;
	ID3D11SamplerState* m_sampleState;	
	UINT m_frame = 0;

	//===== Compute Shader Components =====
	UINT mGroupSize = 0;
	ID3D11ComputeShader* mComputeShader;

	// Read Only
	ID3D11Buffer* mInputBuffer;
	ID3D11ShaderResourceView* mInputView;

	// Read/Write
	ID3D11Buffer* mOutputBuffer;
	ID3D11Buffer* mOutputResultBuffer;
	ID3D11UnorderedAccessView* mOutputUAV;

	SystemInfoType mHandlerData;
	ID3D11Buffer* mHandlerCBuffer;
};
