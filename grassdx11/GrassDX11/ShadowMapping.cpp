#include "ShadowMapping.h"
#include "mtxfrustum.h"

LiSPSM::LiSPSM(UINT a_uWidth, UINT a_uHeight, ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx)
{
   m_pD3DDevice = a_pD3DDevice;
   m_pD3DDeviceCtx = a_pD3DDeviceCtx;
   m_bUseUniformSM = true;
   /* Init ViewPort */
   m_ViewPort.Height = a_uHeight;
   m_ViewPort.Width = a_uWidth;
   m_ViewPort.TopLeftX = 0;
   m_ViewPort.TopLeftY = 0;
   m_ViewPort.MinDepth = 0.0f;
   m_ViewPort.MaxDepth = 1.0f;

   //Texture, SRV, DSV, RTV...
   D3D11_TEXTURE2D_DESC TexDesc;
   TexDesc.Width = a_uWidth;
   TexDesc.Height = a_uHeight;
   TexDesc.MipLevels = 1;
   TexDesc.ArraySize = 1;
   TexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
   TexDesc.SampleDesc.Count = 1;
   TexDesc.SampleDesc.Quality = 0;
   TexDesc.Usage = D3D11_USAGE_DEFAULT;
   TexDesc.CPUAccessFlags = 0;
   TexDesc.MiscFlags = 0;
   TexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
   m_pD3DDevice->CreateTexture2D(&TexDesc, NULL, &m_pTexture);


   TexDesc.Format = DXGI_FORMAT_R8_UNORM;
   TexDesc.SampleDesc.Count = 1;
   TexDesc.SampleDesc.Quality = 0;
   TexDesc.Usage = D3D11_USAGE_DEFAULT;
   TexDesc.CPUAccessFlags = 0;
   TexDesc.MiscFlags = 0;
   TexDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
   m_pD3DDevice->CreateTexture2D(&TexDesc, NULL, &m_pRTT);

   /* Creating RTV */
   D3D11_RENDER_TARGET_VIEW_DESC RTVdesc;
   ZeroMemory(&RTVdesc, sizeof(RTVdesc));
   RTVdesc.Format = DXGI_FORMAT_R8_UNORM;
   RTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
   RTVdesc.Texture2D.MipSlice = 0;
   m_pD3DDevice->CreateRenderTargetView(m_pRTT, &RTVdesc, &m_pRTV);

   /* Creating DSV */
   D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
   ZeroMemory(&descDSV, sizeof(descDSV));
   descDSV.Format = DXGI_FORMAT_D32_FLOAT;
   descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
   descDSV.Texture2D.MipSlice = 0;
   m_pD3DDevice->CreateDepthStencilView(m_pTexture, &descDSV, &m_pDSV);

   /* Creating Shader Resource View */
   D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
   ZeroMemory(&SRVDesc, sizeof(SRVDesc));
   SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
   SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
   SRVDesc.Texture2D.MostDetailedMip = 0;
   SRVDesc.Texture2D.MipLevels = 1;
   m_pD3DDevice->CreateShaderResourceView(m_pTexture, &SRVDesc, &m_pSRV);
   /* Rasterizer State */
   D3D11_RASTERIZER_DESC descRS;
   descRS.FillMode = D3D11_FILL_SOLID;
   descRS.CullMode = D3D11_CULL_NONE;
   descRS.FrontCounterClockwise = true;
   descRS.DepthBias = 0;
   descRS.DepthBiasClamp = 0;
   descRS.SlopeScaledDepthBias = 0.0f;
   descRS.DepthClipEnable = false;
   descRS.ScissorEnable = false;
   descRS.MultisampleEnable = false;
   descRS.AntialiasedLineEnable = false;
   m_pD3DDevice->CreateRasterizerState(&descRS, &m_pRS);
}

LiSPSM::~LiSPSM()
{
   SAFE_RELEASE(m_pRTT);
   SAFE_RELEASE(m_pRTV);
   SAFE_RELEASE(m_pRS);
   SAFE_RELEASE(m_pDSV);
   SAFE_RELEASE(m_pSRV);
   SAFE_RELEASE(m_pTexture);
}

