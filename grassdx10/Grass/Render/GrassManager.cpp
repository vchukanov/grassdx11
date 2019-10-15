#include "GrassManager.h"
#include "StateManager.h"

bool GrassInitState::ReadFromFile( const char *a_pFileName )
{
    ifstream ioInput;
    ioInput.open(a_pFileName);
    if ( !ioInput.is_open() )
        return false;
    ioInput >> dwBladesPerPatchSide;
    ioInput >> dwPatchesPerSide;
    ioInput >> fMostDetailedDist;
    ioInput >> fLastDetailedDist;
	ioInput >> fGrassRadius;    
    ioInput.close();
    return true;
}

void GrassManager::LoadIndexData( )
{
    /* Creating texture for binding to shader */
    ID3D10Resource *pRes = NULL;
    m_pIndexMapData.pData = NULL;
    m_pIndexTexSRV = NULL;
    ID3D10Texture2D *pStagingTex;
    ID3D10Texture2D *pSRTex;
    HRESULT hr;
    D3DX10_IMAGE_LOAD_INFO loadInfo;
    ZeroMemory(&loadInfo, sizeof(loadInfo));
    loadInfo.BindFlags = D3D10_BIND_SHADER_RESOURCE;
    loadInfo.Format    = DXGI_FORMAT_R8_UNORM;
    loadInfo.MipLevels = 1;
    hr = D3DX10CreateTextureFromFile(m_GrassState.pD3DDevice, m_GrassState.sIndexTexPath.c_str(), &loadInfo, 0, &pRes, 0);
    if (hr != S_OK)
        return;
    D3D10_TEXTURE2D_DESC SRTexDesc;
    D3D10_SHADER_RESOURCE_VIEW_DESC IndexMapSRVDesc;
    pRes->QueryInterface( __uuidof( ID3D10Texture2D ), (LPVOID*)&pSRTex );
    pSRTex->GetDesc(&SRTexDesc);
    ZeroMemory( &IndexMapSRVDesc, sizeof(IndexMapSRVDesc) );
    IndexMapSRVDesc.Format = SRTexDesc.Format;
    IndexMapSRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
    IndexMapSRVDesc.Texture2D.MostDetailedMip = 0;
    IndexMapSRVDesc.Texture2D.MipLevels = 1;
    V(m_GrassState.pD3DDevice->CreateShaderResourceView(pSRTex, &IndexMapSRVDesc, &m_pIndexTexSRV));
    //pESRV->SetResource(m_pHeightMapSRV);
    /* Creating texture for reading on CPU */
    SRTexDesc.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
    SRTexDesc.BindFlags      = 0;
    SRTexDesc.Usage          = D3D10_USAGE_STAGING;
    V(m_GrassState.pD3DDevice->CreateTexture2D(&SRTexDesc, 0, &pStagingTex));
    /* Copying data from one texture to another */
    m_GrassState.pD3DDevice->CopyResource(pStagingTex, pSRTex);
    D3D10_MAPPED_TEXTURE2D MappedTexture;
    pStagingTex->Map(D3D10CalcSubresource(0, 0, 1), D3D10_MAP_READ, 0, &MappedTexture);
    {
        UCHAR* pTexels = (UCHAR*)MappedTexture.pData;
        m_pIndexMapData.pData    = new UCHAR[SRTexDesc.Height * SRTexDesc.Width];
        m_pIndexMapData.uHeight  = SRTexDesc.Height;
        m_pIndexMapData.uWidth   = SRTexDesc.Width;
        
        for( UINT row = 0; row < SRTexDesc.Height; row++ )
        {
            UINT rowStart = row * MappedTexture.RowPitch;
            for( UINT col = 0; col < SRTexDesc.Width; col++ )
            {
                UINT colStart = col;// * 3;//RGB
                m_pIndexMapData.pData[row * SRTexDesc.Width + col] = UCHAR((float)pTexels[rowStart + colStart] / 255.f * 9.0f);
                /*pData[row * a_TexDesc.Width + col].y = (float)pTexels[rowStart + colStart + 1] / 255.0f;
                pData[row * a_TexDesc.Width + col].z = (float)pTexels[rowStart + colStart + 2] / 255.0f;*/
            }
        }
    }
    pStagingTex->Unmap(D3D10CalcSubresource(0, 0, 1));
    SAFE_RELEASE(pRes);
    SAFE_RELEASE(pSRTex);
    SAFE_RELEASE(pStagingTex);
}

