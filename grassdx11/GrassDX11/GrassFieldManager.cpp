#include "includes.h"

#include "GrassFieldManager.h"
#include "GrassTrack.h"

#include <DDSTextureLoader.h>


GrassFieldManager::GrassFieldManager (GrassFieldState &a_InitState)
{  
	char	 *errStr;
	ID3DBlob *pErrorBlob = NULL;

	HRESULT hr = D3DX11CompileEffectFromFile(a_InitState.sSceneEffectPath.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_SKIP_OPTIMIZATION, 0, a_InitState.InitState[0].pD3DDevice, &m_pSceneEffect, &pErrorBlob);

	if (pErrorBlob)
	{
		errStr = static_cast<char*>(pErrorBlob->GetBufferPointer());
	}

    /* Grass track unit */
    m_pGrassTracker = new GrassTracker(a_InitState.InitState[0].pD3DDevice, a_InitState.InitState[0].pD3DDeviceCtx);

    /* Grass managers */
	UINT i;

    a_InitState.InitState[0].fTerrRadius = a_InitState.fTerrRadius;
    a_InitState.InitState[1].fTerrRadius = a_InitState.fTerrRadius;
    a_InitState.InitState[2].fTerrRadius = a_InitState.fTerrRadius;


  #pragma omp parallel sections 
    {
        #pragma omp section
        {
            m_pGrassTypes[0] = new GrassManager(a_InitState.InitState[0], m_pGrassTracker);
        }
        
        #pragma omp section
        {
            m_pGrassTypes[2] = new GrassManager(a_InitState.InitState[2], m_pGrassTracker);
        }
    }
 
    /* Terrain and wind */
    m_pTerrain = new Terrain(a_InitState.InitState[0].pD3DDevice, a_InitState.InitState[0].pD3DDeviceCtx, m_pSceneEffect, a_InitState.fTerrRadius);
    m_pTerrain->BuildHeightMap(a_InitState.fHeightScale);
	m_fTerrRadius = a_InitState.fTerrRadius;
    
	m_pWind = new Wind(a_InitState.InitState[0].pD3DDevice, a_InitState.InitState[0].pD3DDeviceCtx);
    
    m_pGrassTypes[0]->SetHeightDataPtr(m_pTerrain->HeightDataPtr());
    m_pGrassTypes[0]->SetWindDataPtr(m_pWind->WindDataPtr());
    m_pGrassTypes[2]->SetHeightDataPtr(m_pTerrain->HeightDataPtr());
    m_pGrassTypes[2]->SetWindDataPtr(m_pWind->WindDataPtr());

    /* Shadow mapping unit */
       
	CreateDDSTextureFromFile(a_InitState.InitState[0].pD3DDevice, a_InitState.sNoiseMapPath.c_str(), nullptr, &m_pNoiseESV);
	CreateDDSTextureFromFile(a_InitState.InitState[0].pD3DDevice, a_InitState.sColorMapPath.c_str(), nullptr, &m_pGrassColorESV);
    
    /* ...and lots of variables... */
    ID3DX11EffectShaderResourceVariable *pESRV;
    ID3D11ShaderResourceView *pHeightMapSRV = m_pTerrain->HeightMapSRV();
    XMVECTOR vLightDir = create(-1.0f, -0.1f, 1.0f);

	for (int i = 0; i < GrassTypeNum; i++)
    {
        if (i == 1)
            continue;
        m_pViewProjEMV[i]    = m_pGrassTypes[i]->GetEffect()->GetVariableByName( "g_mViewProj" )->AsMatrix();
        m_pViewEMV[i]        = m_pGrassTypes[i]->GetEffect()->GetVariableByName( "g_mView" )->AsMatrix();
        m_pMeshesEVV[i]      = m_pGrassTypes[i]->GetEffect()->GetVariableByName( "g_vMeshSpheres" )->AsVector();
        m_pHardness[i]       = m_pGrassTypes[i]->GetEffect()->GetVariableByName( "g_fHardness" )->AsScalar();
        m_pSegMass[i]        = m_pGrassTypes[i]->GetEffect()->GetVariableByName( "g_fMass" )->AsScalar();
        m_pWindMapTile[i]    = m_pGrassTypes[i]->GetEffect()->GetVariableByName( "g_fWindTexTile" )->AsScalar();
        m_pHeightScale[i]    = m_pGrassTypes[i]->GetEffect()->GetVariableByName( "g_fHeightScale" )->AsScalar();
        m_pLightDirEVV[i]    = m_pGrassTypes[i]->GetEffect()->GetVariableByName( "vLightDir" )->AsVector();
        m_pLightDirEVV[i]->SetFloatVector((float*)&vLightDir);
        
        m_pShadowMapESRV[i]    = m_pGrassTypes[i]->GetEffect()->GetVariableByName( "g_txShadowMap" )->AsShaderResource();
        m_pLightViewProjEMV[i] = m_pGrassTypes[i]->GetEffect()->GetVariableByName( "g_mLightViewProj" )->AsMatrix();
        pESRV = m_pGrassTypes[i]->GetEffect()->GetVariableByName("g_txWindTex")->AsShaderResource();
        pESRV->SetResource(m_pWind->GetMap());
        /* Seating maps */
		CreateDDSTextureFromFile(a_InitState.InitState[0].pD3DDevice, a_InitState.InitState[i].sSeatingTexPath.c_str(), nullptr, &m_pSeatingMapESV[i]);

        pESRV = m_pGrassTypes[i]->GetEffect()->GetVariableByName("g_txSeatingMap")->AsShaderResource();
        pESRV->SetResource(m_pSeatingMapESV[i]);

        pESRV = m_pGrassTypes[i]->GetEffect()->GetVariableByName("g_txHeightMap")->AsShaderResource();
        pESRV->SetResource(pHeightMapSRV);

        m_pGrassTime[i] = m_pGrassTypes[i]->GetEffect()->GetVariableByName( "g_fTime" )->AsScalar();
    }

    m_pTerrRadiusESV[0]                   = m_pGrassTypes[0]->GetEffect()->GetVariableByName( "g_fTerrRadius" )->AsScalar();
    m_pTerrRadiusESV[1]                   = m_pGrassTypes[2]->GetEffect()->GetVariableByName( "g_fTerrRadius" )->AsScalar();
    m_pTerrRadiusESV[2]                   = m_pSceneEffect->GetVariableByName( "g_fTerrRadius" )->AsScalar();
    for (i = 0; i < 3; i++)
    {
        m_pTerrRadiusESV[i]->SetFloat(a_InitState.fTerrRadius);
    }
	m_pTerrRGBEVV[0]					   = m_pGrassTypes[0]->GetEffect()->GetVariableByName( "g_vTerrRGB" )->AsVector();
    m_pTerrRGBEVV[2]					   = m_pGrassTypes[2]->GetEffect()->GetVariableByName( "g_vTerrRGB" )->AsVector();
	m_pTerrRGBEVV[1]					   =				m_pSceneEffect->GetVariableByName( "g_vTerrRGB" )->AsVector();
    m_pFogColorEVV[0]					  = m_pGrassTypes[0]->GetEffect()->GetVariableByName( "g_vFogColor" )->AsVector();
	m_pFogColorEVV[1]					  = m_pGrassTypes[2]->GetEffect()->GetVariableByName( "g_vFogColor" )->AsVector();
    m_pFogColorEVV[2]									 = m_pSceneEffect->GetVariableByName( "g_vFogColor" )->AsVector();
    m_pViewProjEMV[GrassTypeNum]      = m_pSceneEffect->GetVariableByName( "g_mViewProj" )->AsMatrix();
    m_pInvView[0]     = m_pGrassTypes[0]->GetEffect()->GetVariableByName( "g_mInvCamView" )->AsMatrix();
    m_pInvView[1]     = m_pGrassTypes[2]->GetEffect()->GetVariableByName( "g_mInvCamView" )->AsMatrix();
    m_pInvView[2]     = m_pSceneEffect->GetVariableByName( "g_mInvCamView" )->AsMatrix();
    m_pHeightScale[GrassTypeNum]      = m_pSceneEffect->GetVariableByName( "g_fHeightScale" )->AsScalar();
    m_pTime                           = m_pSceneEffect->GetVariableByName( "g_fTime" )->AsScalar();
    m_pShadowMapESRV[GrassTypeNum]    = m_pSceneEffect->GetVariableByName( "g_txShadowMap" )->AsShaderResource();
    m_pLightViewProjEMV[GrassTypeNum] = m_pSceneEffect->GetVariableByName( "g_mLightViewProj" )->AsMatrix();
    m_pLightDirEVV[GrassTypeNum]      = m_pSceneEffect->GetVariableByName( "vLightDir" )->AsVector();
    m_pLightDirEVV[GrassTypeNum]->SetFloatVector((float*)&vLightDir);


    ID3DX11EffectScalarVariable *pESV = m_pSceneEffect->GetVariableByName("g_fGrassRadius")->AsScalar();    
    pESV->SetFloat(a_InitState.InitState[0].fGrassRadius);

    pESRV = m_pGrassTypes[0]->GetEffect()->GetVariableByName("g_txTerrainLightMap")->AsShaderResource();
    if (pESRV) 
        pESRV->SetResource(m_pTerrain->LightMapSRV());
    /* Noise maps for alpha dissolve */
    pESRV = m_pGrassTypes[0]->GetEffect()->GetVariableByName("g_txNoise")->AsShaderResource();
    if (pESRV) 
        pESRV->SetResource(m_pNoiseESV);
	
	pESRV = m_pGrassTypes[2]->GetEffect()->GetVariableByName("g_txNoise")->AsShaderResource();
    if (pESRV) 
        pESRV->SetResource(m_pNoiseESV);
    /*color maps...*/
    pESRV = m_pGrassTypes[0]->GetEffect()->GetVariableByName("g_txGrassColor")->AsShaderResource();
    if (pESRV) 
        pESRV->SetResource(m_pGrassColorESV);
    pESRV = m_pGrassTypes[2]->GetEffect()->GetVariableByName("g_txGrassColor")->AsShaderResource();
    if (pESRV) 
        pESRV->SetResource(m_pGrassColorESV);
    pESRV = m_pSceneEffect->GetVariableByName("g_txGrassColor")->AsShaderResource();
    if (pESRV) 
        pESRV->SetResource(m_pGrassColorESV);

    /* Loading subtypes info */
    m_pT1SubTypes = new GrassPropertiesT1(a_InitState.InitState[0].sSubTypesPath);
    m_pT2SubTypes = new GrassPropertiesT2(a_InitState.InitState[1].sSubTypesPath);
    m_pT3SubTypes = new GrassPropertiesT3(a_InitState.InitState[2].sSubTypesPath);

    ID3DX11EffectVariable *pEV1, *pEV3;
    pEV1 = m_pGrassTypes[0]->GetEffect()->GetVariableByName( "SubTypes" ); 
    pEV1->SetRawValue(m_pT1SubTypes->GetDataPtr(), 0, m_pT1SubTypes->GetDataNum() * sizeof( GrassProps1 ));

    pEV3 = m_pGrassTypes[2]->GetEffect()->GetVariableByName( "SubTypes" ); 
    pEV3->SetRawValue(m_pT3SubTypes->GetDataPtr(), 0, m_pT3SubTypes->GetDataNum() * sizeof( GrassProps3 ));
    
	/* Updating subtypes data on cpu */
    for (i = 0; i < m_pT1SubTypes->GetDataNum(); i++)
    {      
        m_pGrassTypes[0]->AddSubType(m_pT1SubTypes->GetProperty(i));
    }
    
	for (i = 0; i < m_pT3SubTypes->GetDataNum(); i++)
    {      
        m_pGrassTypes[2]->AddSubType(m_pT3SubTypes->GetProperty(i));
    }
    SetHeightScale(a_InitState.fHeightScale);
}


