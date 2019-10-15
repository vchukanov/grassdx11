#pragma once

#include <math.h>

#include "includes.h"
#include "Mesh/mesh.h"

#define EPS 0.1

#define ABS(a) ((a) > 0 ? (a) : -(a))

class Plane: public Mesh
{
public:
    enum MOVE_TYPE
    {
        MOVE_TANGENT = 0,
        MOVE_NORMAl
    };

    Plane( ID3D10Device *a_pD3DDevice, ID3D10Effect *a_pEffect, D3DXVECTOR4 &a_vPosAndRadius, 
        float a_fWidth, float a_fHeight, float a_fWheelBeg, float a_fWheelEnd );


	virtual void SetPosAndRadius( D3DXVECTOR4 &a_vPosAndRadius );

    virtual void SetHeight( float a_fH );

    virtual D3DXVECTOR4 GetPosAndRadius( void );

    virtual void SetTransform( D3DXMATRIX &a_mTransform );

    virtual void SetInvTransform( D3DXMATRIX &a_mInvTransform );

    virtual void Render( void );


    virtual bool CheckCollision( D3DXVECTOR3 &Beg, D3DXVECTOR3 &End, float *Dist );
        
    virtual bool Collide( D3DXVECTOR3 *Ret, D3DXVECTOR3 &Beg, D3DXVECTOR3 &End,
        PhysPatch::BladePhysData *a_pBladePhysData, int a_iSegmentIndex );

    virtual float GetDist( D3DXVECTOR3 &Pnt, bool *IsUnderWheel );

    virtual void RotateToEdge( D3DXVECTOR3 *Ret, D3DXVECTOR3 &Beg, D3DXVECTOR3 &End );

protected:

    virtual void CreateVertexBuffer( void );
    virtual void CreateInputLayout( void );

private:
    D3DXMATRIX m_mInvTransform;
    float m_fWidth, m_fHeight;
    float m_fWheelBeg, m_fWheelEnd;

};
