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

   XMVECTOR GetPosition (void);

   void SetMaxHorizFlow  (float a_fMaxFlowStrength) {m_pAxesFanFlow->SetMaxFlowStrength(a_fMaxFlowStrength); }
   void SetFanRadius     (float a_fR);
   void SetDeltaSlices   (float a_fDeltaSlices);
   void SetShift         (float a_fShift);
   void SetDirection     (const float3& a_vDir);
   void SetTransform     (const float3& a_vPos);
   void SetAngleSpeed    (float a_fS);

public:
   AxesFan*     m_pAxesFan      = NULL;
   AxesFanFlow* m_pAxesFanFlow  = NULL;

   ID3D11Device         *m_pD3DDevice;
   ID3D11DeviceContext  *m_pD3DDeviceCtx;

   float          m_fTerrRadius;
   ID3DX11Effect* m_Effect;

   UINT m_uFlowTexSize = 512;

   ID3D11ShaderResourceView *m_NoiseSRV;

   XMVECTOR m_vPosition;
};

