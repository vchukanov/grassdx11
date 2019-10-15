#include "PhysPatch.h"
#include "GrassManager.h"


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
			D3DXMatrixMultiply(bp->T + j, &MakeRotationMatrix(bp->startDirectionY), &MakeRotationMatrix(bp->startDirection));
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
   return;
}


/**
*/
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
            }

            vpd->Position      = bp->startPosition;
            vpd->segmentHeight = bp->segmentHeight;
            vpd->segmentWidth  = bp->segmentWidth;
            vpd->fTransparency = m_pBasePatch->m_pVertices[i].fTransparency;
            m_dwVerticesCount[0]++;
            vpd++;
        }
        else
        {
            vad->vPos          = bp->startPosition;
            vad->vRotAxe       = m_pBasePatch->m_pVertices[i].vRotAxe;//bp->startDirection;
            vad->vYRotAxe      = m_pBasePatch->m_pVertices[i].vYRotAxe;//bp->startDirectionY;
            vad->fTransparency = m_pBasePatch->m_pVertices[i].fTransparency;
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
void PhysPatch::RotateSegment(PhysPatch::BladePhysData *bp, int j, const float3 &psi, const GrassPropsUnified &props)
{
    D3DXMatrixMultiply(&bp->T[j], &bp->T[j], &MakeRotationMatrix(psi));

	float3 axis = float3(0.0, bp->segmentHeight, 0.0);
    float3x3 transposed_t;

    D3DXMatrixTranspose(&transposed_t, &bp->T[j-1]);
    D3DXMatrixMultiply(&bp->R[j], &transposed_t, &bp->T[j]);

    float3 pos_delta;
    D3DXMatrixTranspose(&transposed_t, &bp->T[j]);
    D3DXVec3TransformCoord(&pos_delta, &axis, &transposed_t);
	bp->position[j] = bp->position[j-1] + pos_delta;
		
	//compute spring moment
	float3 phi = MakeRotationVector(bp->R[j]);

    if (bp->brokenFlag == 1)
		if (bp->brokenTime < maxBrokenTime * 0.5f)
			bp->g[j] = float3(0.0f, 0.0f, 0.0f);
		else
			bp->g[j] = phi * props.vHardnessSegment[j-1] * hardness * 2.0f * (bp->brokenTime / maxBrokenTime - 0.5f);
	else
		bp->g[j] = phi * props.vHardnessSegment[j-1] * hardness;

	//restore brokeness
	if (bp->brokenFlag == 1 && bp->brokenTime > maxBrokenTime)
	{
		bp->brokenTime = 0.0f;
		bp->brokenFlag = 0;
        bp->grabbedMeshIndex = -1;
	}
}

void PhysPatch::UpdatePhysics(const float3 &viewPos, float physLodDst, bool collision, float3 spherePosition, float sphereRadius, float dTime, const std::vector<GrassPropsUnified> &grassProps, const IndexMapData &indexMapData, Mesh *a_pMeshes[], UINT a_uNumMeshes )
{
    float3 w(0.0f, 0.0f, 0.0f);
	float3 g = float3(0.0f, -10.0f, 0.0f);
	
	if (dTime >= 0.1f)
		dTime = 0.1f;

	if (dTime <= 0.001f)
		dTime = 0.001f;

	float d = powf(dampfing, dTime * 200.0f);
	if (d > 0.99f) d = 0.99f;

	float3 sum, localSum;
	float3 psi;

  	for (DWORD i = 0; i < numBlades; i++)
	{
		BladePhysData *bp = &bladePhysData[i];
                      
		//texcoord
        float2 vTexCoord = float2(bp->startPosition.x / fTerrRadius * 0.5f + 0.5f, bp->startPosition.z / fTerrRadius * 0.5f + 0.5f);
		w = pWindData->GetWindValue(vTexCoord, fWindTexTile, windStrength);
		
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
		{
            bool disable_physics = false;
          
            bp->segmentHeight = props.vSizes.y;
            bp->segmentWidth = props.vSizes.x;
			
			float3 halfAxis = float3(0.0f, bp->segmentHeight*0.5f, 0.0f);
            float3 axis = float3(0.0, bp->segmentHeight, 0.0);

            sum = float3(0.0f, 0.0f, 0.0f);

			//J
			//float r = sqrtf(sphereRadius*sphereRadius + bp->segmentHeight*bp->segmentHeight/4.0f);
            float r = sphereRadius + 0.12f;
            float br = sphereRadius + NUM_SEGMENTS * bp->segmentHeight;

			float planeY = bp->startPosition.y + 0.05f;

            if (bp->NeedPhysics)
              bp->physicTime += dTime;

            if (D3DXVec3Length(&(bp->position[0] - spherePosition)) > br)
				disable_physics = true;

            bool is_under_wheel = false;
/*
            for (UINT k = 0; k < a_uNumMeshes; k++)
            {
                bool current_wheel_flag;
                if (a_pMeshes[k]->GetDist(bp->position[0], &current_wheel_flag) >= INVALID_DIST)
                {
                    disable_physics = true;
                    break;
                }
                if (current_wheel_flag)
                    is_under_wheel = true;
            }
 */
            if (bp->brokenFlag == 1)
            {
                bp->brokenTime += dTime;

                // Animate track to make it more funny
                if (!is_under_wheel && a_uNumMeshes >= 1)
                {
                    float3 proj, dir, axis;

                    // Rotate in plane between blade and its projection on the ground
                    dir = bp->position[1] - bp->position[0];
                    proj = a_pMeshes[0]->GetMoveDir();
                    D3DXVec3Cross(&axis, &dir, &proj);
                    D3DXVec3Normalize(&axis, &axis);
                    axis = proj;

                    // Calculate angle of rotation
                    float phase = bp->startDirectionY.y;

                    axis *= sin(phase * bp->brokenTime * 10.0f) * 0.02f;

                    D3DXVec3TransformCoord(&axis, &axis, &bp->T[1]);
                    RotateSegment(bp, 1, axis, props);
                }
                continue;
            }

            /*********************
			Segments
			*********************/
            bool old_broken = bp->brokenFlag;

            bp->disableCollision = false;

            for (int j = 1; j < NUM_SEGMENTS; j++)   
            {
                float3x3 transposed_t;
                D3DXMatrixTranspose(&transposed_t, &bp->T[j]);

                float3 pos_delta;
                D3DXVec3TransformCoord(&pos_delta, &axis, &transposed_t);

                bp->position[j] = bp->position[j-1] + pos_delta;

                if (!disable_physics && !bp->NeedPhysics &&
                    a_pMeshes[0]->CheckCollision(bp->position[j - 1], bp->position[j], NULL))
                {
                    bp->NeedPhysics = true;
                    bp->physicTime = 0;
                }
                if (bp->physicTime > maxBrokenTime * 1.5)
                {
                    bp->NeedPhysics = false;
                    bp->grabbedMeshIndex = -1;
                    bp->disableCollision = false;
                }

				/*****************************
				Animation
				*****************************/
			    sum = g * mass * props.vMassSegment[j-1] + w;           
                D3DXVec3TransformCoord(&localSum, &sum, &bp->T[j - 1]);
				float oneThirdMMulLSqr = /*2.0f * 1.0f/3.0f*/0.33f * props.vMassSegment[j-1] * mass*bp->segmentHeight*bp->segmentHeight;
                float3 invJ = oneOver(float3(oneThirdMMulLSqr, oneThirdMMulLSqr, oneThirdMMulLSqr));

				/*****************************
				Physics
				*****************************/
                if (!bp->brokenFlag)
 					sum = g * mass * props.vMassSegment[j-1] + w;
                else if (bp->brokenFlag && bp->brokenTime > maxBrokenTime * 0.5f)
                    sum = 2 * (g * mass * props.vMassSegment[j-1] + w) * (bp->brokenTime / maxBrokenTime - 0.5f);
				else
					sum = float3(0.0f, 0.0f, 0.0f);

		        D3DXVec3TransformCoord(&localSum, &sum, &bp->T[j]);

                float3 m_f;
                D3DXVec3Cross(&m_f, &halfAxis, &localSum);
                bp->w[j] += dTime * mul(m_f - bp->g[j], invJ);
                bp->w[j] *= d;
                psi = dTime * bp->w[j];
                RotateSegment(bp, j, psi, props);
        
				
                /*********************
                Collision
                *********************/
                float3 psi;

                if (a_pMeshes[0]->Collide(&psi, bp->position[j - 1], bp->position[j], bp, j))
                {
                    D3DXVec3TransformCoord(&psi, &psi, &bp->T[j]);                        
                  //  RotateSegment(bp, j, psi, props);
                    bp->g[j] = float3(0, 0, 0);
                    bp->w[j] = float3(0, 0, 0);

                    // Rotate next segment backwards to make more realistic collision with bottom of the plane
                    if (j < NUM_SEGMENTS - 1)
                    {
                        psi = -psi / 2.0f;
                    //    RotateSegment(bp, j + 1, psi, props);
                    }
				}
			}
            // If we have just broke blade, rotate it to to make more real car track
            if (bp->brokenFlag && !old_broken)
            {
                //float3 tmp(0, a_pBladePhysData->startDirection.y, 0);
                float3 psi;

                for (int j = 1; j < NUM_SEGMENTS; j++)
                {
                    float3 left, up, dir;

                    dir = bp->position[j] - bp->position[j - 1];

                    // We need to rotate around landscape normal
                    D3DXVec3Cross(&left, &dir, &D3DXVECTOR3(0, 1, 0));
                    D3DXVec3Cross(&up, &left, &dir);
                    D3DXVec3Normalize(&up, &up);

                    // Randomly choose direction of rotate
                    psi = up * (D3DXVec3Length(&bp->startDirectionY));
                    if  (is_under_wheel)
                        psi /= 6;
                    if (rand() % 2)
                      psi = -psi;

                    // Rotate
                    D3DXVec3TransformCoord(&psi, &psi, &bp->T[j]);
                    RotateSegment(bp, j, psi, props);
                }
            }
		}
	}

    UpdateBuffer();
}
