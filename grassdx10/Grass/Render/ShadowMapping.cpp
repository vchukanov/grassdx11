#include "ShadowMapping.h"


LiSPSM::LiSPSM( UINT a_uWidth, UINT a_uHeight, ID3D10Device *a_pD3DDevice )
{
    m_pD3DDevice = a_pD3DDevice;
    m_bUseUniformSM = false;
    /* Init ViewPort */
    m_ViewPort.Height   = a_uHeight;
    m_ViewPort.Width    = a_uWidth;
    m_ViewPort.TopLeftX = 0;
    m_ViewPort.TopLeftY = 0;
    m_ViewPort.MinDepth = 0.0f;
    m_ViewPort.MaxDepth = 1.0f;    
    
    //Texture, SRV, DSV, RTV...
    D3D10_TEXTURE2D_DESC TexDesc;
    TexDesc.Width = a_uWidth;
    TexDesc.Height = a_uHeight;
    TexDesc.MipLevels = 1;
    TexDesc.ArraySize = 1;
    TexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    TexDesc.SampleDesc.Count = 1;
    TexDesc.SampleDesc.Quality = 0;
    TexDesc.Usage = D3D10_USAGE_DEFAULT;
    TexDesc.CPUAccessFlags = 0;
    TexDesc.MiscFlags = 0;
    TexDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE;
    m_pD3DDevice->CreateTexture2D(&TexDesc, NULL, &m_pTexture);

    
    TexDesc.Format = DXGI_FORMAT_R8_UNORM;
    TexDesc.SampleDesc.Count = 1;
    TexDesc.SampleDesc.Quality = 0;
    TexDesc.Usage = D3D10_USAGE_DEFAULT;
    TexDesc.CPUAccessFlags = 0;
    TexDesc.MiscFlags = 0;
    TexDesc.BindFlags = D3D10_BIND_RENDER_TARGET;
    m_pD3DDevice->CreateTexture2D(&TexDesc, NULL, &m_pRTT);

    /* Creating RTV */
    D3D10_RENDER_TARGET_VIEW_DESC RTVdesc;
    ZeroMemory( &RTVdesc, sizeof(RTVdesc) );
    RTVdesc.Format = DXGI_FORMAT_R8_UNORM;
    RTVdesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
    RTVdesc.Texture2D.MipSlice = 0;
    m_pD3DDevice->CreateRenderTargetView(m_pRTT, &RTVdesc, &m_pRTV);

    /* Creating DSV */
    D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
    descDSV.Format = DXGI_FORMAT_D32_FLOAT;
    descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    m_pD3DDevice->CreateDepthStencilView(m_pTexture, &descDSV, &m_pDSV);

    /* Creating Shader Resource View */
    D3D10_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    ZeroMemory( &SRVDesc, sizeof(SRVDesc) );
    SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
    SRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Texture2D.MostDetailedMip = 0;
    SRVDesc.Texture2D.MipLevels = 1;
    m_pD3DDevice->CreateShaderResourceView(m_pTexture, &SRVDesc, &m_pSRV);
    /* Rasterizer State */
    D3D10_RASTERIZER_DESC descRS;
    descRS.FillMode = D3D10_FILL_SOLID;
    descRS.CullMode = D3D10_CULL_NONE;
    descRS.FrontCounterClockwise = true;
    descRS.DepthBias             = 0;
    descRS.DepthBiasClamp        = 0;
    descRS.SlopeScaledDepthBias  = 0.0f;
    descRS.DepthClipEnable       = false;
    descRS.ScissorEnable         = false;
    descRS.MultisampleEnable     = false;
    descRS.AntialiasedLineEnable = false;
    m_pD3DDevice->CreateRasterizerState( &descRS, &m_pRS );
}

LiSPSM::~LiSPSM( )
{
    SAFE_RELEASE(m_pRTT);
    SAFE_RELEASE(m_pRTV);
    SAFE_RELEASE(m_pRS);
    SAFE_RELEASE(m_pDSV);
    SAFE_RELEASE(m_pSRV);
    SAFE_RELEASE(m_pTexture);
}

