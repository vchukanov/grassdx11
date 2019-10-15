#include "PhysPatch.h"
#include "GrassManager.h"

/**
*/
PhysPatch::PhysPatch(GrassPatch *a_pGrassPatch)
{        
    m_pBasePatch = a_pGrassPatch;
    m_pD3DDevice = a_pGrassPatch->GetD3DDevicePtr();
    m_dwVertexStride[0]  = sizeof(VertexPhysData);
    m_dwVertexStride[1]  = sizeof(VertexAnimData);
    m_dwVertexOffset     = 0;
	
	this->numBlades = a_pGrassPatch->VerticesCount();
	this->bladePhysData = new BladePhysData[numBlades];
    GenerateBuffer();
}

/**
*/
void PhysPatch::GenerateBuffer()
{
    D3D10_BUFFER_DESC bufferDesc =
    {
        numBlades * sizeof( VertexPhysData ),
        D3D10_USAGE_DYNAMIC,
        D3D10_BIND_VERTEX_BUFFER,
        D3D10_CPU_ACCESS_WRITE,
        0
    };

    m_pD3DDevice->CreateBuffer( &bufferDesc, NULL, &m_pPhysVertexBuffer );
    bufferDesc.ByteWidth = numBlades * sizeof( VertexAnimData );
    m_pD3DDevice->CreateBuffer( &bufferDesc, NULL, &m_pAnimVertexBuffer );
}

/**
*/
void PhysPatch::IASetPhysVertexBuffer0( )
{
    m_pD3DDevice->IASetVertexBuffers(0, 1, &m_pPhysVertexBuffer, m_dwVertexStride, &m_dwVertexOffset);
}

void PhysPatch::IASetAnimVertexBuffer0( )
{
    m_pD3DDevice->IASetVertexBuffers(0, 1, &m_pAnimVertexBuffer, m_dwVertexStride + 1, &m_dwVertexOffset);
}

/**
*/
DWORD PhysPatch::AnimVerticesCount( )
{
    return m_dwVerticesCount[1];
}

DWORD PhysPatch::PhysVerticesCount( )
{
    return m_dwVerticesCount[0];
}

/**
*/
PhysPatch::~PhysPatch()
{
	delete [] bladePhysData;
    SAFE_RELEASE(m_pAnimVertexBuffer);
    SAFE_RELEASE(m_pPhysVertexBuffer);
}

/**
*/
void PhysPatch::Reinit()
{
	for (UINT i = 0; i < numBlades; i++)
    {
		BladePhysData *bp = &bladePhysData[i];   
        
        bp->startPosition   = PosToWorld(m_pBasePatch->m_pVertices[i].vPos);
        
        bp->startDirection  = m_pBasePatch->m_pVertices[i].vRotAxe * 0.01745f;//(float)PI / 180.0f; //DirToWorld(m_pBasePatch->m_pVertices[i].vRotAxe * (float)PI / 180.0f);
        bp->startDirectionY = m_pBasePatch->m_pVertices[i].vYRotAxe * 0.01745f;//(float)PI / 180.0f; //DirToWorld(m_pBasePatch->m_pVertices[i].vYRotAxe * (float)PI / 180.0f);

		for (UINT j = 0; j < NUM_SEGMENTS; j++)
        {
			bp->w[j] = float3(0, 0, 0);
            bp->g[j] = float3(0, 0, 0);
            bp->R[j] = MakeIdentityMatrix();
		//	float3x3 RotY = MakeRotationMatrix(bp->startDirectionY);
		//	float3x3 RotD = MakeRotationMatrix(bp->startDirection);
			D3DXMatrixMultiply(bp->T + j, &MakeRotationMatrix(bp->startDirectionY), &MakeRotationMatrix(bp->startDirection));
            //if (j == 1)
            //    bp->R[1] = bp->T[1];
			//bp->T[j] = RotD;
            //bp->A[j] = bp->T[j];
          
            
            bp->NeedPhysics = 0;
            bp->brokenFlag = 0;
			bp->brokenTime = 0.0f;
        }

		bp->A[0] = bp->T[0];
		bp->position[0] = bp->startPosition;
        bp->grabbedMeshIndex = -1;
        //bp->fTransparency = m_pBasePatch->m_pVertices[i].fTransparency;

        //
        //VertexData *vd = &VertexBufData[i];
        //vd->fTransparency = m_pBasePatch->m_pVertices[i].fTransparency;;
    }
}

