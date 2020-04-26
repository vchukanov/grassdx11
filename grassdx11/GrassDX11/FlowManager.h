#pragma once

#include <vector>

#include "includes.h"

#include "AxesFan.h"
#include "AxesFanFlow.h"

#define FLOW_TEX_SIZE 512

class FlowManager;

struct AxesFanDesc {
public:
   AxesFanDesc (void);

   void Setup               (void);
   void UpdateFromTransform (const XMMATRIX& transform);

public:
   float      radius     = 10;
   XMFLOAT3   direction  = XMFLOAT3(0, -1, 0);
   XMFLOAT3   position   = XMFLOAT3(0, 20, 0);
   XMFLOAT4X4 transform;
   XMFLOAT4X4 initialTransform;

   float    angleSpeed = 50;
   int      bladesNum  = 2;

   AxesFanFlow *pAxesFanFlow = NULL; 
   AxesFan     *pAxesFan     = NULL;
   FlowManager *pFlowManager = NULL;
};


class FlowManager {
public:
   FlowManager  (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, ID3DX11Effect* a_Effect, float a_fTerrRadius, ID3D11ShaderResourceView* a_NoiseSRV);
   ~FlowManager (void);

   int CreateAxesFan (const XMMATRIX& initialTransform);

   void Update          (float a_fElapsedTime, float a_fTime);
   void RenderFans      (bool isVelPass = false, bool isBlured = false);
 
   ID3D11ShaderResourceView* GetFlowSRV (void);

   void SetMaxHorizFlow  (float a_fMaxFlowStrength);
   void SetDeltaSlices   (float a_fDeltaSlices);
   void SetShift         (float a_fShift);
 
public:
   std::vector<AxesFanDesc> fans;

   ID3D11Device         *m_pD3DDevice;
   ID3D11DeviceContext  *m_pD3DDeviceCtx;

   float          m_fTerrRadius;
   ID3DX11Effect *m_Effect;

   ID3D11ShaderResourceView *m_NoiseSRV;
  
   AxesFanFlow *m_pAxesFanFlow = NULL;
};