void LiSPSM::UpdatePointSet( const D3DXMATRIX &a_mCamMV, const D3DXMATRIX &a_mCamProj )
{
    //int i;
    D3DXVECTOR3 vIsect;
    D3DXVECTOR3 cubeMin(-1.0f, -1.0f, 0.0f);
    D3DXVECTOR3 cubeMax(1.0f, 1.0f, 1.0f);
    maths::AABox FrustumBBox(cubeMin, cubeMax);
    //maths::PointArray FrustumPts;
    D3DXMATRIX M;
    D3DXMATRIX InvEyeProjView;// = !(CamMvMtx * CamPjMtx);
    D3DXMatrixMultiply(&M, &a_mCamMV, &a_mCamProj);
    D3DXMatrixInverse(&InvEyeProjView, NULL, &M);
    

    m_ShadowOcclAndCasters.SetSize(8);
    FrustumBBox.GetPoints(&m_ShadowOcclAndCasters);
    m_ShadowOcclAndCasters.Transform(InvEyeProjView);
    
    //D3DXVECTOR3 vIsectDir = (-1.0f) * LightDir;
    //m_ShadowOcclAndCasters = FrustumPts;    
    /*D3DXVECTOR3 vIsectDir = (-1.0f) * m_vLightDir;
    for (i = 0; i < 8; ++i)
    {
        if (m_pSceneBBox->LastLineISect(&vIsect, maths::Line(vIsectDir, Frustum[i] )) )
        {
            m_ShadowOcclAndCasters.AppendObj(&vIsect);
        }
    }*/
}

void LiSPSM::GenUniformMtx( )
{
    maths::AABox LightSpaceBBox;
    D3DXVECTOR3 vUp;
    D3DXVec3Cross(&vUp, &m_vLightDir, &m_vCamDir);
    ModelViewMtx(&m_mModelView, m_vCamPos + (-1.0f) * m_vLightDir, m_vLightDir, vUp);
    m_ShadowOcclAndCasters.Transform(m_mModelView);
    m_ShadowOcclAndCasters.CalcAABBox(&LightSpaceBBox);
    OrthoMtx(&m_mProjection, LightSpaceBBox.Min(), LightSpaceBBox.Max());
}

void LiSPSM::GenLiSPSMMtx( )
{
    /* LiSPSM */
    float cosGamma = D3DXVec3Dot(&m_vCamDir, &m_vLightDir);
    float sinGamma = sqrtf(1.0f - cosGamma * cosGamma);
    const float factor = 1.0f / sinGamma;
    float Offset;
    float yMin;
    float yMax;
    float fNear;
    float fFar;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 lookat;
    int i;
    D3DXVECTOR3 Up;

    D3DXVec3Cross(&lookat, &m_vLightDir, &m_vCamDir);
    D3DXVec3Cross(&Up, &lookat, &m_vLightDir);
    //Up = (m_vLightDir ^ m_vCamDir) ^ m_vLightDir;
    //Up.Normalize();
    D3DXVec3Normalize(&Up, &Up);
    ModelViewMtx(&m_mModelView, m_vCamPos, m_vLightDir, Up);
    /*lookat = m_vCamPos + m_vLightDir;
    D3DXMatrixLookAtLH(&m_mModelView, &m_vCamPos, &lookat, &Up);*/

    /* Getting real light-space volume */
    m_ShadowOcclAndCasters.Transform(m_mModelView);

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
    fNear = (yMin + sqrt(yMin + (yMax - yMin)* factor)) * factor;
    fFar = yMax + (fNear - yMin);
    /*after next step, Offset will be > 0 anyway*/
    Offset += fNear - yMin;
    //new observer point behind eye position
    pos = m_vCamPos - (Up * Offset);


    //we have Near and Far plane, now we need to calculate Right and Top planes
    float fRight = 0.0f;
    float fTop   = 0.0f;
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

    ModelViewMtx(&m_mModelView, pos, m_vLightDir, Up);
    /*lookat = pos + m_vLightDir;
    D3DXMatrixLookAtLH(&m_mModelView, &pos, &lookat, &Up);*/

    //symmetric perspective transformation matrix
    //fNear and fFar in y direction    
    float *pProj = (float*)(&m_mProjection);
    pProj[ 0]  = fNear / fRight;
    pProj[ 1]  = 0.0f;
    pProj[ 2]  = 0.0f;
    pProj[ 3]  = 0.0f;
    pProj[ 4]  = 0.0f;
    pProj[ 5]  = fFar / (fFar - fNear);//(fFar + fNear) / (fFar - fNear);		    
    pProj[ 6]  = 0.0f;
    pProj[ 7]  = 1.0;
    pProj[ 8]  = 0.0;
    pProj[ 9]  = 0.0;
    pProj[10]  = fNear / fTop;
    pProj[11]  = 0.0;
    pProj[12]  = 0.0;
    pProj[13]  = -fFar * fNear / (fFar - fNear);//-2.0f * fFar * fNear / (fFar - fNear);		
    pProj[14]  = 0.0;				            
    pProj[15]  = 0.0;
    //D3DXMATRIX mScale;
    //pProj = (float*)(&mScale);
    //pProj[ 0]  = 1.0f; pProj[ 1]  = 0.0f; pProj[ 2]  = 0.0f; pProj[ 3]  = 0.0f;
    //pProj[ 4]  = 0.0f; pProj[ 5]  = 1.0f; pProj[ 6]  = 0.0f; pProj[ 7]  = 0.0f;
    //pProj[ 8]  = 0.0f; pProj[ 9]  = 0.0f; pProj[10]  = 1.0f;pProj[11]  = 0.0f;
    //pProj[12]  = 0.0f; pProj[13]  = 0.0f; pProj[14]  = 0.0f;pProj[15]  = 1.0f;
    //D3DXMatrixMultiply(&m_mProjection, &m_mProjection, &mScale);
}

