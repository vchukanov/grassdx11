#pragma once

#include "includes.h"
#include "maths.h"

class LiSPSM
{
protected:
    bool                      m_bUseUniformSM;
    ID3D10Device             *m_pD3DDevice;
    D3DXVECTOR3               m_vLightDir;
    D3DXVECTOR3               m_vCamDir;
    D3DXVECTOR3               m_vCamPos;
    D3DXMATRIX                m_mModelView;
    D3DXMATRIX                m_mProjection;
    D3DXMATRIX                m_mViewProj;
    ID3D10Texture2D          *m_pTexture;
    ID3D10Texture2D          *m_pRTT;//rendertarget texture
    ID3D10DepthStencilView   *m_pDSV;
    ID3D10RenderTargetView   *m_pRTV;
    ID3D10ShaderResourceView *m_pSRV;
    ID3D10RasterizerState    *m_pRS;    
    D3D10_VIEWPORT            m_ViewPort;
    ID3D10RenderTargetView   *m_pOrigRTV;
    ID3D10DepthStencilView   *m_pOrigDSV;
    ID3D10RasterizerState    *m_pOrigRS;
    D3D10_VIEWPORT            m_OrigVP;    
    
    /* All shadow casters and receivers 
     * are inside this set of points 
     */
    maths::PointArray         m_ShadowOcclAndCasters;
    /* constructs m_ShadowOcclAndCasters */
    void UpdatePointSet                         ( const D3DXMATRIX &a_mCamMV, const D3DXMATRIX &a_mCamProj );
    void GenLiSPSMMtx                           ( );
    void GenUniformMtx                          ( );

public:
    LiSPSM                                      ( UINT a_uWidth, UINT a_uHeight, ID3D10Device *a_pD3DDevice );
    ~LiSPSM                                     ( );    
    //void SwitchToUniformSM                      ( );
    //void SwitchToLiSPSM                         ( );
    void UpdateLightDir                         ( const D3DXVECTOR3& a_vLightDir );
    void UpdateMtx                              ( const D3DXMATRIX &a_mCamMV, const D3DXMATRIX &a_mCamProj, 
        const D3DXVECTOR3& a_vCamPos, const D3DXVECTOR3& a_vCamDir );
    void BeginShadowMapPass                     ( );
    ID3D10ShaderResourceView *EndShadowMapPass  ( );
    const D3DXMATRIX &GetViewMtx                ( );
    const D3DXMATRIX &GetProjMtx                ( );
    const D3DXMATRIX &GetViewProjMtx            ( );
};