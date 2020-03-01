cbuffer cAxesFanSettings
{
    float3 g_vAxesFanPosOnTex;  // point: center of fan
    float3 g_vDir; // normal 
    int    g_uRingNumber;
    float  g_fTime;

    float  g_fMaxFlowStrength;
    float  g_fFanRadius;
    float  g_fDeltaSlices;
    float  g_fShift;
    float  g_fAngleSpeed;

    float  g_fHeightScale;

    float  g_iResolution;
};

static const float PI = 3.14159265f;

Texture2D g_txNoise;
Texture2D g_txHeightMap;

Texture2D g_txShadowMap; // Hack


#include "Samplers.fx"


struct AxesFanFlowVSIn
{
    float3 vPos      : POSITION;
    float2 vTexCoord : TEXCOORD0;
};

struct AxesFanFlowPSIn
{
    float4 vPos      : SV_Position;
    float2 vsPos     : POSITION;
    float2 vTexCoord : TEXCOORD0;
};


struct AxesFanFlowPSOut
{
    float4 vFlow0 : SV_Target0;
    float4 vFlow1 : SV_Target1;
    float4 vFlow2 : SV_Target2;
};

  
AxesFanFlowPSIn VS( AxesFanFlowVSIn In )
{
    AxesFanFlowPSIn Out;
	Out.vPos = float4(In.vPos, 1.0);
	Out.vTexCoord = In.vTexCoord;

    // make y axis parallel to texture coordinate v axis
    Out.vsPos = float2(In.vPos.x, -In.vPos.y);

    return Out;
}


// E(M)
float EvaluateSecondEllipticIntegralApproximation ( float x )
{
   return PI / 2 * ( 1 - pow((1 / 2), 2) * pow(x, 2) - pow((1 * 3 / (2 * 4)), 2) * pow(x, 4) / 3 - pow((1 * 3 * 5) / (2 * 4 * 6), 4) * pow(x, 4) / 5); 
}


// K(M)
float EvaluateFirstEllipticIntegralApproximation ( float x )
{
   return PI / 2 * ( 1 + pow((1 / 2), 2) * pow(x, 2) + pow((1 * 3 / (2 * 4)), 2) * pow(x, 4) + pow((1 * 3 * 5) / (2 * 4 * 6), 4) * pow(x, 4));
}


float sqr ( float x )
{
    return pow(x, 2);
}


float distanceToPlane (float3 pNormal, float3 pPoint, float3 vPoint) 
{
    float3 normal = normalize(pNormal);
    return abs(dot(vPoint - pPoint, normal));
}


float3 getReflectedVec (float3 vec)
{
    return float3(vec.x, vec.y, -vec.z);
}


float3 getProjectionToPlane (float3 pNormal, float3 pPoint, float3 pt)
{
    float3 normal = normalize(pNormal);
    float  dist  = dot(pt - pPoint, normal);
    return pt - pNormal * dist;
}



