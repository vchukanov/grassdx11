#include "Wind.h"

static float ClearColor[4] = {0.0f, 0.0f, 1.0f, 0.0f};

WindData::WindData( )
{
    pData = NULL;
    pWindMapData = NULL;
}

WindData::~WindData( )
{
    if (pData)
        delete [] pData;
    if (pWindMapData)
        delete [] pWindMapData;

}

D3DXVECTOR3 WindData::GetValue( const D3DXVECTOR2 &a_vTexCoord, const float a_fWindTexTile ) const
{
    //float fX = a_vTexCoord.x * (float)(uWidth - 1);
    //float fY = a_vTexCoord.y * (float)(uHeight - 1);
    float fX  = (a_vTexCoord.x * a_fWindTexTile);
    float fY  = (a_vTexCoord.y * a_fWindTexTile);
    
    /* bilinear interpolation... */
    
    float fFracX = fX - floor(fX);
    float fFracY = fY - floor(fY);
    UINT uLX = UINT(fX * (fWidth - 1.0f)) % uWidth;
    UINT uHX = uLX + 1;
    UINT uLY = UINT(fY * (fHeight - 1.0f)) % uHeight;
    UINT uHY = uLY + 1;
    if (uHX > uWidth - 1)
        uHX = uWidth - 1;
    if (uHY > uHeight - 1)
        uHY = uHeight - 1;
    D3DXVECTOR3 fLL = pData[uWidth * uLY + uLX];
    D3DXVECTOR3 fHL = pData[uWidth * uHY + uLX];
    D3DXVECTOR3 fLR = pData[uWidth * uLY + uHX];
    D3DXVECTOR3 fHR = pData[uWidth * uHY + uHX];
    //return fLL;
    return ( (1.0f - fFracX) * fLL + fFracX * fLR ) * (1.0f - fFracY) + 
        fFracY * ( (1.0f - fFracX) * fHL + fFracX * fHR );
}

D3DXVECTOR3 WindData::BiLinear( const D3DXVECTOR2 &a_vTexCoord )
{
    float fX  = (a_vTexCoord.x * (fWidth - 1.0f));
    float fY  = (a_vTexCoord.y * (fHeight - 1.0f));

    /* bilinear interpolation... */
    float fFloorX = floor(fX);
    float fFloorY = floor(fY);
    UINT uLX = ((UINT)fX) % uWidth;
    UINT uHX = uLX + 1;
    UINT uLY = ((UINT)fY) % uHeight;
    UINT uHY = uLY + 1;
    float fFracX = fX - fFloorX;
    float fFracY = fY - fFloorY;
    if (uHX > uWidth - 1)
        uHX = uWidth - 1;
    if (uHY > uHeight - 1)
        uHY = uHeight - 1;
    D3DXVECTOR3 fLL = pWindMapData[uWidth * uLY + uLX];
    D3DXVECTOR3 fHL = pWindMapData[uWidth * uHY + uLX];
    D3DXVECTOR3 fLR = pWindMapData[uWidth * uLY + uHX];
    D3DXVECTOR3 fHR = pWindMapData[uWidth * uHY + uHX];
    //return fLL;
    return ( (1.0f - fFracX) * fLL + fFracX * fLR ) * (1.0f - fFracY) + 
        fFracY * ( (1.0f - fFracX) * fHL + fFracX * fHR );
}

inline D3DXVECTOR2 Transform(D3DXVECTOR2 vec, D3DXVECTOR4 rot, float off)
{
    D3DXVECTOR2 *rotxy = (D3DXVECTOR2*)(float*)&rot;
    D3DXVECTOR2 *rotzw = (D3DXVECTOR2*)((float*)&rot + 2);
    return D3DXVECTOR2(D3DXVec2Dot(&vec, rotxy), D3DXVec2Dot(&vec, rotzw)) + *rotxy * off;
}

