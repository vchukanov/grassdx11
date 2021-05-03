/*=====================================================
					COMPUTE SHADER
			Particle motion calculations
/=====================================================*/

//cbuffer PER_FRAME_DELTA : register(b2)
//{
//	float4 deltaAndPadding;
//}
#include "Noises.hlsli"
cbuffer TornadoBuffer : register(b0)
{
	float3 tornadoPos;
	bool tornadoActive;
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
	///==========================================================
	//float  timeStepFactor = 0.03f; // LAPTOP
	//float  timeStepFactor	= 0.003f;
	//float  timeStep = deltaAndPadding.x * timeStepFactor;
	//float  acceleration = -9.82f;
	//float  verticalLimit = -140.0f;
	//float3 worldOrigo = float3(0.0f, 0.0f, 0.0f);
	///==========================================================

	// Calculate new position
	//uint index = groupID.x * 1024 + groupID.y * 16 * 1024 + groupIndex;
	uint index = dispatchThreadID.x;
	float4 color;
	bool inTornado;
	float x, y, z, yAmplitude = -0.5f, yVelocity = -5.5f;
	float age = inputConstantParticleData[index].age;
	float offset = inputConstantParticleData[index].offset;
	float3 initialPos = inputConstantParticleData[index].initPos;
	float3 curPos = inputConstantParticleData[index].curPos;

	//x = yAmplitude * sin(age * 1.f * offset);
	//x += yAmplitude * sin(age * 0.5f * offset);
	//x += initialPos.x;

	//z = yAmplitude * sin(age * 0.66f * offset);
	//z += yAmplitude * sin(age * 0.4f * offset);
	//z += initialPos.z;

	if (tornadoActive && IsInTornado(GetRadiusOnHeight(curPos.y), curPos))
	{
		inTornado = true;
		yVelocity = -0.25f;
		//Движение по касательным
		/*float k2 = (tornadoPos.x - curPos.x) / (curPos.z - tornadoPos.z);
		float b2 = curPos.z - k2 * curPos.x;

		float _a, _b, _c;
		_a = (1 + k2 * k2);
		_b = (k2 * b2 - curPos.z * k2 - curPos.x);
		_c = (curPos.x * curPos.x + curPos.z * curPos.z - 2 * curPos.z * b2 + b2 * b2 - 1);
		float D = _b * _b - _a * _c;
		if (tornadoPos.z > curPos.z)
			x = (-_b + sqrt(D)) / _a;
		else
			x = (-_b - sqrt(D)) / _a;
		z = k2 * x + b2;*/
		
		float step = 1.5f;
		float radius = sqrt((curPos.z - tornadoPos.z) * (curPos.z - tornadoPos.z) + (curPos.x - tornadoPos.x) * (curPos.x - tornadoPos.x));
		radius = radius + 0.075f * radius * snoise(float4(curPos, age));
		float alpha1 = atan((curPos.z - tornadoPos.z) / (curPos.x - tornadoPos.x));
		if (curPos.x < tornadoPos.x)
			alpha1 = alpha1 + PI;
		float dalpha = acos(1 - step * step / 2 / radius / radius);
		float alpha2 = alpha1 + dalpha;

		x = tornadoPos.x + radius * cos(alpha2);
		z = tornadoPos.z + radius * sin(alpha2);
		//y = curPos.y;
		y = yAmplitude * sin(age * 0.5f * offset);
		y += yAmplitude * sin(age * 0.66f * offset);
		y += age * yVelocity + initialPos.y;
		/*if (y < 0)
			y = 0.1f;*/

		color = float4(0.6f, 0.6f, 0.6f, 1);
	}
	else {
		inTornado = false;

		float angle = turbulence(float4(curPos.x / 50, curPos.z / 50, curPos.y, age), 2) * PI * 2;
		float length = turbulence(float4(curPos.x / 10 + 4000, curPos.z / 10 + 4000, curPos.y, age), 1);
		length = length * 2.f;

		x = curPos.x + length * cos(angle);
		z = curPos.z + length * sin(angle);

		y = yAmplitude * sin(age * 0.5f * offset);
		y += yAmplitude * sin(age * 0.66f * offset);
		y += age * yVelocity + initialPos.y;
		color = float4(1, 1, 1, 1);
		//color = float4(0, 0, 0, 0);
	}
	
	outputParticleData[index].color = color;
	outputParticleData[index].position = float3(x, y, z);
	outputParticleData[index].inTornado = inTornado;
}