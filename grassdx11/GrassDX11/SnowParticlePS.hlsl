Texture2D shaderTexture : register(t0);
SamplerState sampleType : register(s0);

RasterizerOrderedTexture2D<float4> rovTexture : register(u1);

struct PSIn
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR;
};

float4 PS_main(PSIn input) : SV_TARGET
{
	float4 textureColor;
	float4 finalColor;

	textureColor = shaderTexture.Sample(sampleType, input.tex);
	finalColor = textureColor * input.color;

	return finalColor;
}