#include "car.h"
#include "PhysPatch.h"
#include "StateManager.h"

Car::Car( ID3D10Device *a_pD3DDevice, ID3D10Effect *a_pEffect, D3DXVECTOR4 &a_vPosAndRadius,
          Terrain * const a_pTerrain, float a_fHeightScale, float a_fGrassRadius,
          float a_fCarWidth, float a_fCarHeight, float a_fCarLength, float a_fAmplAngle )
{
    m_pD3DDevice    = a_pD3DDevice;
    m_vPosAndRadius = a_vPosAndRadius;
    m_uVertexStride = sizeof(Vertex);
    m_uIndexStride  = sizeof(UINT32);
    m_uVertexOffset = 0;
    m_uIndexOffset  = 0;
    m_pTerrain      = a_pTerrain;
    m_fHeightScale  = a_fHeightScale;
    m_fGrassRadius  = a_fGrassRadius;
    m_pIndexBuffer  = NULL;
    m_pVertexBuffer = NULL;
    m_pInputLayout  = NULL;
    m_pTextureSRV   = NULL;
    

    // Set car sizes
    m_fCarWidth = a_fCarWidth;
    m_fCarHeight = a_fCarHeight;
    m_fCarLength = a_fCarLength;
    m_fAmplAngle = a_fAmplAngle;
    m_fCarBackWidth = m_fCarWidth + m_fCarLength * tanf(m_fAmplAngle);

    // Set plane sizes
    m_fPlaneLength = m_fCarLength;
    m_fPlaneWidth = m_fCarWidth;
    m_fPlaneHeight = m_fCarHeight;

    m_uNumPlanes = 4;
    // Front
    m_pPlanes[0] = new Plane(a_pD3DDevice, a_pEffect, a_vPosAndRadius, m_fPlaneWidth, m_fPlaneHeight,
        m_fPlaneWidth - 0.5f, m_fPlaneWidth - 0.01f);
    // Right
    m_pPlanes[1] = new Plane(a_pD3DDevice, a_pEffect, a_vPosAndRadius, m_fPlaneLength, m_fPlaneHeight, INVALID_DIST, INVALID_DIST);
    // Left
    m_pPlanes[2] = new Plane(a_pD3DDevice, a_pEffect, a_vPosAndRadius, m_fPlaneLength, m_fPlaneHeight, INVALID_DIST, INVALID_DIST);
    // Back
    m_pPlanes[3] = new Plane(a_pD3DDevice, a_pEffect, a_vPosAndRadius, m_fPlaneWidth, m_fPlaneHeight, INVALID_DIST, INVALID_DIST);

    /* just one technique in effect */
    ID3D10EffectTechnique *pTechnique = a_pEffect->GetTechniqueByName("RenderCar");
    m_pPass = pTechnique->GetPassByIndex(0);

    // Setup shader variables
    m_pMeshMapKdESRV = a_pEffect->GetVariableByName("g_txMeshMapKd")->AsShaderResource();
    m_pMeshMapKsESRV = a_pEffect->GetVariableByName("g_txMeshMapKs")->AsShaderResource();
    m_pMaterialCoefsESV[0] = a_pEffect->GetVariableByName("g_vKd")->AsVector();
    m_pMaterialCoefsESV[1] = a_pEffect->GetVariableByName("g_vKs")->AsVector();
    m_pMaterialCoefsESV[2] = a_pEffect->GetVariableByName("g_vKa")->AsVector();
    m_pMaterialShininessESV = a_pEffect->GetVariableByName("g_nNs")->AsScalar();
    m_pNormalMatrixEMV = a_pEffect->GetVariableByName("g_mNormalMatrix")->AsMatrix();

    m_pTransformEMV = a_pEffect->GetVariableByName("g_mWorld")->AsMatrix();

    //CreateVertexBuffer();
    CreateInputLayout();
    m_Mesh.Create(a_pD3DDevice, L"resources\\SuperCar\\carFordtest.obj");
}

Car::~Car( void )
{
    delete m_pPlanes[0];
    delete m_pPlanes[1];
    delete m_pPlanes[2];
    delete m_pPlanes[3];
    m_Mesh.Destroy();
}

