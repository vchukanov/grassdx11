#include "PhysPatch.h"
#include "GrassManager.h"

/**
*/
PhysPatch::PhysPatch(GrassPatch *a_pGrassPatch)
{        
    m_pBasePatch = a_pGrassPatch;
    m_pD3DDevice = a_pGrassPatch->GetD3DDevicePtr();
    m_dwVertexStride  = sizeof(VertexData);
    m_dwVertexOffset  = 0;
	
	this->numBlades = a_pGrassPatch->VerticesCount();
	this->bladePhysData = new BladePhysData[numBlades];
	this->animationPass = true;
    
	VertexBufData = new VertexData[numBlades];
     
	//
    GenerateBuffer();
}

/**
*/
void PhysPatch::GenerateBuffer()
{
    D3D10_BUFFER_DESC bufferDesc =
    {
        numBlades * sizeof( VertexData ),
        D3D10_USAGE_DYNAMIC,
        D3D10_BIND_VERTEX_BUFFER,
        D3D10_CPU_ACCESS_WRITE,
        0
    };

    m_pD3DDevice->CreateBuffer( &bufferDesc, NULL, &m_pVertexBuffer );

    VertexData* pVertices = NULL;
    m_pVertexBuffer->Map( D3D10_MAP_WRITE_DISCARD, NULL, ( void** )&pVertices );

    memcpy( pVertices, &VertexBufData[0], numBlades * sizeof( VertexData ) );

    m_pVertexBuffer->Unmap();
}

/**
*/
void PhysPatch::IASetVertexBuffer0( )
{
    m_pD3DDevice->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_dwVertexStride, &m_dwVertexOffset);
}

/**
*/
DWORD PhysPatch::VerticesCount( )
{
    return numBlades;
}

/**
*/
PhysPatch::~PhysPatch()
{
	delete [] bladePhysData;
    delete [] VertexBufData;
    SAFE_RELEASE(m_pVertexBuffer);
}

