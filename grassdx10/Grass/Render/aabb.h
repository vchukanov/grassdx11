#pragma once
#include <d3d10.h>
#include <d3dx10.h>
#define CVec3 D3DXVECTOR3

__declspec(align(16))
struct AABB
{
	CVec3 center;
	float x; //align to 4 floats for SSE instructions
	CVec3 halfSize;
	float y;
	
	AABB () {}
    AABB (const CVec3 &_center, const CVec3 &_halfSize) : center(_center), halfSize(_halfSize) {}

	void Set( float minX, float maxX, float minY, float maxY, float minZ, float maxZ );

	void Set( const CVec3 &vmin, const CVec3 &vmax );

	void Calculate( int numPoints, const CVec3 *pPoints, int stride	);
};