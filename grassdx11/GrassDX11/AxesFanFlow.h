#pragma once

#include "includes.h"


struct AxesFanFlowVertex
{
   XMFLOAT3 vPos;
   XMFLOAT2 vTexCoord;
};

class AxesFanFlow {
public:
   AxesFanFlow  (ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceCtx, int textureWidth, int textureHeight, float a_fTerrRadius);
   ~AxesFanFlow (void);

   void SetRenderTarget   (ID3D11DepthStencilView* pDSV);
   void ClearRenderTarget (ID3D11DepthStencilView* pDSV);
   
   ID3D11ShaderResourceView* GetShaderResourceView (void);

   void Update          (void);
   void MakeFlowTexture (void);

   void CreateVertexBuffer (void);
   void CreateInputLayout  (void);

   void SetPosition    (const float3& a_vValue);
   void SetDirection   (const float3& a_vValue);
   void SetRingsNumber (int  a_fValue);
   void SetTime        (float a_fTime);

   void SetNoise     (ID3D11ShaderResourceView *a_pNoiseSRV);
   void SetHeightMap (ID3D11ShaderResourceView* a_pHeightMapSRV);


   void SetHeightScale (float a_fHeightScale);

   void SetMaxFlowStrength  (float a_fValue);
   void SetFanRadius        (float a_fValue);
   void SetDeltaSlices      (float a_fValue);
   void SetShift            (float a_fValue);
   void SetAngleSpeed       (float a_fValue);

private: 
   ID3D11Device          *m_pD3DDevice;
   ID3D11DeviceContext   *m_pD3DDeviceCtx;

   ID3D11Texture2D           *m_renderTargetsTexture;
   ID3D11RenderTargetView    *m_renderTargetsView[NUM_SEGMENTS - 1];
   ID3D11ShaderResourceView  *m_shaderResourceView;

   ID3D11InputLayout *m_pInputLayout;
   ID3D11Buffer      *m_pVertexBuffer;
   UINT               m_uVertexStride;
   UINT               m_uVertexOffset;

   ID3DX11Effect     *m_pEffect;
   ID3DX11EffectPass *m_pPass;

   ID3DX11EffectVectorVariable *m_vPositionESV;
   ID3DX11EffectVectorVariable *m_vDirectionESV;
   ID3DX11EffectScalarVariable *m_pRingsNumber;
   ID3DX11EffectScalarVariable *m_pTime;
   ID3DX11EffectScalarVariable* m_pHeightScale;

   ID3DX11EffectShaderResourceVariable *m_pNoiseSRV;
   ID3DX11EffectShaderResourceVariable* m_pHeightMapSRV;

   ID3DX11EffectScalarVariable* m_pMaxFlowStrengthESV;
   ID3DX11EffectScalarVariable* m_pFanRadiusESV;
   ID3DX11EffectScalarVariable* m_pDeltaSlicesESV;
   ID3DX11EffectScalarVariable* m_pShiftESV;
   ID3DX11EffectScalarVariable* m_pAngleSpeedESV;


   float                        m_fTerrRadius;

   int m_width;
   int m_height;
};
