#include <xmmintrin.h>

#include "plane.h"

#include <cmath>

Plane::Plane( ID3D10Device *a_pD3DDevice, ID3D10Effect *a_pEffect, D3DXVECTOR4 &a_vPosAndRadius, 
       float a_fWidth, float a_fHeight, float a_fWheelBeg, float a_fWheelEnd ):
    m_fWidth(a_fWidth), m_fHeight(a_fHeight)
{
    m_pD3DDevice    = a_pD3DDevice;
    m_vPosAndRadius = a_vPosAndRadius;
    m_uVertexStride = sizeof(MeshVertex);
    m_uIndexStride  = sizeof(UINT32);
    m_uVertexOffset = 0;
    m_uIndexOffset  = 0;
    m_pIndexBuffer  = NULL;
    m_fWheelBeg = a_fWheelBeg;
    m_fWheelEnd = a_fWheelEnd;

    // Render if we need 
    ID3D10EffectTechnique *pTechnique = a_pEffect->GetTechniqueByIndex(0);
    m_pPass = pTechnique->GetPassByName("RenderMeshPass");
    m_pTextureESRV = a_pEffect->GetVariableByName("g_txMeshDiffuse")->AsShaderResource();
    D3DX10CreateShaderResourceViewFromFile(m_pD3DDevice, L"resources/stone.dds", 0, 0, &m_pTextureSRV, 0); 
    m_pTextureESRV->SetResource(m_pTextureSRV);
    D3DXMatrixTranslation(&m_mTransform, a_vPosAndRadius.x, a_vPosAndRadius.y, a_vPosAndRadius.z);
    m_pTransformEMV = a_pEffect->GetVariableByName("g_mWorld")->AsMatrix();

    CreateVertexBuffer();
    CreateInputLayout();

    D3DXMatrixIdentity(&m_mRotation);
    D3DXMatrixIdentity(&m_mTranslation);
}

void Plane::CreateVertexBuffer( void )
{
    MeshVertex Vertices[4];

    Vertices[3].vPos = D3DXVECTOR3(-m_fWidth, -m_fHeight, 0.0f);
    Vertices[3].vTexCoord = D3DXVECTOR2(0.0f, 1.0f);
    Vertices[2].vPos = D3DXVECTOR3(m_fWidth, -m_fHeight, 0.0f);
    Vertices[2].vTexCoord = D3DXVECTOR2(1.0f, 1.0f);
    Vertices[1].vPos = D3DXVECTOR3(-m_fWidth, m_fHeight, 0.0f);
    Vertices[1].vTexCoord = D3DXVECTOR2(0.0f, 0.0f);
    Vertices[0].vPos = D3DXVECTOR3(m_fWidth, m_fHeight, 0.0f);
    Vertices[0].vTexCoord = D3DXVECTOR2(1.0f, 0.0f);

    D3D10_BUFFER_DESC BufferDesc = 
    {
        4 * sizeof(MeshVertex),
        D3D10_USAGE_DEFAULT,
        D3D10_BIND_VERTEX_BUFFER,
        0, 0
    };
    D3D10_SUBRESOURCE_DATA BufferInitData;
    BufferInitData.pSysMem = Vertices;
    BufferInitData.SysMemPitch = 0;
    BufferInitData.SysMemSlicePitch = 0;
    m_pD3DDevice->CreateBuffer(&BufferDesc, &BufferInitData, &m_pVertexBuffer);
}

void Plane::CreateInputLayout( void )
{
    D3D10_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D10_INPUT_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 16, D3D10_INPUT_PER_VERTEX_DATA, 0}
    };

    // Create the input layout
    D3D10_PASS_DESC PassDesc;
    m_pPass->GetDesc(&PassDesc);
    m_pD3DDevice->CreateInputLayout(layout, 2, PassDesc.pIAInputSignature, 
        PassDesc.IAInputSignatureSize, &m_pInputLayout);
}

