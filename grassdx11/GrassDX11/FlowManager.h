#pragma once

#include <vector>

#include "includes.h"

#include "AxesFan.h"
#include "AxesFanFlow.h"

class FlowManager {
public:
   FlowManager  (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect* a_Effect, float a_fTerrRadius, ID3D11ShaderResourceView* a_NoiseSRV);
   ~FlowManager (void);

   void CreateAxesFan (const XMFLOAT3& a_vPosition);

   void Update    (float a_fElapsedTime, float a_fTime);
   void RenderFan (void);

   ID3D11ShaderResourceView* GetFlowSRV (void) { return m_pAxesFanFlow->GetShaderResourceView(); }

   void SetMaxHorizFlow  (float a_fMaxHorizFlow) {m_pAxesFanFlow->SetMaxHorizFlow(a_fMaxHorizFlow); }
   void SetMaxVertFlow   (float a_fMaxVertFlow) { m_pAxesFanFlow->SetMaxVertFlow(a_fMaxVertFlow); }
   void SetDampPower     (float a_fDampPower) {m_pAxesFanFlow->SetDampPower(a_fDampPower); }
   void SetDistPower     (float a_fDistPower) {m_pAxesFanFlow->SetDistPower(a_fDistPower); }
   void SetMaxFlowRadius (float a_fMaxFlowRadius) { m_pAxesFanFlow->SetMaxFlowRadius(a_fMaxFlowRadius); }
   void SetShift         (float a_fShift) { m_pAxesFanFlow->SetShift(a_fShift); }

   void SetDirection     (const float3& a_vDir) 
   {
      m_pAxesFanFlow->SetDirection(a_vDir); 
      m_pAxesFan->SetDirection(a_vDir);
   }
   
   void SetTransform     (const float3& a_vPos) 
   { 
      m_pAxesFanFlow->SetPosition(a_vPos); 
      m_pAxesFan->SetPosition(a_vPos);
   }

   void SetRadius (float a_fR)
   {
      m_pAxesFanFlow->SetR(a_fR);
      m_pAxesFan->SetR(a_fR);
   }

public:
   AxesFan*     m_pAxesFan      = NULL;
   AxesFanFlow* m_pAxesFanFlow  = NULL;

   ID3D11Device         *m_pD3DDevice;
   ID3D11DeviceContext  *m_pD3DDeviceCtx;

   float          m_fTerrRadius;
   ID3DX11Effect* m_Effect;

   UINT m_uFlowTexSize = 512;

   ID3D11ShaderResourceView *m_NoiseSRV;
};

