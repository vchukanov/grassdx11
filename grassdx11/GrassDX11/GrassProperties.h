#pragma once

#include <fstream>
#include "includes.h"
#include "PhysMath.h"

enum GrassPropsType
{
	GRASSPROPSTYPE1,
	GRASSPROPSTYPE2,
	GRASSPROPSTYPE3,
};

/* structures MUST be 16-byte aligned and have all variables aligned by 16 (4xfloat size) */
__declspec(align(16))
struct GrassProps1
{
	float3 vHardnessSegment;
	float    fPad01;
	float3 vMassSegment;
	float    fPad1;
	float2 vSizes;

	//
	float2 vPad2;
	float4 vColor;
	UINT     uTexIndex;
	float3 vPad3;

	void Read (std::ifstream& a_ifIn);

	GrassPropsType GetType (void) { return GRASSPROPSTYPE1; }
};

typedef GrassProps1 GrassProps2;
__declspec(align(16))
struct GrassProps3
{
	float3 vHardnessSegment;
	float  fPad01;
	float3 vMassSegment;
	float  fPad1;
	float2 vSizes;

	//
	UINT   uTexIndex;
	UINT   uTopTexIndex;

	void Read (std::ifstream& a_ifIn);

	GrassPropsType GetType (void) { return GRASSPROPSTYPE3; }
};

/* united grass properties */
struct GrassPropsUnified
{
	float3 vHardnessSegment;
	float  fPad01;
	float3 vMassSegment;
	float  fPad1;
	float2 vSizes;
};

/* class for loading properties */
template <typename TProperties >
class GrassProperties
{
private:
	TProperties* m_pProperties;
	UINT         m_uNumProps;

public:
	GrassProperties(std::wstring a_sFileName)
	{
		std::ifstream ifStr;
		ifStr.open(a_sFileName.c_str());
		ifStr >> m_uNumProps;
		m_pProperties = new TProperties[m_uNumProps];
		for (UINT i = 0; i < m_uNumProps; i++)
		{
			m_pProperties[i].Read(ifStr);
		}
		ifStr.close();
	}

	~GrassProperties()
	{
		delete[] m_pProperties;
	}

	TProperties* GetDataPtr()
	{
		return m_pProperties;
	}

	GrassPropsUnified GetProperty(UINT a_uIndex)
	{
		if (a_uIndex > m_uNumProps)
			a_uIndex = 0;
		GrassPropsUnified Res;
		Res = *((GrassPropsUnified*)(&m_pProperties[a_uIndex]));

		return Res;
	}

	UINT         GetDataNum()
	{
		return m_uNumProps;
	}
};

typedef GrassProperties< GrassProps1 > GrassPropertiesT1;
typedef GrassProperties< GrassProps2 > GrassPropertiesT2;
typedef GrassProperties< GrassProps3 > GrassPropertiesT3;