GrassFieldManager::~GrassFieldManager (void)
{
    SAFE_RELEASE(m_pSceneEffect);
    SAFE_RELEASE(m_pNoiseESV);
    SAFE_RELEASE(m_pGrassColorESV);
    UINT i;
    for (i = 0; i < GrassTypeNum; i++)
    {
        if (i == 1)
            continue;
        delete m_pGrassTypes[i];
        SAFE_RELEASE(m_pSeatingMapESV[i]);
    }
    delete m_pTerrain;
    delete m_pWind;
    delete m_pGrassTracker;

    delete m_pT1SubTypes;
    delete m_pT2SubTypes;
    delete m_pT3SubTypes;
}


void GrassFieldManager::Reinit (GrassFieldState &a_InitState)
{
    UINT i;
    for (i = 0; i < GrassTypeNum; i++)
    {
        if (i == 1)
            continue;
        m_pGrassTypes[i]->Reinit(a_InitState.InitState[i]);
    }

    /* Reinit terrain */
    delete m_pTerrain;

    m_pTerrain = new Terrain(
		a_InitState.InitState[0].pD3DDevice,
		a_InitState.InitState[0].pD3DDeviceCtx,
		m_pSceneEffect,
		a_InitState.fTerrRadius
	);
}

void GrassFieldManager::SetTerrRGB (float3&a_vValue)
{
	m_pTerrRGBEVV[0]->SetFloatVector((float*)&a_vValue);
	m_pTerrRGBEVV[1]->SetFloatVector((float*)&a_vValue);
	m_pTerrRGBEVV[2]->SetFloatVector((float*)&a_vValue);
}