void Plane::Render( void )
{
    //D3DXMatrixTranslation(&m_mTransform, m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z);
    //D3DXMatrixInverse(&m_mInvTransform, NULL, &m_mTransform);
    
    m_pTransformEMV->SetMatrix((FLOAT*)m_mTransform);

    m_pD3DDevice->IASetInputLayout(m_pInputLayout);
    m_pD3DDevice->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
    m_pD3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_pPass->Apply(0);
    m_pD3DDevice->Draw(4, 0);
}

static void SuperNormalize( D3DXVECTOR3 *Out, D3DXVECTOR3 *In )
{
    __m128 v = _mm_set_ps(0.0f, In->z, In->y, In->x);
    __m128 sq_v;

    sq_v = _mm_mul_ps(v, v);

}

bool Plane::Collide( D3DXVECTOR3 *Ret, D3DXVECTOR3 &Beg, D3DXVECTOR3 &End,
    PhysPatch::BladePhysData *a_pBladePhysData, int a_iSegmentIndex )
{
    D3DXVECTOR3 loc_beg, loc_end, loc_pos;
    D3DXVECTOR3 psi;
    float gamma;
    bool ret_code = true, ret_code1 = true;

    // In model coordinate system
    D3DXVec3TransformCoord(&loc_beg, &Beg, &m_mInvTransform);
    D3DXVec3TransformCoord(&loc_end, &End, &m_mInvTransform);
    
    // Count plane interval intersection 
    float t;
    D3DXVECTOR3 dir = loc_end - loc_beg;
    float x, y;
    D3DXVECTOR3 intersection;

	if (loc_end.z * loc_beg.z < 0.0)
//	if (dir.z != 0)    
    {
      t = -loc_beg.z / dir.z;
      x = loc_beg.x + t * dir.x;
      y = loc_beg.y + t * dir.y;
    }
    else
      return false;

    intersection = D3DXVECTOR3(x, y, 0);

    if (t < 0 || t > 1)
      return false;

    if (fabs(x) > m_fWidth || fabs(y) > m_fHeight)
        return false;

    // Count rotation angle
    D3DXVECTOR3 proj = D3DXVECTOR3(dir.x, dir.y, 0);
    float a = D3DXVec3Length(&(loc_beg - intersection)),
        b = D3DXVec3Length(&dir), alpha, beta;

    D3DXVec3Normalize(&dir, &dir);
    //SuperNormalize(&dir, &dir);
    D3DXVec3Normalize(&proj, &proj);

    float angle = D3DXVec3Dot(&proj, &dir);

    if (angle < -1)
        angle = -1;
    else if (angle > 1)
        angle = 1;
    alpha = (float)M_PI - acosf(angle);
    beta = a * sinf(alpha) / b;
    beta = sqrt(1 - beta * beta);
    if (beta < -1)
        beta = -1;
    else if (beta > 1)
        beta = 1;
    beta = acosf(beta);
    gamma = (float)M_PI - beta - alpha;

    // Count rotation axis
    D3DXVec3Cross(&psi, &dir, &proj);
    D3DXVec3Normalize(&psi, &psi);


    D3DXVECTOR3 beg1(0, 0, 0), beg2(psi.x, psi.y, psi.z);

    D3DXVec3TransformCoord(&beg1, &beg1, &m_mTransform);
    D3DXVec3TransformCoord(&beg2, &beg2, &m_mTransform);
    *Ret = beg2 - beg1;
    D3DXVec3Normalize(Ret, Ret);
    *Ret = (*Ret) * gamma * 1.1f;

    return true;
}

D3DXVECTOR4 Plane::GetPosAndRadius( void )
{
    if (m_fWidth > m_fHeight)
        return D3DXVECTOR4(m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z, m_fWidth);
    else
        return D3DXVECTOR4(m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z, m_fHeight);
}

void Plane::SetPosAndRadius( D3DXVECTOR4 &a_vPosAndRadius )
{
    m_vPosAndRadius = a_vPosAndRadius;
}

void Plane::SetHeight( float a_fH )
{
    ;
}

