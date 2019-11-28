struct InstVSIn
{
    float3 vPos                   : POSITION;
    float3 vRotAxe                : TEXCOORD0;
    float3 vYRotAxe               : TEXCOORD1;
    float3 vColor                 : TEXCOORD2;
    float  fTransparency          : TRANSPARENCY;
    row_major float4x4 mTransform : mTransform;
    //uint   uOnEdge				  : uOnEdge;
};

struct PhysVSIn
{
    row_major float4x4 mR0        : R0MTX;
    row_major float4x4 mR1        : R1MTX;
    row_major float4x4 mR2        : R2MTX;
    float3 vPos                   : POSITION;
    float fTransparency           : TRANSPARENCY;
    float3 vColor                 : BLADECOLOR;
};