void GrassFieldManager::SetFogColor (float4&a_vColor)
{
    m_pFogColorEVV[0]->SetFloatVector((float*)&a_vColor);
    m_pFogColorEVV[1]->SetFloatVector((float*)&a_vColor);
	m_pFogColorEVV[2]->SetFloatVector((float*)&a_vColor);
}

void GrassFieldManager::SetQuality( float a_fQuality )
{    
    UINT i;
    for (i = 0; i < GrassTypeNum; i++)
    {
        if (i == 1)
            continue;
        m_pGrassTypes[i]->SetQuality(a_fQuality);
    }
}

void GrassFieldManager::SetGrassAmbient( float a_fGrassAmbient )
{
    UINT i;
    for (i = 0; i < GrassTypeNum; i++)
    {
        if (i == 1)
            continue;
        m_pGrassTypes[i]->SetGrassAmbient(a_fGrassAmbient);
    }
}

void GrassFieldManager::SetGrassLodBias( float a_fGrassLodBias )
{
    UINT i;
    for (i = 0; i < GrassTypeNum; i++)
    {
        if (i == 1)
            continue;
        m_pGrassTypes[i]->SetGrassLodBias(a_fGrassLodBias);
    }
}

void GrassFieldManager::SetSubScatterGamma( float a_fGrassSubScatterGamma )
{
    UINT i;
    for (i = 0; i < GrassTypeNum; i++)
    {
        if (i == 1)
            continue;
        m_pGrassTypes[i]->SetSubScatterGamma(a_fGrassSubScatterGamma);
    }
}

