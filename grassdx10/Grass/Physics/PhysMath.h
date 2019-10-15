#pragma once

/**
*/
#include <d3dx10.h>

/**
*/
#define PI       3.14159265358979323846
//#define EPSILON  0.000001f
#define EPSILON  1e-9

#define float2   D3DXVECTOR2
#define float3   D3DXVECTOR3
#define float4   D3DXVECTOR4
#define float3x3 D3DXMATRIX

float3 mul(const float3 &n, const float3 &m);
float3 mul(const float3 &v, const float3x3 &m);
float4 mul(const float4 &v, const float3x3 &m);
float3 mul(const float3x3 &m, const float3 &v);
float4 mul(const float3x3 &m, const float4 &v);
float3x3 mul(const float3x3 &n, const float3x3 &m);
float3x3 transpose(const float3x3 &m);
float3x3 inverse(const float3x3 &m);
float3x3 identity();
float dot(const float3 &a, const float3 &b);
float3 cross(const float3 &a, const float3 &b);
float length(const float3 &v);
float3 normalize(const float3 &v);
float clamp(float v, float a, float b);
float3 oneOver(const float3 &v);

float3x3 MakeRotationMatrix(const float3 &axis);
float3 MakeRotationVector(const float3x3 &m);
float3x3 MakeIdentityMatrix();