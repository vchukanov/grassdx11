BlendState AlphaBlendState
{
    AlphaToCoverageEnable    = TRUE;
    RenderTargetWriteMask[0] = 0x0F;
};

BlendState Alpha2CovBlendState
{
    AlphaToCoverageEnable    = TRUE;
    RenderTargetWriteMask[0] = 0x0F;
};

BlendState NonAlphaState
{
    AlphaToCoverageEnable    = FALSE;
    RenderTargetWriteMask[0] = 0x0F;
};

RasterizerState EnableMSAA
{
    CullMode          = NONE;
    //FillMode          = WIREFRAME;
    MultisampleEnable = TRUE;
};

RasterizerState EnableMSAACulling
{
    CullMode          = BACK;
    //FillMode          = WIREFRAME;
    MultisampleEnable = TRUE;
};

DepthStencilState EnableDepthTestWrite
{
    DepthEnable    = TRUE;
    DepthWriteMask = ALL;
};