void GrassFieldManager::SetWindStrength( float a_fWindStrength )
{
    UINT i;
    for (i = 0; i < GrassTypeNum; i++)
    {
        if (i == 1)
            continue;
        m_pGrassTypes[i]->SetWindStrength(a_fWindStrength);
    }
}

void GrassFieldManager::SetWindSpeed( float a_fWindSpeed )
{
    m_pWind->SetWindSpeed(a_fWindSpeed);
}

void GrassFieldManager::SetViewMtx (float4x4& a_mView)
{
    UINT i;
    m_pView = &a_mView;
	
	m_mInvView = inverse(a_mView);

	for (i = 0; i < GrassTypeNum; i++)
    {        
        if (i == 1)
            continue;
        m_pViewEMV[i]->SetMatrix((float *)&a_mView);
    }
}

void GrassFieldManager::SetProjMtx (float4x4& a_mProj)
{
    m_pProj = &a_mProj;
}

void GrassFieldManager::SetViewProjMtx (float4x4& a_mViewProj)
{
    UINT i;
    m_pViewProj = &a_mViewProj;
    for (i = 0; i <= GrassTypeNum; i++)
    {        
        if (i == 1)
            continue;
        m_pViewProjEMV[i]->SetMatrix((float *)&a_mViewProj);
    }
}