void PhysPatch::TransferFromOtherLod( const PhysPatch &a_PhysPatch, bool a_bLod0ToLod1 )
{
    /* just 2 cases: transfer data from lod0 to lod1 and from lod1 to lod0*/
    UINT i, j;
    DWORD dwBladeX;
    DWORD dwBladeZ;
    DWORD dwBaseVertIndex;
    DWORD dwStartVertIndex = 0;
    /* 1st case: lod0 => lod1 */
    if (a_bLod0ToLod1)
    {
        /*delete [] bladePhysData;
        delete [] VertexBufData;
        numBlades = a_PhysPatch.numBlades / 4;
        bladePhysData = new BladePhysData[numBlades];
        VertexBufData = new VertexData[numBlades];*/
        /* Each 16 vertices in a_PhysPatch representing 16 grass blades in 4x4 block */
        for (dwBaseVertIndex = 0; dwBaseVertIndex < a_PhysPatch.numBlades; dwBaseVertIndex += 16)
        {
            dwBladeZ = 0;
            dwBladeX = 0;
            for ( i = 0; i < 2; ++i )
                for ( j = 0; j < 2; ++j )
                {
                    if (a_PhysPatch.bladePhysData[dwBaseVertIndex + i * 4 + j].fTransparency > 
                        a_PhysPatch.bladePhysData[dwBaseVertIndex + dwBladeZ * 4 + dwBladeX].fTransparency)
                    {
                        dwBladeZ = i;
                        dwBladeX = j;
                    }
                }
            bladePhysData[dwStartVertIndex++] = a_PhysPatch.bladePhysData[dwBaseVertIndex + dwBladeZ * 4 + dwBladeX];

            dwBladeZ = 2;
            dwBladeX = 0;
            for ( i = 2; i < 4; ++i )
                for ( j = 0; j < 2; ++j )
                {
                    if (a_PhysPatch.bladePhysData[dwBaseVertIndex + i * 4 + j].fTransparency > 
                        a_PhysPatch.bladePhysData[dwBaseVertIndex + dwBladeZ * 4 + dwBladeX].fTransparency)
                    {
                        dwBladeZ = i;
                        dwBladeX = j;
                    }
                }
            bladePhysData[dwStartVertIndex++] = a_PhysPatch.bladePhysData[dwBaseVertIndex + dwBladeZ * 4 + dwBladeX];

            dwBladeZ = 0;
            dwBladeX = 2;
            for ( i = 0; i < 2; ++i )
                for ( j = 2; j < 4; ++j )
                {
                    if (a_PhysPatch.bladePhysData[dwBaseVertIndex + i * 4 + j].fTransparency > 
                        a_PhysPatch.bladePhysData[dwBaseVertIndex + dwBladeZ * 4 + dwBladeX].fTransparency)
                    {
                        dwBladeZ = i;
                        dwBladeX = j;
                    }
                }
            bladePhysData[dwStartVertIndex++] = a_PhysPatch.bladePhysData[dwBaseVertIndex + dwBladeZ * 4 + dwBladeX];

            dwBladeZ = 2;
            dwBladeX = 2;
            for ( i = 2; i < 4; ++i )
                for ( j = 2; j < 4; ++j )
                {
                    if (a_PhysPatch.bladePhysData[dwBaseVertIndex + i * 4 + j].fTransparency > 
                        a_PhysPatch.bladePhysData[dwBaseVertIndex + dwBladeZ * 4 + dwBladeX].fTransparency)
                    {
                        dwBladeZ = i;
                        dwBladeX = j;
                    }
                }
            bladePhysData[dwStartVertIndex++] = a_PhysPatch.bladePhysData[dwBaseVertIndex + dwBladeZ * 4 + dwBladeX];
        }        
    }
    else
    {
        /* fuck, that was just 1st case... so, now from lod1 => lod0 */
        /*delete [] bladePhysData;
        delete [] VertexBufData;
        numBlades = a_PhysPatch.numBlades * 4;
        bladePhysData = new BladePhysData[numBlades];
        VertexBufData = new VertexData[numBlades];*/
        for (dwBaseVertIndex = 0; dwBaseVertIndex < a_PhysPatch.numBlades; dwBaseVertIndex++)
        { 
            for ( i = 0; i < 4; ++i )
            {
                //bladePhysData[dwStartVertIndex] = a_PhysPatch.bladePhysData[dwBaseVertIndex];
                memcpy(bladePhysData + dwStartVertIndex, a_PhysPatch.bladePhysData + dwBaseVertIndex, sizeof(BladePhysData));
                BladePhysData *bp =  &bladePhysData[dwStartVertIndex];

                bp->position[0] = 
                bp->startPosition   = PosToWorld(m_pBasePatch->m_pVertices[dwBaseVertIndex * 4 + i].vPos);
                bp->startDirection  = m_pBasePatch->m_pVertices[dwBaseVertIndex * 4 + i].vRotAxe * (float)PI / 180.0f;
                bp->startDirectionY = m_pBasePatch->m_pVertices[dwBaseVertIndex * 4 + i].vYRotAxe * (float)PI / 180.0f;
                bp->fTransparency   = m_pBasePatch->m_pVertices[dwBaseVertIndex * 4 + i].fTransparency;
                dwStartVertIndex++;
            }
        }              
    }
}


