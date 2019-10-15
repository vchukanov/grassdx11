#pragma once

#include "includes.h"

#include "GrassManager.h"

#define TRACE_LEN 100
#define NUM_TRACK_MESHES 1
#define MIN_DIST 0.1

class GrassTracker
{
public:    
    GrassTracker( ID3D10Device *a_pD3DDevice );

    ~GrassTracker( void );

    void UpdateTrack( D3DXMATRIX &a_mView, D3DXMATRIX &a_mProj,
        D3DXVECTOR3 a_vCamPos, D3DXVECTOR3 a_vCamDir,
        Mesh *a_pMeshes[], UINT a_uNumMeshes );

    ID3D10ShaderResourceView* GetTrackSRV( void )
    {
        return m_pTrackSRV;
    }

    D3DXMATRIX& GetCameraMatrix( void )
    {
        return m_CameraMatrix;
    }

    D3DXMATRIX& GetProjectionMatrix( void )
    {
        return m_ProjectionMatrix;
    }

    D3DXMATRIX& GetViewProjMatrix( void )
    {
        return m_ViewProjMatrix;
    }

private:
    D3DXMATRIX m_CameraMatrix;
    D3DXMATRIX m_ProjectionMatrix;
    D3DXMATRIX m_ViewProjMatrix;

    ID3D10Texture2D *m_TrackTexture;
    ID3D10RenderTargetView *m_pTrackRTV;
    ID3D10ShaderResourceView *m_pTrackSRV;

    ID3D10Device *m_pD3DDevice;

    ID3D10Effect *m_TrackEffect;
    ID3D10EffectTechnique *m_TrackTechnique;
    ID3D10EffectMatrixVariable *m_ViewProjVariable;

    struct VertexDescription
    {
        D3DXVECTOR3 Pos;
    };
    ID3D10InputLayout *m_TrackLayout;
    ID3D10Buffer *m_TrackVB;

    std::vector<D3DXVECTOR3> m_MeshPositions[NUM_TRACK_MESHES];
    unsigned int m_TotalPoints; // Total points in all meshes

    bool UpdateMeshPositions( Mesh *a_pMeshes[], UINT a_uNumMeshes );

    void PrepareVertexBuffer( Mesh *a_pMeshes[], UINT a_uNumMeshes );

    void UpdateMatrices( D3DXMATRIX &a_mView, D3DXMATRIX &a_mProj,
        D3DXVECTOR3 a_vCamPos, D3DXVECTOR3 a_vCamDir );
};