D3DXVECTOR3 WindData::GetWindValue( const D3DXVECTOR2 &a_vTexCoord, const float a_fWindTexTile, const float a_fWindStrength ) const
{
    //D3DXVECTOR4 g_vRotate45  = D3DXVECTOR4(0.7071f, 0.7071f, -0.7071f, 0.7071f);
    //D3DXVECTOR4 g_vRotate315 = D3DXVECTOR4(0.7071f, -0.7071f, 0.7071f, 0.7071f);
    ///* Getting UV's */
    //D3DXVECTOR2 vUV[3];
    //vUV[0] = a_vTexCoord + D3DXVECTOR2(*pTime * 0.02f * *pWindSpeed, 0.0f);
    //vUV[1] = 0.707f * Transform(a_vTexCoord, g_vRotate45 , *pTime * 0.04f * *pWindSpeed);
    //vUV[2] = 0.707f * Transform(a_vTexCoord, g_vRotate315, *pTime * 0.08f * *pWindSpeed);
    ///* so result, as in the shader */
    //D3DXVECTOR3 vRes = 
    //    0.5714f * GetValue(vUV[0], a_fWindTexTile) + 
    //    0.2857f * GetValue(vUV[1], a_fWindTexTile) + 
    //    0.1428f * GetValue(vUV[2], a_fWindTexTile);
    ////D3DXVec3Normalize(&vRes, &vRes);
    //return vRes * a_fWindStrength;
    return GetValue(a_vTexCoord, a_fWindTexTile) * a_fWindStrength;
}

void WindData::ConvertFrom( const D3D10_MAPPED_TEXTURE2D &a_MappedTex, const D3D10_TEXTURE2D_DESC &a_TexDesc )
{
    float* pTexels = (float*)a_MappedTex.pData;
    if (pData == NULL)
    {
        pData = new D3DXVECTOR3[a_TexDesc.Height * a_TexDesc.Width];
        pWindMapData = new D3DXVECTOR3[a_TexDesc.Height * a_TexDesc.Width];
        uHeight = a_TexDesc.Height;
        uWidth  = a_TexDesc.Width;
        fHeight = (float)uHeight;
        fWidth  = (float)uWidth;
    }
    
    for( UINT row = 0; row < a_TexDesc.Height; row++ )
    {
        UINT rowStart = row * a_MappedTex.RowPitch / sizeof(float);
        for( UINT col = 0; col < a_TexDesc.Width; col++ )
        {
            UINT colStart = col * 4;//RGBA
            pWindMapData[row * a_TexDesc.Width + col].x = pTexels[rowStart + colStart + 0];// / 255.0f; 
            pWindMapData[row * a_TexDesc.Width + col].y = pTexels[rowStart + colStart + 1];// / 255.0f;
            pWindMapData[row * a_TexDesc.Width + col].z = pTexels[rowStart + colStart + 2];// / 255.0f;
            /*pData[row * a_TexDesc.Width + col].x = pTexels[rowStart + colStart + 0];// / 255.0f; 
            pData[row * a_TexDesc.Width + col].y = pTexels[rowStart + colStart + 1];// / 255.0f;
            pData[row * a_TexDesc.Width + col].z = pTexels[rowStart + colStart + 2];// / 255.0f;*/
        }
    }
}

void WindData::UpdateWindTex( D3D10_MAPPED_TEXTURE2D &a_MappedTex )
{
    float2 vTexCoord;
    float *pTexels = (float*)a_MappedTex.pData;
    /* Getting UV's */
    D3DXVECTOR2 vUV;
    D3DXVECTOR3 vWind;
    for( UINT row = 0; row < uHeight; row++ )
    {
        UINT rowStart = row * a_MappedTex.RowPitch / sizeof(float);
        vTexCoord.y = float(row) / (fHeight - 1.0f);  
        for( UINT col = 0; col < uWidth; col++ )
        {
            UINT colStart = col * 4;//RGBA
            vTexCoord.x = float(col) / (fWidth - 1.0f);
            vUV = vTexCoord - D3DXVECTOR2(*pTime * 0.02f * *pWindSpeed, 0.0f);
            vWind = BiLinear(vUV);
            pTexels[rowStart + colStart + 0] = vWind.x;//for GPU
            pTexels[rowStart + colStart + 1] = vWind.y;//for GPU
            pTexels[rowStart + colStart + 2] = vWind.z;//for GPU
            pTexels[rowStart + colStart + 3] = 1.0f;//for GPU
            pData[row * uWidth + col] = vWind;//for CPU
        }
    }
}