/**
*/
static float maxvphi = 0.0;
void PhysPatch::UpdateBuffer()
{
    /* copy data */
    DWORD i, k;
    BladePhysData *bp;
    VertexPhysData  *vpd;
    VertexAnimData  *vad;
    /* updating buffer */
    VertexPhysData* pPhysVertices = NULL;
    VertexAnimData* pAnimVertices = NULL;
    m_dwVerticesCount[0] = m_dwVerticesCount[1] = 0;
    m_pPhysVertexBuffer->Map( D3D10_MAP_WRITE_DISCARD, NULL, ( void** )&pPhysVertices );
    m_pAnimVertexBuffer->Map( D3D10_MAP_WRITE_DISCARD, NULL, ( void** )&pAnimVertices );
    vpd = pPhysVertices;
    vad = pAnimVertices;
    for (i = 0; i < numBlades; i++)
    {
        bp = bladePhysData + i;
        
        if (bp->NeedPhysics)
        {
            for (k = 1; k < NUM_SEGMENTS; k++)
            {
                vpd->R[k-1] = bp->T[k];
                //	float3 vphi = MakeRotationVector(bp->T[k]);
                //	if (maxvphi < abs(vphi.z)) maxvphi = abs(vphi.z);
            }

            vpd->Position      = bp->startPosition;
            vpd->fTransparency = m_pBasePatch->m_pVertices[i].fTransparency;
            vpd->vColor = m_pBasePatch->m_pVertices[i].vColor;
            //if (bp->NeedPhysics == 1)
            //  vpd->vColor = float3(1, 0, 0);
            //if (bp->NeedPhysics == 2)
            //{
            //    if (bp->brokenFlag == -1)
            //        vpd->vColor = float3(1, 0, 0);
            //    else if (bp->brokenFlag == 0)
            //      vpd->vColor = float3(0, 1, 0);
            //    else if (bp->brokenFlag == 1)
            //        vpd->vColor = float3(0, 0, 1);
            //    else if (bp->brokenFlag == 2)
            //        vpd->vColor = float3(1, 0, 1);
            //    else if (bp->brokenFlag == 3)
            //        vpd->vColor = float3(1, 1, 1);
            //}
            m_dwVerticesCount[0]++;
            vpd++;
        }   
        else
        {
            vad->vPos          = bp->startPosition;
            vad->vRotAxe       = m_pBasePatch->m_pVertices[i].vRotAxe;//bp->startDirection;
            vad->vYRotAxe      = m_pBasePatch->m_pVertices[i].vYRotAxe;//bp->startDirectionY;
            vad->fTransparency = m_pBasePatch->m_pVertices[i].fTransparency;
            vad->vColor        = m_pBasePatch->m_pVertices[i].vColor;
            //vad->vColor        = float3(0, 0, 1);
            m_dwVerticesCount[1]++;
            vad++;
        } 
    }
    m_pPhysVertexBuffer->Unmap();
    m_pAnimVertexBuffer->Unmap();
}

/**
*/
void PhysPatch::SetTransform( const D3DXMATRIX * a_pMtx )
{
    m_pTransform = a_pMtx;
    /*if (a_pMtx)
        D3DXMatrixInverse(&m_InvTransform, 0, a_pMtx);*/
}

/**
*/
float3 PhysPatch::PosToWorld(const float3 &v)
{
    /*float4 Temp(v.x, v.y, v.z, 1.0f);
    float4 res;
    D3DXVec4Transform(&res, &Temp, m_pTransform);
    return *(float3*)(float*)(&res);*/
    float3 res;
    D3DXVec3TransformCoord(&res, &v, m_pTransform);
    return res;
}

/**
*/
float3 PhysPatch::DirToWorld(const float3 &v)
{
    float4 Temp(v.x, v.y, v.z, 0.0f);
    float4 res;
    D3DXVec4Transform(&res, &Temp, m_pTransform);
    return *(float3*)(float*)(&res);
}


/**
*/
//controlled
float PhysPatch::hardness;
float PhysPatch::mass;
float PhysPatch::windStrength;
float PhysPatch::fTerrRadius;
float PhysPatch::fWindTexTile;
const WindData *PhysPatch::pWindData = NULL;
float PhysPatch::fHeightScale = 0;
const TerrainHeightData *PhysPatch::pHeightData = NULL;

