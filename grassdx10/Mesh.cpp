#include "Mesh.h"

Mesh::Mesh( ID3D10Device *a_pD3DDevice, ID3D10Effect *a_pEffect, D3DXVECTOR4 &a_vPosAndRadius )
{
    m_pD3DDevice    = a_pD3DDevice;
    m_vPosAndRadius = a_vPosAndRadius;
    m_uVertexStride = sizeof(MeshVertex);
    m_uIndexStride  = sizeof(UINT32);
    m_uVertexOffset = 0;
    m_uIndexOffset  = 0;
    /* just one technique in effect */
    ID3D10EffectTechnique *pTechnique = a_pEffect->GetTechniqueByIndex(0);
    m_pPass = pTechnique->GetPassByName("RenderMeshPass");
    m_pTextureESRV = a_pEffect->GetVariableByName("g_txMeshDiffuse")->AsShaderResource();
    D3DX10CreateShaderResourceViewFromFile(m_pD3DDevice, L"resources/stone.dds", 0, 0, &m_pTextureSRV, 0); 
    m_pTextureESRV->SetResource(m_pTextureSRV);
    CreateVertexBuffer();
    D3DXMatrixTranslation(&m_mTransform, a_vPosAndRadius.x, a_vPosAndRadius.y, a_vPosAndRadius.z);
    m_pTransformEMV = a_pEffect->GetVariableByName("g_mWorld")->AsMatrix();    
    CreateInputLayout();

    D3DXMatrixIdentity(&m_mRotation);
    m_Angle = 0;
}

Mesh::~Mesh()
{
    SAFE_RELEASE(m_pInputLayout);
    SAFE_RELEASE(m_pVertexBuffer);
    SAFE_RELEASE(m_pIndexBuffer);
    SAFE_RELEASE(m_pTextureSRV);
}

/*zero-centered sphere*/
void Mesh::CreateVertexBuffer( )
{
    /* Initializing vertices */
    int i, j, ind;
    int NumStacks = 20;
    int NumSlices = 20;
    float _2PI = 2.0f * (float)M_PI;
    m_uVertexCount = NumSlices * (NumStacks + 1);
    m_uIndexCount  = 6 * NumSlices * NumStacks;
    MeshVertex *Vertices = new MeshVertex[m_uVertexCount];
    UINT32     *Indices  = new UINT32[m_uIndexCount];
    
    for (i = 0; i < NumStacks; i++)
    {
        for (j = -NumSlices / 2; j < NumSlices / 2; j++)
        {
            ind = (i) * NumSlices + j + NumSlices / 2;
            Vertices[ind].vPos.y = m_vPosAndRadius.w * cos(float(i) / NumStacks * (float)M_PI );
            Vertices[ind].vPos.x = m_vPosAndRadius.w * cos(float(j) / NumSlices * (float)_2PI) * sin(float(i) / NumStacks * (float)M_PI );
            Vertices[ind].vPos.z = m_vPosAndRadius.w * sin(float(j) / NumSlices * (float)_2PI) * sin(float(i) / NumStacks * (float)M_PI );
            Vertices[ind].vTexCoord.y = float(j + NumSlices / 2) / NumSlices;
            Vertices[ind].vTexCoord.x = float(i) / NumStacks;
        }
    }

    ind = 0;
    for (i = 1; i <= NumStacks; i++)
    {
        for (j = 1; j < NumSlices; j++)
        {
            Indices[ind++] = i       * NumSlices + j      ;
            Indices[ind++] = (i - 1) * NumSlices + j      ;
            Indices[ind++] = (i - 1) * NumSlices + (j - 1);

            Indices[ind++] = i       * NumSlices + j      ;
            Indices[ind++] = (i - 1) * NumSlices + (j - 1);
            Indices[ind++] = i       * NumSlices + (j - 1);
        }

        Indices[ind++] = i       * NumSlices    ;
        Indices[ind++] = (i - 1) * NumSlices    ;
        Indices[ind++] = i       * NumSlices - 1;

        Indices[ind++] = i       * NumSlices    ;
        Indices[ind++] = i       * NumSlices - 1;
        Indices[ind++] = (i + 1) * NumSlices - 1;
    }

    
    /* Initializing buffer */
    D3D10_BUFFER_DESC VBufferDesc = 
    {
        m_uVertexCount * sizeof(MeshVertex),
        D3D10_USAGE_DEFAULT,
        D3D10_BIND_VERTEX_BUFFER,
        0, 0
    };
    D3D10_SUBRESOURCE_DATA VBufferInitData;
    VBufferInitData.pSysMem = Vertices;
    m_pD3DDevice->CreateBuffer(&VBufferDesc, &VBufferInitData, &m_pVertexBuffer);

    D3D10_BUFFER_DESC IBufferDesc = 
    {
        m_uIndexCount * sizeof(UINT32),
        D3D10_USAGE_DEFAULT,
        D3D10_BIND_INDEX_BUFFER,
        0, 0
    };
    D3D10_SUBRESOURCE_DATA IBufferInitData;
    IBufferInitData.pSysMem = Indices;
    m_pD3DDevice->CreateBuffer(&IBufferDesc, &IBufferInitData, &m_pIndexBuffer);
    delete [] Indices;
    delete [] Vertices;
}

