#pragma once
#include "DXUT.h"
#include <thread>
#include "SnowParticleTypes.h"

using namespace DirectX;

class ParticleShader;

class SnowParticleSystem
{
public:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT4 color;
	};

public:
	SnowParticleSystem();
	SnowParticleSystem(const SnowParticleSystem&);
	~SnowParticleSystem();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, const WCHAR*, int);
	void SetParticleShader(ParticleShader* ps) { m_pParticleShader = ps; }
	bool Frame(float, ID3D11DeviceContext*);
	void Render(ID3D11DeviceContext*);

	void FillConstantDataBuffer(ID3D11DeviceContext* deviceContext, ID3D11Buffer* inputBuffer);
	void UpdatePosition(InstanceType* dataView);

	//ID3D11UnorderedAccessView* GetUAV() { return m_uav; }
	ID3D11ShaderResourceView* GetTexture() { return m_TextureView; }
	int GetIndexCount() { return m_indexCount; }
	int GetVertexCount() { return m_vertexCount; }
	int GetInstaceCount() { return m_instanceCount; }
	int GetCurrentParticleCount() { return m_currentParticleCount; }
	int GetParticlesPerSecond() { return m_particlePerSecond; }
	float** GetSnowCover() { return m_snowCover; }
	void SetParticlesPerSecond(int value) { m_particlePerSecond = value; }
	void ToggleSnowCover() { isSnowCoverActive = !isSnowCoverActive; }

	/*TORNADO*/
	XMFLOAT3 GetTornadoPos() { return m_tornadoPos; }
	void ToggleTornado() { tornadoActive = !tornadoActive; }
	bool IsTornadoActive() { return tornadoActive; }
	void MoveTornadoForward() { m_tornadoPos.z += 1.1f; m_deltaTorandoPos.z += 1.1f; }
	void MoveTornadoBack() { m_tornadoPos.z -= 1.1f; m_deltaTorandoPos.z -= 1.1f;}
	void MoveTornadoLeft() { m_tornadoPos.x -= 1.1f; m_deltaTorandoPos.x -= 1.1f;}
	void MoveTornadoRight() { m_tornadoPos.x += 1.1f; m_deltaTorandoPos.x += 1.1f;}
private:
	// Initialize
	bool LoadTexture(ID3D11Device*, ID3D11DeviceContext*, const WCHAR*);
	bool InitializeParticleSystem(int);
	bool InitializeBuffers(ID3D11Device*, ID3D11DeviceContext*);
	void CalculateInstancePositions(int, int);
	void UpdateCloudPosition();
	XMUINT2 GetIntCoord(XMFLOAT2 pos);

	// Particle methods
	void EmitParticles(float);
	void UpdateParticles(float);
	void SpawnParticle();

	// Render & Update
	bool UpdateBuffers(ID3D11DeviceContext*);
	void RenderBuffers(ID3D11DeviceContext*);
private:
	ParticleShader* m_pParticleShader;
	XMFLOAT3 m_cloudPos{ 0.f, 80.f, 0.f };
	XMFLOAT3 m_tornadoPos{ 0.f, 0.f, 0.f };;
	XMFLOAT3 m_deltaTorandoPos{ 0.f, 0.f, 0.f };;
	bool tornadoActive = false;
	float m_particleDeviationX, m_particleDeviationY, m_particleDeviationZ;
	float m_particleVeclocity, m_particleVelocityVariation;
	float m_particleSize, m_particlePerSecond;
	int m_maxParticles;

	int m_currentParticleCount;
	float m_accumulatedTime;

	ID3D11UnorderedAccessView* m_uav;
	ID3D11Texture2D* m_uavTex;

	ID3D11Resource* m_Texture;
	ID3D11ShaderResourceView* m_TextureView;

	ParticleType* m_particleList;

	int m_vertexCount, m_indexCount, m_instanceCount;
	InstanceType* m_instance;
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer, * m_instanceBuffer;
	float** m_snowCover = nullptr;
	bool isSnowCoverActive = false;

	std::vector<std::thread> m_threads;
};