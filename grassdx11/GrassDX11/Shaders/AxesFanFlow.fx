cbuffer cAxesFanSettings
{
    float3 g_vAxesFanPosOnTex;  // point: center of fan
    float3 g_vDir; // normal 
    int    g_uRingNumber;
    float  g_fTime;

    float  g_fMaxFlowStrength; // unused

    float  g_fFanRadius; 
    float  g_fDeltaSlices;
    float  g_fShift;

    float  g_fAngleSpeed; // 0.f .. 100.f

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


float getAngle (float2 radial_, float angle)
{
   float2 radial;
   radial.x = radial_.x * cos(angle) - radial_.y * sin(angle);
   radial.y = radial_.x * sin(angle) + radial_.y * cos(angle);

    float arccos = acos(radial.x);
    float crossv1 = dot(cross(float3(radial.x, radial.y, 0), float3(1, 0, 0)), float3(0, 0, 1));
    if (crossv1 >= 0) {
        return acos(radial.x);
    } 
    arccos = acos(-radial.x);
    return PI + arccos; 
}


float random (float2 _st) 
{
    return frac(sin(dot(_st.xy,
                             float2(12.9898,78.233)))*
        43758.5453123);
}


float noise (float2 _st) 
{
    float2 i = floor(_st);
    float2 f = frac(_st);

    float a = random(i);
    float b = random(i + float2(1.0, 0.0));
    float c = random(i + float2(0.0, 1.0));
    float d = random(i + float2(1.0, 1.0));

    float2 u = f * f * (3.0 - 2.0 * f);

    return lerp(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}


float fbm (float2 st) 
{
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;

    for (int i = 0; i < 5; i++) {
        value += amplitude * noise(st);
        st *= 2.;
        amplitude *= .5;
    }
    return value;
}


float GetLandscapeDamping (float3 queryPoint, float3 fanCenter)
{
    float dampAccum = 1;

    float3 dir = fanCenter - queryPoint;
    float3 normalizedDir = normalize(dir);
    float stepLen = 1.0 / 256.0; //HeightMapDimention
    float stepsCount = length(dir) / stepLen;
    float3 step = normalizedDir * stepLen;
    float3 landPoint = float3(0, 0, 0);
    float4 vHeightData = float4(0, 0, 0, 0);
    float fY = 0;
    float3 landNormal = float3(0, 0, 0);
  
    [loop]
    for (int i = 0; i < stepsCount; i++) {
        landPoint = queryPoint + i * step;
        vHeightData = g_txHeightMap.Sample(g_samLinear, landPoint.xy * 0.5 + 0.5);
        fY = vHeightData.a * g_fHeightScale;
        landNormal = vHeightData.xyz;
            
        if (fY > landPoint.z) {
            dampAccum += (fY - landPoint.z) * 10;
        }
    }

    return dampAccum;
}

float4 PSRingSourcePotentialFlowModel( AxesFanFlowPSIn In ) : SV_Target
{
   float4 Out = float4(0, 0, 0, 1);
   
    if (g_fFanRadius == 0 || g_fAngleSpeed == 0) {
        return Out;
    }

   float fY = g_txHeightMap.SampleLevel(g_samLinear, (In.vsPos) * 0.5 + 0.5, 0).a * g_fHeightScale; 
   float3 fNoise = g_txNoise.Sample(g_samLinear, In.vsPos).rgb;
   
   float3 fanNormal = normalize(g_vDir.xzy);
   float3 fanPoint = g_vAxesFanPosOnTex.xzy;
   fanNormal.y = -fanNormal.y;
   
   float3 fanNormal_m = normalize(getReflectedVec(fanNormal));
   float3 fanPoint_m = getReflectedVec(fanPoint);
   float  z = fY + 0.015;
   
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
   float  R     = g_fFanRadius / 2;
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
   float angle = acos(dot(radial, float3(0, 1, 0)));
   
   int k;
   
   s_max = 6 * N * R / (2 * N * N + 1);
   s_max *= sqrt(g_fAngleSpeed / 100);
   
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
   
   
   float randRadialMagn = fbm(((g_fTime + 20145) / (1 /*+ 1 / g_fAngleSpeed*/)) + radial.xy * 10) - 0.1;
   float randDistMagn = fbm(((g_fTime + 20145) / (1 /*+ 1 / g_fAngleSpeed*/)) - float2(fDist, fDist) * 10) - 0.1;
   randRadialMagn = lerp(1, 2, randRadialMagn);
   randDistMagn = lerp(1, 2, randDistMagn);
   float randMagn = (randRadialMagn + randDistMagn) / 2;
   
   float randRadialMagn1 = fbm(((g_fTime + 1015) / (1 /*+ 1 / g_fAngleSpeed*/)) + radial.xy * 10)  - 0.1;
   float randDistMagn1 = fbm(((g_fTime + 1015) / (1 /*+ 1 / g_fAngleSpeed*/)) - float2(fDist, fDist) * 10) - 0.1;
   //randRadialMagn1 = lerp(1, g_fMaxFlowStrength, randRadialMagn1); 
   //randDistMagn1 = lerp(1, g_fMaxFlowStrength, randDistMagn1);
   float randMagn1 = (randRadialMagn1 + randDistMagn1) / 2;
   
   //float randDist = fbm((g_fTime / (1 + 1 / g_fAngleSpeed)) - float2(fDist, fDist) * 10);
   //randDist = lerp(0.5, 1, randDist);
   float arg = (g_fTime / 1);

   float randDCompMagn = fbm(arg - float2(fDist, fDist) * 10) - 0.476;
   float randRCompMagn = fbm(arg + radial.xy * 10) - 0.476;
   float randCompMagn = (randDCompMagn + randRCompMagn) / 2;
   
   float randDCompMagn1 = fbm(arg - float2(fDist, fDist) * 10) - 0.476;
   float randRCompMagn1 = fbm(arg + radial.xy * 10) - 0.476;
   float randCompMagn1 = (randDCompMagn1 + randRCompMagn1) / 2;
   
   float3 normalFlow   = -w_k  * fanNormal;
   float3 normalFlow_m = -wm_k * fanNormal_m;
   
   float3 radialFlow   = v_k  * radial;
   float3 radialFlow_m = vm_k * radial_m;
   
   float3 randComp = normalize(cross(normalFlow, radialFlow));
   randComp *= sqrt(length(normalFlow) * length(radialFlow));
   randComp *= 4;
   float3 randComp_m = normalize(cross(normalFlow_m, radialFlow_m));
   randComp_m *= sqrt(length(normalFlow) * length(radialFlow));
   randComp *= 4;
   
   float3 staticFlow = normalFlow * randMagn + normalFlow_m * randMagn
      + radialFlow * randMagn1 + radialFlow_m * randMagn1
      + randComp * randCompMagn + randComp_m * randCompMagn1;
   
   float3 flow = staticFlow;
   
   flow = clamp(flow, -0.0275, 0.0275);
   flow /= GetLandscapeDamping(queryPoint, fanPoint);
   
   
   Out.xz = flow.yx;
   Out.z = -Out.z;
   
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
