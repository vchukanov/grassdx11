matrix World;
matrix View;
matrix Projection;

struct VS_OUTPUT
{
  float4 Pos: POSITION;
};

typedef VS_OUTPUT PS_INPUT;

//
// Vertex Shader
//
VS_OUTPUT VS( float4 Pos : POSITION )
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    //output.Pos = mul(Pos, World);
    //output.Pos = mul(output.Pos, View);
    //output.Pos = mul(output.Pos, Projection);
    output.Pos = Pos;
    
    return output;
}

float4 PS( PS_INPUT In ): SV_Target
{
  return float4(1, 0, 0, 1);
}

technique10 Render
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, PS()));
    }
}
