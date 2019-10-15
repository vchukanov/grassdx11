#include "Terrain.h"
#include "StateManager.h"

static float ClearColor[4] = {0.0f, 0.0f, 1.0f, 0.0f};

TerrainHeightData::TerrainHeightData( )
{
    pData    = NULL;
    pNormals = NULL;
}

TerrainHeightData::~TerrainHeightData( )
{
    if (pData)
        delete [] pData;
    if (pNormals)
        delete [] pNormals;
}

float TerrainHeightData::GetHeight( float a_fX, float a_fY ) const
{     
    float fX = a_fX * (float)(uWidth - 1);
    float fY = a_fY * (float)(uHeight - 1);
    UINT uLX = (UINT)floor(fX);
    UINT uHX = uLX + 1;
    UINT uLY = (UINT)floor(fY);
    UINT uHY = uLY + 1;
    float fFracX = (fX) - floor(fX);
    float fFracY = (fY) - floor(fY);
    if (uHX > uWidth - 1)
        uHX = uWidth - 1;
    if (uHY > uHeight - 1)
        uHY = uHeight - 1;
    float fLL = pData[uWidth * uLY + uLX];
    float fHL = pData[uWidth * uHY + uLX];
    float fLR = pData[uWidth * uLY + uHX];
    float fHR = pData[uWidth * uHY + uHX];
    return ( (1.0f - fFracX) * fLL + fFracX * fLR ) * (1.0f - fFracY) + 
        fFracY * ( (1.0f - fFracX) * fHL + fFracX * fHR );
}

D3DXVECTOR3 TerrainHeightData::GetNormal( float a_fX, float a_fY ) const
{     
    float fX = a_fX * (float)(uWidth - 1);
    float fY = a_fY * (float)(uHeight - 1);
    UINT uLX = (UINT)floor(fX);
    UINT uHX = uLX + 1;
    UINT uLY = (UINT)floor(fY);
    UINT uHY = uLY + 1;
    float fFracX = (fX) - floor(fX);
    float fFracY = (fY) - floor(fY);
    if (uHX > uWidth - 1)
        uHX = uWidth - 1;
    if (uHY > uHeight - 1)
        uHY = uHeight - 1;
    D3DXVECTOR3 fLL = pNormals[uWidth * uLY + uLX];
    D3DXVECTOR3 fHL = pNormals[uWidth * uHY + uLX];
    D3DXVECTOR3 fLR = pNormals[uWidth * uLY + uHX];
    D3DXVECTOR3 fHR = pNormals[uWidth * uHY + uHX];
    return ( (1.0f - fFracX) * fLL + fFracX * fLR ) * (1.0f - fFracY) + 
        fFracY * ( (1.0f - fFracX) * fHL + fFracX * fHR );
}

float TerrainHeightData::GetHeight3x3( float a_fX, float a_fY ) const
{     
    float dU = 1.0f / (fWidth);
    float dV = 1.0f / (fWidth);
    float h0 = GetHeight(a_fX, a_fY);
    float h1 = GetHeight(a_fX-dU, a_fY-dV);
    float h2 = GetHeight(a_fX, a_fY-dV);
    float h3 = GetHeight(a_fX-dU, a_fY);

    return (h0 + h1 + h2 + h3) * 0.25f;
}