void LiSPSM::UpdateLightDir( const D3DXVECTOR3& a_vLightDir )
{
    m_vLightDir = a_vLightDir;
}

void LiSPSM::UpdateMtx( const D3DXMATRIX &a_mCamMV, const D3DXMATRIX &a_mCamProj, 
                       const D3DXVECTOR3& a_vCamPos, const D3DXVECTOR3& a_vCamDir )
{
    maths::AABox BB;
    m_vCamDir = a_vCamDir;
    D3DXVec3Normalize(&m_vCamDir, &m_vCamDir);
    m_vCamPos = a_vCamPos;
    static const float fProjElement33 = 25.f/(25.f - 0.1f);
    static const float fProjElement43 = -2.5f/(25.f - 0.1f);
    D3DXMATRIX mProj = a_mCamProj;
    mProj._33 = fProjElement33;
    mProj._43 = fProjElement43;
    UpdatePointSet(a_mCamMV, mProj);
    //m_bUseUniformSM = (D3DXVec3Dot(&m_vCamDir, &m_vLightDir) > 0.9f) ? true : false;
    if (m_bUseUniformSM)
    {
        GenUniformMtx();
    }
    else
    {
        GenLiSPSMMtx();
    }
    D3DXMatrixMultiply(&m_mViewProj, &m_mModelView, &m_mProjection);
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

void LiSPSM::BeginShadowMapPass( )
{
    //float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    UINT uNumViewPorts = 1;
    /* Saving DSV & ViewPort */
    m_pD3DDevice->OMGetRenderTargets(1, &m_pOrigRTV, &m_pOrigDSV);
    m_pD3DDevice->RSGetViewports(&uNumViewPorts, &m_OrigVP);
    m_pD3DDevice->RSGetState(&m_pOrigRS);
    /* Setting up DSV & ViewPort */
    m_pD3DDevice->RSSetViewports(uNumViewPorts, &m_ViewPort);
    m_pD3DDevice->RSSetState(m_pRS);
    //m_pD3DDevice->ClearRenderTargetView( m_pRTV, ClearColor);
    m_pD3DDevice->ClearDepthStencilView( m_pDSV, D3D10_CLEAR_DEPTH, 1.0f, 0);
    m_pD3DDevice->OMSetRenderTargets( 1, &m_pRTV, m_pDSV );
}

ID3D10ShaderResourceView *LiSPSM::EndShadowMapPass( )
{
    /* Restore DSV & ViewPort */
    m_pD3DDevice->RSSetViewports(1, &m_OrigVP);
    m_pD3DDevice->RSSetState(m_pOrigRS);
    m_pD3DDevice->OMSetRenderTargets( 1, &m_pOrigRTV, m_pOrigDSV );
    SAFE_RELEASE(m_pOrigRTV);
    SAFE_RELEASE(m_pOrigDSV);
    SAFE_RELEASE(m_pOrigRS);
    return m_pSRV;
}

const D3DXMATRIX& LiSPSM::GetViewMtx( )
{
    return m_mModelView;
}

const D3DXMATRIX& LiSPSM::GetProjMtx( )
{
    return m_mProjection;
}

const D3DXMATRIX& LiSPSM::GetViewProjMtx( )
{
    return m_mViewProj;
}