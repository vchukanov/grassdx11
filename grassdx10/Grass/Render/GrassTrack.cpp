#include "GrassTrack.h"

#include "../system/mtxfrustum.h"
#include "../system/maths.h"

GrassTracker::GrassTracker( ID3D10Device *a_pD3DDevice ):
    m_pD3DDevice(a_pD3DDevice), m_TrackVB(NULL), m_TotalPoints(0)
{
    /* Create texture for mesh track */
    D3D10_TEXTURE2D_DESC TexStagingDesc;
    ZeroMemory(&TexStagingDesc, sizeof(TexStagingDesc));
    TexStagingDesc.Width = 1024;
    TexStagingDesc.Height = 768;
    TexStagingDesc.MipLevels = 1;
    TexStagingDesc.ArraySize = 1;
    TexStagingDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    TexStagingDesc.SampleDesc.Count = 1;
    TexStagingDesc.Usage = D3D10_USAGE_DEFAULT;
    TexStagingDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
    m_pD3DDevice->CreateTexture2D(&TexStagingDesc, NULL, &m_TrackTexture);

    /* Creating Render Target View */
    D3D10_RENDER_TARGET_VIEW_DESC TexRTVDesc;
    ZeroMemory(&TexRTVDesc, sizeof(TexRTVDesc));
    TexRTVDesc.Format = TexStagingDesc.Format;
    TexRTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
    TexRTVDesc.Texture2D.MipSlice = 0;
    m_pD3DDevice->CreateRenderTargetView(m_TrackTexture, &TexRTVDesc, &m_pTrackRTV);

    /* Creating Shader Resource View */
    D3D10_SHADER_RESOURCE_VIEW_DESC TexSRVDesc;
    ZeroMemory(&TexSRVDesc, sizeof(TexSRVDesc));
    TexSRVDesc.Format = TexStagingDesc.Format;
    TexSRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
    TexSRVDesc.Texture2D.MostDetailedMip = 0;
    TexSRVDesc.Texture2D.MipLevels = 1;
    m_pD3DDevice->CreateShaderResourceView(m_TrackTexture, &TexSRVDesc, &m_pTrackSRV);

    /* Init matrices */
    D3DXMatrixIdentity(&m_CameraMatrix);
    D3DXMatrixIdentity(&m_ProjectionMatrix);

    /* Load shader */
    ID3D10Blob *pErrors;

    if (FAILED(D3DX10CreateEffectFromFile(L"Shaders/TrackEffect.fx", NULL, &g_D3D10Include, 
        "fx_4_0", D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_DEBUG,
        0, m_pD3DDevice,
        NULL, NULL,
        &m_TrackEffect,
        &pErrors, NULL)))
    {
        char *errStr;

        if (pErrors)
        {
            errStr = static_cast<char*>(pErrors->GetBufferPointer());

            //MessageBoxA(NULL, errStr, "123", MB_OK);
        }
    }

    m_TrackTechnique = m_TrackEffect->GetTechniqueByName("Render");

    m_ViewProjVariable = m_TrackEffect->GetVariableByName("g_mViewProj")->AsMatrix();

    /* Prepare geometry input layout */
    D3D10_INPUT_ELEMENT_DESC layout[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0},
    };
    UINT num_elements = sizeof(layout) / sizeof(layout[0]);

    // Create the input layout
    D3D10_PASS_DESC PassDesc;
    m_TrackTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
    m_pD3DDevice->CreateInputLayout(layout, num_elements, PassDesc.pIAInputSignature, 
        PassDesc.IAInputSignatureSize, &m_TrackLayout);

    float clear_color[4] = {1, 1, 1, 1};
    m_pD3DDevice->ClearRenderTargetView(m_pTrackRTV, clear_color);
}

GrassTracker::~GrassTracker( void )
{
    SAFE_RELEASE(m_pTrackRTV);
    SAFE_RELEASE(m_pTrackSRV);
    SAFE_RELEASE(m_TrackTexture);

    SAFE_RELEASE(m_TrackEffect);

    SAFE_RELEASE(m_TrackLayout);

    SAFE_RELEASE(m_TrackVB);
}

bool GrassTracker::UpdateMeshPositions( Mesh *a_pMeshes[], UINT a_uNumMeshes )
{
    bool changed = FALSE;
    
    m_TotalPoints = 0;
    for (UINT i = 0; i < a_uNumMeshes; ++i)
    {
        bool is_first = FALSE;
        
        D3DXVECTOR4 pos_radius = a_pMeshes[i]->GetPosAndRadius();
        D3DXVECTOR3 pos, prev_pos;
        pos = D3DXVECTOR3(pos_radius.x, pos_radius.y, pos_radius.z);

        if (m_MeshPositions[i].size() < 1)
            is_first = TRUE;
        else                    
            prev_pos = *(m_MeshPositions[i].end() - 1);
        
        //float tmp = D3DXVec3Length(D3DXVec3Subtract(&prev_pos, &prev_pos, &pos));
        //if (is_first || tmp >= MIN_DIST)
        {
            if (m_MeshPositions[i].size() >= TRACE_LEN)
                m_MeshPositions[i].erase(m_MeshPositions[i].begin());

            m_MeshPositions[i].push_back(pos);

            m_TotalPoints += m_MeshPositions[i].size();

            changed = TRUE;
        }
    }

    return changed;
}

