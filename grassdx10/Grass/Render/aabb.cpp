#include <float.h>
#include "aabb.h"

void AABB::Set( float minX, float maxX, float minY, float maxY, float minZ, float maxZ )
{
	center.x = ( maxX + minX ) * 0.5f;
	center.y = ( maxY + minY ) * 0.5f;
	center.z = ( maxZ + minZ ) * 0.5f;
	halfSize.x = ( maxX - minX ) * 0.5f;
	halfSize.y = ( maxY - minY ) * 0.5f;
	halfSize.z = ( maxZ - minZ ) * 0.5f;
}

void AABB::Set( const CVec3 &vmin, const CVec3 &vmax )
{
	Set( vmin.x, vmax.x, vmin.y, vmax.y, vmin.z, vmax.z );
}

void AABB::Calculate( int numPoints, const CVec3 *pPoints, int stride	)
{
	CVec3 minV( pPoints[0] ); 
	CVec3 maxV( pPoints[0] ); 

	pPoints = (const CVec3*)( ((const char*)pPoints) + stride );
	for (int i = 1; i < numPoints; ++i)
	{
		CVec3 const &pnt = pPoints[0];
		pPoints = (const CVec3*)( ((const char*)pPoints) + stride );

		if (pnt.x < minV.x)
			minV.x = pnt.x;
		else if (pnt.x > maxV.x)
			maxV.x = pnt.x;

		if (pnt.y < minV.y)
			minV.y = pnt.y;
		else if (pnt.y > maxV.y)
			maxV.y = pnt.y;

		if (pnt.z < minV.z)
			minV.z = pnt.z;
		else if (pnt.z > maxV.z)
			maxV.z = pnt.z;
	}

	center   = (minV + maxV) * 0.5f;
	halfSize = (maxV - minV) * 0.5f;
}