GrassManager::GrassManager( GrassInitState &a_pInitState, GrassTracker *a_pGrassTracker )
{
    m_GrassState = a_pInitState;
    ID3D10Blob *pErrors;
    m_pEffect           = NULL;
    m_pDiffuseTexSRV    = NULL;
    m_pDiffuseTex       = NULL;
    m_pTopDiffuseTexSRV = NULL;
    m_pTopDiffuseTex    = NULL;
    m_pHeightData       = NULL;
    m_pWindData         = NULL;
    m_pRotatePattern    = NULL;
    m_pLowGrassTexSRV   = NULL;
    m_fHeightScale      = 0.0f;
    m_pGrassTracker     = a_pGrassTracker;
    HRESULT hr;
    
    
    D3DXLoadTextureArray(m_GrassState.pD3DDevice, m_GrassState.sTexPaths, &m_pDiffuseTex, &m_pDiffuseTexSRV);
    D3DXLoadTextureArray(m_GrassState.pD3DDevice, m_GrassState.sTopTexPaths, &m_pTopDiffuseTex, &m_pTopDiffuseTexSRV);
    hr = D3DX10CreateShaderResourceViewFromFileW(m_GrassState.pD3DDevice, m_GrassState.sLowGrassTexPath.c_str(), NULL, NULL, &m_pLowGrassTexSRV, NULL);
    m_bUseLowGrass = (hr == S_OK);
    //D3DX10CreateShaderResourceViewFromFile(m_GrassState.pD3DDevice, m_GrassState.sIndexTexPath.c_str(), NULL, NULL, &m_pIndexTexSRV, NULL);
    LoadIndexData( );
    D3DX10CreateEffectFromFile(a_pInitState.sEffectPath.c_str(), NULL, &g_D3D10Include, 
        "fx_4_0", D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_DEBUG,
        0, m_GrassState.pD3DDevice,
        NULL, NULL,
        &m_pEffect,
        &pErrors, NULL);
    char *errStr;
    if (pErrors)
    {
        errStr = static_cast<char*>(pErrors->GetBufferPointer());
    }

    const D3D10_INPUT_ELEMENT_DESC TransformLayout[] =
    {
        { "POSITION"     , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0 , D3D10_INPUT_PER_VERTEX_DATA  , 0 },
        { "TEXCOORD"     , 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 12, D3D10_INPUT_PER_VERTEX_DATA  , 0 },
        { "TEXCOORD"     , 1, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 24, D3D10_INPUT_PER_VERTEX_DATA  , 0 },
        { "TEXCOORD"     , 2, DXGI_FORMAT_R32G32B32_FLOAT   , 0, 36, D3D10_INPUT_PER_VERTEX_DATA  , 0 },
        { "TRANSPARENCY" , 0, DXGI_FORMAT_R32_FLOAT         , 0, 48, D3D10_INPUT_PER_VERTEX_DATA  , 0 },
        { "mTransform"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0 , D3D10_INPUT_PER_INSTANCE_DATA, 1 },
        { "mTransform"   , 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D10_INPUT_PER_INSTANCE_DATA, 1 },
        { "mTransform"   , 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D10_INPUT_PER_INSTANCE_DATA, 1 },
        { "mTransform"   , 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D10_INPUT_PER_INSTANCE_DATA, 1 },
        //{ "uOnEdge"      , 0, DXGI_FORMAT_R32_UINT          , 1, 64, D3D10_INPUT_PER_INSTANCE_DATA, 1 }
    };
    int iNumElements = sizeof( TransformLayout ) / sizeof( D3D10_INPUT_ELEMENT_DESC );

    ID3D10EffectTechnique *pRenderLod0Technique = m_pEffect->GetTechniqueByName("RenderGrass"); 
    m_pRenderPass   = pRenderLod0Technique->GetPassByName("RenderLod0");
    m_pShadowPass   = pRenderLod0Technique->GetPassByName("ShadowPass");
	m_pLowGrassDiffuseEVV = m_pEffect->GetVariableByName("g_vLowGrassDiffuse")->AsVector();
	if (!m_pLowGrassDiffuseEVV->IsValid())
		m_pLowGrassDiffuseEVV = NULL;
    if (m_bUseLowGrass)
    {
        m_pLowGrassPass = m_pEffect->GetTechniqueByName("RenderLowGrass")->GetPassByIndex(0);
        m_pLowGrassDiffuseEVV = m_pEffect->GetVariableByName("g_vLowGrassDiffuse")->AsVector();
    }
    else
        m_pLowGrassPass = NULL;

    D3D10_PASS_DESC RenderLod0PassDesc;
    m_pRenderPass->GetDesc(&RenderLod0PassDesc);
    m_GrassState.pD3DDevice->CreateInputLayout(TransformLayout, iNumElements, 
        RenderLod0PassDesc.pIAInputSignature, RenderLod0PassDesc.IAInputSignatureSize, 
        &m_pInputLayout);
    ID3D10EffectShaderResourceVariable *pESRV = m_pEffect->GetVariableByName("g_txGrassDiffuseArray")->AsShaderResource();
    pESRV->SetResource(m_pDiffuseTexSRV);       
    pESRV = m_pEffect->GetVariableByName("g_txLowGrassDiffuse")->AsShaderResource();
    pESRV->SetResource(m_pLowGrassTexSRV);
    pESRV = m_pEffect->GetVariableByName("g_txIndexMap")->AsShaderResource();
    pESRV->SetResource(m_pIndexTexSRV);
    /* if tops needed, then load them */
    if (m_pTopDiffuseTexSRV)
    {
        pESRV = m_pEffect->GetVariableByName("g_txTopDiffuseArray")->AsShaderResource();
        pESRV->SetResource(m_pTopDiffuseTexSRV);
    }
    m_pGrassLodBiasESV          = m_pEffect->GetVariableByName("g_fGrassLodBias")->AsScalar();
    m_pGrassSubScatterGammaESV  = m_pEffect->GetVariableByName("g_fGrassSubScatterGamma")->AsScalar();
    m_pGrassAmbientESV          = m_pEffect->GetVariableByName("g_fGrassAmbient")->AsScalar();
    m_pWindStrengthESV          = m_pEffect->GetVariableByName("g_fWindStrength")->AsScalar();
    /* Initializing Lod ESVs */
    m_pMostDetailedDistESV      = m_pEffect->GetVariableByName("g_fMostDetailedDist")->AsScalar();
    m_pLastDetailedDistESV      = m_pEffect->GetVariableByName("g_fLastDetailedDist")->AsScalar();    
    m_pGrassRadiusESV           = m_pEffect->GetVariableByName("g_fGrassRadius")->AsScalar();
    m_pQualityESV               = m_pEffect->GetVariableByName("g_fQuality")->AsScalar();
    m_pTrackViewProjEMV         = m_pEffect->GetVariableByName("g_mTrackViewProj")->AsMatrix();
    m_pTrackMapSRV              = m_pEffect->GetVariableByName("g_txTrackMap")->AsShaderResource();
    m_pTrackMapSRV->SetResource(m_pGrassTracker->GetTrackSRV());
    ID3D10EffectScalarVariable *pESV = m_pEffect->GetVariableByName("g_fMaxQuality")->AsScalar();
    pESV->SetFloat(m_GrassState.fMaxQuality);

    /*************************/
    Init();
}

