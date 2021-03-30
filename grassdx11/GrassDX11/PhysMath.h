#pragma once

#include <DirectXMath.h>

using namespace DirectX;

#define PI       3.14159265358979323846
#define EPSILON  1e-9

#define float2   XMVECTOR
#define float3   XMVECTOR
#define float4   XMVECTOR
#define float3x3 XMMATRIX
#define float4x4 XMMATRIX

float2   create    (float x, float y);
float3   create    (float x, float y, float z);
float4   create    (float x, float y, float z, float w);

float    getx      (const float4& v);
float    gety      (const float4& v);
float    getz      (const float4& v);
float    getw      (const float4& v);

float    getcoord  (const float4& v, int idx);

void     setx      (float4& v, float x);
void     sety      (float4& v, float y);
void     setz      (float4& v, float z);
void     setw      (float4& v, float w);
void     setcoord  (float4& v, int idx, float val);

float3   scale     (const float s, const float3& m);
float3   mul       (const float3 &n, const float3 &m);
float3   mul       (const float3 &v, const float3x3 &m);
//float4   mul       (const float4 &v, const float3x3 &m);
float3   mul       (const float3x3 &m, const float3 &v);
//float4   mul       (const float3x3 &m, const float4 &v);
float3x3 mul       (const float3x3 &n, const float3x3 &m);
float3x3 transpose (const float3x3 &m);
float3x3 inverse   (const float3x3 &m);
float3x3 identity  (void);
float    dot       (const float3 &a, const float3 &b);
float3   cross     (const float3 &a, const float3 &b);
float    length    (const float3 &v);
float3   normalize (const float3 &v);
float    clamp     (float v, float a, float b);
int     iclamp(int v, int a, int b);
float3   oneOver   (const float3 &v);

float3x3 MakeRotationMatrix (const float3 &axis);
float3   MakeRotationVector (const float3x3 &m);
float3x3 MakeIdentityMatrix (void);