void LiSPSM::UpdatePointSet(const XMMATRIX& a_mCamMV, const XMMATRIX& a_mCamProj)
{
   //int i;
   XMVECTOR vIsect;
   XMVECTOR cubeMin = create(-1.0f, -1.0f, 0.0f);
   XMVECTOR cubeMax = create(1.0f, 1.0f, 1.0f);

   V_TO_XM(cubeMin, cmin, 3);
   V_TO_XM(cubeMax, cmax, 3);

   maths::AABox FrustumBBox(cmin, cmax);
   //maths::PointArray FrustumPts;
   XMMATRIX M;
   XMMATRIX InvEyeProjView;// = !(CamMvMtx * CamPjMtx);
   
   M = XMMatrixMultiply(a_mCamMV, a_mCamProj);
   InvEyeProjView = XMMatrixInverse(NULL, M);


   m_ShadowOcclAndCasters.SetSize(8);
   FrustumBBox.GetPoints(&m_ShadowOcclAndCasters);
   m_ShadowOcclAndCasters.Transform(InvEyeProjView);

   //XMVECTOR vIsectDir = (-1.0f) * LightDir;
   //m_ShadowOcclAndCasters = FrustumPts;    
   /*XMVECTOR vIsectDir = (-1.0f) * m_vLightDir;
   for (i = 0; i < 8; ++i)
   {
       if (m_pSceneBBox->LastLineISect(&vIsect, maths::Line(vIsectDir, Frustum[i] )) )
       {
           m_ShadowOcclAndCasters.AppendObj(&vIsect);
       }
   }*/
}

void LiSPSM::GenUniformMtx()
{
   maths::AABox LightSpaceBBox;
   XMVECTOR vUp;

   XM_TO_V(m_vLightDir, ldir, 3);
   XM_TO_V(m_vCamDir, cdir, 3);
   XM_TO_V(m_vCamPos, cpos, 3);
   XM_TO_M(m_mModelView, mv);
   XM_TO_M(m_mProjection, p);

   vUp = XMVector3Cross(ldir, cdir);
   
   ModelViewMtx(&mv, cpos + (-1.0f) * ldir, ldir, vUp);
   m_ShadowOcclAndCasters.Transform(mv);
   m_ShadowOcclAndCasters.CalcAABBox(&LightSpaceBBox);

   auto& xmin = LightSpaceBBox.Min();
   auto& xmax = LightSpaceBBox.Max();
   
   XM_TO_V(xmin, vMin, 3);
   XM_TO_V(xmax, vMax, 3);
   
   OrthoMtx(&p, vMin, vMax);

   XMStoreFloat4x4(&m_mModelView, mv);
   XMStoreFloat4x4(&m_mProjection, p);
}