/**
*/
void PhysPatch::Reinit()
{
	this->animationPass = true;

	for (UINT i = 0; i < numBlades; i++)
    {
		BladePhysData *bp = &bladePhysData[i];   
        
        bp->startPosition   = PosToWorld(m_pBasePatch->m_pVertices[i].vPos);
        bp->startDirection  = m_pBasePatch->m_pVertices[i].vRotAxe * 0.01745f;//* (float)PI / 180.0f; 
        bp->startDirectionY = m_pBasePatch->m_pVertices[i].vYRotAxe * 0.01745f;//* (float)PI / 180.0f; 

		for (UINT j = 0; j < NUM_SEGMENTS; j++)
        {
			bp->w[j] = float3(0, 0, 0);
            bp->g[j] = float3(0, 0, 0);
            bp->R[j] = identity();
			bp->T[j] = identity();//MakeRotationMatrix(bp->startDirectionY) * MakeRotationMatrix(bp->startDirection);
            bp->A[j] = identity();//bp->T[j];
        }
        bp->NeedPhysics = 0;
        bp->brokenFlag = 0;
        bp->brokenTime = 0.0f;

		//bp->A[0] = bp->T[0];
        bp->T[0] = MakeRotationMatrix(bp->startDirectionY) * MakeRotationMatrix(bp->startDirection);
        bp->A[0] = bp->T[0];
		bp->position[0] = bp->startPosition;

        //
        VertexData *vd = &VertexBufData[i];
        vd->fTransparency = m_pBasePatch->m_pVertices[i].fTransparency;;
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
void PhysPatch::UpdateBuffer()
{
    /* copy data */
    DWORD i, k;
    BladePhysData *bp;
    VertexData  *vd;
    /* updating buffer */
    VertexData* pVertices = NULL;
    m_pVertexBuffer->Map( D3D10_MAP_WRITE_DISCARD, NULL, ( void** )&pVertices );
    for (i = 0; i < numBlades; i++)
    {
        bp = &bladePhysData[i];
        vd = pVertices + i;//&VertexBufData[i];
        for (k = 1; k < NUM_SEGMENTS; k++)
		{
           	/*if (bp->lerpCoef == 1.0f)
			{
				vd->R[k-1] = bp->A[k];
			}
			else if (bp->lerpCoef == 0.0f)
			{
				vd->R[k-1] = bp->T[k];
			}
			else
			{
				float3 va = MakeRotationVector(bp->A[k]);
				float3 vt = MakeRotationVector(bp->T[k]);
				float3 vr = va * bp->lerpCoef + vt * (1.0f - bp->lerpCoef);

				vd->R[k-1] = MakeRotationMatrix(vr);
			}*/
            vd->R[k-1] = bp->T[k];
		}

		vd->Position = bp->startPosition;
        vd->SegmentHeight = bp->segmentHeight;
        vd->SegmentWidth = bp->segmentWidth;
        vd->fTransparency = m_pBasePatch->m_pVertices[i].fTransparency;
    }

    //memcpy( pVertices, &VertexBufData[0], numBlades * sizeof( VertexData ) );

    m_pVertexBuffer->Unmap();
}

/**
*/
void PhysPatch::SetTransform( const D3DXMATRIX * a_pMtx )
{
    m_pTransform = a_pMtx;
    D3DXMatrixInverse(&m_InvTransform, 0, a_pMtx);
}

/**
*/
float3 PhysPatch::PosToWorld(const float3 &v)
{
    float4 Temp(v.x, v.y, v.z, 1.0f);
    float4 res;
    D3DXVec4Transform(&res, &Temp, m_pTransform);
    return *(float3*)(float*)(&res);
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
float PhysPatch::fGrassRadius;
float PhysPatch::fWindTexTile;
const WindData *PhysPatch::pWindData = NULL;
float PhysPatch::fHeightScale = 0;
const TerrainHeightData *PhysPatch::pHeightData = NULL;

//non controlled
float PhysPatch::gravity = 10.0f;
float PhysPatch::dampfing = 0.98f;
float PhysPatch::maxAngle = (float)PI/2.0;
float PhysPatch::maxBrokenTime = 8.0f;
bool  PhysPatch::transmitSpringMomentDownwards = false;
float PhysPatch::eTime = 0.0f;



/***
*/
void PhysPatch::RotateSegment(PhysPatch::BladePhysData *bp, int j, const float3 &psi, const GrassPropsUnified &props)
{
	bp->T[j] = mul(bp->T[j], MakeRotationMatrix(psi));

	float3 axis = float3(0.0, bp->segmentHeight, 0.0);
	bp->R[j] = mul(transpose(bp->T[j-1]), bp->T[j]);
	bp->position[j] = bp->position[j-1] + mul(bp->T[j], axis);
		
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
	}
}

void PhysPatch::UpdatePhysics(const float3 &viewPos, float physLodDst, bool collision, float3 spherePosition, float sphereRadius, float dTime, const std::vector<GrassPropsUnified> &grassProps, const IndexMapData &indexMapData)
{
	float3 w;
	float3 g = float3(0.0f, -10.0f, 0.0f);
	
	if (dTime >= 0.1f)
		dTime = 0.1f;

	if (dTime <= 0.001f)
		dTime = 0.001f;

	float d = powf(dampfing, dTime * 200.0f);
	//if (d < 0.9f) d = 0.9f;
	if (d > 0.99f) d = 0.99f;

	float3 sum, localSum;
	float3 psi;

	//omp_set_dynamic(0);     
    //omp_set_num_threads(numBlades);
	//#pragma omp parallel shared(a, b, c) private(i)
	//{
	//#pragma omp for

  	for (DWORD i = 0; i < numBlades; i++)
	{
		BladePhysData *bp = &bladePhysData[i];
                      
		//texcoord
        float2 vTexCoord = float2(bp->startPosition.x / fGrassRadius * 0.5f + 0.5f, bp->startPosition.z / fGrassRadius * 0.5f + 0.5f);
        
        //wind
        //UINT uX = UINT(vTexCoord.x * fWindTexTile * (pWindData->uWidth-1)) % pWindData->uWidth;
        //UINT uY = UINT(vTexCoord.y * fWindTexTile * (pWindData->uHeight-1)) % pWindData->uHeight;
        //float3 vGlobalWind = pWindData->pData[uX + uY * pWindData->uWidth];//(2.0f * pWindData->pData[uX + uY * pWindData->uWidth] - float3(1.0f, 1.0f, 1.0f));
        //float4 v4Wind = float4(vGlobalWind.x, vGlobalWind.y, vGlobalWind.z, 0.0f);        
        //float3 *pWind = (float3*)((float*)(&v4Wind));
        //w = windStrength * *pWind;
        w = pWindData->GetWindValue(vTexCoord, fWindTexTile, windStrength);
        
        	
        //index map
        UINT uX = UINT(vTexCoord.x * (indexMapData.uWidth-1));
        UINT uY = UINT(vTexCoord.y * (indexMapData.uHeight-1));
        UCHAR subTypeIndex;
        if (indexMapData.pData)
            subTypeIndex = indexMapData.pData[uX + uY * indexMapData.uWidth];
        else 
            subTypeIndex = 0;
        const GrassPropsUnified &props = grassProps[subTypeIndex];

		//height map
		float height = pHeightData->GetHeight(vTexCoord.x, vTexCoord.y) * fHeightScale;
		bp->startPosition.y = height;
		bp->position[0] = bp->startPosition;

		/*********************
		Calculations
		*********************/
		{
            //
            bp->segmentHeight = props.vSizes.y;
            bp->segmentWidth = props.vSizes.x;
			
			
			//interpolation
			/*bp->lerpCoef = length(bp->startPosition - viewPos) / physLodDst;
			bp->lerpCoef = (bp->lerpCoef - 0.5f) * 2.0f;
            bp->lerpCoef *= bp->lerpCoef;
            if (bp->lerpCoef < 0.0f) bp->lerpCoef = 0.0f;
			if (bp->lerpCoef > 1.0f) bp->lerpCoef = 1.0f;*/
            
			float3 halfAxis = float3(0.0f, bp->segmentHeight*0.5f, 0.0f);
            float3 axis = float3(0.0, bp->segmentHeight, 0.0);

            //sum = float3(0.0f, 0.0f, 0.0f);

			//J
			//float r = sqrtf(sphereRadius*sphereRadius + bp->segmentHeight*bp->segmentHeight/4.0f);
            float r = sphereRadius + 0.12f;
            float br = sphereRadius + 0.2f;

			float planeY = bp->startPosition.y;

			//collision
			if (bp->brokenFlag == 1)
				bp->brokenTime += dTime;
			
			//
			if (length(bp->position[0] - spherePosition) < br)
				bp->brokenFlag = 1;

			/*********************
			Segments
			*********************/
			for (int j = 1; j < NUM_SEGMENTS; j++)   
			{	
				bp->position[j] = bp->position[j-1] + mul(bp->T[j], axis);
				bool sphereCollision = length(bp->position[j] - spherePosition) < r;
				bool planeCollision = bp->position[j].y < planeY - 0.1f;
                //bool bNoMove = (D3DXVec3Length(bp->w+j) < 0.001f);

                //if (sphereCollision) bp->brokenFlag = 1;

				/*****************************
				Animation
				*****************************/
				//if (animationPass || bp->lerpCoef > 0.0f)
                if (sphereCollision || planeCollision)
                    bp->NeedPhysics = 1;
                //bool bIsStatic = !sphereCollision & !planeCollision;// & bNoMove;               
                
                //sum = g * mass * props.vMassSegment[j-1] + w;
                
                localSum = mul(transpose(bp->T[j-1]), sum);
                if (!bp->NeedPhysics)
				{
					sum = g * mass * props.vMassSegment[j-1] + w;
					localSum = mul(transpose(bp->A[j-1]), sum);
					float h = props.vHardnessSegment[j-1] * hardness;

					float3 G = cross(halfAxis, localSum);
					float FL = (bp->segmentHeight * 0.5f * length(localSum));
					float lG = length(G);
					float sinBetha = lG / (FL);
					float betha = asinf(sinBetha);
					
					float phi = FL*sin(betha) / h;
                    phi = FL * sin(betha + phi) / h;
					phi = FL * sin(betha + phi) / h;
					phi = FL * sin(betha + phi) / h;
					phi = FL * sin(betha + phi) / h;
					phi = FL * sin(betha + phi) / h;

					bp->A[j] = mul(bp->A[j-1], MakeRotationMatrix(phi * G / lG));
                    bp->T[j] = bp->A[j];                    
				}
				
				//if (animationPass)
				//{
				//	bp->T[j] = bp->A[j];
				//}
				else //if (bp->lerpCoef < 1.0f)
				{
					float oneThirdMMulLSqr = /*2.0f * 1.0f/3.0f*/0.66f * props.vMassSegment[j-1] * mass*bp->segmentHeight*bp->segmentHeight;
                    float3 invJ = oneOver(float3(oneThirdMMulLSqr, oneThirdMMulLSqr, oneThirdMMulLSqr));                    

					/*****************************
					Physics
					*****************************/
					if (!planeCollision && !bp->brokenFlag)
						sum = g * mass * props.vMassSegment[j-1] + w;
					else
					{
						//bp->w[j] = float3(0.0f, 0.0f, 0.0f);
						sum = float3(0.0f, 0.0f, 0.0f);
					}
                    if (bp->NeedPhysics)
                    {
                        localSum = mul(transpose(bp->T[j]), sum);
                        bp->w[j] += dTime * mul(cross(halfAxis, localSum) - bp->g[j], invJ);
                        bp->w[j] *= d;
                        psi = dTime * bp->w[j];
                        RotateSegment(bp, j, psi, props);
                    }

					//if (1) //!sphereCollision)
					//{
					//	localSum = mul(transpose(bp->T[j]), sum);

					//	//compute speed
					//	if (transmitSpringMomentDownwards)
					//	{
					//		if (j == NUM_SEGMENTS-1)
					//			bp->w[j] += dTime * mul(cross(halfAxis, localSum) - bp->g[j], invJ);
					//		else
					//		{
					//			bp->w[j] += dTime * mul(cross(halfAxis, localSum) - bp->g[j] + mul(bp->R[j+1], bp->g[j+1]), invJ);
					//		}
					//	}
					//	else
					//	{
					//		bp->w[j] += dTime * mul(cross(halfAxis, localSum) - bp->g[j], invJ);
					//	}
					//	bp->w[j] *= d;

					//	//compute angle
					//	psi = dTime * bp->w[j];
					//	RotateSegment(bp, j, psi, props);
					//}
					
					/*********************
					Sphere collision
					*********************/
					if (sphereCollision)// && collision)
					{
						float3 va = spherePosition - bp->position[j-1];
						float a = length(va);

						if (a > r)
						{
							float3 vb = spherePosition - bp->position[j];
							float3 vl = bp->position[j] - bp->position[j-1];
							
							float3 ax = normalize(cross(va, vl));
							
							float b = length(vb);
							float l = bp->segmentHeight;

							//tochka na okr-ti
							float cosAlpha = (a*a + l*l - b*b) / (2.0f * a * l);
							float cosBetha = (a*a + l*l - r*r) / (2.0f * a * l);
							float sinAlpha = sqrtf(1.0f - cosAlpha*cosAlpha);
							float sinBetha = sqrtf(1.0f - cosBetha*cosBetha);

							float sinPsi = sinBetha*cosAlpha - sinAlpha*cosBetha;
			
							//clamp
							float angle = asinf(sinPsi);
							//if (angle > 0.05)
							//	angle = 0.05;

							float3 psi = angle * ax;
							psi = mul(transpose(bp->T[j]), psi);
							bp->w[j] = 0.5f * psi / dTime;

							//bp->w[j] = float3(0.0f, 0.0f, 0.0f);
							bp->g[j] = float3(0.0f, 0.0f, 0.0f);
							RotateSegment(bp, j, psi, props);
						}
                        bp->NeedPhysics = 1;
					}

					/*********************
					Plane collision
					*********************/
					if (planeCollision)
					{
						/*if (j == 1 || bp->position[j-1].y >= planeY)
						{
							float3 dir = bp->position[j] - bp->position[j-1];
							float xzLength = dir.x*dir.x + dir.z*dir.z;

							if (xzLength > EPSILON)
							{
								float k = (bp->segmentHeight*bp->segmentHeight - (bp->position[j-1].y-planeY)*(bp->position[j-1].y-planeY)) 
									/ xzLength;
								k = sqrtf(k);
								
								float3 resDir = float3(dir.x * k, planeY - bp->position[j-1].y, dir.z * k);
								float3 psi = cross(normalize(dir), normalize(resDir));
								psi = mul(transpose(bp->T[j]), psi);

								bp->w[j] = float3(0.0f, 0.0f, 0.0f);
								RotateSegment(bp, j, psi, props);
							}
						}*/
						bp->w[j-1] = float3(0.0f, 0.0f, 0.0f);
                        //bp->brokenFlag = 1;
                        bp->NeedPhysics = 1;
					}
				}
			}
		}
	}

    UpdateBuffer();

	/*if (animationPass)
		animationPass = false;*/
}