void Car::CreateVertexBuffer( void )
{
    /* Initializing vertices */
    //int ind;
    m_uVertexCount = 0;//8;
    m_uIndexCount  = 0;//36;

    Vertex *Vertices = new Vertex[m_uVertexCount];
    UINT32 *Indices = new UINT32[m_uIndexCount];    

    //#include "cube.inc"

    /* Initializing buffer */
    D3D10_BUFFER_DESC VBufferDesc = 
    {
        m_uVertexCount * sizeof(Vertex),
        D3D10_USAGE_DEFAULT,
        D3D10_BIND_VERTEX_BUFFER,
        0, 0
    };
    D3D10_SUBRESOURCE_DATA VBufferInitData;
    VBufferInitData.pSysMem = Vertices;
    m_pD3DDevice->CreateBuffer(&VBufferDesc, &VBufferInitData, &m_pVertexBuffer);

    D3D10_BUFFER_DESC IBufferDesc = 
    {
        m_uIndexCount * sizeof(UINT32),
        D3D10_USAGE_DEFAULT,
        D3D10_BIND_INDEX_BUFFER,
        0, 0
    };
    D3D10_SUBRESOURCE_DATA IBufferInitData;
    IBufferInitData.pSysMem = Indices;
    m_pD3DDevice->CreateBuffer(&IBufferDesc, &IBufferInitData, &m_pIndexBuffer);
    delete [] Indices;
    delete [] Vertices;
}

void Car::CreateInputLayout( void )
{
    const D3D10_INPUT_ELEMENT_DESC InputDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };   
    D3D10_PASS_DESC PassDesc;
    m_pPass->GetDesc(&PassDesc);
    int InputElementsCount = sizeof(InputDesc) / sizeof(D3D10_INPUT_ELEMENT_DESC);
    m_pD3DDevice->CreateInputLayout(InputDesc, InputElementsCount, 
        PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, 
        &m_pInputLayout);
}

void Car::Render( void )
{    
    //m_pPlanes[0]->Render();
    //m_pPlanes[1]->Render();
    //m_pPlanes[2]->Render();

    m_pD3DDevice->IASetInputLayout(m_pInputLayout);
    m_pTransformEMV->SetMatrix((float *)m_mTransform);
    m_pNormalMatrixEMV->SetMatrix((float *)m_mNormalMatrix);
    if (GetGlobalStateManager().UseWireframe())
        GetGlobalStateManager().SetRasterizerState("EnableMSAACulling_Wire");
    else
        GetGlobalStateManager().SetRasterizerState("EnableMSAACulling");

    m_Mesh.Render(m_pPass, m_pMeshMapKdESRV, m_pMeshMapKsESRV, m_pMaterialCoefsESV, m_pMaterialShininessESV);
/*    m_pD3DDevice->IASetInputLayout(m_pInputLayout);
    m_pD3DDevice->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
    m_pD3DDevice->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    m_pD3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pPass->Apply(0);
    m_pD3DDevice->DrawIndexed(m_uIndexCount, 0, 0);    */
}