void GrassManager::ReInitLods()
{    
    /*float b = m_fGrassLodBias;
    
    float fLerpCoef = (0.35f - b) / (0.5f - b);
    fLerpCoef *= fLerpCoef;
    m_fGrassLod0Dist = fLerpCoef * (m_GrassState.fLastDetailedDist - m_GrassState.fMostDetailedDist) + m_GrassState.fMostDetailedDist;
    m_fGrassLod1Dist = m_GrassState.fLastDetailedDist;
    DWORD dwCount[GrassLodsCount] = {
        4 * (DWORD)( (m_fGrassLod0Dist * m_fGrassLod0Dist) / (m_fPatchSize * m_fPatchSize) ),
        4 * (DWORD)( (m_fGrassLod1Dist * m_fGrassLod1Dist) / (m_fPatchSize * m_fPatchSize) ),
        m_GrassState.dwPatchesPerSide * m_GrassState.dwPatchesPerSide
    };*/
	DWORD dwCount = m_GrassState.dwPatchesPerSide * m_GrassState.dwPatchesPerSide;
    m_GrassLod[0]->SetTransformsCount(dwCount);//WARNING!!: too big pool 
    m_GrassLod[1]->SetTransformsCount(dwCount);
    m_GrassLod[2]->SetTransformsCount(dwCount);
    /*ID3D10EffectScalarVariable *pESV = m_pEffect->GetVariableByName("g_fGrassLod0Dist")->AsScalar();
    pESV->SetFloat(m_fGrassLod0Dist);*/

    for (DWORD i = 0; i < GrassLodsCount; ++i)
        m_GrassLod[i]->GenTransformBuffer();
}

