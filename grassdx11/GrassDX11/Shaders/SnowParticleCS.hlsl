/*=====================================================
					COMPUTE SHADER
			Particle motion calculations
/=====================================================*/

//cbuffer PER_FRAME_DELTA : register(b2)
//{
//	float4 deltaAndPadding;
//}
#include "Noises.hlsli"

struct ParticleType
{
	float3 initPos;
	float3 initVel;
	float age;
	float offset;
};

struct InstanceType
{
	float3 position;
	float4 color;
};


StructuredBuffer<ParticleType>	inputConstantParticleData	: register(t0);
RWStructuredBuffer<InstanceType>		outputParticleData			: register(u0);


[numthreads(512, 1, 1)]
void CS_main(int3 dispatchThreadID : SV_DispatchThreadID)
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
	float x, y, z, age, offset, yAmplitude = -0.5f, yVelocity = -1.f;
	age = inputConstantParticleData[dispatchThreadID.x].age;
	offset = inputConstantParticleData[dispatchThreadID.x].offset;
	float3 initialPos = inputConstantParticleData[dispatchThreadID.x].initPos;
	float3 curPos = outputParticleData[dispatchThreadID.x].position;

	float angle = turbulence(float4(curPos.x / 50, curPos.z / 50, curPos.y, age), 2) * PI * 2;
	float length = turbulence(float4(curPos.x / 10 + 4000, curPos.z / 10 + 4000, curPos.y, age), 1);

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

	outputParticleData[dispatchThreadID.x].position = float3(x, y, z);	
	outputParticleData[dispatchThreadID.x].color = float4(1, 1, 1, 1);
}