//non controlled
float PhysPatch::gravity = 10.0f;
float PhysPatch::dampfing = 0.98f;
float PhysPatch::maxAngle = (float)PI/2.0;
float PhysPatch::maxBrokenTime = 10.0f;
bool  PhysPatch::transmitSpringMomentDownwards = false;
float PhysPatch::eTime = 0.0f;



/***
*/
void PhysPatch::RotateSegment(PhysPatch::BladePhysData *bp, int j, const float3 &psi, const GrassPropsUnified &props){};
void RotateSegment(PhysPatch::BladePhysData *bp, int j, const float3 &psi, const GrassPropsUnified &props)
{
    D3DXMatrixMultiply(&bp->R[j], &bp->R[j], &MakeRotationMatrix(psi));
    D3DXMatrixMultiply(&bp->T[j], &bp->T[j - 1], &bp->R[j]);
    
    float3x3 transposed_t;
    float3 pos_delta, axis = float3(0.0, bp->segmentHeight, 0.0);
    D3DXMatrixTranspose(&transposed_t, &bp->T[j]);
    D3DXVec3TransformCoord(&pos_delta, &axis, &transposed_t);
	bp->position[j] = bp->position[j-1] + pos_delta;
}

float3 GetDw(float3x3 &T, float3x3 &R, float3 &fw, float3 &sum, float3 &w, float invJ, float segmentHeight, float Hardness, int j )
{
	float3 localSum, g, m_f;
	float3 halfAxis = float3(0.0f, segmentHeight*0.5f, 0.0f);
	
	D3DXVec3TransformCoord(&localSum, &sum, &T);
	D3DXVec3Cross(&m_f, &halfAxis, &localSum);
	
	g = Hardness * MakeRotationVector(R);
	return (m_f - g) * invJ;
}
void CalcTR(float3x3 &Tres, float3x3 &Rres, float3x3 &T, float3x3 &T_1, float3x3 &R, float3 &psi)
{
    D3DXMatrixMultiply(&Rres, &R, &MakeRotationMatrix(psi));
    D3DXMatrixMultiply(&Tres, &T_1, &Rres);
}