void GrassManager::Init( )
{
    /* initializing lod0 */
    m_fPatchSize = m_GrassState.fGrassRadius * 2.0f / m_GrassState.dwPatchesPerSide;
    /* These patches will be destroyed inside ~GrassLod() */
    GrassPatchLod0 *pGrassPatchLod0 = new GrassPatchLod0(m_GrassState.pD3DDevice,  
        m_fPatchSize, 
        m_GrassState.dwBladesPerPatchSide);    
    GrassPatch *pGrassPatchLod1 = new GrassPatchLod1(pGrassPatchLod0);
    GrassPatch *pGrassPatchLod2 = new GrassPatchLod2(pGrassPatchLod1);
    m_GrassLod[0] = new GrassLod(pGrassPatchLod0);
    m_GrassLod[1] = new GrassLod(pGrassPatchLod1);    
    m_GrassLod[2] = new GrassLod(pGrassPatchLod2);
    ReInitLods();
    /* pools for physics */
//    m_GrassPool[0] = new GrassPool(m_GrassState.pD3DDevice, m_pInputLayout, m_pEffect, pGrassPatchLod0, 25, m_bUseLowGrass);
    m_GrassPool[0] = new GrassPool(m_GrassState.pD3DDevice, m_pInputLayout, m_pEffect, pGrassPatchLod0, 35, m_bUseLowGrass);
    //m_GrassPool[1] = new GrassPool(m_GrassState.pD3DDevice, m_pEffect, pGrassPatchLod1, 100);
    //PhysPatch::fGrassRadius = m_GrassState.fGrassRadius;
    PhysPatch::fTerrRadius = m_GrassState.fTerrRadius;

    /* patch orientation pattern */
	m_uNumPatchesPerTerrSide = UINT(m_GrassState.fTerrRadius / m_fPatchSize);
    DWORD dwPatchCount = m_uNumPatchesPerTerrSide * m_uNumPatchesPerTerrSide;
    if (m_pRotatePattern)
        delete [] m_pRotatePattern;
    m_pRotatePattern = new DWORD[dwPatchCount];
    for (DWORD i = 0; i < dwPatchCount; i++)
    {
        m_pRotatePattern[i] = rand() % 4;
    }

    m_pMostDetailedDistESV->SetFloat(m_GrassState.fMostDetailedDist);
    m_pLastDetailedDistESV->SetFloat(m_GrassState.fLastDetailedDist);
    m_pGrassRadiusESV->SetFloat(m_GrassState.fGrassRadius);
    /*m_pGrassCollideStatic = new GrassCollideStatic(*pGrassPatchLod1, m_pEffect);
    m_pGrassCollideStatic->SetNumInstanses(m_GrassState.uNumCollidedPatchesPerMesh * m_GrassState.uMaxColliders);
    m_pGrassCollideStatic->GenTransformBuffer();*/
}

void GrassManager::Reinit( GrassInitState &a_pInitState )
{
    /* Releasing memory */
    for (DWORD i = 0; i < GrassLodsCount; ++i)
    {
        delete m_GrassLod[i];
    }
    delete m_GrassPool[0];
    //delete m_GrassPool[1];
    //delete m_pGrassCollideStatic;
    m_GrassState = a_pInitState;
    Init();
}

void GrassManager::SetGrassLodBias( float a_fGrassLodBias )
{
    m_fGrassLodBias = a_fGrassLodBias;
    m_pGrassLodBiasESV->SetFloat(m_fGrassLodBias);
    /* rebuilding lods using new bias value */
    GrassInitState GrassState = m_GrassState;
    Reinit(GrassState);
}

void GrassManager::SetSubScatterGamma( float a_fGrassSubScatterGamma )
{
    m_fGrassSubScatterGamma = a_fGrassSubScatterGamma;
    m_pGrassSubScatterGammaESV->SetFloat(m_fGrassSubScatterGamma);
}

void GrassManager::SetQuality( float a_fQuality )
{
    m_pQualityESV->SetFloat(a_fQuality);
    m_fQuality = a_fQuality;
}

void GrassManager::SetGrassAmbient( float a_fGrassAmbient )
{
    m_fGrassAmbient = a_fGrassAmbient;
    m_pGrassAmbientESV->SetFloat(m_fGrassAmbient);
}

void GrassManager::SetLowGrassDiffuse( D3DXVECTOR4 &a_vValue )
{
    if (m_pLowGrassDiffuseEVV)
        m_pLowGrassDiffuseEVV->SetFloatVector((float*)&a_vValue);
}

void GrassManager::SetWindStrength( float a_fWindStrength )
{
    m_fWindStrength = a_fWindStrength;
    PhysPatch::windStrength = a_fWindStrength;
    m_pWindStrengthESV->SetFloat(m_fWindStrength);
}

