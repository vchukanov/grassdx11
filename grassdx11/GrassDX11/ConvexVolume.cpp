#include "ConvexVolume.h"

#define SIGN_BIT (~(1 << 31))
static __m128 SSE_SIGN_MASK;
static float __mask;

#include "aabb.h"


static struct SRegister_hack
{
	SRegister_hack()
	{
		*(unsigned int*)(&__mask) = 0x7FFFFFFF;
		SSE_SIGN_MASK = _mm_set_ps(__mask, __mask, __mask, __mask);
	}
} _hack;

#define VectorAbs( Vec )				_mm_and_ps(Vec, SSE_SIGN_MASK)

namespace
{
	__forceinline void MakeFrustumPlane(float A, float B, float C, float D, FPlane& OutPlane)
	{
		const float epsilon = 0.00001f;
		const float	LengthSquared = A * A + B * B + C * C;
		//ASSERT(LengthSquared > epsilon*epsilon);
		const float	InvLength = invSqrt(LengthSquared);
		OutPlane = FPlane(-A * InvLength, -B * InvLength, -C * InvLength, D * InvLength);
	}

	__forceinline void GetFrustumNearPlane (FPlane& OutPlane, const XMMATRIX& Matr)
	{
		M_TO_XM(Matr, M);

		MakeFrustumPlane(
			M.m[0][2],
			M.m[1][2],
			M.m[2][2],
			M.m[3][2],
			OutPlane
		);
	}

	__forceinline void GetFrustumFarPlane (FPlane& OutPlane, const XMMATRIX& Matr)
	{
		M_TO_XM(Matr, M);

		MakeFrustumPlane(
			M.m[0][3] - M.m[0][2],
			M.m[1][3] - M.m[1][2],
			M.m[2][3] - M.m[2][2],
			M.m[3][3] - M.m[3][2],
			OutPlane
		);
	}

	__forceinline void GetFrustumLeftPlane (FPlane& OutPlane, const XMMATRIX& Matr)
	{
		M_TO_XM(Matr, M);

		MakeFrustumPlane(
			M.m[0][3] + M.m[0][0],
			M.m[1][3] + M.m[1][0],
			M.m[2][3] + M.m[2][0],
			M.m[3][3] + M.m[3][0],
			OutPlane
		);
	}

	__forceinline void GetFrustumRightPlane (FPlane& OutPlane, const XMMATRIX& Matr)
	{
		M_TO_XM(Matr, M);

		MakeFrustumPlane(
			M.m[0][3] - M.m[0][0],
			M.m[1][3] - M.m[1][0],
			M.m[2][3] - M.m[2][0],
			M.m[3][3] - M.m[3][0],
			OutPlane
		);
	}

	__forceinline void GetFrustumTopPlane (FPlane& OutPlane, const XMMATRIX& Matr)
	{
		M_TO_XM(Matr, M);

		MakeFrustumPlane(
			M.m[0][3] - M.m[0][1],
			M.m[1][3] - M.m[1][1],
			M.m[2][3] - M.m[2][1],
			M.m[3][3] - M.m[3][1],
			OutPlane
		);
	}

	__forceinline void GetFrustumBottomPlane (FPlane& OutPlane, const XMMATRIX& Matr)
	{
		M_TO_XM(Matr, M);

		MakeFrustumPlane(
			M.m[0][3] + M.m[0][1],
			M.m[1][3] + M.m[1][1],
			M.m[2][3] + M.m[2][1],
			M.m[3][3] + M.m[3][1],
			OutPlane
		);
	}
}


void ConvexVolume::BuildFrustum (const XMMATRIX& viewProjectionMatrix)
{
	BuildPlanes(viewProjectionMatrix);
	BuildPermutedPlanes();
}

void ConvexVolume::BuildNearAndFarPlanes (const XMMATRIX& ViewProjectionMatrix)
{
	FPlane	Temp;

	// Near clipping plane.
	GetFrustumNearPlane(Temp, ViewProjectionMatrix);
	planes.push_back(Temp);

	// Far clipping plane.
	GetFrustumFarPlane(Temp, ViewProjectionMatrix);
	planes.push_back(Temp);
}


void ConvexVolume::BuildPlanes (const float4x4& ViewProjectionMatrix)
{
	FPlane	Temp;

	planes.clear();

	// Left clipping plane.
	GetFrustumLeftPlane(Temp, ViewProjectionMatrix);
	planes.push_back(Temp);

	// Right clipping plane.
	GetFrustumRightPlane(Temp, ViewProjectionMatrix);
	planes.push_back(Temp);

	// Top clipping plane.
	GetFrustumTopPlane(Temp, ViewProjectionMatrix);
	planes.push_back(Temp);

	// Bottom clipping plane.
	GetFrustumBottomPlane(Temp, ViewProjectionMatrix);
	planes.push_back(Temp);

	/*Near & Far */
	//BuildNearAndFarPlanes(ViewProjectionMatrix);
}

