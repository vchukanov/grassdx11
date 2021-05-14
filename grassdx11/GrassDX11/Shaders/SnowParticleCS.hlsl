/*=====================================================
					COMPUTE SHADER
			Particle motion calculations
/=====================================================*/

//cbuffer PER_FRAME_DELTA : register(b2)
//{
//	float4 deltaAndPadding;
//}
#include "Noises.hlsli"

//SamplerState g_samLinear
//{
//	Filter = MIN_MAG_LINEAR_MIP_POINT;
//	AddressU = Wrap;
//	AddressV = Wrap;
//	MIPLODBIAS = -1.0;
//};

cbuffer TornadoBuffer : register(b0)
{
	float3 tornadoPos;
	bool tornadoActive;
	float3 copterPos;
	float dt;
};

struct ParticleType
{
	float3 initPos;
	float3 curPos;
	float age;
	float offset;
};

struct InstanceType
{
	float3 position;
	float4 color;
	bool inTornado;
};


StructuredBuffer<ParticleType>	inputConstantParticleData	: register(t0);
RWStructuredBuffer<InstanceType>		outputParticleData			: register(u0);
//Texture2D g_txAxesFanFlow : register(t1);

float GetRadiusOnHeight(float height) {
	//return 15.f;
	return 0.0015 * height * height + 5;
	//return height / 3.33333f;
};

bool IsInTornado(float radius, float3 particlePos) {
	return ((particlePos.x - tornadoPos.x) * (particlePos.x - tornadoPos.x) +
		(particlePos.z - tornadoPos.z) * (particlePos.z - tornadoPos.z) < radius * radius);
};


[numthreads(512, 1, 1)]
//void CS_main(uint3 groupID : SV_GroupID, uint groupIndex : SV_GroupIndex)
void CS_main(int3 dispatchThreadID : SV_DispatchThreadID)
{
	// Calculate new position
	//uint index = groupID.x * 1024 + groupID.y * 16 * 1024 + groupIndex;
	uint index = dispatchThreadID.x;
	float4 color;
	bool inTornado;
	float copterFieldRadius = 25.f;
	float x, y, z, yAmplitude = -0.5f, yVelocity = -10.5f;
	float age = inputConstantParticleData[index].age;
	float offset = inputConstantParticleData[index].offset;
	float3 initialPos = inputConstantParticleData[index].initPos;
	float3 curPos = inputConstantParticleData[index].curPos;

	float windScaleFactor = 60.f;

	//x = yAmplitude * sin(age * 1.f * offset);
	//x += yAmplitude * sin(age * 0.5f * offset);
	//x += initialPos.x;

	//z = yAmplitude * sin(age * 0.66f * offset);
	//z += yAmplitude * sin(age * 0.4f * offset);
	//z += initialPos.z;

	float3 vec;
	vec = curPos - copterPos;
	float len = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);

	float fanVx = 0.f, fanVy = 0.f, fanVz = 0.f;
	float fanScaleFactor = 100.f;
	if (len <= copterFieldRadius + copterFieldRadius * 0.1f * snoise(float4(curPos, age))) {
		fanVx = fanScaleFactor * vec.x / len;
		fanVy = fanScaleFactor * vec.y / len;
		fanVz = fanScaleFactor * vec.z / len;

		//vec = vec / 400.f; //normalize
	}
	if (tornadoActive && IsInTornado(GetRadiusOnHeight(curPos.y), curPos))
	{
		inTornado = true;
		yVelocity = -0.4f;		
		float radV = 65.f;

		float radius = sqrt((curPos.z - tornadoPos.z) * (curPos.z - tornadoPos.z) + (curPos.x - tornadoPos.x) * (curPos.x - tornadoPos.x) + 0.00000001f);
		radius = radius + 0.06f * radius * snoise(float4(curPos, age)); 
		float alpha1 = atan((curPos.z - tornadoPos.z) / (curPos.x - tornadoPos.x + 0.00000001f));
		if (curPos.x < tornadoPos.x)
			alpha1 = alpha1 + PI;
		float dalpha = acos(1 - radV * radV * dt * dt / 2 / radius / radius);
		float alpha2 = alpha1 + dalpha;

		x = tornadoPos.x + radius * cos(alpha2) + fanVx * dt;
		z = tornadoPos.z + radius * sin(alpha2) + fanVz * dt;
		//y = curPos.y;
		y = yAmplitude * sin(age * 0.5f * offset);
		y += yAmplitude * sin(age * 0.66f * offset);
		y += age * yVelocity + initialPos.y + fanVy * dt;
		/*if (y < 0)
			y = 0.1f;*/

		color = float4(0.6f, 0.6f, 0.6f, 1);
	}
	else {
		inTornado = false;

		float angle = turbulence(float4(curPos.x / 50, curPos.z / 50, curPos.y, age), 2) * PI * 2;
		float value = turbulence(float4(curPos.x / 10 + 4000, curPos.z / 10 + 4000, curPos.y, age), 1);
		value = value * windScaleFactor;

		x = curPos.x + (value * cos(angle) + fanVx) * dt;
		z = curPos.z + (value * sin(angle) + fanVz) * dt;


		y = yAmplitude * sin(age * 0.5f * offset);
		y += yAmplitude * sin(age * 0.66f * offset);
		y += age * yVelocity + initialPos.y + fanVy * dt;
		color = float4(1, 1, 1, 1);
		//color = float4(0, 0, 0, 0);
	}
	
	outputParticleData[index].color = color;
	outputParticleData[index].position = float3(x, y, z);
	outputParticleData[index].inTornado = inTornado;
}