//void WindData::Update( )
//{
//    float2 vTexCoord;
//    D3DXVECTOR4 g_vRotate45  = D3DXVECTOR4(0.7071f, 0.7071f, -0.7071f, 0.7071f);
//    D3DXVECTOR4 g_vRotate315 = D3DXVECTOR4(0.7071f, -0.7071f, 0.7071f, 0.7071f);
//    /* Getting UV's */
//    D3DXVECTOR2 vUV[3];
//    for( UINT row = 0; row < uHeight; row++ )
//    {
//        vTexCoord.y = float(row) / (fHeight - 1.0f);  
//        for( UINT col = 0; col < uWidth; col++ )
//        {
//            vTexCoord.x = float(col) / (fWidth - 1.0f);  
//            vUV[0] = vTexCoord + D3DXVECTOR2(*pTime * 0.02f * *pWindSpeed, 0.0f);
//            vUV[1] = 0.707f * Transform(vTexCoord, g_vRotate45 , *pTime * 0.03f * *pWindSpeed);
//            vUV[2] = 0.707f * Transform(vTexCoord, g_vRotate315, *pTime * 0.04f * *pWindSpeed);
///*            pData[row * uWidth + col] = 0.5714f * BiLinear(vUV[0]) + 
//                0.2857f * BiLinear(vUV[1]) + 
//                0.1428f * BiLinear(vUV[2]);
//*/
//            pData[row * uWidth + col] = 0.5714f * BiLinear(vUV[0]) + 
//                0.1857f * BiLinear(vUV[1]) + 
//                0.0828f * BiLinear(vUV[2]);
//           // pData[row * uWidth + col] = BiLinear(vUV[0]);
//        }
//    }
//}