void ConvexVolume::BuildPermutedPlanes (void)
{
	//ASSERT(planes.size());

	const int numToAdd = planes.size() / 4;
	const int numRemaining = planes.size() % 4;
	// Presize the array
	//permutedPlanes.reserve(numToAdd * 4 + (numRemaining ? 4 : 0));
	// For each set of four planes
	permutedPlanes.clear();
	for (int Count = 0, Offset = 0; Count < numToAdd; Count++, Offset += 4)
	{
		// Add them in SSE ready form
		permutedPlanes.push_back(FPlane(planes[Offset + 0].X, planes[Offset + 1].X, planes[Offset + 2].X, planes[Offset + 3].X));
		permutedPlanes.push_back(FPlane(planes[Offset + 0].Y, planes[Offset + 1].Y, planes[Offset + 2].Y, planes[Offset + 3].Y));
		permutedPlanes.push_back(FPlane(planes[Offset + 0].Z, planes[Offset + 1].Z, planes[Offset + 2].Z, planes[Offset + 3].Z));
		permutedPlanes.push_back(FPlane(planes[Offset + 0].W, planes[Offset + 1].W, planes[Offset + 2].W, planes[Offset + 3].W));
	}
	// Pad the last set so we have an even 4 planes of vert data
	if (numRemaining)
	{
		FPlane Last1, Last2, Last3, Last4;
		// Read the last set of verts
		switch (numRemaining)
		{
		case 3:
		{
			Last1 = planes[numToAdd * 4 + 0];
			Last2 = planes[numToAdd * 4 + 1];
			Last3 = planes[numToAdd * 4 + 2];
			Last4 = Last1;
			break;
		}
		case 2:
		{
			Last1 = planes[numToAdd * 4 + 0];
			Last2 = planes[numToAdd * 4 + 1];
			Last3 = Last4 = Last1;
			break;
		}
		case 1:
		{
			Last1 = planes[numToAdd * 4 + 0];
			Last2 = Last3 = Last4 = Last1;
			break;
		}
		default:
		{
			Last1 = FPlane();
			Last2 = Last3 = Last4 = Last1;
			break;
		}
		}
		// Add them in SIMD ready form
		permutedPlanes.push_back(FPlane(Last1.X, Last2.X, Last3.X, Last4.X));
		permutedPlanes.push_back(FPlane(Last1.Y, Last2.Y, Last3.Y, Last4.Y));
		permutedPlanes.push_back(FPlane(Last1.Z, Last2.Z, Last3.Z, Last4.Z));
		permutedPlanes.push_back(FPlane(Last1.W, Last2.W, Last3.W, Last4.W));
	}
}

bool ConvexVolume::IntersectBox (const AABB& aabb/*const CVec3& Origin, const CVec3& Extent*/) const
{
	int planeCount = permutedPlanes.size();
	//NI_ASSERT(0 < planeCount && planeCount <= 12, NStr::StrFmt("Invalid plane count (%i) for convex volume", planeCount));
	//ASSERT(planeCount % 4 == 0);
	// Load the origin & extent
	//ASSERT( (((UINT)(void*)(&aabb)) & 0xF) == 0);
	__m128 Orig = _mm_load_ps((const float*)& aabb.center);
	__m128 Ext = _mm_load_ps((const float*)& aabb.halfSize);
	// Splat origin into 3 vectors
	__m128 OrigX = _mm_shuffle_ps(Orig, Orig, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 OrigY = _mm_shuffle_ps(Orig, Orig, _MM_SHUFFLE(1, 1, 1, 1));
	__m128 OrigZ = _mm_shuffle_ps(Orig, Orig, _MM_SHUFFLE(2, 2, 2, 2));
	//// Splat extent into 3 vectors
	//__m128 ExtentX = _mm_shuffle_ps(Ext, Ext, _MM_SHUFFLE(0,0,0,0));
	//__m128 ExtentY = _mm_shuffle_ps(Ext, Ext, _MM_SHUFFLE(1,1,1,1));
	//__m128 ExtentZ = _mm_shuffle_ps(Ext, Ext, _MM_SHUFFLE(2,2,2,2));
	// Splat the abs for the pushout calculation
	//__m128 AbsExt = _mm_and_ps(Ext, SSE_SIGN_MASK);
	__m128 AbsExtentX = _mm_shuffle_ps(Ext, Ext, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 AbsExtentY = _mm_shuffle_ps(Ext, Ext, _MM_SHUFFLE(1, 1, 1, 1));
	__m128 AbsExtentZ = _mm_shuffle_ps(Ext, Ext, _MM_SHUFFLE(2, 2, 2, 2));
	// Since we are moving straight through get a pointer to the data
	const FPlane* __restrict PermutedPlanePtr = (const FPlane*)& permutedPlanes[0];
	// Process four planes at a time until we have < 4 left
	for (int Count = 0; Count < planeCount; Count += 4)
	{
		// Load 4 planes that are already all Xs, Ys, ...
		__m128 PlanesX = _mm_load_ps((const float*)PermutedPlanePtr);
		PermutedPlanePtr++;
		__m128 PlanesY = _mm_load_ps((const float*)PermutedPlanePtr);
		PermutedPlanePtr++;
		__m128 PlanesZ = _mm_load_ps((const float*)PermutedPlanePtr);
		PermutedPlanePtr++;
		__m128 PlanesW = _mm_load_ps((const float*)PermutedPlanePtr);
		PermutedPlanePtr++;
		// Calculate the distance (x * x) + (y * y) + (z * z) - w
		__m128 DistX = _mm_mul_ps(OrigX, PlanesX);
		__m128 DistY = _mm_add_ps(_mm_mul_ps(OrigY, PlanesY), DistX);
		__m128 DistZ = _mm_add_ps(_mm_mul_ps(OrigZ, PlanesZ), DistY);
		__m128 Distance = _mm_sub_ps(DistZ, PlanesW);
		// Now do the push out Abs(x * x) + Abs(y * y) + Abs(z * z)
		__m128 ggg = VectorAbs(PlanesX);
		__m128 PushX = _mm_mul_ps(AbsExtentX, ggg);
		__m128 PushY = _mm_add_ps(_mm_mul_ps(AbsExtentY, VectorAbs(PlanesY)), PushX);
		__m128 PushOut = _mm_add_ps(_mm_mul_ps(AbsExtentZ, VectorAbs(PlanesZ)), PushY);

		//// Check for completely outside
		if (_mm_movemask_ps(_mm_cmpgt_ps(Distance, PushOut)))
			return false;
	}
	return true;
}
