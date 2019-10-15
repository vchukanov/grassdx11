#pragma once

#include <xmmintrin.h>
#include <d3d10.h>
#include <d3dx10.h>
#include "fixedvector.h"
#include "aabb.h"

__declspec(align(16))
struct FPlane
{
	float X, Y, Z, W;
	FPlane() : X(0), Y(0), Z(0), W(0) {}
	FPlane( float x, float y, float z, float w ) : X(x), Y(y), Z(z), W(w)
	{

	}
};

__forceinline float invSqrt( float F )
{

	static const __m128 fThree = _mm_set_ss( 3.0f );
	static const __m128 fOneHalf = _mm_set_ss( 0.5f );
	__m128 Y0, X0, Temp;
	float temp;

	Y0 = _mm_set_ss( F );
	X0 = _mm_rsqrt_ss( Y0 );	// 1/sqrt estimate (12 bits)

	// Newton-Raphson iteration (X1 = 0.5*X0*(3-(Y*X0)*X0))
	Temp = _mm_mul_ss( _mm_mul_ss(Y0, X0), X0 );	// (Y*X0)*X0
	Temp = _mm_sub_ss( fThree, Temp );				// (3-(Y*X0)*X0)
	Temp = _mm_mul_ss( X0, Temp );					// X0*(3-(Y*X0)*X0)
	Temp = _mm_mul_ss( fOneHalf, Temp );			// 0.5*X0*(3-(Y*X0)*X0)
	_mm_store_ss( &temp, Temp );

	return temp;
} 

__declspec(align(16))
class ConvexVolume
{
	enum
	{
		PLANE_NEAR = 0,
		PLANE_LEFT,
		PLANE_RIGHT,
		PLANE_TOP,
		PLANE_BOTTOM,
		PLANE_FAR,
		PLANE_COUNT,
	};

	__declspec(align(16)) FixedVector<FPlane, 12> planes;
	__declspec(align(16)) FixedVector<FPlane, 12> permutedPlanes;
	//__declspec(align(16)) D3DXMATRIXA16 viewProjection;

	void BuildPlanes           (const D3DXMATRIXA16& ViewProjectionMatrix);
	void BuildNearAndFarPlanes ( const D3DXMATRIXA16& ViewProjectionMatrix );
	void BuildPermutedPlanes   ( );

public:
    void BuildFrustum          ( const D3DXMATRIXA16& viewProjectionMatrix );
	bool IntersectBox          ( const AABB &aabb ) const;
};