void GrassManager::SetHeightDataPtr( const TerrainHeightData* a_pHeightData )
{
    m_pHeightData = a_pHeightData;
	PhysPatch::pHeightData = a_pHeightData;
}

void GrassManager::SetWindDataPtr( const WindData *a_pWindData )
{
    m_pWindData = a_pWindData;
    PhysPatch::pWindData = m_pWindData;
}

void GrassManager::SetHeightScale( float a_fHeightScale )
{
    m_fHeightScale = a_fHeightScale;
}

void GrassManager::Render( bool a_bShadowPass )
{   
    if (GetGlobalStateManager().UseWireframe())
        GetGlobalStateManager().SetRasterizerState("EnableMSAA_Wire");
    else
        GetGlobalStateManager().SetRasterizerState("EnableMSAA");

    //m_GrassPool[1]->Render(a_bShadowPass);
    //m_pGrassCollideStatic->Render();
    /* Grass lods rendering */
    m_GrassState.pD3DDevice->IASetInputLayout(m_pInputLayout);
    m_GrassState.pD3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
	if (a_bShadowPass)
    {
        m_pShadowPass->Apply(0);
        m_GrassLod[0]->IASetVertexBuffers();        
        m_GrassState.pD3DDevice->DrawInstanced(m_GrassLod[0]->VerticesCount(), m_GrassLod[0]->GetTransformsCount(), 0, 0);
        if (m_bUseLowGrass)
        {
            m_pLowGrassPass->Apply(0);
            m_GrassState.pD3DDevice->DrawInstanced(m_GrassLod[0]->VerticesCount(), m_GrassLod[0]->GetTransformsCount(), 0, 0);
        }        
    }
    else
    {
        m_GrassLod[0]->IASetVertexBuffers();        
        if (m_bUseLowGrass)
        {
            m_pLowGrassPass->Apply(0);
            m_GrassState.pD3DDevice->DrawInstanced(m_GrassLod[0]->VerticesCount(), m_GrassLod[0]->GetTransformsCount(), 0, 0);
        }

        m_pRenderPass->Apply(0);
        //m_GrassLod[0]->IASetVertexBuffers();

        m_GrassState.pD3DDevice->DrawInstanced(m_GrassLod[0]->VerticesCount(), m_GrassLod[0]->GetTransformsCount(), 0, 0);
        m_GrassLod[1]->IASetVertexBuffers();        
        m_GrassState.pD3DDevice->DrawInstanced(m_GrassLod[1]->VerticesCount(), m_GrassLod[1]->GetTransformsCount(), 0, 0);
        m_GrassLod[2]->IASetVertexBuffers();        
        m_GrassState.pD3DDevice->DrawInstanced(m_GrassLod[2]->VerticesCount(), m_GrassLod[2]->GetTransformsCount(), 0, 0);
    }

    /* Grass physics rendering */
    m_GrassPool[0]->Render(a_bShadowPass);
}

bool GrassManager::IsPatchVisible(ConvexVolume &a_cvFrustum, D3DXVECTOR3 &a_vPatchPos)
{
    static AABB AABbox;
    //static float fHalfSize = m_fPatchSize * 0.5f;
    AABbox.Set(a_vPatchPos.x - m_fPatchSize, a_vPatchPos.x + m_fPatchSize, 
               a_vPatchPos.y - m_fPatchSize * 2.0f, a_vPatchPos.y + m_fPatchSize * 2.0f,
               a_vPatchPos.z - m_fPatchSize, a_vPatchPos.z + m_fPatchSize);
    return a_cvFrustum.IntersectBox(AABbox);
}  

float GrassManager::GetPatchHeight( UINT a_uX, UINT a_uY )
{
    if (m_pHeightData == NULL)
        return 0.0f;

    /* Getting UV-coordinates of patch mid-point (a_vPatchPos) */
    /*UINT uX, uY;
    uX = (UINT)( ((a_vPatchPos.x / m_GrassState.fGrassRadius) * 0.5 + 0.5) * m_pHeightData->fWidth);
    uY = (UINT)( ((a_vPatchPos.z / m_GrassState.fGrassRadius) * 0.5 + 0.5) * m_pHeightData->fHeight);*/
    return m_pHeightData->pData[m_pHeightData->uWidth * a_uY + a_uX] * m_fHeightScale;    
}