void LiSPSM::GenLiSPSMMtx()
{
   XM_TO_V(m_vLightDir, ldir, 3);
   XM_TO_V(m_vCamDir, cdir, 3);
   XM_TO_V(m_vCamPos, cpos, 3);
   XM_TO_M(m_mModelView, mv);
   
   /* LiSPSM */
   float cosGamma = getx(XMVector3Dot(normalize(cdir), normalize(ldir)));
   float sinGamma = sqrtf(1.0f - cosGamma * cosGamma);
   const float factor = 1.0f / sinGamma;
   float Offset;
   float yMin;
   float yMax;
   float fNear;
   float fFar;
   XMVECTOR pos;
   XMVECTOR lookat;
   int i;
   XMVECTOR Up;
   
   lookat = XMVector3Cross(ldir, cdir);
   Up = XMVector3Cross(lookat, ldir);
   //Up = (m_vLightDir ^ m_vCamDir) ^ m_vLightDir;
   //Up.Normalize();
   Up = XMVector3Normalize(Up);
   ModelViewMtx(&mv, cpos, ldir, Up);
   /*lookat = m_vCamPos + m_vLightDir;
   XMMATRIXLookAtLH(&m_mModelView, &m_vCamPos, &lookat, &Up);*/

   /* Getting real light-space volume */
   m_ShadowOcclAndCasters.Transform(mv);

   yMax = yMin = m_ShadowOcclAndCasters[0].y;
   for (i = 1; i < m_ShadowOcclAndCasters.GetSize(); ++i)
   {
      if (m_ShadowOcclAndCasters[i].y > yMax)
      {
         yMax = m_ShadowOcclAndCasters[i].y;
      }
      if (m_ShadowOcclAndCasters[i].y < yMin)
      {
         yMin = m_ShadowOcclAndCasters[i].y;
      }
   }
   /*extreme case: yMin < 0*/
   Offset = com::maximum(-yMin, 0.0f);
   yMin += Offset;
   yMax += Offset;
   //smth like Nopt value :)
   fNear = (yMin + sqrt(yMin + (yMax - yMin) * factor)) * factor;
   fFar = yMax + (fNear - yMin);
   /*after next step, Offset will be > 0 anyway*/
   Offset += fNear - yMin;
   //new observer point behind eye position
   pos = cpos - (Up * Offset);


   //we have Near and Far plane, now we need to calculate Right and Top planes
   float fRight = 0.0f;
   float fTop = 0.0f;
   //getting maximum of fovX and fovZ tangent values, 
   //than calculating Right = Near * tg(fovX), Top = Near * tg(fovZ)
   float TanFovX;
   float TanFovZ;
   for (i = 0; i < m_ShadowOcclAndCasters.GetSize(); ++i)
   {
      /*m_ShadowOcclAndCasters[i]->y always > 0 because of "Offset" value*/
      m_ShadowOcclAndCasters[i].y += Offset;

      TanFovX = fabs(m_ShadowOcclAndCasters[i].x / m_ShadowOcclAndCasters[i].y);
      if (TanFovX > fRight)
      {
         fRight = TanFovX;
      }

      TanFovZ = fabs(m_ShadowOcclAndCasters[i].z / m_ShadowOcclAndCasters[i].y);
      if (TanFovZ > fTop)
      {
         fTop = TanFovZ;
      }
   }
   fRight *= fNear;
   fTop *= fNear;

   ModelViewMtx(&mv, pos, ldir, Up);
   /*lookat = pos + m_vLightDir;
   XMMATRIXLookAtLH(&m_mModelView, &pos, &lookat, &Up);*/

   //symmetric perspective transformation matrix
   //fNear and fFar in y direction    
   float* pProj = (float*)(&m_mProjection);
   pProj[0] = fNear / fRight;
   pProj[1] = 0.0f;
   pProj[2] = 0.0f;
   pProj[3] = 0.0f;
   pProj[4] = 0.0f;
   pProj[5] = fFar / (fFar - fNear);//(fFar + fNear) / (fFar - fNear);		    
   pProj[6] = 0.0f;
   pProj[7] = 1.0;
   pProj[8] = 0.0;
   pProj[9] = 0.0;
   pProj[10] = fNear / fTop;
   pProj[11] = 0.0;
   pProj[12] = 0.0;
   pProj[13] = -fFar * fNear / (fFar - fNear);//-2.0f * fFar * fNear / (fFar - fNear);		
   pProj[14] = 0.0;
   pProj[15] = 0.0;
   //XMMATRIX mScale;
   //pProj = (float*)(&mScale);
   //pProj[ 0]  = 1.0f; pProj[ 1]  = 0.0f; pProj[ 2]  = 0.0f; pProj[ 3]  = 0.0f;
   //pProj[ 4]  = 0.0f; pProj[ 5]  = 1.0f; pProj[ 6]  = 0.0f; pProj[ 7]  = 0.0f;
   //pProj[ 8]  = 0.0f; pProj[ 9]  = 0.0f; pProj[10]  = 1.0f;pProj[11]  = 0.0f;
   //pProj[12]  = 0.0f; pProj[13]  = 0.0f; pProj[14]  = 0.0f;pProj[15]  = 1.0f;
   //XMMATRIXMultiply(&m_mProjection, &m_mProjection, &mScale);

   XMStoreFloat4x4(&m_mModelView, mv);
}