void TerrainHeightData::ConvertFrom( const D3D10_MAPPED_TEXTURE2D &a_MappedTex, const D3D10_TEXTURE2D_DESC &a_TexDesc )
{
    UCHAR* pTexels = (UCHAR*)a_MappedTex.pData;
    if (pData)
        return;
    pData    = new float[a_TexDesc.Height * a_TexDesc.Width];
    pNormals = new D3DXVECTOR3[a_TexDesc.Height * a_TexDesc.Width];
	uHeight = a_TexDesc.Height;
	uWidth  = a_TexDesc.Width;
	fHeight = (float)uHeight;
	fWidth  = (float)uWidth;
    for( UINT row = 0; row < a_TexDesc.Height; row++ )
    {
        UINT rowStart = row * a_MappedTex.RowPitch;
        for( UINT col = 0; col < a_TexDesc.Width; col++ )
        {
            UINT colStart = col;// * 4;//RGBA
			/*pNormals[row * uWidth + col].x     = (float)pTexels[rowStart + colStart + 0] / 255.0f * 2.0f - 1.0f;
			pNormals[row * uWidth + col].z     = (float)pTexels[rowStart + colStart + 1] / 255.0f * 2.0f - 1.0f;
			pNormals[row * uWidth + col].y     = (float)pTexels[rowStart + colStart + 2] / 255.0f * 2.0f - 1.0f;*/
            pData[row * a_TexDesc.Width + col] = ((float)pTexels[rowStart + colStart + 0]) / 255.0f; 
            /*pData[row * a_TexDesc.Width + col].y = (float)pTexels[rowStart + colStart + 1] / 255.0f;
            pData[row * a_TexDesc.Width + col].z = (float)pTexels[rowStart + colStart + 2] / 255.0f;*/
        }
    }    
}

void TerrainHeightData::CalcNormals( float a_fHeightScale, float a_fDistBtwVertices )
{
    enum DIR
    {
        LEFT = 0,
        RIGHT,
        UP,
        DOWN
    };
    D3DXVECTOR3 vToVertex[4];//vector from current vertex to another one
    vToVertex[LEFT ].x = -a_fDistBtwVertices;
    vToVertex[LEFT ].z =  0.0f;
    vToVertex[RIGHT].x =  a_fDistBtwVertices;
    vToVertex[RIGHT].z =  0.0f;
    vToVertex[UP   ].x =  0.0f;
    vToVertex[UP   ].z =  a_fDistBtwVertices;
    vToVertex[DOWN ].x =  0.0f;
    vToVertex[DOWN ].z = -a_fDistBtwVertices;

    D3DXVECTOR3 vCrossProduct;
    
    for( UINT row = 0; row < uHeight; row++ )
    {
        for( UINT col = 0; col < uWidth; col++ )
        {
            if (col == 0 || row == 0 || col == uWidth - 1 || row == uHeight - 1)
            {
                pNormals[row * uWidth + col].x = 0.0f;
                pNormals[row * uWidth + col].y = 1.0f;
                pNormals[row * uWidth + col].z = 0.0f;
                continue;
            }
            float fH = pData[row * uWidth + col];
            vToVertex[LEFT ].y = pData[row * uWidth + col - 1] * a_fHeightScale - fH * a_fHeightScale;
            vToVertex[RIGHT].y = pData[row * uWidth + col + 1] * a_fHeightScale - fH * a_fHeightScale;
            vToVertex[DOWN ].y = pData[row * uWidth + col - uWidth] * a_fHeightScale - fH * a_fHeightScale;
            vToVertex[UP   ].y = pData[row * uWidth + col + uWidth] * a_fHeightScale - fH * a_fHeightScale;
            pNormals[row * uWidth + col].x =  
            pNormals[row * uWidth + col].y = 
            pNormals[row * uWidth + col].z = 0.0f;
            
            D3DXVec3Cross(&vCrossProduct, &vToVertex[DOWN], &vToVertex[LEFT]);
            D3DXVec3Normalize(&vCrossProduct, &vCrossProduct);
            pNormals[row * uWidth + col] += vCrossProduct;

            D3DXVec3Cross(&vCrossProduct, &vToVertex[RIGHT], &vToVertex[DOWN]);
            D3DXVec3Normalize(&vCrossProduct, &vCrossProduct);
            pNormals[row * uWidth + col] += vCrossProduct;

            D3DXVec3Cross(&vCrossProduct, &vToVertex[UP], &vToVertex[RIGHT]);
            D3DXVec3Normalize(&vCrossProduct, &vCrossProduct);
            pNormals[row * uWidth + col] += vCrossProduct;

            D3DXVec3Cross(&vCrossProduct, &vToVertex[LEFT], &vToVertex[UP]);
            D3DXVec3Normalize(&vCrossProduct, &vCrossProduct);
            pNormals[row * uWidth + col] += vCrossProduct;

            D3DXVec3Normalize(&pNormals[row * uWidth + col], &pNormals[row * uWidth + col]);
			/*
			if (pNormals[row * uWidth + col].y > 0.999)
			{
				pNormals[row * uWidth + col].x = 0.0f;
				pNormals[row * uWidth + col].y = 1.0f;
				pNormals[row * uWidth + col].z = 0.0f;
			}
            */
        }
    }

}