Wind::Wind( ID3D10Device *a_pD3DDevice )
{
    m_fTime = 0.0f;
    m_pD3DDevice = a_pD3DDevice;
    /* Loading effect */
    ID3D10Blob *pErrors;
    D3DX10CreateEffectFromFile(L"Shaders/WindEffect.fx", NULL, NULL, 
        "fx_4_0", D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_DEBUG,
        0, m_pD3DDevice,
        NULL, NULL,
        &m_pWindEffect,
        &pErrors, NULL);
    char *errStr;

    if (pErrors)
    {
        errStr = static_cast<char*>(pErrors->GetBufferPointer());
    }

	m_pHeightMapPass = m_pWindEffect->GetTechniqueByIndex(0)->GetPassByName("HeighttexToHeightmap");
    m_pWindMapPass   = m_pWindEffect->GetTechniqueByIndex(0)->GetPassByName("HeightmapToWindmap");
    m_pWindTexPass   = m_pWindEffect->GetTechniqueByIndex(0)->GetPassByName("WindmapToWindtex");

    m_uVertexStride = sizeof(QuadVertex);
    m_uVertexOffset = 0;
    CreateVertexBuffer();
    CreateInputLayout();
    D3DX10CreateShaderResourceViewFromFile( m_pD3DDevice, L"resources/Wind.dds", NULL, NULL, &m_pHeightTexSRV, NULL );
	m_pHeightTexESRV = m_pWindEffect->GetVariableByName("g_txHeightTex")->AsShaderResource();
	m_pHeightTexESRV->SetResource(m_pHeightTexSRV);

    m_pWindSpeedESV = m_pWindEffect->GetVariableByName( "g_fWindSpeed" )->AsScalar();
    m_pTimeESV      = m_pWindEffect->GetVariableByName( "g_fTime"      )->AsScalar();
    m_pWindBias     = m_pWindEffect->GetVariableByName( "g_fWindBias"  )->AsScalar();
    m_pWindScale    = m_pWindEffect->GetVariableByName( "g_fWindScale" )->AsScalar();
    m_pTimeESV->SetFloat(m_fTime);

#pragma region Height Map Creation    
	D3D10_TEXTURE2D_DESC HeightMapDesc;
    ID3D10Resource *pRes;
    ID3D10Texture2D *pHeightTex;
    m_pHeightTexSRV->GetResource(&pRes);
    pRes->QueryInterface(__uuidof(ID3D10Texture2D), (void**)&pHeightTex);
    pHeightTex->GetDesc(&HeightMapDesc);
    m_uViewPortWidth = HeightMapDesc.Width;
    m_uViewPortHeight = HeightMapDesc.Height;
    SAFE_RELEASE(pRes);
    SAFE_RELEASE(pHeightTex);
	ZeroMemory( &HeightMapDesc, sizeof(HeightMapDesc) );
	HeightMapDesc.Width = m_uViewPortWidth;
	HeightMapDesc.Height = m_uViewPortHeight;
	HeightMapDesc.MipLevels = 1;
	HeightMapDesc.ArraySize = 1;
	HeightMapDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	HeightMapDesc.SampleDesc.Count = 1;
	HeightMapDesc.Usage = D3D10_USAGE_DEFAULT;
	HeightMapDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
	m_pD3DDevice->CreateTexture2D(&HeightMapDesc, NULL, &m_pHeightMap);

	/* Creating Render Target View for Height Map */
	D3D10_RENDER_TARGET_VIEW_DESC HeightMapRTVDesc;
	ZeroMemory( &HeightMapRTVDesc, sizeof(HeightMapRTVDesc) );
	HeightMapRTVDesc.Format = HeightMapDesc.Format;
	HeightMapRTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
	HeightMapRTVDesc.Texture2D.MipSlice = 0;
	m_pD3DDevice->CreateRenderTargetView(m_pHeightMap, &HeightMapRTVDesc, &m_pHeightMapRTV);

	/* Creating Shader Resource View for Height Map */
	D3D10_SHADER_RESOURCE_VIEW_DESC HeightMapSRVDesc;
	ZeroMemory( &HeightMapSRVDesc, sizeof(HeightMapSRVDesc) );
	HeightMapSRVDesc.Format = HeightMapDesc.Format;
	HeightMapSRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	HeightMapSRVDesc.Texture2D.MostDetailedMip = 0;
	HeightMapSRVDesc.Texture2D.MipLevels = 1;
	m_pD3DDevice->CreateShaderResourceView(m_pHeightMap, &HeightMapSRVDesc, &m_pHeightMapSRV);
#pragma endregion

	m_pHeightMapESRV = m_pWindEffect->GetVariableByName("g_txHeightMap")->AsShaderResource();
	m_pHeightMapESRV->SetResource(m_pHeightMapSRV);

#pragma region Wind Map Creation    
    D3D10_TEXTURE2D_DESC WindMapDesc;
    ZeroMemory( &WindMapDesc, sizeof(WindMapDesc) );
    WindMapDesc.Width = m_uViewPortWidth;
    WindMapDesc.Height = m_uViewPortHeight;
    WindMapDesc.MipLevels = 1;
    WindMapDesc.ArraySize = 1;
    WindMapDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    WindMapDesc.SampleDesc.Count = 1;
    WindMapDesc.Usage = D3D10_USAGE_DEFAULT;
    WindMapDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
    m_pD3DDevice->CreateTexture2D(&WindMapDesc, NULL, &m_pWindMap);

    /* Creating Render Target View for Wind Map */
    D3D10_RENDER_TARGET_VIEW_DESC WindMapRTVDesc;
    ZeroMemory( &WindMapRTVDesc, sizeof(WindMapRTVDesc) );
    WindMapRTVDesc.Format = WindMapDesc.Format;
    WindMapRTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
    WindMapRTVDesc.Texture2D.MipSlice = 0;
    m_pD3DDevice->CreateRenderTargetView(m_pWindMap, &WindMapRTVDesc, &m_pWindMapRTV);

    /* Creating Shader Resource View for Wind Map */
    D3D10_SHADER_RESOURCE_VIEW_DESC WindMapSRVDesc;
    ZeroMemory( &WindMapSRVDesc, sizeof(WindMapSRVDesc) );
    WindMapSRVDesc.Format = WindMapDesc.Format;
    WindMapSRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
    WindMapSRVDesc.Texture2D.MostDetailedMip = 0;
    WindMapSRVDesc.Texture2D.MipLevels = 1;
    m_pD3DDevice->CreateShaderResourceView(m_pWindMap, &WindMapSRVDesc, &m_pWindMapSRV);
#pragma endregion

    m_pWindMapESRV = m_pWindEffect->GetVariableByName("g_txWindMap")->AsShaderResource();
     
    ID3D10EffectVectorVariable *pPixSize = m_pWindEffect->GetVariableByName("g_vPixSize")->AsVector();
    D3DXVECTOR2 vPixSize(1.0f / (float)m_uViewPortWidth, 1.0f / (float)m_uViewPortHeight); // 1/256
    pPixSize->SetFloatVector((FLOAT*)vPixSize);

#pragma region Wind Tex Creation
    /* And now the same operations for Wind Tex :) */
    /* Creating Wind Tex texture */
    ZeroMemory( &m_WindTexStagingDesc, sizeof(m_WindTexStagingDesc) );
    m_WindTexStagingDesc.Width = m_uViewPortWidth;
    m_WindTexStagingDesc.Height = m_uViewPortHeight;
    m_WindTexStagingDesc.MipLevels = 1;
    m_WindTexStagingDesc.ArraySize = 1;
    m_WindTexStagingDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    m_WindTexStagingDesc.SampleDesc.Count = 1;
    m_WindTexStagingDesc.Usage = D3D10_USAGE_DYNAMIC;//D3D10_USAGE_DEFAULT;
    m_WindTexStagingDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;//D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
    m_WindTexStagingDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    m_pD3DDevice->CreateTexture2D(&m_WindTexStagingDesc, NULL, &m_pWindTex);
    
    /* Creating Render Target View for Wind Tex */
    /*D3D10_RENDER_TARGET_VIEW_DESC WindTexRTVDesc;
    ZeroMemory( &WindTexRTVDesc, sizeof(WindTexRTVDesc) );
    WindTexRTVDesc.Format = m_WindTexStagingDesc.Format;
    WindTexRTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
    WindTexRTVDesc.Texture2D.MipSlice = 0;
    m_pD3DDevice->CreateRenderTargetView(m_pWindTex, &WindTexRTVDesc, &m_pWindTexRTV);*/

    /* Creating Shader Resource View for Wind Tex */
    D3D10_SHADER_RESOURCE_VIEW_DESC WindTexSRVDesc;
    ZeroMemory( &WindTexSRVDesc, sizeof(WindTexSRVDesc) );
    WindTexSRVDesc.Format = m_WindTexStagingDesc.Format;
    WindTexSRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
    WindTexSRVDesc.Texture2D.MostDetailedMip = 0;
    WindTexSRVDesc.Texture2D.MipLevels = 1;
    m_pD3DDevice->CreateShaderResourceView(m_pWindTex, &WindTexSRVDesc, &m_pWindTexSRV);

    /* Creating texture for reading on CPU */
    m_WindTexStagingDesc.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
    m_WindTexStagingDesc.BindFlags      = 0;
    m_WindTexStagingDesc.Usage          = D3D10_USAGE_STAGING;
    m_pD3DDevice->CreateTexture2D(&m_WindTexStagingDesc, 0, &m_pWindTexStaging);
#pragma endregion

    /*m_pWindTexESRV = a_pGrassEffect->GetVariableByName("g_txWindTex")->AsShaderResource();
    m_pWindTexESRV->SetResource(m_pWindTexSRV);*/

#pragma region depth stencil texture
    m_pDepthTex = NULL;
    D3D10_TEXTURE2D_DESC dsDesc;
    dsDesc.Width = m_uViewPortWidth;
    dsDesc.Height = m_uViewPortHeight;
    dsDesc.MipLevels = 1;
    dsDesc.ArraySize = 1;
    dsDesc.Format = DXGI_FORMAT_D32_FLOAT;//R32_FLOAT ?
    dsDesc.SampleDesc.Count = 1;
    dsDesc.SampleDesc.Quality = 0;
    dsDesc.Usage = D3D10_USAGE_DEFAULT;
    dsDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
    dsDesc.CPUAccessFlags = 0;
    dsDesc.MiscFlags = 0;
    m_pD3DDevice->CreateTexture2D( &dsDesc, NULL, &m_pDepthTex );

    // Create the depth stencil view
    D3D10_DEPTH_STENCIL_VIEW_DESC DescDS;
    ZeroMemory( &DescDS, sizeof(DescDS) );
    DescDS.Format = dsDesc.Format;
    DescDS.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
    m_pD3DDevice->CreateDepthStencilView( m_pDepthTex, &DescDS, &m_pDSV );    
#pragma endregion

    SAFE_RELEASE(m_pHeightMap);
	MakeHeightMap();
	MakeWindMap();
    
    m_WindData.pTime      = &m_fTime;
    m_WindData.pWindSpeed = &m_fWindSpeed;
}

