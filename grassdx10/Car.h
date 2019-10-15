#pragma once

#include "mesh.h"
#include "plane.h"
#include "terrain.h"
#include "MeshLoader10.h"

class Car: public Mesh
{
public:
    Car( ID3D10Device *a_pD3DDevice, ID3D10Effect *a_pEffect, D3DXVECTOR4 &a_vPosAndRadius,
        Terrain * const a_pTerrain, float a_fHeightScale, float a_fGrassRadius,
        float a_fCarWidth, float a_fCarHeight, float a_fCarLength, float a_fAmplAngle );

    virtual ~Car( void );


    virtual void SetPosAndRadius( D3DXVECTOR4 &a_vPosAndRadius );

    virtual void SetHeight( float a_fH );

    virtual D3DXVECTOR4 GetPosAndRadius( void );

    virtual void Render( void );


    virtual bool CheckCollision( D3DXVECTOR3 &Beg, D3DXVECTOR3 &End, float *Dist );

    virtual bool Collide( D3DXVECTOR3 *Ret, D3DXVECTOR3 &Beg, D3DXVECTOR3 &End,
        PhysPatch::BladePhysData *a_pBladePhysData, int a_iSegmentIndex );

    virtual float GetDist( D3DXVECTOR3 &Pnt, bool *IsUnderWheel );

    virtual void RotateToEdge( D3DXVECTOR3 *Ret, D3DXVECTOR3 &Beg, D3DXVECTOR3 &End )
    {
        ;
    }
	virtual int IsBottom(D3DXVECTOR3 &Pnt, D3DXVECTOR3 &vNormal);

private:
    struct Vertex
    {
        D3DXVECTOR3 vPos;
        D3DXVECTOR3 vNormal;
        D3DXVECTOR2 vTexCoord;        

        Vertex( void )
        {
            ;
        }

        Vertex( D3DXVECTOR3 a_vPos, D3DXVECTOR2 a_vTexCoord, D3DXVECTOR3 a_vNormal):
            vPos(a_vPos), vTexCoord(a_vTexCoord), vNormal(a_vNormal)
        {
            ;
        }
    };
    
    // Car sizes
    float m_fCarLength;
    float m_fCarHeight;
    float m_fCarWidth;
    float m_fAmplAngle;
    float m_fCarBackWidth;

    // Plane sizes
    float m_fPlaneLength;
    float m_fPlaneWidth;
    float m_fPlaneHeight;

	// Inverse Matrix of bottom plane  
    D3DXMATRIX m_mInvBottom; 

    // Use to convert normals to world space in shader
    D3DXMATRIX m_mNormalMatrix;
    ID3D10EffectMatrixVariable *m_pNormalMatrixEMV;

    // Planes for physics
    Plane *m_pPlanes[3];
    UINT m_uNumPlanes;

    // Car Mesh
    CMeshLoader10 m_Mesh;
    // Terrain height data
    Terrain *m_pTerrain;
    float m_fHeightScale, m_fGrassRadius;

    // Shader variables for material parameters
    ID3D10EffectVectorVariable *m_pMaterialCoefsESV[3];
    ID3D10EffectScalarVariable *m_pMaterialShininessESV;
    ID3D10EffectShaderResourceVariable *m_pMeshMapKdESRV;
    ID3D10EffectShaderResourceVariable *m_pMeshMapKsESRV;

    void CreateVertexBuffer( void );
    void CreateInputLayout( void );
};