static int StartFlag = 1;
void Car::SetPosAndRadius( D3DXVECTOR4 &a_vPosAndRadius )
{
    float3 vY(0, 1, 0), dir, plane_cur_pos, plane_prev_pos, cur_pos, offset;
    D3DXMATRIX translate, scale, rotate, plane_coords, model_coords;

    // Calculate move direction
    m_vPrevPos = float3(m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z);
    cur_pos = float3(a_vPosAndRadius.x, a_vPosAndRadius.y, a_vPosAndRadius.z);
    m_vPosAndRadius = a_vPosAndRadius;

    m_vPrevPos.y = 0;
	if (StartFlag)
	{
		dir = float3(0.0, 0.0, 1.0);
		StartFlag = 0;
	}
	else
	    dir = cur_pos - m_vPrevPos;
    
	D3DXVec3Normalize(&dir, &dir);
//    dir = float3(0.0, 0.0, 1.0);
    m_vMoveDir = dir;
    
    // Calculate car height
    TerrainHeightData *pHD = m_pTerrain->HeightDataPtr();
    float2 vTexCoord;
    float fHeightOffset = 1.5f;
	float g_fCarLength = 1.0;

    // Calculate up dir
    D3DXVECTOR3 up;
    D3DXVECTOR3 norm_dir, v1, v2, p0, p1, p2, p3;

    D3DXVec3Cross(&norm_dir, &dir, &vY);
    D3DXVec3Normalize(&norm_dir, &norm_dir);
    norm_dir *= m_fCarWidth;
	dir *= m_fCarLength;

    p0 = cur_pos - dir;
    p1 = p0 + norm_dir;
    p2 = p0 - norm_dir;
    p3 = cur_pos + dir;

    vTexCoord = float2(p0.x / m_fGrassRadius * 0.5f + 0.5f,
        p0.z / m_fGrassRadius * 0.5f + 0.5f);
    p0.y = pHD->GetHeight(vTexCoord.x, vTexCoord.y) * m_fHeightScale + fHeightOffset;

       vTexCoord = float2(p1.x / m_fGrassRadius * 0.5f + 0.5f,
        p1.z / m_fGrassRadius * 0.5f + 0.5f);
//	vTexCoord = float2((cur_pos.x + dir.x + norm_dir.x) / m_fGrassRadius * 0.5f + 0.5f,
//        (cur_pos.z + dir.z + norm_dir.z) / m_fGrassRadius * 0.5f + 0.5f);
    p1.y = pHD->GetHeight(vTexCoord.x, vTexCoord.y) * m_fHeightScale + fHeightOffset;
    //v1 = dir + (p1 - p0);

     vTexCoord = float2(p2.x / m_fGrassRadius * 0.5f + 0.5f,
        p2.z / m_fGrassRadius * 0.5f + 0.5f);
	 //vTexCoord = float2((cur_pos.x + dir.x - norm_dir.x) / m_fGrassRadius * 0.5f + 0.5f,
       // (cur_pos.z + dir.z - norm_dir.z) / m_fGrassRadius * 0.5f + 0.5f);
    p2.y = pHD->GetHeight(vTexCoord.x, vTexCoord.y) * m_fHeightScale + fHeightOffset;
    //v2 = dir + (p2 - p0);

    vTexCoord = float2(p3.x / m_fGrassRadius * 0.5f + 0.5f,
        p3.z / m_fGrassRadius * 0.5f + 0.5f);
    p3.y = pHD->GetHeight(vTexCoord.x, vTexCoord.y) * m_fHeightScale + fHeightOffset;

//	p1.y = p2.y = p3.y = 5.0;
    v1 = p3 - p1;
    v2 = p3 - p2;
	D3DXVec3Normalize(&v1, &v1);
	D3DXVec3Normalize(&v2, &v2);
    D3DXVec3Cross(&up, &v2, &v1);
	D3DXVec3Normalize(&up, &up);
//	cur_pos.y = ((p1.y + p2.y)/2.0 + p3.y) / 2.0;
	cur_pos = ((p1 + p2)/2.0 + p3) / 2.0;

    // Calculate TBN matrix
    D3DXMATRIX tmp;
//    dir.y = p3.y - cur_pos.y;
    dir = p3 - cur_pos;
    D3DXMatrixLookAtLH(&model_coords, &(cur_pos), &(cur_pos + dir), &up);
	D3DXMatrixInverse(&m_mMatr, NULL, &model_coords);
	//m_mMatr = model_coords;

//    D3DXMatrixInverse(&m_mInvBottom, NULL, &model_coords);
  	m_mInvBottom = model_coords;
  	// Scale
    D3DXMatrixScaling(&scale, 1.0f / m_fCarWidth, 1.0f / m_fCarHeight, 1.0f / m_fCarLength);
 //   D3DXMatrixScaling(&scale, 1.0f , 1.0f , 1.0f);
    D3DXMatrixMultiply(&m_mTransform, &model_coords, &scale);

    // Set base mesh transform
    D3DXMatrixInverse(&m_mTransform, NULL, &m_mTransform);
    D3DXMatrixMultiply(&m_mTransform, &m_Mesh.GetNormalizeMt(), &m_mTransform);
    // Use this to convert normals to world space in shader
    D3DXMatrixInverse(&m_mNormalMatrix, NULL, &m_mTransform);
	
//	m_mInvBottom = m_mTransform;
    
	// Place front plane
    offset = float3(0.0f, 0.0f, -m_fPlaneLength);

    D3DXMatrixTranslation(&translate, offset.x, offset.y, offset.z);
    D3DXMatrixMultiply(&plane_coords, &model_coords, &translate);

    m_pPlanes[0]->SetInvTransform(plane_coords);
    m_pPlanes[0]->SetPosAndRadius(
        float4(cur_pos.x, cur_pos.y, cur_pos.z, 0) + float4(offset.x, offset.y, offset.z, 0));

    // Place right plane
    offset = float3(0.0f, 0.0f, -m_fPlaneWidth);

    D3DXMatrixRotationAxis(&rotate, &vY, 3.0f * (float)M_PI / 2.0f);
    D3DXMatrixMultiply(&plane_coords, &model_coords, &rotate);

    D3DXMatrixTranslation(&translate, offset.x, offset.y, offset.z);
    D3DXMatrixMultiply(&plane_coords, &plane_coords, &translate);

    D3DXMatrixTranslation(&translate, m_fPlaneLength, 0, 0);
    D3DXMatrixMultiply(&plane_coords, &plane_coords, &translate);

    D3DXMatrixRotationY(&rotate, m_fAmplAngle);
    D3DXMatrixMultiply(&plane_coords, &plane_coords, &rotate);

    D3DXMatrixTranslation(&translate, -m_fPlaneLength, 0, 0);
    D3DXMatrixMultiply(&plane_coords, &plane_coords, &translate);

    m_pPlanes[1]->SetInvTransform(plane_coords);
    m_pPlanes[1]->SetPosAndRadius(
        float4(cur_pos.x, cur_pos.y, cur_pos.z, 0) + float4(offset.x, offset.y, offset.z, 0));

    // Place left plane
    offset = float3(0.0f, 0.0f, -m_fPlaneWidth);

    D3DXMatrixRotationAxis(&rotate, &vY, (float)M_PI / 2.0f);
    D3DXMatrixMultiply(&plane_coords, &model_coords, &rotate);

    D3DXMatrixTranslation(&translate, offset.x, offset.y, offset.z);
    D3DXMatrixMultiply(&plane_coords, &plane_coords, &translate);

    D3DXMatrixTranslation(&translate, -m_fPlaneLength, 0, 0);
    D3DXMatrixMultiply(&plane_coords, &plane_coords, &translate);

    D3DXMatrixRotationY(&rotate, -m_fAmplAngle);
    D3DXMatrixMultiply(&plane_coords, &plane_coords, &rotate);

    D3DXMatrixTranslation(&translate, m_fPlaneLength, 0, 0);
    D3DXMatrixMultiply(&plane_coords, &plane_coords, &translate);

    m_pPlanes[2]->SetInvTransform(plane_coords);
    m_pPlanes[2]->SetPosAndRadius(
        float4(cur_pos.x, cur_pos.y, cur_pos.z, 0) + float4(offset.x, offset.y, offset.z, 0));

    m_vPosAndRadius = float4(cur_pos.x, cur_pos.y, cur_pos.z, 0.0f);

    // Place back plane
    offset = float3(0.0f, 0.0f, m_fPlaneLength);

    D3DXMatrixTranslation(&translate, offset.x, offset.y, offset.z);
    D3DXMatrixMultiply(&plane_coords, &model_coords, &translate);

    m_pPlanes[3]->SetInvTransform(plane_coords);
    m_pPlanes[3]->SetPosAndRadius(
        float4(cur_pos.x, cur_pos.y, cur_pos.z, 0) + float4(offset.x, offset.y, offset.z, 0));
}

