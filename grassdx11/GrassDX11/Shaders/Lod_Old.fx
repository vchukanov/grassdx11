#ifndef _LOD_H_
#define _LOD_H_

float g_fDissolve = 0.15;

inline float4 CalcTerrNormal(float2 a_vTexCoord)
{
    return float4(g_txHeightMap.SampleLevel(g_samLinear, a_vTexCoord, 0).rgb * 2.0 - 1.0, 0.0);
}

inline float LodAlphaOffset(float4 a_vWorldPt)
{
	float3 vViewPos     = mul(a_vWorldPt, g_mView).xyz;
    float fBladeDist        = length(vViewPos);
	if (fBladeDist<0.1) return 0.0;
//	else return 1.0;

    float fLerpCoef1 = (fBladeDist+0.1) /g_fGrassRadius;
    fLerpCoef1 *= fLerpCoef1;
   
    float2 vTexCoord    = ((a_vWorldPt.xz / g_fTerrRadius) * 0.5 + 0.5 );
    float3 vNormal = CalcTerrNormal(vTexCoord).xyz;
	float3 vV = g_mInvCamView[3].xyz - a_vWorldPt.xyz; //vViewPos/fBladeDist;
	float tmp = 0.43; //= 0.44 + 0.1 * vV.y/(5.0 + abs(vV.y));
	if (vV.y<0.0) tmp += 0.1 * vV.y/(5.0 + abs(vV.y));
	vV = normalize(vV);
	float fdot = dot(vV, vNormal);
	tmp = tmp - fdot;
	if (fdot<0) fdot*=-0.4;
	float t = 1.f - fdot;
	if ((a_vWorldPt.y > 6.0)&&(t > 0.92)) return 0.2;
	else return tmp *(1.0+3.0*fLerpCoef1);
}

/*
 * returns: x - Alpha, y - segwidth, z - texIndex, w - right x-TexCoord
 */
inline float4 CalcTransparency( float a_fBaseAlpha, float4 a_vFirstPt, out float a_fDissolve, out uint a_uNumVertices )
{
/*
	float2 vUV = (a_vFirstPt.xz / g_fTerrRadius) * 0.5 + 0.5;
	if (GetSeatingInfo(vUV) < 0.1)
		return 0.0;
*/
    float fDist = length(a_vFirstPt.xyz - g_mInvCamView[3].xyz);
	a_uNumVertices = 7;
    if (fDist > 25.0)
       a_uNumVertices = 4;
	if (fDist > 30.0)
	   a_uNumVertices = 3;
    if (fDist > 40.0)
       a_uNumVertices = 2;

    float fAlphaOffset = LodAlphaOffset(a_vFirstPt);
    float fAlpha = a_fBaseAlpha - fAlphaOffset;
    a_fDissolve = 1.0;
//    if ((fAlpha > 0.0)&&(fAlpha < g_fDissolve))
    if (fAlpha < g_fDissolve)
    {
		a_fDissolve = fAlpha / g_fDissolve;		
    }
//      if (fDist > 40.0)
//		 return float4(fAlpha-0.33, 12.0, 1, 1.0);
//	  else
	     return float4(fAlpha, 1.0, 0.0, 1.0);
}

inline float3 CalcWind( float3 a_vPos )
{
    float2 vTexCoord = ((a_vPos.xz / g_fTerrRadius) * 0.5 + 0.5 )  * g_fWindTexTile;
	float3 vValue = g_txWindTex.SampleLevel(g_samLinear, vTexCoord, 0).rgb;// - 1.0).xyz; 
    float3 vWind = (vValue) * g_fWindStrength;
//    float3 vWind = float3(0.0, 0.0, 0.0);

    return vWind;
}

#endif

