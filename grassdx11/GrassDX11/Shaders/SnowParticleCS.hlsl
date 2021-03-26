/*=====================================================
					COMPUTE SHADER
			Particle motion calculations
/=====================================================*/

//cbuffer PER_FRAME_DELTA : register(b2)
//{
//	float4 deltaAndPadding;
//}
#include "Noises.hlsli"

struct ConstantParticleData
{
	float3 initPos;
	float3 curPos;
	float age;
	float offset;
};

struct ParticleData
{
	float3 position;
};

StructuredBuffer<ConstantParticleData>	inputConstantParticleData	: register(t0);
RWStructuredBuffer<ParticleData>		outputParticleData			: register(u0);

#define THREAD_GROUP_X 32
#define THREAD_GROUP_Y 32
#define THREAD_GROUP_TOTAL 1024

[numthreads(32, 32, 1)]
void CS_main(uint3 groupID : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
	///==========================================================
	//float  timeStepFactor = 0.03f; // LAPTOP
	//float  timeStepFactor	= 0.003f;
	//float  timeStep = deltaAndPadding.x * timeStepFactor;
	//float  acceleration = -9.82f;
	//float  verticalLimit = -140.0f;
	//float3 worldOrigo = float3(0.0f, 0.0f, 0.0f);
	///==========================================================

	// Calculate new position
	uint index = groupID.x * THREAD_GROUP_TOTAL + groupID.y * GroupDim * THREAD_GROUP_TOTAL + groupIndex;

	float x, y, z, age, offset, yAmplitude = -0.5f, yVelocity = -1.f;
	age = inputConstantParticleData[index].age;
	offset = inputConstantParticleData[index].offset;
	float3 initialPos = inputConstantParticleData[index].initPos;
	float3 curPos = inputConstantParticleData[index].curPos;

	float angle = turbulence(float4(curPos.x / 50, curPos.z / 50, curPos.y, age), 2) * PI * 2;
	float length = turbulence(float4(curPos.x / 10 + 4000, curPos.z / 10 + 4000, curPos.y, age), 1);
	length *= 0.2f;

	x = curPos.x + length * cos(angle);
	z = curPos.z + length * sin(angle);

	//x = yAmplitude * sin(age * 1.f * offset);
	//x += yAmplitude * sin(age * 0.5f * offset);
	//x += initialPos.x;

	//z = yAmplitude * sin(age * 0.66f * offset);
	//z += yAmplitude * sin(age * 0.4f * offset);
	//z += initialPos.z;

	y = yAmplitude * sin(age * 0.5f * offset);
	y += yAmplitude * sin(age * 0.66f * offset);
	y += age * yVelocity + initialPos.y;

	outputParticleData[index].position = float3(x, y, z);	
}