float GrassManager::LodAlphaOffset( const D3DXVECTOR3 &a_vCamPos, const D3DXVECTOR3 &a_vPatchPos, const float a_fDist, const float a_fIsCorner )
{
//	if (a_fDist<30.0) return 1.0;
//	else return 1.0;
	float fLerpCoef1 = (a_fDist+0.1f) /m_GrassState.fGrassRadius;
	fLerpCoef1 *= fLerpCoef1; 
    UINT uX = (UINT)( ((a_vPatchPos.x / m_GrassState.fTerrRadius) * 0.5f + 0.5f) * m_pHeightData->fWidth);
    UINT uY = (UINT)( ((a_vPatchPos.z / m_GrassState.fTerrRadius) * 0.5f + 0.5f) * m_pHeightData->fHeight);
    D3DXVECTOR3 vNormal = m_pHeightData->pNormals[m_pHeightData->uWidth * uY + uX];
    D3DXVECTOR3 vV = a_vCamPos - a_vPatchPos;
	float tmp = 0.43f; //= 0.44 + 0.1 * vV.y/(5.0 + abs(vV.y));
	if (vV.y<0.0f) tmp += 0.1f * vV.y/(5.0f + abs(vV.y));
//	float tmp = 0.44 + 0.1 * vV.y/(5.0 + fabs(vV.y));
    D3DXVec3Normalize(&vV, &vV);
    float fDot = D3DXVec3Dot( &vV, &vNormal );
	tmp = tmp - fDot;
	float t = 1.f - fDot;
	float h = a_vCamPos.y;
	if (h < 17.0f)  h= 0.0f;
	else
	{
		h = (h-17.0f)/50.0f;
//		h = h*h;
	}
	if ((a_vPatchPos.y > 6.0f)&&(t > 0.92f)) return (0.2f + h);
	else return (tmp *(1.0f+3.0f*fLerpCoef1) + h);
//	return tmp;
}

