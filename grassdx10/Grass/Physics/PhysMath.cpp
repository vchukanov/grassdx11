#include "PhysMath.h"

/**
*/
float3x3 transpose(const float3x3 &m)
{
	float3x3 res;
	D3DXMatrixTranspose(&res, &m);
	return res;
}

/**
*/
float3x3 inverse(const float3x3 &m)
{
	float3x3 res;
	D3DXMatrixInverse(&res, NULL, &m);
	return res;
}

/**
*/
float3x3 identity()
{
	float3x3 res;
	D3DXMatrixIdentity(&res);
	return res;
}

/**
*/
float3 mul(const float3 &n, const float3 &m)
{
	return float3(n.x*m.x, n.y*m.y, n.z*m.z);	
}

/**
*/
float3 mul(const float3 &v, const float3x3 &m)
{
	float4 res;
	D3DXVec3Transform(&res, &v, &m);
	return float3(res.x, res.y, res.z);
}

/**
*/
float4 mul(const float4 &v, const float3x3 &m)
{
	float4 res;
	D3DXVec4Transform(&res, &v, &m);
	return res;
}

/**
*/
float3 mul(const float3x3 &m, const float3 &v)
{
	float4 res;
	D3DXVec3Transform(&res, &v, &transpose(m));
	return float3(res.x, res.y, res.z);
}

/**
*/
float4 mul(const float3x3 &m, const float4 &v)
{
	float4 res;
	D3DXVec4Transform(&res, &v, &transpose(m));
	return res;
}

/**
*/
float3x3 mul(const float3x3 &n, const float3x3 &m)
{
	float3x3 res;
	D3DXMatrixMultiply(&res, &n, &m);
	return res;
}


/**
*/
float3 cross(const float3 &a, const float3 &b)
{
	float3 res;
	D3DXVec3Cross(&res, &a, &b);
	return res;
}

/**
*/
float dot(const float3 &a, const float3 &b)
{
	return D3DXVec3Dot(&a, &b);
}


/**
*/
float length(const float3 &v)
{
	return D3DXVec3Length(&v);
}

/**
*/
float3 normalize(const float3 &v)
{
	float3 res;
	D3DXVec3Normalize(&res, &v);
	return res;
}

/**
*/
float clamp(float v, float a, float b)
{
	if (v < a) v = a;
	if (v > b) v = b;
	return v;
}

/**
*/
float3 oneOver(const float3 &v)
{
    return float3(1.0f/v.x, 1.0f/v.y, 1.0f/v.z);

}
/**
*/
float3x3 MakeRotationMatrix(const float3 &axis)
{
	if (length(axis) < EPSILON)
		return identity();

	float angle = D3DXVec3Length(&axis);
    float3 nAxis = normalize(axis);
    
	float fCos = cos(angle);
    float l_fCos = 1 - fCos;
    float fSin = sin(angle);

    float x2 = nAxis.x * nAxis.x;
    float y2 = nAxis.y * nAxis.y;
    float z2 = nAxis.z * nAxis.z;

    float xy = nAxis.x * nAxis.y;
    float xz = nAxis.x * nAxis.z;    
    float yz = nAxis.y * nAxis.z;

    return float3x3(fCos + l_fCos * x2,             l_fCos * xy - fSin * nAxis.z, l_fCos * xz + fSin * nAxis.y, 0, 
				   l_fCos * xy + fSin * nAxis.z, fCos + l_fCos * y2,           l_fCos * yz - fSin * nAxis.x, 0, 
                   l_fCos * xz - fSin * nAxis.y, l_fCos * yz + fSin * nAxis.x, fCos + l_fCos * z2,           0,
                   0,						     0,							   0,						     1);

    //return res;
}

/**
*/
#define GET_3x3_ELEM(i, j) ((m)[(i)*4+(j)])
#define SQR(x) ((x)*(x))
/**
*/
float3 MakeRotationVector(const float3x3 &m)
{
	float3 v;
	float thetha;
	float3 sp = float3(GET_3x3_ELEM(2,1)-GET_3x3_ELEM(1,2),
					GET_3x3_ELEM(0,2)-GET_3x3_ELEM(2,0),
					GET_3x3_ELEM(1,0)-GET_3x3_ELEM(0,1));


	if (abs(sp.x) < EPSILON && abs(sp.y) < EPSILON && abs(sp.z) < EPSILON)
	{
		//identity matrix
		if (abs(GET_3x3_ELEM(1,0)+GET_3x3_ELEM(0,1)) < EPSILON &&
			abs(GET_3x3_ELEM(0,2)+GET_3x3_ELEM(2,0)) < EPSILON &&
			abs(GET_3x3_ELEM(1,2)+GET_3x3_ELEM(2,1)) < EPSILON)
			return float3(0.0f, 0.0f, 0.0f);
		else
		{
			thetha = (float)PI;
			float xx = (GET_3x3_ELEM(0, 0) + 1.0f) / 2.0f;
			float yy = (GET_3x3_ELEM(1, 1) + 1.0f) / 2.0f;
			float zz = (GET_3x3_ELEM(2, 2) + 1.0f) / 2.0f;

			float xy = (GET_3x3_ELEM(0, 1) + GET_3x3_ELEM(1, 0)) / 4.0f;
			float xz = (GET_3x3_ELEM(0, 2) + GET_3x3_ELEM(2, 0)) / 4.0f;
			float yz = (GET_3x3_ELEM(1, 2) + GET_3x3_ELEM(2, 1)) / 4.0f;

			if ((xx > yy) && (xx > zz))
			{
				if (xx < EPSILON)
				{
					v = float3(0.0f, 0.7071f, 0.7071f);
					return v * thetha;
				}
				else
				{
					v = float3(xx, xy, xz) / sqrtf(xx);
					return v * thetha;
				}
			}
			else if (yy > zz)
			{
				if (yy < EPSILON)
				{
					v = float3(0.7071f, 0.0f, 0.7071f);
					return v * thetha;
				}
				else
				{
					v = float3(yy, xy, yz) / sqrtf(yy);
					return v * thetha;
				}
			}
			else
			{
				if (zz < EPSILON)
				{
					v = float3(0.7071f, 0.7071f, 0.0f);
					return v * thetha;
				}
				else
				{
					v = float3(zz, xz, yz) / sqrtf(zz);
					return v * thetha;
				}
			}
		}
	}

	//non singular
	float spp = sqrtf(SQR(sp.x) + 
					SQR(sp.y) +
					SQR(sp.z));
	
	if (spp < EPSILON) 
		spp = 1.0f;

	v = sp / spp;
	float cosAngle = (GET_3x3_ELEM(0,0) + GET_3x3_ELEM(1,1) + GET_3x3_ELEM(2,2) - 1.0f) / 2.0f;
	
	if (cosAngle > 1.0f) cosAngle = 1.0f;
	if (cosAngle < -1.0) cosAngle = -1.0f;
    
	thetha = acos(cosAngle);

	return thetha * v;
}

/**
*/
float3x3 MakeIdentityMatrix()
{	
    float3x3 res(1, 0, 0, 0, 
        0, 1, 0, 0, 
        0, 0, 1, 0,
        0, 0, 0, 1
        );

    return res;
}