//void TerrainHeightData::UpdateNormals( float a_fHeightScale )
//{
    //for( UINT row = 0; row < uHeight; row++ )
    //{
    //    for( UINT col = 0; col < uWidth; col++ )
    //    {
    //        if (col == 0 || row == 0 || col == uWidth - 1 || row == uHeight - 1)//that shit sucks :(
    //        {
    //            pNormals[row * uWidth + col].x = 0.0f;
    //            pNormals[row * uWidth + col].y = 1.0f;
    //            pNormals[row * uWidth + col].z = 0.0f;
    //            continue;
    //        }
    //        //that's much better case :)
    //        float heightL = pData[row * uWidth + col - 1] * a_fHeightScale;
    //        float heightR = pData[row * uWidth + col + 1] * a_fHeightScale;
    //        float heightT = pData[row * uWidth + col - uWidth] * a_fHeightScale;
    //        float heightB = pData[row * uWidth + col + uWidth] * a_fHeightScale;
    //        pNormals[row * uWidth + col].x = heightR - heightL;
    //        pNormals[row * uWidth + col].y = 1.0f;
    //        pNormals[row * uWidth + col].z = heightB - heightT;
    //        D3DXVec3Normalize(&pNormals[row * uWidth + col], &pNormals[row * uWidth + col]);
    //    }
    //}
//}

Terrain::Terrain( ID3D10Device *a_pD3DDevice, ID3D10Effect *a_pEffect, float a_fSize )
{
    m_pD3DDevice    = a_pD3DDevice;
    m_uVertexStride = sizeof(TerrainVertex);
    m_uVertexOffset = 0;
    /* just one technique in effect */
    ID3D10EffectTechnique *pTechnique = a_pEffect->GetTechniqueByIndex(0);
    m_pPass                           = pTechnique->GetPassByName("RenderTerrainPass");
    m_pLightMapPass                   = pTechnique->GetPassByName("RenderLightMapPass");
    m_pLightMapESRV                   = a_pEffect->GetVariableByName("g_txLightMap")->AsShaderResource();
    ID3D10EffectShaderResourceVariable *pESRV;
    pESRV = a_pEffect->GetVariableByName("g_txGrassDiffuse")->AsShaderResource();
    D3DX10CreateShaderResourceViewFromFile(m_pD3DDevice, L"resources/Grass.dds", 0, 0, &m_pGrassSRV, 0);
    pESRV->SetResource(m_pGrassSRV);
    /*pESRV = a_pEffect->GetVariableByName("g_txSandDiffuse")->AsShaderResource();
    D3DX10CreateShaderResourceViewFromFile(m_pD3DDevice, L"resources/Sand.dds", 0, 0, &m_pSandSRV, 0);
    pESRV->SetResource(m_pSandSRV);*/
    m_fCellSize = 0.0f;
    CreateInputLayout();
    CreateBuffers(a_fSize);//initializing m_fCellSize
    m_pHeightMapESRV = a_pEffect->GetVariableByName("g_txHeightMap")->AsShaderResource();
    m_pHeightMapSRV  = NULL; 
}