void Mesh::CreateInputLayout( )
{
    D3D10_INPUT_ELEMENT_DESC InputDesc[] = 
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D10_INPUT_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0}
    };
    D3D10_PASS_DESC PassDesc;
    m_pPass->GetDesc(&PassDesc);
    int InputElementsCount = sizeof(InputDesc) / sizeof(D3D10_INPUT_ELEMENT_DESC);
    m_pD3DDevice->CreateInputLayout(InputDesc, InputElementsCount, 
        PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, 
        &m_pInputLayout);   
}

D3DXVECTOR4 Mesh::GetPosAndRadius()
{
    return m_vPosAndRadius;
}

void Mesh::SetPosAndRadius( D3DXVECTOR4 &a_vPosAndRadius )
{
    D3DXVECTOR3 *v3Pos = (D3DXVECTOR3 *)(float*)(&m_vPosAndRadius);
    m_vPrevPos = *v3Pos;
    m_vPosAndRadius = a_vPosAndRadius;
    *v3Pos = (float*)(&m_vPosAndRadius);
    m_vMoveDir = *v3Pos - m_vPrevPos;
    m_vMoveDir.y = 0;

    static D3DXVECTOR3 vY(0.0f, 1.0f, 0.0f);
    D3DXVECTOR3 vRotAxis;
    D3DXVec3Cross(&vRotAxis, &vY, &m_vMoveDir);
    D3DXVec3Normalize(&vRotAxis, &vRotAxis);
    D3DXMatrixTranslation(&m_mTranslation, m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z);

    m_Angle += D3DXVec3Length(&m_vMoveDir) / m_vPosAndRadius.w;
    D3DXMatrixRotationAxis(&m_mRotation, &vRotAxis, m_Angle);
    D3DXMatrixMultiply(&m_mTransform, &m_mRotation, &m_mTranslation);

    D3DXVec3Normalize(&m_vMoveDir, &m_vMoveDir);
        /*
    m_vPosAndRadius = a_vPosAndRadius;
    D3DXMatrixTranslation(&m_mTransform, m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z);
    */
}

D3DXVECTOR3 Mesh::GetMoveDir( )
{
    return m_vMoveDir;
}

D3DXMATRIX Mesh::GetMatr()
{
    return m_mMatr;
}

void Mesh::SetHeight( float a_fH )
{       
    m_vPosAndRadius.y = a_fH;
    D3DXMatrixTranslation(&m_mTranslation, m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z);
    D3DXMatrixMultiply(&m_mTransform, &m_mRotation, &m_mTranslation);
}

void Mesh::Render( )
{
    m_pTransformEMV->SetMatrix((FLOAT*)m_mTransform);
    m_pD3DDevice->IASetInputLayout(m_pInputLayout);
    m_pD3DDevice->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
    m_pD3DDevice->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    m_pD3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pPass->Apply(0);
    m_pD3DDevice->DrawIndexed(m_uIndexCount, 0, 0);
}

void Mesh::SetTransform( D3DXMATRIX &a_mTransform )
{
    m_mTransform = a_mTransform;
}

void Mesh::SetInvTransform( D3DXMATRIX &a_mInvTransform )
{
    D3DXMatrixInverse(&m_mTransform, NULL, &a_mInvTransform);
}
