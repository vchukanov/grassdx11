
struct VSIn
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR0;
	float3 instancePosition : TEXCOORD1;
	float4 instanceColor : COLOR1;
};

struct GSIn
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR;
};

GSIn VS_main(VSIn input)
{
	GSIn output;

	output.position = input.position + float4(input.instancePosition, 1.0f);
	output.color = input.instanceColor;
	output.tex = input.tex;

	return output;
}