Wind::~Wind( )
{
    SAFE_RELEASE(m_pInputLayout);
    SAFE_RELEASE(m_pVertexBuffer);

	SAFE_RELEASE(m_pHeightTexSRV);

	//SAFE_RELEASE(m_pHeightMap);
	SAFE_RELEASE(m_pHeightMapRTV);	
    SAFE_RELEASE(m_pHeightMapSRV);

    SAFE_RELEASE(m_pWindMap);
    SAFE_RELEASE(m_pWindMapRTV);
    SAFE_RELEASE(m_pWindMapSRV);

    SAFE_RELEASE(m_pWindTex);
    SAFE_RELEASE(m_pWindTexStaging);
    //SAFE_RELEASE(m_pWindTexRTV);
    SAFE_RELEASE(m_pWindTexSRV);

    SAFE_RELEASE(m_pDepthTex);
    SAFE_RELEASE(m_pDSV);
    SAFE_RELEASE(m_pWindEffect);
}

void Wind::SetWindSpeed( float a_fWindSpeed )
{
    m_pWindSpeedESV->SetFloat(a_fWindSpeed);
    m_fWindSpeed = a_fWindSpeed;
}

void Wind::SetWindBias( float a_fBias )
{
    m_pWindBias->SetFloat(a_fBias);
    MakeWindMap();
    UpdateWindData();
}