void BrokenAnim(float gTime, PhysPatch::BladePhysData *bp, const GrassPropsUnified &props)
{
  float3 proj, dir, axis;
  float3 vY = float3(0.0f, 1.0f, 0.0f);

        // Rotate in plane between blade and its projection on the ground
        dir = bp->position[1] - bp->position[0];
//        proj = a_pMeshes[0]->GetMoveDir();
        D3DXVec3Cross(&axis, &dir, &vY);
        D3DXVec3Normalize(&axis, &axis);
       // axis = proj;

        // Calculate angle of rotation
        float phase = bp->startDirectionY.y;

        axis *= sin(20.f*phase + gTime * 0.1f) * 0.005f;
		float3x3 mR = MakeRotationMatrix(axis);
		D3DXMatrixMultiply(&bp->T[1], &mR, &bp->T[1]);
//                axis *= sin(phase * gTimebp->brokenTime * 10.0f) * 0.02f;

//        D3DXVec3TransformCoord(&axis, &axis, &bp->T[1]);
//        RotateSegment(bp, 1, axis, props);
}
void PhysPatch::Broken(float3 &vNormal, float3 &dir, PhysPatch::BladePhysData *bp, float fDist, const GrassPropsUnified &props)
{
	float3 vx, dir1;
	D3DXMATRIX mRot, mRotX, mRotY, mRotXY, mRotZ;
	float a[3]={0.95f, 0.9f, 0.8f};
	//float a[3]={0.75f, 0.75f, 0.75f};

	//return;

//	float a[3]={0.92f, 0.85f, 0.7f};
//	float a[3]={0.5f, 0.5f, 0.5f};
	float3 cur_pos = float3(0.0f, 0.0f, 0.0f);
	
    int uS = rand()%2;
    if (uS==0) uS = -1;
    if (bp->brokenFlag == 2) uS = 1;
    if (bp->brokenFlag == 3) uS = -1; 

    float3 pos_delta;
    D3DXMATRIX transposed_t;
    float2 vTexCoord;
    float3 axis = float3(0.0, bp->segmentHeight, 0.0);

    if (bp->brokenFlag > 1)
    {
        float YRot = 640.f;
        int uR = 320;//uR = 150;

        /*
        D3DXVec3Cross(&dir1, &dir, &vNormal);

        D3DXMatrixLookAtLH(&mRot, &(cur_pos), &(cur_pos + dir), &dir1);

        float XRot = fDist * a[0];
        D3DXMatrixRotationZ(&mRotX, (float)XRot);
        D3DXMatrixIdentity(&mRotX);

        D3DXMatrixMultiply(&bp->T[1], &mRot, &mRotX);
        */
        D3DXMatrixIdentity(&bp->R[1]);
        D3DXMatrixIdentity(&bp->T[1]);

        D3DXVec3Cross(&dir1, &vNormal, &dir);
        D3DXVec3Cross(&dir, &vNormal, &dir1);
        D3DXVec3Normalize(&dir, &dir);

        float XRot = fDist * 1.2f;
        bp->T[1] = MakeRotationMatrix(dir * XRot * uS);
        D3DXMatrixTranspose(&transposed_t, &bp->T[0]);
        D3DXMatrixMultiply(&bp->R[1], &transposed_t, &bp->T[1]);
        
        D3DXMatrixTranspose(&transposed_t, &bp->T[1]);
        D3DXVec3TransformCoord(&pos_delta, &axis, &transposed_t);
        bp->position[1] = bp->position[0] + pos_delta;
        bp->w[1] = float3(0, 0, 0);

        // Update positions
        for (int j = 2; j < NUM_SEGMENTS; j++)
        {
            //D3DXMatrixTranspose(&transposed_t, &bp->T[j-1]);
            //D3DXMatrixMultiply(&bp->R[j], &transposed_t, &bp->T[j]);
            D3DXMatrixMultiply(&bp->T[j], &bp->T[j - 1], &bp->R[j]);
            
            D3DXMatrixTranspose(&transposed_t, &bp->T[j]);
            D3DXVec3TransformCoord(&pos_delta, &axis, &transposed_t);
            bp->position[j] = bp->position[j - 1] + pos_delta;
        }
        return;
    }

	for (int j = 1; j < NUM_SEGMENTS; j++)
	{
		int uR = rand()%128;//64;
		float YRot = 640.f;
		if (bp->brokenFlag > 1) uR = 320;//uR = 150;
		D3DXMatrixRotationY(&mRotY, (float)(uS*uR)*(float)M_PI / YRot);
		D3DXVec3TransformCoord(&dir1, &dir, &mRotY);
		D3DXVec3Cross(&vx, &dir1, &vNormal);

		D3DXVec3Cross(&dir1, &vNormal, &vx);
		D3DXMatrixLookAtLH(&mRot, &(cur_pos), &(cur_pos + dir1), &vNormal);

		float XRot = 0.8f;
		if (j>1)XRot = 0.99f;
		if (bp->brokenFlag > 1) XRot = fDist * a[j-1];
		D3DXMatrixRotationX(&mRotX, -(float)M_PI * 0.5f * XRot);
		D3DXMatrixMultiply(&bp->T[j], &mRot, &mRotX);

        D3DXMatrixTranspose(&transposed_t, &bp->T[j]);
        D3DXVec3TransformCoord(&pos_delta, &axis, &transposed_t);
        bp->position[j] = bp->position[j - 1] + pos_delta;

        vTexCoord = float2(bp->position[j].x / fTerrRadius * 0.5f + 0.5f, bp->position[j].z / fTerrRadius * 0.5f + 0.5f);
        float height = pHeightData->GetHeight(vTexCoord.x, vTexCoord.y) * fHeightScale;

        float angle = 0.0f;
        if (bp->position[j].y > height + 0.2f)
            angle = -asinf(clamp((bp->position[j].y - height - 0.2f) / bp->segmentHeight, 0, 1));
        else if (bp->position[j].y < height)
            angle = asinf(clamp((height + 0.5f - bp->position[j].y) / bp->segmentHeight, 0, 1));

        if (angle != 0.0f)
        {
            D3DXMatrixRotationX(&mRotX, angle);
            D3DXMatrixMultiply(&bp->T[j], &bp->T[j], &mRotX);

            // Update position
            D3DXMatrixTranspose(&transposed_t, &bp->T[j]);
            D3DXVec3TransformCoord(&pos_delta, &axis, &transposed_t);
            bp->position[j] = bp->position[j - 1] + pos_delta;
        }
    }
}
/*void Broken(float3 &vNormal, float3 &dir, PhysPatch::BladePhysData *bp)
{
	float3 vx, vt;
	D3DXMATRIX mRot, mRotX, mRotY, mRotXY;
	float a[3]={(float)M_PI / 2.5f, (float)M_PI / 3.0f, (float)M_PI / 4.0f};

	float TestLen = D3DXVec3Length(&vNormal);
	if (TestLen < 0.001f)
		TestLen = 1.0f;

	D3DXVec3Normalize(&vNormal, &vNormal);
	D3DXVec3Normalize(&dir, &dir);
	D3DXVec3Cross(&vx, &vNormal, &dir);
	D3DXVec3Cross(&dir, &vNormal, &vx);
	if (bp->brokenFlag == 1)
	{
		for (int j = 1; j < NUM_SEGMENTS; j++)   
		{			
			bp->T[j] = MakeRotationMatrix(vx * (float)M_PI / 2.2f);
		}
	}
	else
	{
		float fS = 1.0f;
		if (bp->brokenFlag == 2) fS = -1.0f;
		vt = dir + 2.f*fS*vx;
		D3DXVec3Normalize(&vt, &vt);

		for (int j = 1; j < NUM_SEGMENTS; j++)   
		{			
			bp->T[j] = MakeRotationMatrix(fS * vt * a[j-1]);
		}
	}
	
}*/

