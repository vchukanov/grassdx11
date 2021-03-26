#pragma once
#include "includes.h"
struct ParticleConstantData
{
	XMFLOAT3 initialPos;
	XMFLOAT3 curPos;
	float age;
	float offset;
};

struct ParticleData
{
	XMFLOAT3 pos;
};

struct SystemInfoType
{
	UINT groupDim;
	UINT totalParticles;
	UINT to16;
	UINT byte;
};