float Plane::GetDist( D3DXVECTOR3 &Pnt, bool *IsUnderWheel )
{
    D3DXVECTOR3 loc_coord;

    D3DXVec3TransformCoord(&loc_coord, &Pnt, &m_mInvTransform);

    if (fabs(loc_coord.x) > m_fWidth)
        return INVALID_DIST;

    if (IsUnderWheel != NULL)
        if (fabs(loc_coord.x) >= m_fWheelBeg &&
            fabs(loc_coord.x) <= m_fWheelEnd)
            *IsUnderWheel = true;
    
    return loc_coord.z;
}

void Plane::RotateToEdge( D3DXVECTOR3 *Ret, D3DXVECTOR3 &Beg, D3DXVECTOR3 &End )
{
    D3DXVECTOR3 loc_beg, loc_end, loc_pos;

    /* Convert to model coordinate system */
    D3DXVec3TransformCoord(&loc_beg, &Beg, &m_mInvTransform);
    D3DXVec3TransformCoord(&loc_end, &End, &m_mInvTransform);

    /* Calculate vectors */
    D3DXVECTOR3 dir = loc_end - loc_beg, dest_dir;

    /* Intersect */
    /*
    float t;
    float x;

    if (dir.z != 0)    
    {
        t = -loc_beg.z / dir.z;
        x = loc_beg.x + t * dir.x;
    }

    if (t < 0 || t > 1 || dir.z == 0)
        x = loc_beg.x;
    */

    dest_dir = D3DXVECTOR3(loc_beg.x, -m_fHeight, 0) - loc_beg;

    D3DXVec3Normalize(&dir, &dir);
    D3DXVec3Normalize(&dest_dir, &dest_dir);

    /* Calculate angle */
    float angle = D3DXVec3Dot(&dir, &dest_dir);

    if (angle > 1)
        angle = 1;
    if (angle < -1)
        angle = -1;
    angle = acosf(angle);

    /* Calculate rotation axis */
    D3DXVECTOR3 psi;
    D3DXVec3Cross(&psi, &dir, &dest_dir);
    D3DXVec3Normalize(&psi, &psi);

    /* Convert to back world space */
    D3DXVECTOR3 beg1(0, 0, 0), beg2(psi.x, psi.y, psi.z);

    D3DXVec3TransformCoord(&beg1, &beg1, &m_mTransform);
    D3DXVec3TransformCoord(&beg2, &beg2, &m_mTransform);
    *Ret = beg2 - beg1;
    D3DXVec3Normalize(Ret, Ret);
    *Ret = (*Ret) * angle;
}

void Plane::SetTransform( D3DXMATRIX &a_mTransform )
{
    m_mTransform = a_mTransform;
    D3DXMatrixInverse(&m_mInvTransform, NULL, &m_mTransform);
}

void Plane::SetInvTransform( D3DXMATRIX &a_mInvTransform )
{
    m_mInvTransform = a_mInvTransform;
    D3DXMatrixInverse(&m_mTransform, NULL, &m_mInvTransform);
}

bool Plane::CheckCollision( D3DXVECTOR3 &Beg, D3DXVECTOR3 &End, float *Dist )
{
    D3DXVECTOR3 loc_beg, loc_end;

    if (Dist != NULL)
        *Dist = INVALID_DIST;

    // In model coordinate system
    D3DXVec3TransformCoord(&loc_beg, &Beg, &m_mInvTransform);
    D3DXVec3TransformCoord(&loc_end, &End, &m_mInvTransform);
/*	
	if (loc_end.z > 0.0) return true;
	else return false;
*/	
    // Count plane interval intersection 
    float t = -1;
    D3DXVECTOR3 dir = loc_end - loc_beg;
    float x = 0, y = 0;
    D3DXVECTOR3 intersection;

    if (dir.z != 0)
    {
        t = -loc_beg.z / dir.z;
        x = loc_beg.x + t * dir.x;
        y = loc_beg.y + t * dir.y;
    }

    if (t < 0 || t > 1 || 
        fabs(x) > m_fWidth)
    {
        if (fabs(loc_beg.x) < m_fWidth)
            if (Dist != NULL)
                *Dist = loc_beg.z;
		
        return false;
    }

    return true;
}
