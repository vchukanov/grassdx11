#pragma once

#include "includes.h"


struct AxesFanFlowVertex
{
   XMFLOAT3 vPos;
   XMFLOAT2 vTexCoord;
};

class AxesFanFlow {
public:
   bool Initialize (ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceCtx, int textureWidth, int textureHeight, float a_fTerrRadius);
   void ShutDown   (void);

   void SetRenderTarget   (ID3D11DepthStencilView* pDSV);
   void ClearRenderTarget (ID3D11DepthStencilView* pDSV);
   
   ID3D11ShaderResourceView* GetShaderResourceView (void);

   void Update          (void);
   void MakeFlowTexture (void);

   void CreateVertexBuffer (void);
   void CreateInputLayout  (void);

   void SetPosition  (const float3& a_vValue);
   void SetTime      (float a_fTime);

   void SetMaxHorizFlow  (float a_fValue);
   void SetMaxVertFlow   (float a_fValue);
   void SetDampPower     (float a_fValue);
   void SetDistPower     (float a_fValue);
   void SetMaxFlowRadius (float a_fValue);
   void SetShift         (float a_fValue);

private: 
   ID3D11Device          *m_pD3DDevice;
   ID3D11DeviceContext   *m_pD3DDeviceCtx;

   ID3D11Texture2D           *m_renderTargetTexture;
   ID3D11RenderTargetView    *m_renderTargetView;
   ID3D11ShaderResourceView  *m_shaderResourceView;

   ID3D11InputLayout *m_pInputLayout;
   ID3D11Buffer      *m_pVertexBuffer;
   UINT               m_uVertexStride;
   UINT               m_uVertexOffset;

   ID3DX11Effect     *m_pEffect;
   ID3DX11EffectPass *m_pPass;

   ID3DX11EffectVectorVariable *m_vPositionESV;
   ID3DX11EffectScalarVariable *m_pTime;

   ID3DX11EffectScalarVariable* m_pMaxHorizFlowESV;
   ID3DX11EffectScalarVariable* m_pMaxVertFlowESV;
   ID3DX11EffectScalarVariable* m_pDampPowerESV;
   ID3DX11EffectScalarVariable* m_pDistPowerESV;
   ID3DX11EffectScalarVariable* m_pMaxFlowRadiusESV;
   ID3DX11EffectScalarVariable* m_pShiftESV;

   float                        m_fTerrRadius;

   int m_width;
   int m_height;

   XMFLOAT3 m_position;
   float    m_radius;
};