void GrassFieldManager::SetTime (float a_fTime)
{          
    m_pTime->SetFloat(a_fTime);
    for (int i = 0; i < GrassTypeNum; i++)
    {
        if (i == 1)
            continue;
        m_pGrassTime[i]->SetFloat(a_fTime);
    }
}

void GrassFieldManager::SetWindMapTile (float a_fWindMapTile)
{
    UINT i;
    for (i = 0; i < GrassTypeNum; i++)
    {
        if (i == 1)
            continue;
        m_pWindMapTile[i]->SetFloat(a_fWindMapTile);
    }

	PhysPatch::fWindTexTile = a_fWindMapTile;
}

void GrassFieldManager::SetHardness (float a_fHardness)
{          
    UINT i;
    for (i = 0; i < GrassTypeNum; i++)
    {
        if (i == 1)
            continue;
        m_pHardness[i]->SetFloat(a_fHardness);
    }

	PhysPatch::hardness = a_fHardness;
}

void GrassFieldManager::SetSegMass (float a_fSegMass)
{   
    UINT i;
    for (i = 0; i < GrassTypeNum; i++)
    {
        if (i == 1)
            continue;
        m_pSegMass[i]->SetFloat(a_fSegMass);
    }

	PhysPatch::mass = a_fSegMass;
}

void GrassFieldManager::SetHeightScale (float a_fHeightScale)
{   
    UINT i;
    for (i = 0; i < GrassTypeNum; i++)
    {
        if (i == 1)
            continue;
        m_pHeightScale[i]->SetFloat(a_fHeightScale);
        m_pGrassTypes[i]->SetHeightScale(a_fHeightScale);
    }
    m_pHeightScale[GrassTypeNum]->SetFloat(a_fHeightScale);
    
    PhysPatch::fHeightScale = a_fHeightScale;
	m_fHeightScale = a_fHeightScale;
}

void GrassFieldManager::SetWindScale (float a_fScale)
{
    m_pWind->SetWindScale(a_fScale);
}

void GrassFieldManager::SetWindBias (float a_fBias)
{
    m_pWind->SetWindBias(a_fBias);
}

void GrassFieldManager::SetLowGrassDiffuse (float4 &a_vValue)
{
    UINT i;
    for (i = 0; i < GrassTypeNum; i++)
    {
        if (i == 1)
            continue;
        m_pGrassTypes[i]->SetLowGrassDiffuse(a_vValue);
    }
}

Terrain * const GrassFieldManager::GetTerrain( float *a_fHeightScale, float *a_fGrassRadius )
{
	*a_fHeightScale = m_fHeightScale;
	*a_fGrassRadius = m_fTerrRadius;
	return m_pTerrain;
}

void GrassFieldManager::Render( )
{
    m_pInvView[0]->SetMatrix((float*)&m_mInvView);
    m_pInvView[1]->SetMatrix((float*)&m_mInvView);
    m_pInvView[2]->SetMatrix((float*)&m_mInvView);

	m_pTerrain->Render();
    m_pGrassTypes[0]->Render(false);
	m_pGrassTypes[2]->Render(false);
}

void GrassFieldManager::Update (XMVECTOR a_vCamDir, XMVECTOR a_vCamPos, Mesh *a_pMeshes[], UINT a_uNumMeshes, float a_fElapsedTime)
{
	m_vCamDir = a_vCamDir; 
	m_vCamPos = a_vCamPos;
   
	m_pTerrain->UpdateLightMap( );
	
	m_pGrassTypes[0]->Update(*m_pViewProj, a_vCamPos, a_pMeshes, a_uNumMeshes, a_fElapsedTime);
	m_pGrassTypes[2]->Update(*m_pViewProj, a_vCamPos, a_pMeshes, a_uNumMeshes, a_fElapsedTime);
}

ID3DX11Effect *GrassFieldManager::SceneEffect (void)
{
    return m_pSceneEffect;
}

void GrassFieldManager::ClearGrassPools (void)
{
    for (int i = 0; i < GrassTypeNum; i++)
        if (m_pGrassTypes[i] != NULL)
            m_pGrassTypes[i]->ClearGrassPools();
}
