#pragma once
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>
#include "SnowParticleSystem.h"
#include "includes.h"
#include "DXUTcamera.h"
#include "SnowParticleTypes.h"
#include "GrassFieldManager.h"
#include "CopterController.h"

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
	
	struct TornadoBufferType
	{
		XMFLOAT3 pos;
		bool active;
		XMFLOAT3 fanPos;
		float padding;
	};

public:
	ParticleShader();
	ParticleShader(const ParticleShader&);

	void CalculateInstancePositions(ID3D11DeviceContext* deviceContext/*, ID3D11ShaderResourceView* fanFlowTex*/);

	~ParticleShader();

	void SetParticleSystem(SnowParticleSystem* system) { m_pParticleSystem = system; }
	void SetCopterController(CopterController* controller) { m_pCopterController = controller; }
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, GrassFieldManager* grassFieldManager);
	bool Render(ID3D11DeviceContext* deviceContext, SnowParticleSystem* particlesystem, CFirstPersonCamera* camera);

private:
	bool InitializeShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const WCHAR* vsFilename, const WCHAR* psFilename, const WCHAR* gsFilename, const WCHAR* csFilename);
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext
		, CFirstPersonCamera* cameraPos
		, XMMATRIX worldMatrix
		, XMMATRIX viewMatrix
		, XMMATRIX projectionMatrix
		, ID3D11ShaderResourceView* texture
		, bool tornadoActive
		, XMFLOAT3 tornadoPos);
	void SetSnowCoverTexture(ID3D11DeviceContext* deviceContext);
	void RenderShader(ID3D11DeviceContext* deviceContext, int vertexCount, int instanceCount, int indexCount);

private:
	SnowParticleSystem* m_pParticleSystem;
	CopterController* m_pCopterController;
	GrassFieldManager* m_pGrassFieldManager;
	ID3D11Texture2D* m_pSnowCoverMap;
	ID3DX11EffectShaderResourceVariable* m_pSnowCoverMapESRV;
	ID3D11ShaderResourceView* m_pSnowCoverMapSRV;
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11GeometryShader* m_geometryShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_cameraBuffer;
	ID3D11Buffer* m_tornadoBuffer;
	ID3D11SamplerState* m_sampleState;	
	UINT m_frame = 0;

	//===== Compute Shader Components =====
	ID3D11ComputeShader* mComputeShader;

	// Read Only
	ID3D11Buffer* mInputBuffer;
	ID3D11ShaderResourceView* mInputView;

	// Read/Write
	ID3D11Buffer* mRWBuffer;
	//ID3D11Buffer* mRWInputBuffer;
	ID3D11Buffer* mRWOutputBuffer;
	ID3D11UnorderedAccessView* mRWUAV;
};
