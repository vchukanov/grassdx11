#pragma once

#include "includes.h"
#include "maths.h"

class LiSPSM
{
public:
    bool                      m_bUseUniformSM;
    ID3D11Device             *m_pD3DDevice;
    ID3D11DeviceContext      *m_pD3DDeviceCtx;
    XMFLOAT3                  m_vLightDir;
    XMFLOAT3                  m_vCamDir;
    XMFLOAT3                  m_vCamPos;
    XMFLOAT4X4                  m_mModelView;
    XMFLOAT4X4                  m_mProjection;
    XMFLOAT4X4                  m_mViewProj;
    ID3D11Texture2D          *m_pTexture;
    ID3D11Texture2D          *m_pRTT;//rendertarget texture
    ID3D11DepthStencilView   *m_pDSV;
    ID3D11RenderTargetView   *m_pRTV;
    ID3D11ShaderResourceView *m_pSRV;
    ID3D11RasterizerState    *m_pRS;    
    D3D11_VIEWPORT            m_ViewPort;
    ID3D11RenderTargetView   *m_pOrigRTV;
    ID3D11DepthStencilView   *m_pOrigDSV;
    ID3D11RasterizerState    *m_pOrigRS;
    D3D11_VIEWPORT            m_OrigVP;    
    
    /* All shadow casters and receivers 
     * are inside this set of points 
     */
    maths::PointArray         m_ShadowOcclAndCasters;

    /* constructs m_ShadowOcclAndCasters */
    void UpdatePointSet                         (const XMMATRIX&a_mCamMV, const XMMATRIX&a_mCamProj);
    void GenLiSPSMMtx                           (void);
    void GenUniformMtx                          (void);

public:
    LiSPSM                                      (UINT a_uWidth, UINT a_uHeight, ID3D11Device *a_pD3DDevice, ID3D11DeviceContext * a_pD3DDeviceCtx);
    ~LiSPSM                                     (void);    
    
    //void SwitchToUniformSM                      ( );
    //void SwitchToLiSPSM                         ( );
    
    void                      UpdateLightDir     (const XMVECTOR& a_vLightDir );
    void                      UpdateMtx          (const XMMATRIX&a_mCamMV, const XMMATRIX&a_mCamProj, const XMVECTOR& a_vCamPos, const XMVECTOR& a_vCamDir);
    void                      BeginShadowMapPass (void);
    ID3D11ShaderResourceView* EndShadowMapPass   (void);
    const XMMATRIX           GetViewMtx         (void);
    const XMMATRIX           GetProjMtx         (void);
    const XMMATRIX           GetViewProjMtx     (void);
};