void PhysPatch::Animatin(PhysPatch::BladePhysData *bp, float2 &vTexCoord, Mesh *a_pMeshes[], const GrassPropsUnified &props)
{
	float3 g = float3(0.0f, -9.8f, 0.0f);
	float3 halfAxis = float3(0.0f, bp->segmentHeight*0.5f, 0.0f);
    float3 axis = float3(0.0, bp->segmentHeight, 0.0);
    float3 sum = float3(0.0f, 0.0f, 0.0f), localSum;

//	bp->NeedPhysics = 0;
//	bp->NeedPhysics = 1;
    float3 w, w_;
	for (int j = 1; j < NUM_SEGMENTS; j++)
	{
		bp->w[j] = float3(0, 0, 0);
		bp->T[j] = bp->T[j - 1];
		for (int k = 0; k < 4; k++)
		{
//			sum = g * props.vMassSegment[j-1] * bp->T[j][5];
			sum = g * props.vMassSegment[j-1];
			D3DXVec3TransformCoord(&localSum, &sum, &bp->T[j]);
			float3 G;
			D3DXVec3Cross(&G, &halfAxis, &localSum);
			bp->R[j] = MakeRotationMatrix(G / props.vHardnessSegment[j-1]);
			D3DXMatrixMultiply(&bp->T[j], &bp->T[j-1], &bp->R[j]);
		}

		w_ = pWindData->GetWindValueA(vTexCoord, fWindTexTile, windStrength, j - 1);
//		sum = g * props.vMassSegment[j-1] * bp->T[j][5];
		sum = g * props.vMassSegment[j-1];
		D3DXVec3TransformCoord(&localSum, &sum, &bp->T[j]);
		float3 G;
		D3DXVec3Cross(&G, &halfAxis, &localSum);
		D3DXVec3TransformCoord(&w, &w_, &bp->T[j]);
		G += w;
		bp->R[j] = MakeRotationMatrix(G / props.vHardnessSegment[j-1]);
		D3DXMatrixMultiply(&bp->T[j], &bp->T[j-1], &bp->R[j]);

		float3x3 transposed_t;
		D3DXMatrixTranspose(&transposed_t, &bp->T[j]);
		float3 pos_delta;
		D3DXVec3TransformCoord(&pos_delta, &axis, &transposed_t);
		bp->position[j] = bp->position[j-1] + pos_delta;
		
		if (a_pMeshes[0]->CheckCollision(bp->position[j - 1], bp->position[j], NULL))
		{
			float3 psi;
			if (a_pMeshes[0]->Collide(&psi, bp->position[j - 1], bp->position[j], bp, j))
			{
				D3DXVec3TransformCoord(&psi, &psi, &bp->T[j]);       
				RotateSegment(bp, j, psi, props);
                for (int i = j+1; i < NUM_SEGMENTS; i++)
					D3DXMatrixMultiply(&bp->T[i], &bp->T[i - 1], &bp->R[i]);

				bp->A[j] = bp->T[j];
			}
			bp->NeedPhysics = 2;
			bp->physicTime = 0;
		}
	
	}
}

static float3 GetVel(float3x3 &T, float3 &w, float segmentHeight)
{
	float3 halfAxis = float3(0.0f, segmentHeight *0.5f, 0.0f), r;
	float3x3 transposed_t;
    D3DXMatrixTranspose(&transposed_t, &T);
    D3DXVec3Cross(&r, &w, &halfAxis);
    D3DXVec3TransformCoord(&r, &r, &transposed_t);
    return r;
}