Terrain::~Terrain()
{
    SAFE_RELEASE(m_pInputLayout);
    SAFE_RELEASE(m_pVertexBuffer);
    SAFE_RELEASE(m_pIndexBuffer);
    SAFE_RELEASE(m_pGrassSRV);
    //SAFE_RELEASE(m_pSandSRV);
    SAFE_RELEASE(m_pHeightMapSRV);
    SAFE_RELEASE(m_pLightMapSRV);
    SAFE_RELEASE(m_pLightMapRTV);
    //SAFE_RELEASE(m_pLightMap);
    SAFE_RELEASE(m_pQuadVertexBuffer);
}

TerrainHeightData *Terrain::HeightDataPtr( )
{
    return &m_HeightData;
}

void Terrain::BuildHeightMap( float a_fHeightScale )
{
    ID3D10Resource  *pRes = NULL;
    ID3D10Texture2D *pHeightMap;
    ID3D10Texture2D *pSrcTex;
    HRESULT hr;
    /* Loading height map for future processing... */
    D3DX10_IMAGE_LOAD_INFO loadInfo;
    ZeroMemory(&loadInfo, sizeof(loadInfo));
    loadInfo.Format         = DXGI_FORMAT_R8_UNORM;
    loadInfo.CpuAccessFlags = D3D10_CPU_ACCESS_READ;
    loadInfo.Usage          = D3D10_USAGE_STAGING;
    V(D3DX10CreateTextureFromFile(m_pD3DDevice, L"resources/HeightMap.dds", &loadInfo, 0, &pRes, 0));
    pRes->QueryInterface( __uuidof( ID3D10Texture2D ), (LPVOID*)&pSrcTex );
    SAFE_RELEASE(pRes);
    D3D10_TEXTURE2D_DESC SRTexDesc;
    pSrcTex->GetDesc(&SRTexDesc);
    D3D10_MAPPED_TEXTURE2D MappedTexture;
    pSrcTex->Map(D3D10CalcSubresource(0, 0, 1), D3D10_MAP_READ, 0, &MappedTexture);
    /* Calculating terrain heights */
    m_HeightData.ConvertFrom(MappedTexture, SRTexDesc);
    pSrcTex->Unmap(D3D10CalcSubresource(0, 0, 1));
    /* Ok, we don't need initial heighmap anymore */
    SAFE_RELEASE(pSrcTex);
    /* Calculating normals with respect to HeightScale. 
     * Note, that the m_fCellSize was precomputed in CreateBuffers(). 
     */
    m_HeightData.CalcNormals(a_fHeightScale, m_fCellSize);
    /* Now we need to create a texture to write heights and normals into */
    SRTexDesc.Format         = DXGI_FORMAT_R32G32B32A32_FLOAT;//DXGI_FORMAT_R8G8B8A8_UNORM;//
    SRTexDesc.BindFlags      = D3D10_BIND_SHADER_RESOURCE;
    SRTexDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    SRTexDesc.Usage          = D3D10_USAGE_DYNAMIC;
    SRTexDesc.MipLevels      = 1;
    SRTexDesc.ArraySize      = 1;
    V(m_pD3DDevice->CreateTexture2D(&SRTexDesc, 0, &pSrcTex));
    pSrcTex->Map(D3D10CalcSubresource(0, 0, 1), D3D10_MAP_WRITE_DISCARD, 0, &MappedTexture);
    float* pTexels = (float*)MappedTexture.pData;
    //UCHAR* pTexels = (UCHAR*)MappedTexture.pData;
    for( UINT row = 0; row < SRTexDesc.Height; row++ )
    {
        UINT rowStart = row * MappedTexture.RowPitch / sizeof(float);
        for( UINT col = 0; col < SRTexDesc.Width; col++ )
        {
            UINT colStart = col * 4;
            D3DXVECTOR3 *pN = &m_HeightData.pNormals[row * SRTexDesc.Width + col];
            /*pTexels[rowStart + colStart + 0] = UCHAR( (pN->x * 0.5f + 0.5f) * 255.0f);
            pTexels[rowStart + colStart + 1] = UCHAR( (pN->y * 0.5f + 0.5f) * 255.0f);
            pTexels[rowStart + colStart + 2] = UCHAR( (pN->z * 0.5f + 0.5f) * 255.0f);
            pTexels[rowStart + colStart + 3] = UCHAR(m_HeightData.pData[row * SRTexDesc.Width + col] * 255.0f);*/
            pTexels[rowStart + colStart + 0] = (pN->x * 0.5f + 0.5f);
            pTexels[rowStart + colStart + 1] = (pN->y * 0.5f + 0.5f);
            pTexels[rowStart + colStart + 2] = (pN->z * 0.5f + 0.5f);
            pTexels[rowStart + colStart + 3]  = m_HeightData.pData[row * SRTexDesc.Width + col];
        }
    }
    pSrcTex->Unmap(D3D10CalcSubresource(0, 0, 1));
    /* Ok, creating HeightMap texture finally */
    //SRTexDesc.BindFlags      = D3D10_BIND_SHADER_RESOURCE;
    SRTexDesc.CPUAccessFlags = 0;
    SRTexDesc.Usage          = D3D10_USAGE_DEFAULT;
    V(m_pD3DDevice->CreateTexture2D(&SRTexDesc, 0, &pHeightMap));
    /* Copying data from one texture to another */
    m_pD3DDevice->CopyResource(pHeightMap, pSrcTex);
    SAFE_RELEASE(pSrcTex);

    /* And creating SRV for it */
    D3D10_SHADER_RESOURCE_VIEW_DESC HeightMapSRVDesc;
    ZeroMemory( &HeightMapSRVDesc, sizeof(HeightMapSRVDesc) );
    HeightMapSRVDesc.Format = SRTexDesc.Format;
    HeightMapSRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
    HeightMapSRVDesc.Texture2D.MostDetailedMip = 0;
    HeightMapSRVDesc.Texture2D.MipLevels = 1;
    V(m_pD3DDevice->CreateShaderResourceView(pHeightMap, &HeightMapSRVDesc, &m_pHeightMapSRV));
    /******/
    SAFE_RELEASE(pHeightMap);
    m_pHeightMapESRV->SetResource(m_pHeightMapSRV);

#pragma region Light Map Creation  
    m_ViewPort.MaxDepth = 1.0f;
    m_ViewPort.MinDepth = 0.0f;
    m_ViewPort.TopLeftX = 0;
    m_ViewPort.TopLeftY = 0;
    m_ViewPort.Width    = SRTexDesc.Width;
    m_ViewPort.Height   = SRTexDesc.Height;

    SRTexDesc.MipLevels = 1;
    SRTexDesc.ArraySize = 1;
    SRTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    SRTexDesc.SampleDesc.Count = 1;
    SRTexDesc.Usage = D3D10_USAGE_DEFAULT;
    SRTexDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
    SRTexDesc.CPUAccessFlags = 0;
    m_pD3DDevice->CreateTexture2D(&SRTexDesc, NULL, &m_pLightMap);

    /* Creating Render Target View for Height Map */
    D3D10_RENDER_TARGET_VIEW_DESC LightMapRTVDesc;
    ZeroMemory( &LightMapRTVDesc, sizeof(LightMapRTVDesc) );
    LightMapRTVDesc.Format = SRTexDesc.Format;
    LightMapRTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
    LightMapRTVDesc.Texture2D.MipSlice = 0;
    m_pD3DDevice->CreateRenderTargetView(m_pLightMap, &LightMapRTVDesc, &m_pLightMapRTV);

    /* Creating Shader Resource View for Height Map */
    D3D10_SHADER_RESOURCE_VIEW_DESC LightMapSRVDesc;
    ZeroMemory( &LightMapSRVDesc, sizeof(LightMapSRVDesc) );
    LightMapSRVDesc.Format = LightMapRTVDesc.Format;
    LightMapSRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
    LightMapSRVDesc.Texture2D.MostDetailedMip = 0;
    LightMapSRVDesc.Texture2D.MipLevels = 1;
    m_pD3DDevice->CreateShaderResourceView(m_pLightMap, &LightMapSRVDesc, &m_pLightMapSRV);
    SAFE_RELEASE(m_pLightMap);
#pragma endregion   
}

