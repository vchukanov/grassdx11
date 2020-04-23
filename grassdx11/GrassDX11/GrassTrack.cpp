#include "maths.h"
#include "xtmfrustum.h"

#include "GrassTrack.h"


GrassTracker::GrassTracker (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx)
   : m_pD3DDevice(a_pD3DDevice)
   , m_pD3DDeviceCtx(a_pD3DDeviceCtx)
   , m_TrackVB(NULL)
   , m_TotalPoints(0)
{
   /* Create texture for mesh track */
   D3D11_TEXTURE2D_DESC TexStagingDesc;
   ZeroMemory(&TexStagingDesc, sizeof(TexStagingDesc));
   TexStagingDesc.Width            = 1024;
   TexStagingDesc.Height           = 768;
   TexStagingDesc.MipLevels        = 1;
   TexStagingDesc.ArraySize        = 1;
   TexStagingDesc.Format           = DXGI_FORMAT_R32G32B32A32_FLOAT;
   TexStagingDesc.SampleDesc.Count = 1;
   TexStagingDesc.Usage            = D3D11_USAGE_DEFAULT;
   TexStagingDesc.BindFlags        = D3D11_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
   m_pD3DDevice->CreateTexture2D(&TexStagingDesc, NULL, &m_TrackTexture);

   /* Creating Render Target View */
   D3D11_RENDER_TARGET_VIEW_DESC TexRTVDesc;
   ZeroMemory(&TexRTVDesc, sizeof(TexRTVDesc));
   TexRTVDesc.Format = TexStagingDesc.Format;
   TexRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
   TexRTVDesc.Texture2D.MipSlice = 0;
   m_pD3DDevice->CreateRenderTargetView(m_TrackTexture, &TexRTVDesc, &m_pTrackRTV);

   /* Creating Shader Resource View */
   D3D11_SHADER_RESOURCE_VIEW_DESC TexSRVDesc;
   ZeroMemory(&TexSRVDesc, sizeof(TexSRVDesc));
   TexSRVDesc.Format = TexStagingDesc.Format;
   TexSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
   TexSRVDesc.Texture2D.MostDetailedMip = 0;
   TexSRVDesc.Texture2D.MipLevels = 1;
   m_pD3DDevice->CreateShaderResourceView(m_TrackTexture, &TexSRVDesc, &m_pTrackSRV);

   /* Init matrices */
   XMMATRIX m;
   m = XMMatrixIdentity();
   XMStoreFloat4x4(&m_CameraMatrix, m);
   XMStoreFloat4x4(&m_ProjectionMatrix, m);

   /* Load shader */
   char* errStr;
   ID3DBlob* pErrorBlob = NULL;

   HRESULT hr = D3DX11CompileEffectFromFile(L"Shaders/TrackEffect.fx", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
      D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_SKIP_OPTIMIZATION, 0, m_pD3DDevice, &m_TrackEffect, &pErrorBlob);

   if (pErrorBlob)
   {
      errStr = static_cast<char*>(pErrorBlob->GetBufferPointer());
   }
   

   m_TrackTechnique   = m_TrackEffect->GetTechniqueByName("Render");
   m_ViewProjVariable = m_TrackEffect->GetVariableByName("g_mViewProj")->AsMatrix();

   /* Prepare geometry input layout */
   D3D11_INPUT_ELEMENT_DESC layout[] =
   {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
   };
   UINT num_elements = sizeof(layout) / sizeof(layout[0]);

   // Create the input layout
   D3DX11_PASS_DESC PassDesc;
   m_TrackTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
   m_pD3DDevice->CreateInputLayout(layout, num_elements, PassDesc.pIAInputSignature,
      PassDesc.IAInputSignatureSize, &m_TrackLayout);

   float clear_color[4] = { 1, 1, 1, 1 };
   m_pD3DDeviceCtx->ClearRenderTargetView(m_pTrackRTV, clear_color);
}

GrassTracker::~GrassTracker (void)
{
   SAFE_RELEASE(m_pTrackRTV);
   SAFE_RELEASE(m_pTrackSRV);
   SAFE_RELEASE(m_TrackTexture);

   SAFE_RELEASE(m_TrackEffect);

   SAFE_RELEASE(m_TrackLayout);

   SAFE_RELEASE(m_TrackVB);
}