/*
void Car::SetPosAndRadius( D3DXVECTOR4 &a_vPosAndRadius )
{
    float3 vY(0, 1, 0), dir, plane_cur_pos, plane_prev_pos, cur_pos, offset;
    D3DXMATRIX translate, scale, rotate, plane_coords, model_coords;

    // Calculate move direction
    m_vPrevPos = float3(m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z);
    cur_pos = float3(a_vPosAndRadius.x, a_vPosAndRadius.y, a_vPosAndRadius.z);
    m_vPosAndRadius = a_vPosAndRadius;

    m_vPrevPos.y = 0;
    dir = cur_pos - m_vPrevPos;
    D3DXVec3Normalize(&dir, &dir);
    m_vMoveDir = dir;
    
    // Calculate car height
    TerrainHeightData *pHD = m_pTerrain->HeightDataPtr();
    float2 vTexCoord;
    float front_height, back_height;
    float fHeightOffset = 1.5f;
	float g_fCarLength = 1.0;

    vTexCoord = float2((cur_pos.x - g_fCarLength*dir.x) / m_fGrassRadius * 0.5f + 0.5f,
        (cur_pos.z - g_fCarLength*dir.z) / m_fGrassRadius * 0.5f + 0.5f);
    back_height = pHD->GetHeight(vTexCoord.x, vTexCoord.y) * m_fHeightScale + fHeightOffset;

    vTexCoord = float2((cur_pos.x + g_fCarLength*dir.x) / m_fGrassRadius * 0.5f + 0.5f,
        (cur_pos.z + g_fCarLength*dir.z) / m_fGrassRadius * 0.5f + 0.5f);
    front_height = pHD->GetHeight(vTexCoord.x, vTexCoord.y) * m_fHeightScale + fHeightOffset;

    dir = cur_pos - m_vPrevPos;
    dir.y = front_height - back_height;
    D3DXVec3Normalize(&dir, &dir);
    dir *= m_fCarLength;

    cur_pos.y = (back_height + front_height) / 2.0;

    // Calculate up dir
    D3DXVECTOR3 up;
    D3DXVECTOR3 norm_dir, v1, v2, p0, p1, p2, p3;

    D3DXVec3Cross(&norm_dir, &dir, &vY);
    D3DXVec3Normalize(&norm_dir, &norm_dir);
    norm_dir *= m_fCarWidth;

    p0 = cur_pos - dir;
    p1 = p0 + norm_dir;
    p2 = p0 - norm_dir;
    p3 = cur_pos + dir;

    vTexCoord = float2(p0.x / m_fGrassRadius * 0.5f + 0.5f,
        p0.z / m_fGrassRadius * 0.5f + 0.5f);
    p0.y = pHD->GetHeight(vTexCoord.x, vTexCoord.y) * m_fHeightScale + fHeightOffset;

       vTexCoord = float2(p1.x / m_fGrassRadius * 0.5f + 0.5f,
        p1.z / m_fGrassRadius * 0.5f + 0.5f);
//	vTexCoord = float2((cur_pos.x + dir.x + norm_dir.x) / m_fGrassRadius * 0.5f + 0.5f,
//        (cur_pos.z + dir.z + norm_dir.z) / m_fGrassRadius * 0.5f + 0.5f);
    p1.y = pHD->GetHeight(vTexCoord.x, vTexCoord.y) * m_fHeightScale + fHeightOffset;
    //v1 = dir + (p1 - p0);

     vTexCoord = float2(p2.x / m_fGrassRadius * 0.5f + 0.5f,
        p2.z / m_fGrassRadius * 0.5f + 0.5f);
	 //vTexCoord = float2((cur_pos.x + dir.x - norm_dir.x) / m_fGrassRadius * 0.5f + 0.5f,
       // (cur_pos.z + dir.z - norm_dir.z) / m_fGrassRadius * 0.5f + 0.5f);
    p2.y = pHD->GetHeight(vTexCoord.x, vTexCoord.y) * m_fHeightScale + fHeightOffset;
    //v2 = dir + (p2 - p0);

    vTexCoord = float2(p3.x / m_fGrassRadius * 0.5f + 0.5f,
        p3.z / m_fGrassRadius * 0.5f + 0.5f);
    p3.y = pHD->GetHeight(vTexCoord.x, vTexCoord.y) * m_fHeightScale + fHeightOffset;

    v1 = p3 - p1;
    v2 = p3 - p2;

    D3DXVec3Cross(&up, &v2, &v1);

    // Calculate TBN matrix
    D3DXMATRIX tmp;
    dir.y = p3.y - cur_pos.y;
    D3DXMatrixLookAtLH(&model_coords, &(cur_pos), &(cur_pos + dir), &up);

    // Scale
    D3DXMatrixScaling(&scale, 1.0f / m_fCarWidth, 1.0f / m_fCarHeight, 1.0f / m_fCarLength);
    D3DXMatrixMultiply(&m_mTransform, &model_coords, &scale);

    // Set base mesh transform
    D3DXMatrixInverse(&m_mTransform, NULL, &m_mTransform);
    D3DXMatrixMultiply(&m_mTransform, &m_Mesh.GetNormalizeMt(), &m_mTransform);

    // Place front plane
    offset = float3(0.0f, 0.0f, -m_fPlaneLength);

    D3DXMatrixTranslation(&translate, offset.x, offset.y, offset.z);
    D3DXMatrixMultiply(&plane_coords, &model_coords, &translate);

    m_pPlanes[0]->SetInvTransform(plane_coords);
    m_pPlanes[0]->SetPosAndRadius(
        float4(cur_pos.x, cur_pos.y, cur_pos.z, 0) + float4(offset.x, offset.y, offset.z, 0));

    // Place right plane
    offset = float3(0.0f, 0.0f, -m_fPlaneWidth);

    D3DXMatrixRotationAxis(&rotate, &vY, 3.0f * (float)M_PI / 2.0f);
    D3DXMatrixMultiply(&plane_coords, &model_coords, &rotate);

    D3DXMatrixTranslation(&translate, offset.x, offset.y, offset.z);
    D3DXMatrixMultiply(&plane_coords, &plane_coords, &translate);

    D3DXMatrixTranslation(&translate, m_fPlaneLength, 0, 0);
    D3DXMatrixMultiply(&plane_coords, &plane_coords, &translate);

    D3DXMatrixRotationY(&rotate, m_fAmplAngle);
    D3DXMatrixMultiply(&plane_coords, &plane_coords, &rotate);

    D3DXMatrixTranslation(&translate, -m_fPlaneLength, 0, 0);
    D3DXMatrixMultiply(&plane_coords, &plane_coords, &translate);

    m_pPlanes[1]->SetInvTransform(plane_coords);
    m_pPlanes[1]->SetPosAndRadius(
        float4(cur_pos.x, cur_pos.y, cur_pos.z, 0) + float4(offset.x, offset.y, offset.z, 0));

    // Place left plane
    offset = float3(0.0f, 0.0f, -m_fPlaneWidth);

    D3DXMatrixRotationAxis(&rotate, &vY, (float)M_PI / 2.0f);
    D3DXMatrixMultiply(&plane_coords, &model_coords, &rotate);

    D3DXMatrixTranslation(&translate, offset.x, offset.y, offset.z);
    D3DXMatrixMultiply(&plane_coords, &plane_coords, &translate);

    D3DXMatrixTranslation(&translate, -m_fPlaneLength, 0, 0);
    D3DXMatrixMultiply(&plane_coords, &plane_coords, &translate);

    D3DXMatrixRotationY(&rotate, -m_fAmplAngle);
    D3DXMatrixMultiply(&plane_coords, &plane_coords, &rotate);

    D3DXMatrixTranslation(&translate, m_fPlaneLength, 0, 0);
    D3DXMatrixMultiply(&plane_coords, &plane_coords, &translate);

    m_pPlanes[2]->SetInvTransform(plane_coords);
    m_pPlanes[2]->SetPosAndRadius(
        float4(cur_pos.x, cur_pos.y, cur_pos.z, 0) + float4(offset.x, offset.y, offset.z, 0));

    m_vPosAndRadius = float4(cur_pos.x, cur_pos.y, cur_pos.z, 0.0f);
}
*/
D3DXVECTOR4 Car::GetPosAndRadius( void )
{
    float r = 1.5f*sqrt(m_fCarLength*m_fCarLength + m_fCarHeight*m_fCarHeight);
	return float4(m_vPosAndRadius.x, m_vPosAndRadius.y, m_vPosAndRadius.z, r);
//        max(max(m_fCarBackWidth, m_fCarHeight), m_fCarLength));
}