void Wind::SetWindScale( float a_fScale )
{
    m_pWindScale->SetFloat(a_fScale);
    MakeWindMap();
    UpdateWindData();
}

const WindData *Wind::WindDataPtr( )
{
    return &m_WindData;
}

void Wind::UpdateWindData( )
{
    /* Copying data from one texture to another */
    //m_pD3DDevice->CopyResource(m_pWindTexStaging, m_pWindTex);
    m_pD3DDevice->CopyResource(m_pWindTexStaging, m_pWindMap);
    D3D10_MAPPED_TEXTURE2D MappedTexture;
    m_pWindTexStaging->Map(D3D10CalcSubresource(0, 0, 1), D3D10_MAP_READ, 0, &MappedTexture);
    m_WindData.ConvertFrom(MappedTexture, m_WindTexStagingDesc);
    m_pWindTexStaging->Unmap(D3D10CalcSubresource(0, 0, 1));
}

void Wind::MakeHeightMap( )
{
	/* Saving render targets */
	ID3D10RenderTargetView *pOrigRT;
	ID3D10DepthStencilView *pOrigDS;
	D3D10_VIEWPORT         OrigViewPort[1];
	D3D10_VIEWPORT         ViewPort[1];
	UINT                   NumV = 1;
	m_pD3DDevice->RSGetViewports(&NumV, OrigViewPort);
	ViewPort[0] = OrigViewPort[0];
	ViewPort[0].Height = m_uViewPortHeight;
	ViewPort[0].Width  = m_uViewPortWidth;
	m_pD3DDevice->RSSetViewports(1, ViewPort);

	m_pD3DDevice->OMGetRenderTargets( 1, &pOrigRT, &pOrigDS );
	/* Setting up WindMap and depth stencil */    

	m_pD3DDevice->ClearRenderTargetView( m_pHeightMapRTV, ClearColor );
	//m_pD3DDevice->ClearDepthStencilView( m_pDSV, D3D10_CLEAR_DEPTH, 1.0, 0 );
	m_pD3DDevice->OMSetRenderTargets(1, &m_pHeightMapRTV, NULL);
	/* Executing rendering */
	m_pD3DDevice->IASetInputLayout(m_pInputLayout);
	m_pD3DDevice->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
	m_pD3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pHeightMapPass->Apply(0);
	m_pD3DDevice->Draw(4, 0);

	/* Reverting changes */
	m_pD3DDevice->OMSetRenderTargets(1, &pOrigRT, pOrigDS);
	m_pD3DDevice->RSSetViewports(NumV, OrigViewPort);

	SAFE_RELEASE( pOrigRT );
	SAFE_RELEASE( pOrigDS );
}