void LiSPSM::UpdateLightDir(const XMVECTOR& a_vLightDir)
{
   auto vldir = normalize(a_vLightDir);
   V_TO_XM(vldir, ldir, 3);
   m_vLightDir = ldir;
}

void LiSPSM::UpdateMtx(const XMMATRIX& a_mCamMV, const XMMATRIX& a_mCamProj,
   const XMVECTOR& a_vCamPos, const XMVECTOR& a_vCamDir)
{
   XM_TO_V(m_vCamDir, cdir, 3);
   XM_TO_V(m_vCamPos, cpos, 3);
  
   cdir = a_vCamDir;
   cdir = XMVector3Normalize(cdir);
   cpos = a_vCamPos;

   float f = 50.0, n = 0.1;
   static const float fProjElement33 = (f + n) / (f - n);
   static const float fProjElement43 = -2 * f * n / (f - n);
   XMMATRIX mProj = a_mCamProj;
   
   setz(mProj.r[2], fProjElement33);
   setz(mProj.r[3], fProjElement43);
   
   UpdatePointSet(a_mCamMV, mProj);
   //m_bUseUniformSM = (D3DXVec3Dot(&m_vCamDir, &m_vLightDir) > 0.9f) ? true : false;

   XMStoreFloat3(&m_vCamDir, cdir);
   XMStoreFloat3(&m_vCamPos, cpos);
   if (m_bUseUniformSM)
   {
      GenUniformMtx();
   }
   else
   {
      GenLiSPSMMtx();
   }
   XM_TO_M(m_mModelView, mv);
   XM_TO_M(m_mProjection, p);
   XM_TO_M(m_mViewProj, vp);

   vp = XMMatrixMultiply(mv, p);
   
   XMStoreFloat4x4(&m_mViewProj, vp);
}              

//void LiSPSM::SwitchToLiSPSM( )
//{
//    m_bUseUniformSM = false;
//}
//
//void LiSPSM::SwitchToUniformSM( )
//{
//    m_bUseUniformSM = true;
//}

void LiSPSM::BeginShadowMapPass()
{
   float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

   UINT uNumViewPorts = 1;
   /* Saving DSV & ViewPort */
   m_pD3DDeviceCtx->OMGetRenderTargets(1, &m_pOrigRTV, &m_pOrigDSV);
   m_pD3DDeviceCtx->RSGetViewports(&uNumViewPorts, &m_OrigVP);
   m_pD3DDeviceCtx->RSGetState(&m_pOrigRS);
   /* Setting up DSV & ViewPort */
   m_pD3DDeviceCtx->RSSetViewports(uNumViewPorts, &m_ViewPort);
   m_pD3DDeviceCtx->RSSetState(m_pRS);
   m_pD3DDeviceCtx->ClearRenderTargetView( m_pRTV, ClearColor);
   m_pD3DDeviceCtx->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
   m_pD3DDeviceCtx->OMSetRenderTargets(1, &m_pRTV, m_pDSV);
}

ID3D11ShaderResourceView* LiSPSM::EndShadowMapPass()
{
   /* Restore DSV & ViewPort */
   m_pD3DDeviceCtx->RSSetViewports(1, &m_OrigVP);
   m_pD3DDeviceCtx->RSSetState(m_pOrigRS);
   m_pD3DDeviceCtx->OMSetRenderTargets(1, &m_pOrigRTV, m_pOrigDSV);
   SAFE_RELEASE(m_pOrigRTV);
   SAFE_RELEASE(m_pOrigDSV);
   SAFE_RELEASE(m_pOrigRS);
   return m_pSRV;
}

const XMMATRIX LiSPSM::GetViewMtx()
{
   XM_TO_M(m_mModelView, res);
   return res;
}

const XMMATRIX LiSPSM::GetProjMtx()
{
   XM_TO_M(m_mProjection, res);
   return res;
}

const XMMATRIX LiSPSM::GetViewProjMtx()
{
   XM_TO_M(m_mViewProj, res);
   return res;
}