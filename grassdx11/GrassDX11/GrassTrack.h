#pragma once

#include "includes.h"
#include "mesh.h"

#include "GrassManager.h"

#define TRACE_LEN 100
#define NUM_TRACK_MESHES 1
#define MIN_DIST 0.1

class GrassTracker
{
public:
	GrassTracker(ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx);

	~GrassTracker(void);

	void UpdateTrack(XMFLOAT4X4& a_mView, XMFLOAT4X4& a_mProj,
		XMFLOAT3 a_vCamPos, XMFLOAT3 a_vCamDir,
		Mesh* a_pMeshes[], UINT a_uNumMeshes);

	ID3D11ShaderResourceView* GetTrackSRV(void)
	{
		return m_pTrackSRV;
	}

	XMFLOAT4X4& GetCameraMatrix(void)
	{
		return m_CameraMatrix;
	}

	XMFLOAT4X4& GetProjectionMatrix(void)
	{
		return m_ProjectionMatrix;
	}

	XMFLOAT4X4& GetViewProjMatrix(void)
	{
		return m_ViewProjMatrix;
	}

private:
	XMFLOAT4X4 m_CameraMatrix;
	XMFLOAT4X4 m_ProjectionMatrix;
	XMFLOAT4X4 m_ViewProjMatrix;

	ID3D11Texture2D          *m_TrackTexture;
	ID3D11RenderTargetView   *m_pTrackRTV;
	ID3D11ShaderResourceView *m_pTrackSRV;

	ID3D11Device		*m_pD3DDevice;
	ID3D11DeviceContext *m_pD3DDeviceCtx;

	ID3DX11Effect			    *m_TrackEffect;
	ID3DX11EffectTechnique      *m_TrackTechnique;
	ID3DX11EffectMatrixVariable *m_ViewProjVariable;

	struct VertexDescription
	{
		XMFLOAT3 Pos;
	};
	ID3D11InputLayout *m_TrackLayout;
	ID3D11Buffer      *m_TrackVB;

	std::vector<XMFLOAT3> m_MeshPositions[NUM_TRACK_MESHES];
	unsigned int		  m_TotalPoints; // Total points in all meshes

	bool UpdateMeshPositions (Mesh *a_pMeshes[], UINT a_uNumMeshes);
	void PrepareVertexBuffer (Mesh *a_pMeshes[], UINT a_uNumMeshes);
	void UpdateMatrices	     (XMFLOAT4X4& a_mView, XMFLOAT4X4& a_mProj, XMFLOAT3 a_vCamPos, XMFLOAT3 a_vCamDir);
};
