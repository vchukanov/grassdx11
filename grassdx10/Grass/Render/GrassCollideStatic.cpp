#include "GrassCollideStatic.h"

StaticPatchInfo::StaticPatchInfo( )
{
    uMeshIndex = -1;
}

StaticPatchInfo::StaticPatchInfo( D3DXMATRIX a_mTransform, UINT a_uMeshIndex )
{
    mTransform = a_mTransform;
    uMeshIndex = a_uMeshIndex;
}

GrassCollideStatic::GrassCollideStatic( GrassPatch &a_GrassPatch, ID3D10Effect *a_pEffect )
: m_GrassPatch(a_GrassPatch)
{
    m_pD3DDevice        = a_GrassPatch.GetD3DDevicePtr();
    m_pPass             = a_pEffect->GetTechniqueByName("RenderGrass")->GetPassByName("StaticCollidePass");
    m_uInstanseStride   = sizeof(StaticPatchInfo);
    m_uInstanseOffset   = 0;
    m_pInstansesBuffer  = NULL;
    GenInputLayout     ( );    
}

GrassCollideStatic::~GrassCollideStatic( )
{
    SAFE_RELEASE(m_pInstansesBuffer);
    SAFE_RELEASE(m_pInputLayout);
}

//D3DXVECTOR3 GrassCollideStatic::GetInstPos( int a_iInstanseInd )
//{    
//    return D3DXVECTOR3(m_Instanses[a_iInstanseInd].mTransform._41,
//        m_Instanses[a_iInstanseInd].mTransform._42,
//        m_Instanses[a_iInstanseInd].mTransform._43);   
//}

bool GrassCollideStatic::TakePatch( D3DXMATRIX &a_mTransform, UINT a_uMeshInd, DWORD a_dwIndex /* = -1 */ )
{
    if ( a_dwIndex == -1 )    
        m_Instanses.push_back(StaticPatchInfo(a_mTransform, a_uMeshInd));
    else
    {
        m_Instanses[a_dwIndex].mTransform = a_mTransform;
        m_Instanses[a_dwIndex].uMeshIndex = a_uMeshInd;
    }
    return true;
}

void GrassCollideStatic::SetNumInstanses( DWORD a_dwNumInst )
{
    m_Instanses.resize(a_dwNumInst);
}

//void GrassCollideStatic::Update( D3DXVECTOR3 a_vCamPos, float a_fLod0Dist, float a_fLod1Dist )
//{
//    /*D3DXVECTOR3 vCamToPatch;
//    float fDist;
//    std::vector< StaticPatchInfo >::iterator it;
//    unsigned i = 0;
//    for (it = m_Instanses.begin(); it != m_Instanses.end(); ++it, ++i)
//    {
//        vCamToPatch = GetInstPos(i) - a_vCamPos;
//        fDist = D3DXVec3Length(&vCamToPatch);
//        if ( fDist > a_fLod1Dist || fDist < a_fLod0Dist )
//        {
//            m_Instanses.erase(it);
//        }
//    }*/
//    UpdateTransformBuffer();
//}

void GrassCollideStatic::Render( )
{
    m_pD3DDevice->IASetInputLayout(m_pInputLayout);
    m_GrassPatch.IASetVertexBuffer0();
    m_pD3DDevice->IASetVertexBuffers(1, 1, &m_pInstansesBuffer, &m_uInstanseStride, &m_uInstanseOffset);
    m_pD3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
    m_pPass->Apply(0);
    m_pD3DDevice->DrawInstanced(m_GrassPatch.VerticesCount(), m_Instanses.size(), 0, 0);
}


void GrassCollideStatic::GenInputLayout( )
{
    const D3D10_INPUT_ELEMENT_DESC InputLayout[] =
    {
        { "POSITION"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0 , D3D10_INPUT_PER_VERTEX_DATA  , 0 },
        { "TEXCOORD"      , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 12, D3D10_INPUT_PER_VERTEX_DATA  , 0 },
        { "TEXCOORD"      , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 24, D3D10_INPUT_PER_VERTEX_DATA  , 0 },
        //{ "WIDTH"         , 0, DXGI_FORMAT_R32_FLOAT         , 0, 36, D3D10_INPUT_PER_VERTEX_DATA  , 0 },
        //{ "SEGMENTHEIGHT" , 0, DXGI_FORMAT_R32_FLOAT         , 0, 40, D3D10_INPUT_PER_VERTEX_DATA  , 0 },
        { "TRANSPARENCY"  , 0, DXGI_FORMAT_R32_FLOAT         , 0, 36, D3D10_INPUT_PER_VERTEX_DATA  , 0 },
        { "mTransform"    , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0 , D3D10_INPUT_PER_INSTANCE_DATA, 1 },
        { "mTransform"    , 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D10_INPUT_PER_INSTANCE_DATA, 1 },
        { "mTransform"    , 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D10_INPUT_PER_INSTANCE_DATA, 1 },
        { "mTransform"    , 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D10_INPUT_PER_INSTANCE_DATA, 1 },
        { "UMESHINDEX"    , 0, DXGI_FORMAT_R32_UINT          , 1, 64 , D3D10_INPUT_PER_INSTANCE_DATA, 1 },
    };
    
    int iNumElements = sizeof( InputLayout ) / sizeof( D3D10_INPUT_ELEMENT_DESC );
    D3D10_PASS_DESC PassDesc;
    m_pPass->GetDesc(&PassDesc);
    m_pD3DDevice->CreateInputLayout(InputLayout, iNumElements, 
        PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, 
        &m_pInputLayout);
}

void GrassCollideStatic::GenTransformBuffer( )
{
    if (m_Instanses.empty())
        return;
    SAFE_RELEASE(m_pInstansesBuffer);
    D3D10_BUFFER_DESC bufferDesc =
    {
        m_Instanses.size() * sizeof( StaticPatchInfo ),
        D3D10_USAGE_DYNAMIC,
        D3D10_BIND_VERTEX_BUFFER,
        D3D10_CPU_ACCESS_WRITE,
        0
    };

    m_pD3DDevice->CreateBuffer( &bufferDesc, NULL, &m_pInstansesBuffer );

    StaticPatchInfo* pMatrices = NULL;
    m_pInstansesBuffer->Map( D3D10_MAP_WRITE_DISCARD, NULL, ( void** )&pMatrices );

    memcpy( pMatrices, &m_Instanses[0], m_Instanses.size() * sizeof( StaticPatchInfo ) );

    m_pInstansesBuffer->Unmap();
}


void GrassCollideStatic::UpdateTransformBuffer( )
{
    if (m_Instanses.empty())
        return;

    D3DXMATRIX* pMatrices = NULL;
    m_pInstansesBuffer->Map( D3D10_MAP_WRITE_DISCARD, NULL, ( void** )&pMatrices );

    memcpy( pMatrices, &m_Instanses[0], m_Instanses.size() * sizeof( StaticPatchInfo ) );

    m_pInstansesBuffer->Unmap();
}