void Phisics(PhysPatch::BladePhysData *bp, float3 &w, float dTime, Mesh *a_pMeshes[], const GrassPropsUnified &props)
{
	float3 g = float3(0.0f, -9.8f, 0.0f);
	float3 halfAxis = float3(0.0f, bp->segmentHeight*0.5f, 0.0f);
    float3 axis = float3(0.0, bp->segmentHeight, 0.0);
    float3 sum = float3(0.0f, 0.0f, 0.0f);
	float3 psi;
	float d = powf(0.98f, dTime * 0.01f);
	if (d > 0.9998f) d = 0.9998f;
    int start_segment = 1;
    if (bp->brokenFlag > 1)
        start_segment = 2;

    float3 wind, vZ(0.0f, 0.0f, 0.0f), v, r;
	for (int j = start_segment; j < NUM_SEGMENTS; j++)   
	{
		float oneThirdMMulLSqr = 0.33f * props.vMassSegment[j-1] * bp->segmentHeight*bp->segmentHeight;
		float invJ = 0.75f*1.0f / oneThirdMMulLSqr;
		r = GetVel(bp->T[j], bp->w[j], bp->segmentHeight);
		v = vZ + r;
		wind = 0.02f * (w - v);

		float h = props.vHardnessSegment[j-1];
//		sum = g * props.vMassSegment[j-1] * bp->T[j][5] + w;
		sum = g * props.vMassSegment[j-1] + wind;

		float3 Dw = GetDw(bp->T[j], bp->R[j], bp->w[j], sum, wind, invJ, bp->segmentHeight, h, j);
		float3 w_ = bp->w[j] + dTime * Dw;
		psi = dTime * w_;

		float3x3 Tres, Rres;
		CalcTR(Tres, Rres, bp->T[j], bp->T[j-1], bp->R[j], psi);

		r = GetVel(Tres, w_, bp->segmentHeight);
		v = vZ + r;
		wind = 0.02f * (w - v);
//		sum = g * props.vMassSegment[j-1] * Tres[5] + w;
		sum = g * props.vMassSegment[j-1] + wind;
		float3 Dw_ = GetDw(Tres, Rres, w_, sum, wind, invJ, bp->segmentHeight, h, j);
		bp->w[j] += 0.5f * dTime * (Dw + Dw_);
        bp->w[j] *= d;
		psi = dTime * bp->w[j];
		RotateSegment(bp, j, psi, props);

		r = GetVel(bp->T[j], bp->w[j], bp->segmentHeight);
		vZ = vZ + 2.f * r;

		if (a_pMeshes[0]->Collide(&psi, bp->position[j - 1], bp->position[j], bp, j))
		{
			D3DXVec3TransformCoord(&psi, &psi, &bp->T[j]);
			RotateSegment(bp, j, psi, props);
			bp->w[j] = float3(0, 0, 0);
            
			for (int i = j+1; i < NUM_SEGMENTS; i++)
			{
			    D3DXMatrixMultiply(&bp->T[i], &bp->T[i - 1], &bp->R[i]);
			}
			for (int i = 1; i < NUM_SEGMENTS; i++)
				bp->w[i] = float3(0, 0, 0);
            
		}
	}
}