void Wind::MakeWindMap( )
{
    m_pWindMapESRV->SetResource(NULL);
    /* Saving render targets */
    ID3D10RenderTargetView *pOrigRT;
    ID3D10DepthStencilView *pOrigDS;
    D3D10_VIEWPORT         OrigViewPort[1];
    D3D10_VIEWPORT         ViewPort[1];
    UINT                   NumV = 1;
    m_pD3DDevice->RSGetViewports(&NumV, OrigViewPort);
    ViewPort[0] = OrigViewPort[0];
    ViewPort[0].Height = m_uViewPortHeight;
    ViewPort[0].Width  = m_uViewPortWidth;
    m_pD3DDevice->RSSetViewports(1, ViewPort);

    m_pD3DDevice->OMGetRenderTargets( 1, &pOrigRT, &pOrigDS );
    /* Setting up WindMap and depth stencil */    
    
    m_pD3DDevice->ClearRenderTargetView( m_pWindMapRTV, ClearColor );
    //m_pD3DDevice->ClearDepthStencilView( m_pDSV, D3D10_CLEAR_DEPTH, 1.0, 0 );
    m_pD3DDevice->OMSetRenderTargets(1, &m_pWindMapRTV, NULL);
    /* Executing rendering */
    m_pD3DDevice->IASetInputLayout(m_pInputLayout);
    m_pD3DDevice->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
    m_pD3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_pWindMapPass->Apply(0);
    m_pD3DDevice->Draw(4, 0);

    /* Reverting changes */
    m_pD3DDevice->OMSetRenderTargets(1, &pOrigRT, pOrigDS);
    m_pD3DDevice->RSSetViewports(NumV, OrigViewPort);
    m_pWindMapESRV->SetResource(m_pWindMapSRV);
    
    SAFE_RELEASE( pOrigRT );
    SAFE_RELEASE( pOrigDS );
}

void Wind::MakeWindTex( )
{    
    D3D10_MAPPED_TEXTURE2D MappedTexture;
    m_pWindTex->Map(D3D10CalcSubresource(0, 0, 1), D3D10_MAP_WRITE_DISCARD, 0, &MappedTexture);
    m_WindData.UpdateWindTex(MappedTexture);
    m_pWindTex->Unmap(D3D10CalcSubresource(0, 0, 1));;
}

void Wind::Update( float a_fElapsed )
{
    m_fTime += a_fElapsed;
    m_pTimeESV->SetFloat(m_fTime);
    MakeWindTex();
    //m_WindData.Update();
    //UpdateWindData();
}

void Wind::CreateVertexBuffer( )
{
    /* Initializing vertices */
    QuadVertex Vertices[4];
    Vertices[0].vPos = D3DXVECTOR3(-1.0f, -1.0f, 0.1f);
    Vertices[0].vTexCoord = D3DXVECTOR2(0.0f, 0.0f);

    Vertices[1].vPos = D3DXVECTOR3(1.0f, -1.0f, 0.1f);
    Vertices[1].vTexCoord = D3DXVECTOR2(1.0f, 0.0f);

    Vertices[2].vPos = D3DXVECTOR3(-1.0f, 1.0f, 0.1f);
    Vertices[2].vTexCoord = D3DXVECTOR2(0.0f, 1.0f);

    Vertices[3].vPos = D3DXVECTOR3(1.0f, 1.0f, 0.1f);
    Vertices[3].vTexCoord = D3DXVECTOR2(1.0f, 1.0f);
    /* Initializing buffer */
    D3D10_BUFFER_DESC BufferDesc = 
    {
        4 * sizeof(QuadVertex),
        D3D10_USAGE_DEFAULT,
        D3D10_BIND_VERTEX_BUFFER,
        0, 0
    };
    D3D10_SUBRESOURCE_DATA BufferInitData;
    BufferInitData.pSysMem = Vertices;
    m_pD3DDevice->CreateBuffer(&BufferDesc, &BufferInitData, &m_pVertexBuffer);
}

void Wind::CreateInputLayout( )
{
    D3D10_INPUT_ELEMENT_DESC InputDesc[] = 
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D10_INPUT_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0}
    };
    D3D10_PASS_DESC PassDesc;
    m_pWindMapPass->GetDesc(&PassDesc);
    int InputElementsCount = sizeof(InputDesc) / sizeof(D3D10_INPUT_ELEMENT_DESC);
    m_pD3DDevice->CreateInputLayout(InputDesc, InputElementsCount, 
        PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, 
        &m_pInputLayout);   
}

ID3D10ShaderResourceView *Wind::GetMap( )
{
    return m_pWindTexSRV;
}