void Terrain::UpdateLightMap( )
{
    GetGlobalStateManager().SetRasterizerState("EnableMSAA");

    m_pLightMapESRV->SetResource(NULL);
    /* Saving render targets */
    ID3D10RenderTargetView* pOrigRT;
    ID3D10DepthStencilView* pOrigDS;
    D3D10_VIEWPORT         OrigViewPort[1];
    UINT                   NumV = 1;
    m_pD3DDevice->RSGetViewports(&NumV, OrigViewPort);    
    m_pD3DDevice->RSSetViewports(1, &m_ViewPort);

    m_pD3DDevice->OMGetRenderTargets( 1, &pOrigRT, &pOrigDS );
    /* Setting up WindTex and NULL as depth stencil */
    m_pD3DDevice->OMSetRenderTargets(1, &m_pLightMapRTV, NULL);
    m_pD3DDevice->ClearRenderTargetView( m_pLightMapRTV, ClearColor );
    /* Executing rendering */
    m_pD3DDevice->IASetInputLayout(m_pInputLayout);
    m_pD3DDevice->IASetVertexBuffers(0, 1, &m_pQuadVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
    m_pD3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_pLightMapPass->Apply(0);    
    m_pD3DDevice->Draw(4, 0);

    /* Reverting changes */
    m_pD3DDevice->OMSetRenderTargets(1, &pOrigRT, pOrigDS);
    m_pD3DDevice->RSSetViewports(NumV, OrigViewPort);

    SAFE_RELEASE( pOrigRT );
    SAFE_RELEASE( pOrigDS );
    m_pLightMapESRV->SetResource(m_pLightMapSRV);
}

void Terrain::CreateBuffers( float a_fSize )
{
    /* Initializing vertices; HeightMap dimension equal to 256 */
    // a number of vertices on one side
    UINT  uSideCount     = 256 + 1;
    UINT  uVerticesCount = uSideCount * uSideCount;
    m_uIndicesCount  = (uSideCount - 1) * (uSideCount - 1) * 6;
    m_fCellSize = 2.0f * a_fSize / (float)(uSideCount - 1);
    UINT i, j;

    D3DXVECTOR3 vStartPos(-a_fSize, 0.0f, -a_fSize);
    TerrainVertex *pVertices = new TerrainVertex[uVerticesCount];
    UINT *pIndices = new UINT[m_uIndicesCount];
    UINT uStartInd = 0;
    for (i = 0; i < uSideCount; i++)
        for (j = 0; j < uSideCount; j++)
        {      
            pVertices[i * uSideCount + j].vPos      = vStartPos + D3DXVECTOR3(m_fCellSize * i, 0.0f, m_fCellSize * j);
            pVertices[i * uSideCount + j].vTexCoord = D3DXVECTOR2((float)i / (float)(uSideCount - 1), (float)j / (float)(uSideCount - 1));
        }

    for (i = 0; i < uSideCount - 1; i++)
        for (j = 0; j < uSideCount - 1; j++)
        {
           
            pIndices[uStartInd] = i * uSideCount + j;
            uStartInd++;
            pIndices[uStartInd] = i * uSideCount + j + 1;
            uStartInd++;
            pIndices[uStartInd] = (i + 1) * uSideCount + j;
            uStartInd++;
           
            pIndices[uStartInd] = (i + 1) * uSideCount + j;
            uStartInd++;
            pIndices[uStartInd] = i * uSideCount + j + 1;
            uStartInd++;
            pIndices[uStartInd] = (i + 1) * uSideCount + j + 1;
            uStartInd++;
        }    

    /* Initializing vertex buffer */
    D3D10_BUFFER_DESC VBufferDesc = 
    {
        uVerticesCount * sizeof(TerrainVertex),
        D3D10_USAGE_DEFAULT,
        D3D10_BIND_VERTEX_BUFFER,
        0, 0
    };
    D3D10_SUBRESOURCE_DATA VBufferInitData;
    VBufferInitData.pSysMem = pVertices;
    m_pD3DDevice->CreateBuffer(&VBufferDesc, &VBufferInitData, &m_pVertexBuffer);
    delete [] pVertices;
    /* Initializing index buffer */
    D3D10_BUFFER_DESC IBufferDesc = 
    {
        m_uIndicesCount * sizeof(UINT),
        D3D10_USAGE_DEFAULT,
        D3D10_BIND_INDEX_BUFFER,
        0, 0
    };
    D3D10_SUBRESOURCE_DATA IBufferInitData;
    IBufferInitData.pSysMem = pIndices;
    m_pD3DDevice->CreateBuffer(&IBufferDesc, &IBufferInitData, &m_pIndexBuffer);
    delete [] pIndices;

    /* Initializing vertices */
    TerrainVertex Vertices[4];
    Vertices[0].vPos = D3DXVECTOR3(-1.0f, -1.0f, 0.1f);
    Vertices[0].vTexCoord = D3DXVECTOR2(0.0f, 1.0f);

    Vertices[1].vPos = D3DXVECTOR3(1.0f, -1.0f, 0.1f);
    Vertices[1].vTexCoord = D3DXVECTOR2(1.0f, 1.0f);

    Vertices[2].vPos = D3DXVECTOR3(-1.0f, 1.0f, 0.1f);
    Vertices[2].vTexCoord = D3DXVECTOR2(0.0f, 0.0f);

    Vertices[3].vPos = D3DXVECTOR3(1.0f, 1.0f, 0.1f);
    Vertices[3].vTexCoord = D3DXVECTOR2(1.0f, 0.0f);
    /* Initializing buffer */
    D3D10_BUFFER_DESC BufferDesc = 
    {
        4 * sizeof(TerrainVertex),
        D3D10_USAGE_DEFAULT,
        D3D10_BIND_VERTEX_BUFFER,
        0, 0
    };
    D3D10_SUBRESOURCE_DATA BufferInitData;
    BufferInitData.pSysMem = Vertices;
    m_pD3DDevice->CreateBuffer(&BufferDesc, &BufferInitData, &m_pQuadVertexBuffer);
    
}

void Terrain::CreateInputLayout( )
{
    D3D10_INPUT_ELEMENT_DESC InputDesc[] = 
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D10_INPUT_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0}
    };
    D3D10_PASS_DESC PassDesc;
    m_pPass->GetDesc(&PassDesc);
    int InputElementsCount = sizeof(InputDesc) / sizeof(D3D10_INPUT_ELEMENT_DESC);
    m_pD3DDevice->CreateInputLayout(InputDesc, InputElementsCount, 
        PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, 
        &m_pInputLayout);   
}

ID3D10ShaderResourceView *Terrain::HeightMapSRV( )
{
    return m_pHeightMapSRV;
}

ID3D10ShaderResourceView *Terrain::LightMapSRV( )
{
    return m_pLightMapSRV;
}

void Terrain::Render( )
{
    if (GetGlobalStateManager().UseWireframe())
        GetGlobalStateManager().SetRasterizerState("EnableMSAACulling_Wire");
    else
        GetGlobalStateManager().SetRasterizerState("EnableMSAACulling");

    m_pD3DDevice->IASetInputLayout(m_pInputLayout);
    m_pD3DDevice->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
    m_pD3DDevice->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    m_pD3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pPass->Apply(0);
    m_pD3DDevice->DrawIndexed(m_uIndicesCount, 0, 0);
}