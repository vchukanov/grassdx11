#pragma once
#include "includes.h"

struct ParticleType
{
	XMFLOAT3 initialPos;
	XMFLOAT3 curPos;
	float age;
	float offset;
};

struct InstanceType
{
	XMFLOAT3 position;
	XMFLOAT4 color;
};