AxesFanFlowPSOut PSRingSourcePotentialFlowModel( AxesFanFlowPSIn In )
{
   AxesFanFlowPSOut Out;
   
   float fY = g_txHeightMap.SampleLevel(g_samLinear, (In.vsPos) * 0.5 + 0.5, 0).a * g_fHeightScale; 

   Out.vFlow0 = float4(0, 0, 0, 1);
   Out.vFlow1 = float4(0, 0, 0, 1);
   Out.vFlow2 = float4(0, 0, 0, 1);
   
   [unroll]
   for (int i = 0; i < 4; i++) {
      float fNoise = g_txNoise.Sample(g_samLinear, In.vsPos).r;
      
      float3 fanNormal = normalize(g_vDir.xzy);
      float3 fanPoint = g_vAxesFanPosOnTex.xzy;
      fanNormal.y = -fanNormal.y;

      float3 fanNormal_m = normalize(getReflectedVec(fanNormal));
      float3 fanPoint_m = getReflectedVec(fanPoint);
      float  z = fY + g_fShift + i * g_fDeltaSlices;

      float3 queryPoint = float3(In.vsPos, z);
  
      z = distanceToPlane(fanNormal, fanPoint, queryPoint);
      float3 qpProj = getProjectionToPlane(fanNormal, fanPoint, queryPoint);
      float3 radial = qpProj - fanPoint;
      float  r = length(radial);
      radial = normalize(radial);
      
      float  z_m = distanceToPlane(fanNormal_m, fanPoint_m, queryPoint);
      float3 qpProj_m = getProjectionToPlane(fanNormal_m, fanPoint_m, queryPoint);
      float3 radial_m = qpProj_m - fanPoint_m;
      float  r_m = length(radial_m);
      radial_m = normalize(radial_m);
  
      int    N     = g_uRingNumber; 
      float  R     = g_fFanRadius;
      float  s_max = 0;
      float  M     = 0; 
      float  E_M   = 0;
      float  K_M   = 0;
      
      float  r_k[16]; 
      float  s_k[16];
      
      float p1   = 0;
      float p2   = 0;
      float p1_m = 0;
      float p2_m = 0;

      float v_k  = 0;
      float w_k  = 0;
      float vm_k = 0;
      float wm_k = 0;

      float fDist = length(In.vsPos - fanPoint.xy);

      int k;

      s_max = 6 * N * R / (2 * N * N + 1);
      s_max *= g_fMaxFlowStrength * 0.6 + 0.4 * g_fMaxFlowStrength * sin(i + g_fTime * g_fAngleSpeed - fDist * 100);

      for (k = 0; k < N; k++) {
          r_k[k] = R - k * R / N;
          s_k[k] = s_max * r_k[k] / R;

          p1 = sqr((r + r_k[k])) + sqr(z);
          p2 = sqr((r - r_k[k])) + sqr(z);
          
          p1_m = sqr((r + r_k[k])) + sqr(z_m);
          p2_m = sqr((r - r_k[k])) + sqr(z_m);

          M = 4 * r * r_k[k] / p1;
          K_M = EvaluateFirstEllipticIntegralApproximation(M);
          E_M = EvaluateSecondEllipticIntegralApproximation(M);
          
          w_k  += -s_k[k] * r_k[k] * z * E_M / (PI * p2   * sqrt(p1));
          wm_k += -s_k[k] * r_k[k] * z * E_M / (PI * p2_m * sqrt(p1_m));

          v_k  += s_k[k] * r_k[k] / (2 * PI * r * sqrt(p1))   * (K_M + (sqr(r) - sqr(r_k[k]) - sqr(z)) / p2   * E_M);
          vm_k += s_k[k] * r_k[k] / (2 * PI * r * sqrt(p1_m)) * (K_M + (sqr(r) - sqr(r_k[k]) - sqr(z)) / p2_m * E_M);
      }

      float3 normalFlow   = -w_k  * fanNormal;
      float3 normalFlow_m = -wm_k * fanNormal_m;
      
      float3 radialFlow   = v_k  * radial;
      float3 radialFlow_m = vm_k * radial_m;
      
      float3 flow = normalFlow + normalFlow_m + radialFlow + radialFlow_m;
      
      flow = clamp(flow, -0.0275, 0.0275);

      if (i == 0) {
        Out.vFlow0.xz = flow.yx ;
        Out.vFlow0.z = -Out.vFlow0.z;

        Out.vFlow0.xz = Out.vFlow0.xz;// * 0.8 + 0.2 * sin(i + g_fTime * g_fAngleSpeed - fDist * 100) * Out.vFlow0.xz;
      } else if (i == 1) {
        Out.vFlow1.xz = flow.yx ;
        Out.vFlow1.z = -Out.vFlow1.z;

        Out.vFlow1.xz = Out.vFlow1.xz;// * 0.8 + 0.2 * sin(i + g_fTime * g_fAngleSpeed - fDist * 100) * Out.vFlow1.xz;
      } else if (i == 2) {
        Out.vFlow2.xz = flow.yx ;
        Out.vFlow2.z = -Out.vFlow2.z;
        // TODO: make more realistic
        Out.vFlow2.xz = Out.vFlow2.xz;// * 0.8 + 0.2 * sin(i + g_fTime * g_fAngleSpeed - fDist * 100) * Out.vFlow2.xz;
      }
   }
   return Out;
}


technique10 Render
{
    pass AxesFanFlowPass
    {
        SetVertexShader(CompileShader(vs_4_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PSRingSourcePotentialFlowModel()));
    }
}
