cbuffer cAxesFanSettings
{
    float3 g_vAxesFanPosOnTex;  // point: center of fan
    float3 g_vDir; // normal 
    float  g_fR;
    int    g_uRingNumber;
    float  g_fTime;

    float  g_fMaxHorizFlow;
    float  g_fMaxVertFlow;
    float  g_fDampPower;
    float  g_fDistPower;
    float  g_fMaxFlowRadius;
    float  g_fShift;
};

Texture2D g_txNoise;

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

  
AxesFanFlowPSIn VS( AxesFanFlowVSIn In )
{
    AxesFanFlowPSIn Out;
	Out.vPos = float4(In.vPos, 1.0);
	Out.vTexCoord = In.vTexCoord;

    // make y axis parallel to texture coordinate v axis
    Out.vsPos = float2(In.vPos.x, -In.vPos.y);

    return Out;
}


float4 PS( AxesFanFlowPSIn In ): SV_Target
{
   float2 flowSrc = g_vAxesFanPosOnTex.xz;
   //float2 flowSrc = float2(0, 0);
   
   float2 vFlowDirection = normalize(In.vsPos - flowSrc);
   float fDist = length(In.vsPos - flowSrc); //0, 1
   float fMagnitude = g_fMaxHorizFlow / pow(pow(abs(fDist / g_fMaxFlowRadius), g_fDistPower) + g_fShift, g_fDampPower);

   float2 vFlow = vFlowDirection * (fMagnitude / 2 + abs(sin(g_fTime - fDist * 100) * cos(g_fTime * 0.8)) * fMagnitude / 2);
   
   return float4(vFlow.y, -g_fMaxVertFlow, -vFlow.x, 1);

   /*if (length(In.vsPos - g_vAxesFanPosOnTex.xz) < 0.1) {
       return float4(0, -0.04, 0, 1);
   }
   return float4(0, 0, 0, 1);*/
}


static const float PI = 3.14159265f;


// E(M)
float EvaluateSecondEllipticIntegralApproximation ( float x )
{
   return PI / 2 * ( 1 - pow((1 / 2), 2) * pow(x, 2) - pow((1 * 3 / (2 * 4)), 2) * pow(x, 4) / 3); 
}


// K(M)
float EvaluateFirstEllipticIntegralApproximation ( float x )
{
   return PI / 2 * ( 1 + pow((1 / 2), 2) * pow(x, 2) + pow((1 * 3 / (2 * 4)), 2) * pow(x, 4));
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


float4 PSRingSourcePotentialFlowModel( AxesFanFlowPSIn In ): SV_Target
{
   return float4(0, g_fDampPower / 100, 0, 1);

   float fNoise = g_txNoise.Sample(g_samLinear, In.vsPos).r;
   
   float3 fanNormal = normalize(g_vDir.xzy);
   float3 fanPoint = g_vAxesFanPosOnTex.xzy;

   float3 fanNormal_m = normalize(getReflectedVec(fanNormal));
   float3 fanPoint_m = getReflectedVec(fanPoint);
   float  z = g_fShift; // TODO: make 3 textures on different heights

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
   float  R     = g_fR;
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

   int k;

   float4 V = float4(0.0f, 0.0f, 0.0f, 1.0f); // result

   s_max = 6 * N * R / (2 * N * N + 1);
   s_max *= g_fMaxHorizFlow;

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

   float3 normalFlow   = w_k  * fanNormal;
   float3 normalFlow_m = wm_k * fanNormal_m;
   
   float3 radialFlow   = v_k  * radial;
   float3 radialFlow_m = vm_k * radial_m;
   
   float3 flow = normalFlow + normalFlow_m + radialFlow + radialFlow_m;
  
   V.xz = flow.yx ;
   V.z = -V.z;

   // TODO: make more realistic
   float fDist = length(In.vsPos - fanPoint.xy);
   V.xz = V.xz * 0.8 + 0.2 * sin(g_fTime * 12 + fNoise - fDist * 100) * V.xz;
   
   return V;
}


float4 PSRingSourcePotentialFlowModel1( AxesFanFlowPSIn In ): SV_Target
{
   //float fNoise = g_txNoise.Sample(g_samLinear, In.vsPos).r / 100;
   float2 fanCenter = g_vAxesFanPosOnTex.xz;
   int    N = g_uRingNumber; 
   float  R = g_fR;
   float  r_k[16]; 
   float  s_k[16];
   float  s_max = 0;
   float  M     = 0; 
   float  E_M   = 0;
   float  K_M   = 0;
   float  h = g_vAxesFanPosOnTex.y;
   float  r = length(In.vsPos - fanCenter);
   float2 vFlowDirection = normalize(In.vsPos - fanCenter);
   float  z = g_fShift; // TODO: make 3 textures on different heights

   float p1 = 0;
   float p2 = 0;

   float p1_m = 0;
   float p2_m = 0;

   float v_k, vm_k = 0; // radial velocity
   float w_k, wm_k = 0; // vertical velocity

   float4 V = float4(0.0f, 0.0f, 0.0f, 1.0f); // result
   
   int k;
   
   for (k = 0; k < N; k++) {
      r_k[k] = R - k * R / N;
   }

   s_max = 6 * N * R * g_fMaxHorizFlow / (2 * N * N + 1);

   for (k = 0; k < N; k++) {
      s_k[k] = s_max * r_k[k] / R;
   }

   for (k = 0; k < N; k++) {
       p1 = sqr((r + r_k[k])) + sqr(z);
       p2 = sqr((r - r_k[k])) + sqr(z);
       
       p1_m = sqr((r + r_k[k])) + sqr(2 * h - z);
       p2_m = sqr((r - r_k[k])) + sqr(2 * h - z);

       M = 4 * r * r_k[k] / p1;
       E_M = EvaluateSecondEllipticIntegralApproximation(M);
       K_M = EvaluateFirstEllipticIntegralApproximation(M);
       
       w_k  += -s_k[k] * r_k[k] * z * E_M / (PI * p2 * sqrt(p1));
       wm_k += -s_k[k] * r_k[k] * z * E_M / (PI * p2_m * sqrt(p1_m));
       v_k  += s_k[k] * r_k[k] / (2 * PI * r * sqrt(p1)) * (K_M + (sqr(r) - sqr(r_k[k]) - sqr(z)) / p2 * E_M);
       vm_k += s_k[k] * r_k[k] / (2 * PI * r * sqrt(p1_m)) * (K_M + (sqr(r) - sqr(r_k[k]) - sqr(z)) / p2_m * E_M);
   }

   v_k = v_k + vm_k;
   w_k = w_k + wm_k;
   
   V.xz = vFlowDirection.yx * abs(v_k);
   V.z = -V.z;
   //V.y = -w_k;

   return V;
}


technique10 Render
{
    pass AxesFanFlowPassSimple
    {
        SetVertexShader(CompileShader(vs_4_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, PS()));
    }

     pass AxesFanFlowPass
    {
        SetVertexShader(CompileShader(vs_4_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PSRingSourcePotentialFlowModel()));
    }
}