bool GrassTracker::UpdateMeshPositions (Mesh* a_pMeshes[], UINT a_uNumMeshes)
{
   bool changed = FALSE;

   m_TotalPoints = 0;
   for (UINT i = 0; i < a_uNumMeshes; ++i)
   {
      bool is_first = FALSE;

      XMFLOAT4 pos_radius = a_pMeshes[i]->GetPosAndRadius();
      XMFLOAT3 pos, prev_pos;
      pos = XMFLOAT3(pos_radius.x, pos_radius.y, pos_radius.z);

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

void GrassTracker::PrepareVertexBuffer (Mesh* a_pMeshes[], UINT a_uNumMeshes)
{
   SAFE_RELEASE(m_TrackVB);

   /* Create vertices */
   VertexDescription* vertices;

   if ((vertices = new VertexDescription[m_TotalPoints * 2]) == NULL)
      return;

   unsigned int index = 0;
   for (UINT i = 0; i < a_uNumMeshes; ++i)
      for (UINT j = 0; j < m_MeshPositions[i].size(); ++j)
      {
         XMVECTOR dir;

         if (j < m_MeshPositions[i].size() - 1) {
            XMVECTOR v1 = XMLoadFloat3(&m_MeshPositions[i][j + 1]);
            XMVECTOR v2 = XMLoadFloat3(&m_MeshPositions[i][j]);
            dir = v1 - v2;
         }
         else if (m_MeshPositions[i].size() > 1) {
            XMVECTOR v1 = XMLoadFloat3(&m_MeshPositions[i][j]);
            XMVECTOR v2 = XMLoadFloat3(&m_MeshPositions[i][j - 1]);
            dir = v1 - v2;
         }
         else {
            XMFLOAT3 v(1, 0, 0);
            dir = XMLoadFloat3(&v);
         }
         dir = XMVector3Normalize(dir);
         
         XMVECTOR new_dir;
         
         XMFLOAT3 xmf(XMVectorGetZ(dir), 0, -XMVectorGetX(dir));
         XMVECTOR v = XMLoadFloat3(&xmf);

         new_dir = v * a_pMeshes[i]->GetPosAndRadius().w;

         VertexDescription vertex;

         v = XMLoadFloat3(&m_MeshPositions[i][j]);
         vertex.Pos = m_MeshPositions[i][j];

         XMStoreFloat3(&vertex.Pos, v + new_dir);
         vertex.Pos.y = 0;
         vertices[index++] = vertex;
         XMStoreFloat3(&vertex.Pos, v - new_dir);
         vertex.Pos.y = 0;
         vertices[index++] = vertex;
      }

   /* Create vertex buffer */
   D3D11_BUFFER_DESC bd;
   bd.Usage          = D3D11_USAGE_DEFAULT; // Need to use D3D11_USAGE_DYNAMIC
   bd.ByteWidth      = m_TotalPoints * sizeof(VertexDescription) * 2;
   bd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
   bd.CPUAccessFlags = 0;
   bd.MiscFlags      = 0;
   D3D11_SUBRESOURCE_DATA InitData;
   InitData.pSysMem = vertices;
   m_pD3DDevice->CreateBuffer(&bd, &InitData, &m_TrackVB);

   delete[] vertices;
}

void GrassTracker::UpdateMatrices (XMFLOAT4X4& a_mView, XMFLOAT4X4& a_mProj,
   XMFLOAT3 a_vCamPos, XMFLOAT3 a_vCamDir)
{
   /* Calculate frustum */
    XMFLOAT3 cubeMin(-1.0f, -1.0f, 0.0f);
    XMFLOAT3 cubeMax(1.0f, 1.0f, 1.0f);
    maths::AABox FrustumBBox(cubeMin, cubeMax);
    maths::PointArray Frustum;
    XMMATRIX InvEyeProjView, M;

    Frustum.SetSize(8);

   XM_TO_M(a_mView, mv);
   XM_TO_M(a_mProj, proj);

    M = XMMatrixMultiply(mv, proj);
   InvEyeProjView = XMMatrixInverse(NULL, M);

    FrustumBBox.GetPoints(&Frustum);
    Frustum.Transform(InvEyeProjView);

    /* View matrix */
    XMVECTOR look_at_dir;
   XM_TO_V(a_vCamPos, camPos, 3);
    XMFLOAT3 xmf(a_vCamPos.x, 0, a_vCamPos.z);
   XM_TO_V(xmf, vec, 3)

   look_at_dir = camPos - vec;

   XMFLOAT3 upDir(a_vCamDir);
   upDir.y = 0;
   XM_TO_V(upDir, up_dir, 3);

   V_TO_XM(look_at_dir, xmLAD, 3);
   V_TO_XM(up_dir, xmUD, 3);
    ModelViewMtx(&m_CameraMatrix, a_vCamPos, xmLAD, xmUD);

   XM_TO_M(m_CameraMatrix, camM);
    Frustum.Transform(camM);

    /* Calc light bbox */
    Frustum.CalcAABBox(&FrustumBBox);

    /* Projection matrix */
    OrthoMtx(&m_ProjectionMatrix, FrustumBBox.Min(), FrustumBBox.Max());

   XMStoreFloat4x4(&m_ViewProjMatrix, mv * camM * proj);
}

void GrassTracker::UpdateTrack (XMFLOAT4X4& a_mView, XMFLOAT4X4& a_mProj,
   XMFLOAT3 a_vCamPos, XMFLOAT3 a_vCamDir,
   Mesh* a_pMeshes[], UINT a_uNumMeshes)
{
   float clear_color[4] = { 1, 1, 1, 1 };
   m_pD3DDeviceCtx->ClearRenderTargetView(m_pTrackRTV, clear_color);

   ID3D11RenderTargetView* old_rt;
   ID3D11DepthStencilView* old_dsv;

   m_pD3DDeviceCtx->OMGetRenderTargets(1, &old_rt, &old_dsv);

   m_pD3DDeviceCtx->OMSetRenderTargets(1, &m_pTrackRTV, NULL);

   /* Layout */
   m_pD3DDeviceCtx->IASetInputLayout(m_TrackLayout);

   /* Vertex buffer */
   bool changed;
   changed = UpdateMeshPositions(a_pMeshes, a_uNumMeshes);
   if (changed)
      PrepareVertexBuffer(a_pMeshes, a_uNumMeshes);

   UINT stride = sizeof(VertexDescription);
   UINT offset = 0;
   m_pD3DDeviceCtx->IASetVertexBuffers(0, 1, &m_TrackVB, &stride, &offset);

   m_pD3DDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

   /* Set matrices */
   UpdateMatrices(a_mView, a_mProj, a_vCamPos, a_vCamDir);
   m_ViewProjVariable->SetMatrix((float *) &m_ViewProjMatrix);

   /* Render geometry */
   m_TrackTechnique->GetPassByIndex(0)->Apply(0, m_pD3DDeviceCtx);
   m_pD3DDeviceCtx->Draw(m_TotalPoints * 2, 0);

   m_pD3DDeviceCtx->OMSetRenderTargets(1, &old_rt, old_dsv);
   SAFE_RELEASE(old_rt);
   SAFE_RELEASE(old_dsv);
}