void GrassManager::Update( D3DXMATRIX &a_mViewProj, D3DXVECTOR3 a_vCamPos, Mesh *a_pMeshes[], UINT a_uNumMeshes, float a_fElapsedTime )
{
	float physLodDst = m_GrassState.fGrassRadius * 0.2f;
    float fMaxDist = m_GrassState.fGrassRadius;
    static ConvexVolume cvFrustum;
    cvFrustum.BuildFrustum(a_mViewProj);
    float fHalfSize = m_fPatchSize * 0.5f;
	
	float fCamOffsetX = INT(a_vCamPos.x / m_fPatchSize) * m_fPatchSize;
	float fCamOffsetZ = INT(a_vCamPos.z / m_fPatchSize) * m_fPatchSize;
    D3DXVECTOR3 vPatchPos(-fMaxDist + fHalfSize + fCamOffsetX, 0.0f, -fMaxDist + fHalfSize + fCamOffsetZ);
    D3DXVECTOR3 vNormal;
    //float       fLodFunc;
    //UINT uCornersX[4];
    //UINT uCornersY[4];
    UINT uX = 0, uY = 0;
	float fX, fY;
    /*UINT uDX = 0, uDY = 0;
    uDX = (UINT)( ((m_fPatchSize / m_GrassState.fTerrRadius) * 0.25f) * m_pHeightData->fWidth);
    uDY = (UINT)( ((m_fPatchSize / m_GrassState.fTerrRadius) * 0.25f) * m_pHeightData->fHeight);*/
	float fDX, fDY;
	fDX = (m_fPatchSize / m_GrassState.fTerrRadius) * 0.25f;
    fDY = (m_fPatchSize / m_GrassState.fTerrRadius) * 0.25f;
    int iDotSign[2] = {-1, 1};
    bool bOnEdge = false;
    D3DXVECTOR3 vDist;
    

    D3DXVECTOR3 vSpherePos;
    float fRadius;
    float fDist;
    DWORD i = 0;
    DWORD j = 0;
    DWORD k = 0; // :) 
    DWORD dwRotMtxIndex = 0;
    INT32 dwLodIndex;
    DWORD dwStaticInstanseInd = -1;
    bool  bCollided = false;
    DWORD dwLodTransformsIndex[GrassLodsCount] = {-1, -1, -1};
    /* Patch orientation matrices */
    D3DXMATRIX mRotationMatrices[4];
    D3DXMATRIX mTranslateMtx;
    D3DXMATRIX mCurTransform;
    float fAngle = 0.0f;
	
    /* setting up maximum capacity */   
    DWORD dwCount = m_GrassState.dwPatchesPerSide * m_GrassState.dwPatchesPerSide;
    m_GrassLod[0]->SetTransformsCount(dwCount);
    m_GrassLod[1]->SetTransformsCount(dwCount);
    m_GrassLod[2]->SetTransformsCount(dwCount);
    /* clear dead patches */
    m_GrassPool[0]->ClearDeadPatches(a_fElapsedTime);
    D3DXVECTOR3 vCornerNormals[4];

    for (i = 0; i < 4; ++i)
    {
        D3DXMatrixRotationY(&mRotationMatrices[i], fAngle);
        fAngle += D3DX_PI / 2.0f;
    }

    for (i = 0; i < m_GrassState.dwPatchesPerSide; ++i)
    {
        vPatchPos.z = -fMaxDist + fHalfSize + fCamOffsetZ;
		if (vPatchPos.x > m_GrassState.fTerrRadius)
		{
			break;
		}
		if (-vPatchPos.x > m_GrassState.fTerrRadius)
		{
			vPatchPos.x += m_fPatchSize;
			continue;
		}
        for (j = 0; j < m_GrassState.dwPatchesPerSide; ++j)
        {
			if (vPatchPos.z > m_GrassState.fTerrRadius)
				break;
			if (-vPatchPos.z > m_GrassState.fTerrRadius)
			{
				vPatchPos.z += m_fPatchSize;
				continue;
			}

            if (!IsPatchVisible(cvFrustum, vPatchPos))
            {
				int ind = m_GrassPool[0]->GetPatchIndex(vPatchPos);
				if (ind != NO_VALUE)
					//m_GrassPool[0]->SetPatchVisibility(ind, false);
                    m_GrassPool[0]->FreePatch(ind);
				
                vPatchPos.z += m_fPatchSize;
                continue;
            }
			/* Height map coords */
			fX = (vPatchPos.x / m_GrassState.fTerrRadius) * 0.5f + 0.5f;
			fY = (vPatchPos.z / m_GrassState.fTerrRadius) * 0.5f + 0.5f;
			uX = (UINT)( (fX) * m_pHeightData->fWidth);
			uY = (UINT)( (fY) * m_pHeightData->fHeight);
            
			vPatchPos.y = m_pHeightData->pData[m_pHeightData->uWidth * uY + uX] * m_fHeightScale;			
            vDist = a_vCamPos - vPatchPos;
            fDist = D3DXVec3Length(&vDist);

            /* uX +/- uDX, uY +/- uDY -- patch corners */
            
            /* Corner normals 
			vCornerNormals[0] = m_pHeightData->GetNormal(fX+fDX, fY+fDY);//m_pHeightData->pNormals[m_pHeightData->uWidth * (uY+uDY) + (uX+uDX)];
            vCornerNormals[1] = m_pHeightData->GetNormal(fX-fDX, fY+fDY);//m_pHeightData->pNormals[m_pHeightData->uWidth * (uY-uDY) + (uX+uDX)];
            vCornerNormals[2] = m_pHeightData->GetNormal(fX-fDX, fY-fDY);//m_pHeightData->pNormals[m_pHeightData->uWidth * (uY-uDY) + (uX-uDX)];
            vCornerNormals[3] = m_pHeightData->GetNormal(fX+fDX, fY-fDY);//m_pHeightData->pNormals[m_pHeightData->uWidth * (uY+uDY) + (uX-uDX)];
            iDotSign[0] = -1;
            iDotSign[1] =  1;
            for (int iInd = 0; iInd < 4; iInd++)
            {
                if (D3DXVec3Dot(vCornerNormals + iInd, &vDist) > 0.0f)
                {
                    if (iDotSign[0] == -1)
                        iDotSign[0] = 1;
                }
                else
                    if (iDotSign[1] == 1)
                        iDotSign[1] = -1;
                
            }            
            bOnEdge = (iDotSign[0] * iDotSign[1] < 0.0f);*/

            /* getting patch indices in corresponding pools */            
            //dwRotMtxIndex = m_pRotatePattern[i * m_GrassState.dwPatchesPerSide + j];
			uX = (UINT)( ((vPatchPos.x / m_GrassState.fTerrRadius) * 0.5f + 0.5f) * (m_uNumPatchesPerTerrSide - 1));
			uY = (UINT)( ((vPatchPos.z / m_GrassState.fTerrRadius) * 0.5f + 0.5f) * (m_uNumPatchesPerTerrSide - 1));
			dwRotMtxIndex = m_pRotatePattern[uY * m_uNumPatchesPerTerrSide + uX];
            /* Calculating matrix */
            D3DXMatrixTranslation(&mTranslateMtx, 
                vPatchPos.x, 0.0f, vPatchPos.z);// Position on plane, not on a terrain!
            D3DXMatrixMultiply(&mCurTransform, 
                &mRotationMatrices[dwRotMtxIndex], &mTranslateMtx);            
            /* Lods */
			dwLodIndex = 0;
            fDist -= fHalfSize;//farthest patch point
			float fAlphaOffs = LodAlphaOffset(a_vCamPos, vPatchPos, fDist, bOnEdge);
            if (fAlphaOffs > 0.34f)
                dwLodIndex++;
            if (fAlphaOffs > 0.66f)
                dwLodIndex++;

            /* Determining collision */
            vPatchPos.y = 0.0f; //All collisions ON A PLANE, NOT ON A TERRAIN
            int iLodPatchInd = m_GrassPool[0]->GetPatchIndex(vPatchPos);
           /* if (dwLodIndex > 0)
            {
                m_GrassPool[0]->FreePatch(iLodPatchInd);
            }*/

            /* Determining collision */
            //if (dwLodIndex < 1)// < 2
            if ( fDist < physLodDst )
            {
                for (k = 0; k < a_uNumMeshes; k++)
                {
                    vSpherePos = D3DXVECTOR3(a_pMeshes[k]->GetPosAndRadius().x, 0.0f, a_pMeshes[k]->GetPosAndRadius().z);
                    
                    fRadius = a_pMeshes[k]->GetPosAndRadius().w;
                    vDist = vPatchPos - vSpherePos;
                    fDist = D3DXVec3Length(&vDist);
                    if (fDist < fRadius*1.1f + m_fPatchSize / 2.0f)
                    {
                        bCollided = true;
                        m_GrassPool[0]->TakePatch(mCurTransform, PhysPatch::maxBrokenTime*4.0f, k);
                        iLodPatchInd = m_GrassPool[0]->GetPatchIndex(vPatchPos);
                        if (iLodPatchInd != NO_VALUE)
                          m_GrassPool[0]->SetPatchVisibility(iLodPatchInd, true);
                        break;
                    }
                }
            }
			
	        if (iLodPatchInd == NO_VALUE)
                m_GrassLod[dwLodIndex]->AddTransform(mCurTransform, fDist, bOnEdge, ++dwLodTransformsIndex[dwLodIndex]);
		
            /* Updating parameters */
            vPatchPos.z += m_fPatchSize;
        }
        vPatchPos.x += m_fPatchSize;
    }

    for (i = 0; i < GrassLodsCount; i++)
    {
        dwLodTransformsIndex[i]++;
        m_GrassLod[i]->SetTransformsCount(dwLodTransformsIndex[i]);
        m_GrassLod[i]->UpdateTransformBuffer();
    }
    /*dwStaticInstanseInd++;
    m_pGrassCollideStatic->SetNumInstanses(dwStaticInstanseInd);
    m_pGrassCollideStatic->UpdateTransformBuffer();*/

    /* updating physics */
    m_GrassPool[0]->Update(a_vCamPos, m_GrassState.fCameraMeshDist, a_fElapsedTime, a_pMeshes, a_uNumMeshes, m_SubTypeProps, m_pIndexMapData);
    //m_GrassPool[1]->Update(a_vCamPos, physLodDst, a_fElapsedTime, a_pMeshes, m_SubTypeProps, m_pIndexMapData);

    m_pTrackViewProjEMV->SetMatrix((float *)&m_pGrassTracker->GetViewProjMatrix());
}