static float gTime = 0.f;
void PhysPatch::UpdatePhysics(const float3 &viewPos, float physLodDst, bool collision, float3 spherePosition, float sphereRadius, float dTime, const std::vector<GrassPropsUnified> &grassProps, const IndexMapData &indexMapData, Mesh *a_pMeshes[], UINT a_uNumMeshes )
{
    float3 w(0.0f, 0.0f, 0.0f);
	float a[3]={(float)M_PI / 2.5f, (float)M_PI / 10.0f, (float)M_PI / 10.0f};
	float3 g = float3(0.0f, -9.8f, 0.0f);
	float AnimOrPhys = physLodDst;
//		TerrainHeightData *pHD = m_pTerrain->HeightDataPtr();

	gTime += dTime;
	if (dTime >= 0.1f)
		dTime = 0.1f;

	if (dTime <= 0.001f)
		dTime = 0.001f;

	float3 sum, localSum;
	float3 psi;
	float3 vDir = spherePosition - viewPos;
	vDir.y = 0.0f;
	AnimOrPhys = D3DXVec3Length(&vDir);
	D3DXVec3Normalize(&vDir, &vDir);

  	for (DWORD i = 0; i < numBlades; i++)
//  	for (DWORD i = 0; i < 1; i++)
	{
		BladePhysData *bp = &bladePhysData[i];
		float3 vDirP = bp->startPosition - viewPos;  
//		D3DXVec3Normalize(&vDirP, &vDirP);
	    float dot = D3DXVec3Dot( &vDir, &vDirP );
		//if (dot < 3.5f) 
		//	continue;
                      
		//texcoord
        float2 vTexCoord = float2(bp->startPosition.x / fTerrRadius * 0.5f + 0.5f, bp->startPosition.z / fTerrRadius * 0.5f + 0.5f);
        
		//index map
        UINT uX = UINT(vTexCoord.x * (indexMapData.uWidth-1));
        UINT uY = UINT(vTexCoord.y * (indexMapData.uHeight-1));
        UCHAR subTypeIndex = 0;
        if (indexMapData.pData)
            subTypeIndex = indexMapData.pData[uX + uY * indexMapData.uWidth];
        const GrassPropsUnified &props = grassProps[subTypeIndex];

		// height map
		float height = pHeightData->GetHeight(vTexCoord.x, vTexCoord.y) * fHeightScale;
		bp->startPosition.y = height;
		bp->position[0] = bp->startPosition;

		/*********************
		Calculations
		*********************/

        bool near_car = false;
        bp->segmentHeight = props.vSizes.y;
        bp->segmentWidth = props.vSizes.x;
		
		float r = sphereRadius + 0.12f;
        float br = sphereRadius + NUM_SEGMENTS * bp->segmentHeight;


        if (D3DXVec3Length(&(bp->position[0] - spherePosition)) < 0.7f*br)
			near_car = true;
//		if (bp->brokenFlag == -1) continue;
		if (bp->brokenFlag == -1)
		{
				BrokenAnim(gTime, bp, props);
				bp->NeedPhysics = 1;
				continue;
		}
		if (near_car)
		{
			float3 vNormal, vDist;
			int OldbrokenFlag = bp->brokenFlag;
//			bp->brokenFlag = a_pMeshes[0]->IsBottom(bp->position[0], vNormal);
			bp->brokenFlag = a_pMeshes[0]->IsBottom(bp->position[0], vDist);
			vNormal = pHeightData->GetNormal(vTexCoord.x, vTexCoord.y);
			if (bp->brokenFlag > 0)	
			{
				float3 dir = a_pMeshes[0]->GetMoveDir();
                Broken(vNormal, dir, bp, vDist.x, props);
				bp->NeedPhysics = 1;
				if (bp->brokenFlag == 1) bp->brokenFlag = -1;
                if (bp->brokenFlag > 1)
                    bp->NeedPhysics = 2;
			}
			else
			{
				if (OldbrokenFlag > 0)
				{
					if (OldbrokenFlag == 1)
					{
						bp->brokenFlag = -1;
						bp->NeedPhysics = 1;
						continue;
					}
					else
					{
						bp->NeedPhysics = 2;
                        //if (OldbrokenFlag > 1)
                        //    bp->brokenFlag = OldbrokenFlag;
					}
				}
				//else if (bp->NeedPhysics != 2)
                //    bp->NeedPhysics = 1;
			}
			{
				if (bp->brokenFlag == 0 && bp->NeedPhysics != 2)
				{
					//float3 w = pWindData->GetWindValueA(vTexCoord, fWindTexTile, windStrength, 0);
					Animatin(bp, vTexCoord, a_pMeshes, props);
				}
			}
		}
		if (bp->NeedPhysics == 2)
		{
            float3 w = pWindData->GetWindValue(vTexCoord, fWindTexTile, windStrength);

//					Animatin(bp, vTexCoord, a_pMeshes, props);
			  Phisics(bp, w, dTime, a_pMeshes, props);

//            float3 w = pWindData->GetWindValueA(vTexCoord, fWindTexTile, windStrength, 0);
//            Animatin(bp, vTexCoord, a_pMeshes, props);
        }
	}//for (DWORD i = 0; i < numBlades; i++)

    UpdateBuffer();
}

/*
	if (bp->brokenFlag == -1) continue;
	if (near_car)
	{
		float3 vNormal;
		int OldbrokenFlag = bp->brokenFlag;
		bp->brokenFlag = a_pMeshes[0]->IsBottom(bp->position[0], vNormal);
		if (bp->brokenFlag > 0)	bp->NeedPhysics = 1;
		else
		{
			if (OldbrokenFlag > 0)
			{
				if (OldbrokenFlag == 1)
				{
					bp->brokenFlag = -1;
					bp->NeedPhysics = 1;
					continue;
				}
				else
				{
					bp->NeedPhysics = 2;
				}
			}
			else bp->NeedPhysics = 1;
		}
		if (bp->brokenFlag > 0)
		{
			float3 dir = a_pMeshes[0]->GetMoveDir();
			Broken(vNormal, dir, bp);
			bp->NeedPhysics = 1;
		}
		else
		{
//					if (bp->brokenFlag != -1 && dot < AnimOrPhys)
			{
				float3 w = pWindData->GetWindValueA(vTexCoord, fWindTexTile, windStrength);
				Animatin(bp, w, a_pMeshes, props);
			}
		}
	}
	if (bp->NeedPhysics ==2)			{
		float3 w = pWindData->GetWindValueA(vTexCoord, fWindTexTile, windStrength);
		Phisics(bp, w, dTime, a_pMeshes, props);
	}
*/