void Car::SetHeight( float a_fH )
{
    //D3DXMATRIX trans;

    /*
    D3DXMatrixTranslation(&trans, 0, (0.3f + m_fCarHeight), 0);
    D3DXMatrixMultiply(&m_mTransform, &m_mTransform, &trans);
    m_vPosAndRadius.y += 0.3f + m_fCarHeight;
    */

    //m_pPlanes[0]->SetHeight(a_fH);
    //m_pPlanes[1]->SetHeight(a_fH);
    //m_pPlanes[2]->SetHeight(a_fH);
    ;
}

bool Car::CheckCollision( D3DXVECTOR3 &Beg, D3DXVECTOR3 &End, float *Dist )
{
    float treshold = -1.5f;

    for (UINT k = 1; k < m_uNumPlanes; k++)
    {
        float cur_dist;
        bool collision_flag;

        collision_flag = 
            m_pPlanes[k]->CheckCollision(Beg, End, &cur_dist);
        if (collision_flag ||
            (cur_dist < 0 && cur_dist > treshold))
        {
            if (Dist != NULL)
                *Dist = cur_dist;
            return true;
        }
    }

    if (Dist != NULL)
        *Dist = INVALID_DIST;
    return false;
}


bool Car::Collide( D3DXVECTOR3 *Ret, D3DXVECTOR3 &Beg, D3DXVECTOR3 &End,
    PhysPatch::BladePhysData *a_pBladePhysData, int a_iSegmentIndex )
{
    bool is_grabbed = false, is_collided = false;

    *Ret = D3DXVECTOR3(0, 0, 0);

    //if (a_pBladePhysData->brokenFlag == 1)
    //    return false;

    /*
    float min_dist = INVALID_DIST;
    UINT min_dist_index;

    for (UINT k = 1; k < m_uNumPlanes; k++)
    {
        float dist = fabs(m_pPlanes[k]->GetDist(Beg, NULL));

        if (dist < min_dist)
        {
            min_dist = dist;
            min_dist_index = k;
        }
    }
    */

    for (UINT k = 1; k < m_uNumPlanes; k++)
    {
        D3DXVECTOR3 psi;
        bool collision = m_pPlanes[k]->Collide(&psi, Beg, End, a_pBladePhysData);
        //float dist = m_pPlanes[k]->GetDist(Beg, NULL);

        /*
        if (dist < 0 && dist > -NUM_SEGMENTS * a_pBladePhysData->segmentHeight * 1.1 &&
            (a_pBladePhysData->grabbedMeshIndex == k || k == 0) &&
            (a_pBladePhysData->disableCollision || a_iSegmentIndex == 1))
        */
        /*
        float treshold;

        if (k == 0)
            treshold = -m_fCarLength;
        else
            treshold = -NUM_SEGMENTS * a_pBladePhysData->segmentHeight * 0.5f;
        */

        /*
        if (dist < -0.001 && dist > treshold &&
            ((min_dist_index == k && a_pBladePhysData->grabbedMeshIndex == -1) ||
             a_pBladePhysData->grabbedMeshIndex == k) &&
             (a_pBladePhysData->disableCollision || a_iSegmentIndex == 1))
        {
            m_pPlanes[k]->RotateToEdge(&psi, Beg, End);
            *Ret += psi;
            a_pBladePhysData->disableCollision = true;
            is_grabbed = true;
            is_collided = true;
            a_pBladePhysData->grabbedMeshIndex = k;
        }
        else if (collision && 
                 (!(dist < 0 && dist > treshold) &&
                  !a_pBladePhysData->disableCollision || k > 0))
        */
        if (collision)
        {
            *Ret += psi;
            is_collided = true;
        }

        /*
        if (k == 0 && a_pBladePhysData->grabbedMeshIndex == 0)
        {
            if (dist == INVALID_DIST)
            {
                a_pBladePhysData->disableCollision = false;
                a_pBladePhysData->grabbedMeshIndex = -1;
            }
            else if (dist > treshold * 2 &&
                     dist < treshold - 0.1)
            {
//                a_pBladePhysData->brokenFlag = true;
                a_pBladePhysData->disableCollision = false;
                a_pBladePhysData->grabbedMeshIndex = -1;
            }
        }
        */
    }

    return is_collided;
}