ID3D10Effect *GrassManager::GetEffect( )
{
    return m_pEffect;
}

void GrassManager::AddSubType( const GrassPropsUnified &a_SubTypeData )
{
    m_SubTypeProps.push_back(a_SubTypeData);
}

void GrassManager::ClearSubTypes( )
{
    m_SubTypeProps.clear();
}

GrassManager::~GrassManager( )
{
    //delete [] m_pIndexMapData;
    delete [] m_pRotatePattern;
    for (DWORD i = 0; i < GrassLodsCount; ++i)
    {
        delete m_GrassLod[i];
    }
    delete m_GrassPool[0];
    //delete m_GrassPool[1];
    //delete m_pGrassCollideStatic;
    SAFE_RELEASE(m_pInputLayout);
    SAFE_RELEASE(m_pEffect);
    SAFE_RELEASE(m_pDiffuseTex);
    SAFE_RELEASE(m_pLowGrassTexSRV);
    SAFE_RELEASE(m_pDiffuseTexSRV);
    SAFE_RELEASE(m_pTopDiffuseTex);
    SAFE_RELEASE(m_pTopDiffuseTexSRV);
    //SAFE_RELEASE(m_pSeatingTexSRV);
    SAFE_RELEASE(m_pIndexTexSRV);
}

void GrassManager::ClearGrassPools( void )
{ 
  for (int i = 0; i < m_GrassPool[0]->GetPatchCount(); i++)
    m_GrassPool[0]->FreePatch(i);
}