void GrassTracker::PrepareVertexBuffer( Mesh *a_pMeshes[], UINT a_uNumMeshes )
{
    SAFE_RELEASE(m_TrackVB);
    
    /* Create vertices */
    VertexDescription *vertices;

    if ((vertices = new VertexDescription[m_TotalPoints * 2]) == NULL)
        return;

    unsigned int index = 0;
    for (UINT i = 0; i < a_uNumMeshes; ++i)
        for (UINT j = 0; j < m_MeshPositions[i].size(); ++j)
        {
            D3DXVECTOR3 dir;

            if (j < m_MeshPositions[i].size() - 1)
                dir = m_MeshPositions[i][j + 1] - m_MeshPositions[i][j];
            else if (m_MeshPositions[i].size() > 1)
                dir = m_MeshPositions[i][j] - m_MeshPositions[i][j - 1];
            else
                dir = D3DXVECTOR3(1, 0, 0);

            D3DXVec3Normalize(&dir, &dir);

            D3DXVECTOR3 new_dir;

            new_dir = D3DXVECTOR3(dir.z, 0, -dir.x) * a_pMeshes[i]->GetPosAndRadius().w;

            VertexDescription vertex;            

            vertex.Pos = m_MeshPositions[i][j];

            vertex.Pos = m_MeshPositions[i][j] + new_dir;
            vertex.Pos.y = 0;
            vertices[index++] = vertex;
            vertex.Pos = m_MeshPositions[i][j] - new_dir;
            vertex.Pos.y = 0;
            vertices[index++] = vertex;
        }

    /* Create vertex buffer */
    D3D10_BUFFER_DESC bd;
    bd.Usage = D3D10_USAGE_DEFAULT; // Need to use D3D10_USAGE_DYNAMIC
    bd.ByteWidth = m_TotalPoints * sizeof(VertexDescription) * 2;
    bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;
    m_pD3DDevice->CreateBuffer(&bd, &InitData, &m_TrackVB);

    delete[] vertices;
}

void GrassTracker::UpdateMatrices( D3DXMATRIX &a_mView, D3DXMATRIX &a_mProj,
    D3DXVECTOR3 a_vCamPos, D3DXVECTOR3 a_vCamDir )
{
    /* Calculate frustum */
    CVec3 cubeMin(-1.0f, -1.0f, 0.0f);
    CVec3 cubeMax(1.0f, 1.0f, 1.0f);
    maths::AABox FrustumBBox(cubeMin, cubeMax);
    maths::PointArray Frustum;
    D3DXMATRIX InvEyeProjView, proj, mv, M;

    Frustum.SetSize(8);

    proj = a_mProj;
    mv = a_mView;

    //proj._33 = 25.f/(25.f - 0.1f);
    //proj._43 = -2.5f/(25.f - 0.1f);

    D3DXMatrixMultiply(&M, &mv, &proj);
    D3DXMatrixInverse(&InvEyeProjView, NULL, &M);

    FrustumBBox.GetPoints(&Frustum);
    Frustum.Transform(InvEyeProjView);

    /* View matrix */
    D3DXVECTOR3 look_at_dir;

    D3DXVec3Subtract(&look_at_dir, &a_vCamPos, &D3DXVECTOR3(a_vCamPos.x, 0, a_vCamPos.z));

    D3DXVECTOR3 up_dir;

    up_dir = a_vCamDir;
    up_dir.y = 0;

    ModelViewMtx(&m_CameraMatrix, a_vCamPos, look_at_dir, up_dir);

    Frustum.Transform(m_CameraMatrix);

    /* Calc light bbox */
    Frustum.CalcAABBox(&FrustumBBox);

    /* Projection matrix */
    OrthoMtx(&m_ProjectionMatrix, FrustumBBox.Min(), FrustumBBox.Max());

    D3DXMatrixMultiply(&m_ViewProjMatrix, &m_CameraMatrix, &m_ProjectionMatrix);
}

void GrassTracker::UpdateTrack( D3DXMATRIX &a_mView, D3DXMATRIX &a_mProj,
    D3DXVECTOR3 a_vCamPos, D3DXVECTOR3 a_vCamDir,
    Mesh *a_pMeshes[], UINT a_uNumMeshes )
{
    float clear_color[4] = {1, 1, 1, 1};
    m_pD3DDevice->ClearRenderTargetView(m_pTrackRTV, clear_color);

    ID3D10RenderTargetView *old_rt;
    ID3D10DepthStencilView *old_dsv;

    m_pD3DDevice->OMGetRenderTargets(1, &old_rt, &old_dsv);

    m_pD3DDevice->OMSetRenderTargets(1, &m_pTrackRTV, NULL);

    /* Layout */
    m_pD3DDevice->IASetInputLayout(m_TrackLayout);

    /* Vertex buffer */
    bool changed;
    changed = UpdateMeshPositions(a_pMeshes, a_uNumMeshes);
    if (changed)
      PrepareVertexBuffer(a_pMeshes, a_uNumMeshes);
    
    UINT stride = sizeof(VertexDescription);
    UINT offset = 0;
    m_pD3DDevice->IASetVertexBuffers(0, 1, &m_TrackVB, &stride, &offset);

    m_pD3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    /* Set matrices */
    UpdateMatrices(a_mView, a_mProj, a_vCamPos, a_vCamDir);
    m_ViewProjVariable->SetMatrix(m_ViewProjMatrix);

    /* Render geometry */
    m_TrackTechnique->GetPassByIndex(0)->Apply(0);
    m_pD3DDevice->Draw(m_TotalPoints * 2, 0);

    m_pD3DDevice->OMSetRenderTargets(1, &old_rt, old_dsv);
    SAFE_RELEASE(old_rt);
    SAFE_RELEASE(old_dsv);
}