float Car::GetDist( D3DXVECTOR3 &Pnt, bool *IsUnderWheel )
{
    float min_dist = INVALID_DIST;

    if (IsUnderWheel != NULL)
        *IsUnderWheel = false;

    for (UINT i = 0; i < m_uNumPlanes; i++)
    {
        float dist = fabs(m_pPlanes[i]->GetDist(Pnt, IsUnderWheel));
        if (dist < min_dist)
            min_dist = dist;
    }

    return min_dist;
}

/*int Car::IsBottom(D3DXVECTOR3 &Pnt, D3DXVECTOR3 &vNormal)
{
    D3DXVECTOR3 loc_coord;
	float2 vTexCoord;
	TerrainHeightData *pHD = m_pTerrain->HeightDataPtr();

    D3DXVec3TransformCoord(&loc_coord, &Pnt, &m_mInvBottom);
	if (fabs(loc_coord.x) < m_fCarWidth  && fabs(loc_coord.z) < m_fCarLength ) 
	{
		vTexCoord = float2(Pnt.x / m_fGrassRadius * 0.5f + 0.5f, Pnt.z / m_fGrassRadius * 0.5f + 0.5f);
		vNormal = pHD->GetNormal(vTexCoord.x, vTexCoord.y);
		if (fabs(loc_coord.x) > m_fCarWidth - 0.75f)
		{
			if (loc_coord.x > 0.0f) return 2;
			else return 3;
		}
		else return 1;
//		return 1;
//		if (fabs(loc_coord.x) > m_fCarWidth - 0.3f)	return 2;
		//else return 1;
	}
	else
		return 0;
}
*/
int Car::IsBottom(D3DXVECTOR3 &Pnt, D3DXVECTOR3 &vNormal)
{
	D3DXVECTOR3 loc_coord;
	float2 vTexCoord;
	TerrainHeightData *pHD = m_pTerrain->HeightDataPtr();

    D3DXVec3TransformCoord(&loc_coord, &Pnt, &m_mInvBottom);
	if (fabs(loc_coord.x) <= m_fCarWidth && fabs(loc_coord.z) <= m_fCarLength)
	{
		vTexCoord = float2(Pnt.x / m_fGrassRadius * 0.5f + 0.5f, Pnt.z / m_fGrassRadius * 0.5f + 0.5f);
		//vNormal = pHD->GetNormal(vTexCoord.x, vTexCoord.y);
		vNormal.x = max((m_fCarWidth - fabs(loc_coord.x))/0.75f, 0.3f);
		if (vNormal.x > 1.0f) vNormal.x = 1.0f;
		if (fabs(loc_coord.x) > m_fCarWidth - 0.75f)
		{
			if (loc_coord.x > 0.0f) return 2;
			else return 3;
		}
		else return 1;
//		return 1;
//		if (fabs(loc_coord.x) > m_fCarWidth - 0.3f)	return 2;
		//else return 1;
	}
	else
		return 0;
}

