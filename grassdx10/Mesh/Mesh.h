#pragma once

#include "includes.h"
#include "../Grass/Physics/PhysPatch.h"
#include <math.h>

struct MeshVertex
{
    D3DXVECTOR3 vPos;
    D3DXVECTOR2 vTexCoord;
};

class Mesh
{
protected:
    /* Just a simple textured sphere */
    ID3D10EffectShaderResourceVariable *m_pTextureESRV;
    ID3D10ShaderResourceView           *m_pTextureSRV;
    ID3D10EffectMatrixVariable         *m_pTransformEMV;
    ID3D10EffectPass                   *m_pPass;
    ID3D10Device                       *m_pD3DDevice;
    ID3D10InputLayout                  *m_pInputLayout;
    ID3D10Buffer                       *m_pVertexBuffer;
    UINT                                m_uVertexCount;
    UINT                                m_uVertexStride;
    UINT                                m_uVertexOffset;
    ID3D10Buffer                       *m_pIndexBuffer;
    UINT                                m_uIndexCount;
    UINT                                m_uIndexStride;
    UINT                                m_uIndexOffset;
    D3DXMATRIX                          m_mTransform;
	D3DXMATRIX                          m_mMatr;
    D3DXMATRIX                          m_mRotation;
    D3DXMATRIX                          m_mTranslation;
    D3DXVECTOR4                         m_vPosAndRadius;
    D3DXVECTOR3                         m_vPrevPos;
    D3DXVECTOR3                         m_vMoveDir;

    float                               m_Angle;

    void CreateVertexBuffer      ( );
    void CreateInputLayout       ( );

public:
    Mesh( void )
    {
        ;
    }
    
    Mesh                         ( ID3D10Device *a_pD3DDevice, ID3D10Effect *a_pEffect, D3DXVECTOR4 &a_vPosAndRadius );
    virtual ~Mesh                ( );

	virtual void        SetPosAndRadius  ( D3DXVECTOR4 &a_vPosAndRadius );
    virtual void        SetHeight        ( float a_fH );
    virtual void        SetTransform     ( D3DXMATRIX &a_mTransform );
    virtual void        SetInvTransform  ( D3DXMATRIX &a_mInvTransform );
    virtual D3DXVECTOR4 GetPosAndRadius  ( );
    virtual D3DXVECTOR3         GetMoveDir       ( );
    virtual D3DXMATRIX         GetMatr       ( );
    virtual void        Render           ( );

    virtual bool CheckCollision( D3DXVECTOR3 &Beg, D3DXVECTOR3 &End, float *Dist )
    {
        return false;
    }

    virtual bool Collide( D3DXVECTOR3 *Ret, D3DXVECTOR3 &Beg, D3DXVECTOR3 &End,
        PhysPatch::BladePhysData *a_pBladePhysData, int a_iSegmentIndex )
    {
        return false;
    }

    virtual float GetDist( D3DXVECTOR3 &Pnt, bool *IsUnderWheel )
    {
      return 0;
    }

    virtual void RotateToEdge( D3DXVECTOR3 *Ret, D3DXVECTOR3 &Beg, D3DXVECTOR3 &End )
    {
        ;
    }
	virtual int IsBottom(D3DXVECTOR3 &Pnt, D3DXVECTOR3 &vNormal)
	{
      return 0;
    }

};