/*
float Car::RotateToBottom( PhysPatch::BladePhysData *a_pBladePhysData )
{
    D3DXVECTOR3 loc_coord_beg, loc_coord_end;

    if (a_pBladePhysData->brokenFlag == 0)
        return 0;

    D3DXVec3TransformCoord(&loc_coord_beg, &a_pBladePhysData->position[0], &m_mInvBottom);
    D3DXVec3TransformCoord(&loc_coord_end, &a_pBladePhysData->position[1], &m_mInvBottom);

    // Calculate directions
    D3DXVECTOR3 dir = loc_coord_end - loc_coord_beg;
    D3DXVECTOR3 target_dir;
    if (a_pBladePhysData->brokenFlag == 2)
        target_dir = D3DXVECTOR3(m_fCarWidth, 0, loc_coord_beg.z);
    else if (a_pBladePhysData->brokenFlag == 3)
        target_dir = D3DXVECTOR3(-m_fCarWidth, 0, loc_coord_beg.z);

    D3DXVec3Normalize(&dir, &dir);
    D3DXVec3Normalize(&target_dir, &target_dir);

    // Calculate rotation angle
    float angle = D3DXVec3Dot(&dir, &target_dir);

    if (angle > 1)
        angle = 1;
    if (angle < -1)
        angle = -1;
    return acosf(angle);
}
*/
