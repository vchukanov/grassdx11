#include "Wind.h"
#include "PhysMath.h"

#define NUM_SEGMENTS 4

#define TEX_W 32
#define TEX_H 32

static float SegLen = 0.75f;
static float SegHard[3] = {0.06f, 0.06f, 0.06f};
static float SegMass = 0.01f;
static float ClearColor[4] = {0.0f, 0.0f, 1.0f, 0.0f};
static float3 fmW[TEX_W*TEX_H][NUM_SEGMENTS - 1];
static float3 fmA[TEX_W*TEX_H][NUM_SEGMENTS - 1];
static D3DXMATRIX mT[TEX_W*TEX_H][NUM_SEGMENTS - 1];
static D3DXMATRIX mR[TEX_W*TEX_H][NUM_SEGMENTS - 1];
//static float fmW1[128*128];
//static float fmA1[128*128];
static float fTexOffsets[2] = {0.0f, 0.0f};
static float fTexOffsetKoefs[2] = {0.02f, 0.03f};//{0.03f, 0.015f};
//static float fTexKoefs[3] = {0.5714f, 0.1857f, 0.0428f};
static int Start = 1;

WindData::WindData( )
{
    pData = NULL;
    pWindMapData = NULL;

    for (int i = 0; i < TEX_H; i++)
        for (int j = 0; j < TEX_W; j++)
            for (int k = 0; k < NUM_SEGMENTS - 1; k++)
            {
                D3DXMatrixIdentity(&mT[i * TEX_W + j][k]);
                D3DXMatrixIdentity(&mR[i * TEX_W + j][k]);
            }
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
    float fX  = (a_vTexCoord.x * a_fWindTexTile);
    float fY  = (a_vTexCoord.y * a_fWindTexTile);
     /* bilinear interpolation... */
    fX = (fX - floorf(fX)) * (float)fWidth - 0.5f;
    fY = (fY - floorf(fY)) * (float)fWidth - 0.5f;
	float fFracX = fX - floorf(fX);
	float fFracY = fY - floorf(fY);
    UINT uLX = UINT(fX);
    UINT uHX = uLX + 1;
    UINT uLY = UINT(fY);
    UINT uHY = uLY + 1;
    if (uHX > uWidth - 1)
        uHX = uWidth - 1;
    if (uHY > uHeight - 1)
        uHY = uHeight - 1;
    D3DXVECTOR3 fLL = vWindData[uWidth * uLY + uLX];
    D3DXVECTOR3 fHL = vWindData[uWidth * uHY + uLX];
    D3DXVECTOR3 fLR = vWindData[uWidth * uLY + uHX];
    D3DXVECTOR3 fHR = vWindData[uWidth * uHY + uHX];
	float3 vTmp = ( (1.0f - fFracX) * fLL + fFracX * fLR ) * (1.0f - fFracY) + 
        fFracY * ( (1.0f - fFracX) * fHL + fFracX * fHR );
    return vTmp;
}
D3DXVECTOR3 WindData::GetValueA( const D3DXVECTOR2 &a_vTexCoord, const float a_fWindTexTile,
                                 int a_iSegmentIndex) const
{
    float fX  = (a_vTexCoord.x * a_fWindTexTile);
    float fY  = (a_vTexCoord.y * a_fWindTexTile);
     /* bilinear interpolation... */
    fX = (fX - floorf(fX)) * (float)fWidth - 0.5f;
    fY = (fY - floorf(fY)) * (float)fWidth - 0.5f;
	float fFracX = fX - floorf(fX);
	float fFracY = fY - floorf(fY);
    UINT uLX = UINT(fX);
    UINT uHX = uLX + 1;
    UINT uLY = UINT(fY);
    UINT uHY = uLY + 1;
    if (uHX > uWidth - 1)
        uHX = uWidth - 1;
    if (uHY > uHeight - 1)
        uHY = uHeight - 1;
    D3DXVECTOR3 fLL = fmA[uWidth * uLY + uLX][a_iSegmentIndex];
    D3DXVECTOR3 fHL = fmA[uWidth * uHY + uLX][a_iSegmentIndex];
    D3DXVECTOR3 fLR = fmA[uWidth * uLY + uHX][a_iSegmentIndex];
    D3DXVECTOR3 fHR = fmA[uWidth * uHY + uHX][a_iSegmentIndex];

	float3 vTmp = ( (1.0f - fFracX) * fLL + fFracX * fLR ) * (1.0f - fFracY) + 
        fFracY * ( (1.0f - fFracX) * fHL + fFracX * fHR );
    /*
	vTmp.x = SegHard[a_iSegmentIndex]*vTmp.x/(0.5f*SegLen) - 9.8f*SegMass*sinf(vTmp.x)*cosf(vTmp.x);
	vTmp.y = 0.0;//SegHard*vTmp.y/(0.5f*SegLen) - 9.8f*SegMass*sinf(vTmp.y);
	vTmp.z = SegHard[a_iSegmentIndex]*vTmp.z/(0.5f*SegLen) - 9.8f*SegMass*sinf(vTmp.z)*cosf(vTmp.z);
    */
	return vTmp;
}



float WindData::BiLinear( const D3DXVECTOR2 &a_vTexCoord )
{
	D3DXVECTOR2 vTexCoord = a_vTexCoord;

	if (vTexCoord.x < 0.0)
		vTexCoord.x -= floorf(vTexCoord.x);
	if (vTexCoord.y < 0.0)
		vTexCoord.y -= floorf(vTexCoord.y);
    /*
	while (vTexCoord.x < 0.0)
		vTexCoord.x += 1.0;
	while (vTexCoord.y < 0.0)
		vTexCoord.y += 1.0;
    */
	float fX  = (vTexCoord.x * (fWidth - 1.0f));
    float fY  = (vTexCoord.y * (fHeight - 1.0f));
	
//	if (fX<20.0)return sinf(3.14*fX/40.0); 
//	else return 0.0;
    
	// bilinear interpolation... 
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
    float fLL = pWindMapData[uWidth * uLY + uLX].w;
    float fHL = pWindMapData[uWidth * uHY + uLX].w;
    float fLR = pWindMapData[uWidth * uLY + uHX].w;
    float fHR = pWindMapData[uWidth * uHY + uHX].w;
    //return fLL;
    return ( (1.0f - fFracX) * fLL + fFracX * fLR ) * (1.0f - fFracY) + 
        fFracY * ( (1.0f - fFracX) * fHL + fFracX * fHR );
}


inline D3DXVECTOR2 Transform(D3DXVECTOR2 vec, D3DXVECTOR4 rot, float off)
{
    D3DXVECTOR2 *rotxy = (D3DXVECTOR2*)(float*)&rot;
    D3DXVECTOR2 *rotzw = (D3DXVECTOR2*)((float*)&rot + 2);
    return D3DXVECTOR2(D3DXVec2Dot(&vec, rotxy), D3DXVec2Dot(&vec, rotzw));
}

/*
inline D3DXVECTOR2 Transform(D3DXVECTOR2 vec, D3DXVECTOR4 rot, float off)
{
    D3DXVECTOR2 *rotxy = (D3DXVECTOR2*)(float*)&rot;
    D3DXVECTOR2 *rotzw = (D3DXVECTOR2*)((float*)&rot + 2);
    return D3DXVECTOR2(D3DXVec2Dot(&vec, rotxy), D3DXVec2Dot(&vec, rotzw)) + *rotxy * off;
}

*/
D3DXVECTOR3 WindData::GetWindValue( const D3DXVECTOR2 &a_vTexCoord, const float a_fWindTexTile, const float a_fWindStrength ) const
{
    return GetValue(a_vTexCoord, a_fWindTexTile) * a_fWindStrength;
}

D3DXVECTOR3 WindData::GetWindValueA( const D3DXVECTOR2 &a_vTexCoord, const float a_fWindTexTile, const float a_fWindStrength,
                                     int a_iSegmentIndex ) const
{
    return GetValueA(a_vTexCoord, a_fWindTexTile, a_iSegmentIndex) * a_fWindStrength;
}

void WindData::ConvertFrom( const D3D10_MAPPED_TEXTURE2D &a_MappedTex, const D3D10_TEXTURE2D_DESC &a_TexDesc )
{
    float* pTexels = (float*)a_MappedTex.pData;
	if (pData == NULL)
    {
        pData = new D3DXVECTOR3[a_TexDesc.Height * a_TexDesc.Width];
        pWindMapData = new D3DXVECTOR4[a_TexDesc.Height * a_TexDesc.Width];
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
            pWindMapData[row * a_TexDesc.Width + col].x = 0.0f;//pTexels[rowStart + colStart + 0];// / 255.0f; 
            pWindMapData[row * a_TexDesc.Width + col].y = 0.0f;//pTexels[rowStart + colStart + 1];// / 255.0f;
            pWindMapData[row * a_TexDesc.Width + col].z = 0.0f;//pTexels[rowStart + colStart + 2];// / 255.0f;
		//	if (col < 80) fF = sinf(float(col)*3.14f/128.0f);//1.0f;
		//	else fF = 0.0f;
		//	pWindMapData[row * a_TexDesc.Width + col].w = fF;
			pWindMapData[row * a_TexDesc.Width + col].w = pTexels[rowStart + colStart + 3];// / 255.0f;
            /*pData[row * a_TexDesc.Width + col].x = pTexels[rowStart + colStart + 0];// / 255.0f; 
            pData[row * a_TexDesc.Width + col].y = pTexels[rowStart + colStart + 1];// / 255.0f;
            pData[row * a_TexDesc.Width + col].z = pTexels[rowStart + colStart + 2];// / 255.0f;*/
        }
    }
}


//void WindCopy(D3D10_MAPPED_TEXTURE2D &a_MappedTex)
//{
//	float *pTexels = (float*)a_MappedTex.pData;
//	float3 fa;
//	for( UINT row = 0; row < 64; row++ )
//    {
//        UINT rowStart = row * a_MappedTex.RowPitch / sizeof(float);
//        for( UINT col = 0; col < 64; col++ )
//        {
//            UINT colStart = col * 4;//RGBA
//			fa = fmA[row * 64 + col];
//			/*pTexels[rowStart + colStart + 0] = SegHard*fa.x/(0.5f*SegLen);
//            pTexels[rowStart + colStart + 1] = 0.0;
//			pTexels[rowStart + colStart + 2] = 0.0;//SegHard*fa.z/(0.5f*SegLen);;
//*/
//			pTexels[rowStart + colStart + 0] = SegHard*fa.x/(0.5f*SegLen) - 9.8f*SegMass*sinf(fa.x)*cosf(fa.x);
//	        pTexels[rowStart + colStart + 1] = 0.0;//SegHard*fa.y/(0.5f*SegLen) - 9.8f*SegMass*sinf(fa.y);
//			pTexels[rowStart + colStart + 2] = SegHard*fa.z/(0.5f*SegLen) - 9.8f*SegMass*sinf(fa.z)*cosf(fa.x);
//			pTexels[rowStart + colStart + 3] = 1.0f;
//		}
//	}
//}

void WindData::WindCopy( ID3D10Texture2D *a_pDestTex, ID3D10Device *a_pDevice )
{
    float row_pitch = 4 * sizeof(float) * TEX_W;
    static float pTexels[TEX_W * TEX_H * 4];
    float fMass = 0.23f;
    float fLseg = 0.6f;
    float3 fa;
    D3D10_BOX dest_region;
    //FILE *F0 = fopen("segment_0.txt", "at");
    //FILE *F1 = fopen("segment_1.txt", "at");
    //static int tick = 0;
    //tick++;

    dest_region.left = 0;
    dest_region.right = TEX_W;
    dest_region.top = 0;
    dest_region.bottom = TEX_H;
    dest_region.front = 0;
    dest_region.back = 1;

    for (int segment = 0; segment < NUM_SEGMENTS - 1; segment++)
    {
        for( UINT row = 0; row < TEX_H; row++ )
        {
            UINT rowStart = (UINT)(row * row_pitch / sizeof(float));

            for( UINT col = 0; col < TEX_W; col++ )
            {
                UINT colStart = col * 4;//RGBA

                float3 vec1 = MakeRotationVector(mR[row * TEX_W + col][segment]),
                    vec2 = MakeRotationVector(mT[row * TEX_W + col][segment]);
                float3 halfAxis = float3(0.0f, SegLen*0.5f, 0.0f);

                //float3 mg = SegMass * float3(0, -9.8f, 0) * (mT[row * TEX_W + col][segment][5] - segment * 0.1);
                float3 mg = SegMass * float3(0, -9.8f, 0);
//                float3 mg = SegMass * float3(0, -9.8f, 0) * mT[row * TEX_W + col][segment][5];
                if (segment >= 1)
                    D3DXVec3TransformCoord(&mg, &mg, &mT[row * TEX_W + col][segment - 1]);
                D3DXVec3Cross(&mg, &halfAxis, &mg);
                float3 w = SegHard[segment] * vec1 - mg;

                if (segment >= 1)
                {
                    float3x3 transposed_t;
                    D3DXMatrixTranspose(&transposed_t, &mT[row * TEX_W + col][segment - 1]);
                    D3DXVec3TransformCoord(&w, &w, &transposed_t);
                }

                /*
                //float3 fa = MakeRotationVector(mT[row * TEX_W + col][0]);
                float3 halfAxis = float3(0.0f, SegLen*0.5f, 0.0f);

                float3 vec1 = MakeRotationVector(mR[row * TEX_W + col][segment]),
                    vec2 = MakeRotationVector(mT[row * TEX_W + col][segment]);
                float w;
                float3 up(0, 1, 0), dir1, dir2;
                if (segment >= 1)
                    D3DXVec3TransformCoord(&up, &up, &mT[row * TEX_W + col][segment - 1]);
                D3DXVec3Cross(&dir1, &up, &vec1);
                D3DXVec3Cross(&dir2, &up, &vec2);
                float phi1 = D3DXVec3Length(vec1) * SIGN(), phi2;

                //vec1.z = clamp(vec1.z, -3.14 / 2 - 0.15, 3.14 / 2 - 0.15);
                //vec2.z = clamp(vec2.z, -3.14 / 2 - 0.15, 3.14 / 2 - 0.15);

                //w.x = (SegHard[segment] * vec1.x - halfAxis.y * SegMass * 9.8f * sinf(vec2.x)) / (halfAxis.y * cosf(vec2.x));
                //w.y = 0.0f;
                w = (SegHard[segment] * vec1.z - halfAxis.y * SegMass * 9.8f * sinf(vec2.z)) / (halfAxis.y * cosf(vec2.z));

                //if (row == 0 && col == 0 && segment == 0)
                //    fprintf(F0, "%i; %lf; %lf; %lf; %lf\n", tick, vec1.z, vec2.z, w.z, delta);
                //if (row == 0 && col == 0 && segment == 1)
                //    fprintf(F1, "%i; %lf; %lf; %lf; %lf\n", tick, vec1.z, vec2.z, w.z, delta);
                */
                pTexels[rowStart + colStart + 0] = w.x;
                pTexels[rowStart + colStart + 1] = w.y;
                pTexels[rowStart + colStart + 2] = w.z;
                pTexels[rowStart + colStart + 3] = 1.0f;

                fmA[row * TEX_W + col][segment] = float3(w.x, w.y, w.z);
            }
        }

        a_pDevice->UpdateSubresource(a_pDestTex, 
            D3D10CalcSubresource(0, segment, 1), &dest_region, (const void *)pTexels, (UINT)row_pitch, 0);
    }

    //fclose(F0);
    //fclose(F1);
}

float3 GetWaveW(D3DXVECTOR3 vCamDir)
{
	float fT;
	float3 vWaveW = float3(1.0f, 1.0f, 0.0f);
	vCamDir.y = 0.0f;
	D3DXVec3Normalize( &vCamDir, &vCamDir );
	if ((vCamDir.x < 0.0f) && vCamDir.z < 0.7071f)
	{
		if (vCamDir.z > 0.3827)
		{
			fT = (0.7071f - vCamDir.z) / (0.7071f - 0.3827f);
			vWaveW.x = 1.0f - fT;
		}
		else
		{
			if (vCamDir.z > 0.0f)
			{
				fT = (0.3827f - vCamDir.z) / 0.3827f;
				vWaveW.x = 0.0f;
				vWaveW.z = fT;
			}
			else
			{
				if (vCamDir.z > -0.3827f)
				{
					vWaveW.x = 0.0f;
					fT = (0.3827f + vCamDir.z) / 0.3827f;
					vWaveW.y = fT;
					vWaveW.z = 1.0f;
				}
				else
				{
					if (vCamDir.z > -0.7071f)
					{
						fT = (0.7071f + vCamDir.z) / (0.7071f - 0.3827f);
						vWaveW.x = 1.0f - fT;
						vWaveW.y = 0.0f;
						vWaveW.z = 1.0f;
					}
					else
					{
						if (vCamDir.z > -0.9239f)
						{
							fT = (0.9239f + vCamDir.z) / (0.9239f - 0.7071f);
							vWaveW.x = 1.0f;
							vWaveW.y = 0.0f;
							vWaveW.z = fT;
						}
						else 
						{
							fT = (1.0f + vCamDir.z) / (1.0f - 0.9239f);
							vWaveW.y = 1.0f - fT;
						}
					}
				}
			}
		}
	}
	return vWaveW;
}

static void CalcTR(float3x3 &Tres, float3x3 &Rres, float3x3 &T, float3x3 &T_1, float3x3 &R, float3 &psi)
{
    D3DXMatrixMultiply(&Rres, &R, &MakeRotationMatrix(psi));
    D3DXMatrixMultiply(&Tres, &T_1, &Rres);
    /*
    D3DXMatrixMultiply(&Tres, &T, &MakeRotationMatrix(psi));
    float3x3 transposed_t;
    D3DXMatrixTranspose(&transposed_t, &T_1);
    D3DXMatrixMultiply(&Rres, &transposed_t, &Tres);
    */
}

static float3 GetDw(float3x3 &T, float3x3 &R, float3 &fw, float3 &sum, float3 &w, float invJ, float segmentHeight, float Hardness, int j )
{
    float3 localSum, g, m_f;
    float3 halfAxis = float3(0.0f, segmentHeight*0.5f, 0.0f);

    D3DXVec3TransformCoord(&localSum, &sum, &T);
    D3DXVec3Cross(&m_f, &halfAxis, &localSum);
    g = Hardness * MakeRotationVector(R);

    return (m_f - g) * invJ;
}


static int uCount =0, State = 0;
static float maxW = 0.0f;
static float TimeCont[4] = {4.0f, 1.0f, 5.0f, 1.0f}, CurTime = 0.f;
static float Bound = TimeCont[0];
void WindData::UpdateWindTex( D3D10_MAPPED_TEXTURE2D &a_MappedTex, float a_fElapsed, D3DXVECTOR3 a_vCamDir)
{
//    const GrassPropsUnified &props = grassProps[0];
		if (a_fElapsed >= 0.1f)
		a_fElapsed = 0.1f;

	uCount++;
	if (uCount>500)
		uCount=0;

	CurTime += a_fElapsed;
	if (CurTime > Bound)
	{
		State++;
		if (State ==4)
		{
			State = 0;
			Bound = 0.f;
			CurTime = 0.f;
		}
		Bound += TimeCont[State];
	}
	float fTFull;
  switch (State) {
	  case 0  : fTFull = 0.1f; break;
	  case 1  : fTFull = (CurTime - TimeCont[0])/TimeCont[1]; break;
	  case 2  : fTFull = 1.f; break;
	  case 3  : fTFull = 1.f - (CurTime - (TimeCont[0] + TimeCont[1] + TimeCont[2]))/TimeCont[3]; break;
	  default :
		  fTFull = 0.f; break;
    }


    float3 faRes[3];
	float2 vTexCoord;
    float *pTexels = (float*)a_MappedTex.pData;
    D3DXVECTOR2 vUV;
    D3DXVECTOR4 vWind;
	float2 fPixHeight;
    float3 fw, fa, fw1, fa1;
	float d = powf(0.98f, a_fElapsed * 0.01f);
	if (d > 0.9998f) d = 0.9998f;
	float fDt = 0.05f;
	float fMass = 0.245f * 0.8f;//0.23f;
	float fLseg = 0.75f;//0.6f;
	float fJ_1 = 0.75 *1.0f / (0.33f*SegMass*SegLen*SegLen);
//	float fJ_1 = 0.35f * 1.0f / (0.33f*SegMass*SegLen*SegLen);
	float fJ_11 = 1.0f / (0.43f*SegMass*SegLen*SegLen);
	//	float fJ_1 = 1.0 / (fMass*fLseg*fLseg);
	//float fF, fF1;
	float fFscale=0.6f;
    float4 vRotate45  = float4(0.7071f, 0.7071f, -0.7071f, 0.7071f);
    float4 vRotate315 = float4(0.7071f, -0.7071f, 0.7071f, 0.7071f);

    float3 vWaveW = GetWaveW(a_vCamDir);
	//if (a_fElapsed > 0.1f) a_fElapsed = 0.1f;
    //a_fElapsed = 0.005f;
	//a_fElapsed = 0.1f;
	for (int i = 0; i < 2; i++)
    {
        fTexOffsets[i] -= a_fElapsed * fTexOffsetKoefs[i] * *pWindSpeed;
    }
	
	float fT = 0.5f + 0.5f*sinf(4.0f*fTexOffsets[0]);
	float fT1 = 0.5f + 0.5f*sinf(7.5f*fTexOffsets[0]);
//	float fForse = 1.6 + 0.3*sinf(2.0*fTexOffsets[0]);
	float fForse = 1.0f;
	float fTile = 1.0f;
	float height[3];
	float fTmp1[2];

	for( UINT row = 0; row < uHeight; row++ )
//	for( UINT row = 0; row < 1; row++ )
    {
        UINT rowStart = row * a_MappedTex.RowPitch / sizeof(float);
        vTexCoord.y = float(row) / (fHeight - 1.0f);

        for( UINT col = 0; col < uWidth; col++ )
//        for( UINT col = 0; col < 1; col++ )
        {
            UINT colStart = col * 4;//RGBA
            vTexCoord.x = float(col) / (fWidth - 1.0f);
			float2 vUV;
			
			height[0]=height[1]=height[2]=0.0f;
			
			if (vWaveW.x > 0.001f)
			{
				vUV.x = vTexCoord.x;
				vUV.y = vTexCoord.y;
				vUV.x += 1.5f*fTexOffsets[0];//*fTile;
				height[0] = BiLinear(vUV);
			}            
			if (vWaveW.y > 0.001)
			{
	            vUV = 0.7071f * 2.0f * Transform(vTexCoord, vRotate45, fTexOffsets[1]);
				vUV.x += fTexOffsets[1];
			    height[1] = BiLinear(vUV);
			}
			if (vWaveW.z > 0.001f)
			{
				vUV.x = vTexCoord.y;
				vUV.y = -vTexCoord.x;
				vUV.x += 1.4f*fTexOffsets[0];//*fTile;
				height[2] = BiLinear(vUV);
			}            
/*			
			//if (vWaveW.x > 0.001f)
			{
				vUV.x = 0.5f*vTexCoord.x;
				vUV.y = vTexCoord.y;
				vUV.x += 1.0f*fTexOffsets[0];//*fTile;
				fTmp1[0] = BiLinear(fTile*vUV);
				vUV.x = 0.5f*vTexCoord.x;
				vUV.x += 0.7f*fTexOffsets[0];//*fTile;
				fTmp1[1] = BiLinear(2.0f*fTile*vUV);
				height[0] = (1.0f-fT)*fTmp1[0]+ fT*fTmp1[1]; //*****************
//				height[0] = fTmp1[1];
			}            
			if (vWaveW.y > 0.001)
			{
	            vUV = 0.7071f * 2.0f * fTile * Transform(vTexCoord, vRotate45, fTexOffsets[1]);
				vUV.x += fTexOffsets[1]/fTile;
			    fTmp1[0] = BiLinear(vUV);
				vUV = 0.7071f * 4.0f * fTile * Transform(vTexCoord, vRotate45, fTexOffsets[1]);
				vUV.x += fTexOffsets[1]/(1.5f*fTile);
				fTmp1[1] = BiLinear(vUV);
				height[1] = (1.0f-fT)*fTmp1[0]+ fT*fTmp1[1];

			}
			if (vWaveW.z > 0.001f)
			{
				vUV.x = vTexCoord.y;
				vUV.y = -vTexCoord.x;
				vUV.x += fTexOffsets[0];//*fTile;
				fTmp1[0] = BiLinear(fTile*vUV);
				fTmp1[1] = BiLinear(2.0f*fTile*vUV);
				height[2] = (1.0f-fT)*fTmp1[0]+ fT*fTmp1[1];
			}            
*/			
            
			fPixHeight = (0.9f*fT + 0.1f) * (float2(1.0f, 0.0f)*(1.5f-fT1)*height[0] + float2(1.0f, 1.0f)*(0.5f + fT1)*height[1] 
												+ float2(0.0f, 1.0f)*(1.5f-fT1)*height[2]); 

			float fDamp;
			vUV.x = vTexCoord.x;
			vUV.y = vTexCoord.y;
			vUV.x += 1.f*fTexOffsets[0];
			fDamp = BiLinear(vUV);
			fDamp = fDamp * (1.f - fTFull) + fTFull; 

			fPixHeight *= 6.f*fDamp;
			float fPixHmax = 6.f;
			float fLen = sqrt(fPixHeight.x * fPixHeight.x + fPixHeight.y * fPixHeight.y + 0.0001f); 
			if ( fLen > fPixHmax ) fPixHeight = fPixHmax * fPixHeight / fLen;

		//	float fTmpS = 0.0;
		//	if (uCount<150)	fTmpS = 2.6f;
		//	fPixHeight = float2(1.0, 0.0)*fTmpS;

            float3 windBase(fPixHeight.x, 0.0f, fPixHeight.y), wind, vZ(0.0f, 0.0f, 0.0f), v, r;
			float3 halfAxis = float3(0.0f, SegLen *0.5f, 0.0f);
			for (int segment = 0; segment < NUM_SEGMENTS - 1; segment++)
            {                
                UINT ind = row * uWidth + col;
                float3 psi;
                float3 g(0.0f, -9.8f, 0.0f);

				float3x3 transposed_t;
                D3DXMatrixTranspose(&transposed_t, &mT[ind][segment]);
			    D3DXVec3Cross(&r, &fmW[ind][segment], &halfAxis);
                D3DXVec3TransformCoord(&r, &r, &transposed_t);
				v = vZ + r;
				wind = 0.02f * (windBase - v);
                // Calculate physics
                float h = SegHard[segment];
//                float3 sum = g * SegMass * mT[ind][segment][5] + wind;
                float3 sum = g * SegMass + wind;

                float3 Dw = GetDw(mT[ind][segment], mR[ind][segment], fmW[ind][segment], sum, wind, fJ_1, SegLen, h, segment);
                float3 w_ = fmW[ind][segment] + a_fElapsed * Dw;
                psi = a_fElapsed * w_;

                float3x3 Tres, Rres, T_1;
                if (segment >= 1)
                    T_1 = mT[ind][segment - 1];
                else
                    D3DXMatrixIdentity(&T_1);

                CalcTR(Tres, Rres, mT[ind][segment], T_1, mR[ind][segment], psi);

                D3DXMatrixTranspose(&transposed_t, &Tres);
			    D3DXVec3Cross(&r, &w_, &halfAxis);
                D3DXVec3TransformCoord(&r, &r, &transposed_t);
				v = vZ + r;
				wind = 0.02f * (windBase - v);

//                sum = g * SegMass * Tres[5] + wind;
				sum = g * SegMass + wind;
                float3 Dw_ = GetDw(Tres, Rres, w_, sum, wind, fJ_1, SegLen, h, segment);
                fmW[ind][segment] += 0.5f * a_fElapsed * (Dw + Dw_); 
                fmW[ind][segment] *= d;
                psi = a_fElapsed * fmW[ind][segment];
                
                // Rotate segment
                D3DXMatrixMultiply(&mR[ind][segment], &mR[ind][segment], &MakeRotationMatrix(psi));
                D3DXMatrixMultiply(&mT[ind][segment], &T_1, &mR[ind][segment]);

				D3DXMatrixTranspose(&transposed_t, &mT[ind][segment]);
			    D3DXVec3Cross(&r, &fmW[ind][segment], &halfAxis);
                D3DXVec3TransformCoord(&r, &r, &transposed_t);
				vZ = vZ + 2.f * r;

			}
			/*
			float3 wind(fPixHeight.x, 0.0f, fPixHeight.y);
            for (int segment = 0; segment < NUM_SEGMENTS - 1; segment++)
            {                
                UINT ind = row * uWidth + col;
                float3 psi;
                float3 g(0.0f, -9.8f, 0.0f);

                // Calculate physics
                float h = SegHard[segment];
                float3 sum = g * SegMass * mT[ind][segment][5] + wind;
                //float3 sum = g * SegMass + wind;

                float3 Dw = GetDw(mT[ind][segment], mR[ind][segment], fmW[ind][segment], sum, wind, fJ_1, SegLen, h, segment);
                float3 w_ = fmW[ind][segment] + a_fElapsed * Dw;
                psi = a_fElapsed * w_;

                float3x3 Tres, Rres, T_1;
                if (segment >= 1)
                    T_1 = mT[ind][segment - 1];
                else
                    D3DXMatrixIdentity(&T_1);

                CalcTR(Tres, Rres, mT[ind][segment], T_1, mR[ind][segment], psi);
                sum = g * SegMass * Tres[5] + wind;
                //sum = g * SegMass + wind;
                float3 Dw_ = GetDw(Tres, Rres, w_, sum, wind, fJ_1, SegLen, h, segment);
                fmW[ind][segment] += 0.5f * a_fElapsed * (Dw + Dw_); 
                fmW[ind][segment] *= d;
                psi = a_fElapsed * fmW[ind][segment];
                
                // Rotate segment
                D3DXMatrixMultiply(&mR[ind][segment], &mR[ind][segment], &MakeRotationMatrix(psi));
                D3DXMatrixMultiply(&mT[ind][segment], &T_1, &mR[ind][segment]);
            }
	*/


			vWindData[row * uWidth + col].x = fPixHeight.x;//fa.x;//fF;
			vWindData[row * uWidth + col].y = 0.0f;//fF;
			vWindData[row * uWidth + col].z = fPixHeight.y;//fa.z;
  
		}
    }

}

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
    //D3D10_TEXTURE2D_DESC WindMapDesc;
    //ZeroMemory( &WindMapDesc, sizeof(WindMapDesc) );
    //WindMapDesc.Width = m_uViewPortWidth;
    //WindMapDesc.Height = m_uViewPortHeight;
    //WindMapDesc.MipLevels = 1;
    //WindMapDesc.ArraySize = 1;
    //WindMapDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    //WindMapDesc.SampleDesc.Count = 1;
    //WindMapDesc.Usage = D3D10_USAGE_DEFAULT;
    //WindMapDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
    //m_pD3DDevice->CreateTexture2D(&WindMapDesc, NULL, &m_pWindMap);

    ///* Creating Render Target View for Wind Map */
    //D3D10_RENDER_TARGET_VIEW_DESC WindMapRTVDesc;
    //ZeroMemory( &WindMapRTVDesc, sizeof(WindMapRTVDesc) );
    //WindMapRTVDesc.Format = WindMapDesc.Format;
    //WindMapRTVDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
    //WindMapRTVDesc.Texture2D.MipSlice = 0;
    //m_pD3DDevice->CreateRenderTargetView(m_pWindMap, &WindMapRTVDesc, &m_pWindMapRTV);

    ///* Creating Shader Resource View for Wind Map */
    //D3D10_SHADER_RESOURCE_VIEW_DESC WindMapSRVDesc;
    //ZeroMemory( &WindMapSRVDesc, sizeof(WindMapSRVDesc) );
    //WindMapSRVDesc.Format = WindMapDesc.Format;
    //WindMapSRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
    //WindMapSRVDesc.Texture2D.MostDetailedMip = 0;
    //WindMapSRVDesc.Texture2D.MipLevels = 1;
    //m_pD3DDevice->CreateShaderResourceView(m_pWindMap, &WindMapSRVDesc, &m_pWindMapSRV);
#pragma endregion

    //m_pWindMapESRV = m_pWindEffect->GetVariableByName("g_txWindMap")->AsShaderResource();
     
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
    m_WindTexStagingDesc.ArraySize = NUM_SEGMENTS - 1;
    m_WindTexStagingDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    m_WindTexStagingDesc.SampleDesc.Count = 1;
    m_WindTexStagingDesc.Usage = D3D10_USAGE_DEFAULT;//D3D10_USAGE_DYNAMIC;
    m_WindTexStagingDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;//D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
    //m_WindTexStagingDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
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
    WindTexSRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2DARRAY;
    WindTexSRVDesc.Texture2DArray.MostDetailedMip = 0;
    WindTexSRVDesc.Texture2DArray.ArraySize = NUM_SEGMENTS - 1;
    WindTexSRVDesc.Texture2DArray.FirstArraySlice = 0;
    WindTexSRVDesc.Texture2DArray.MipLevels = 1;
    m_pD3DDevice->CreateShaderResourceView(m_pWindTex, &WindTexSRVDesc, &m_pWindTexSRV);

    /* Creating texture for reading on CPU */
    m_WindTexStagingDesc.CPUAccessFlags = D3D10_CPU_ACCESS_READ | D3D10_CPU_ACCESS_WRITE;
    m_WindTexStagingDesc.BindFlags      = 0;
    m_WindTexStagingDesc.Usage          = D3D10_USAGE_STAGING;
    m_WindTexStagingDesc.ArraySize      = 1;
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

    //SAFE_RELEASE(m_pHeightMap);
	MakeHeightMap();
	//MakeWindMap();
	
    UpdateWindData();
    
    m_WindData.pTime      = &m_fTime;
    m_WindData.pWindSpeed = &m_fWindSpeed;
}

Wind::~Wind( )
{
    SAFE_RELEASE(m_pInputLayout);
    SAFE_RELEASE(m_pVertexBuffer);

	SAFE_RELEASE(m_pHeightTexSRV);

	SAFE_RELEASE(m_pHeightMap);
	SAFE_RELEASE(m_pHeightMapRTV);	
    SAFE_RELEASE(m_pHeightMapSRV);

    //SAFE_RELEASE(m_pWindMap);
    //SAFE_RELEASE(m_pWindMapRTV);
    //SAFE_RELEASE(m_pWindMapSRV);

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
    /*m_pWindBias->SetFloat(a_fBias);
    MakeWindMap();
    UpdateWindData();*/
}

void Wind::SetWindScale( float a_fScale )
{
    /*m_pWindScale->SetFloat(a_fScale);
    MakeWindMap();
    UpdateWindData();*/
}

const WindData *Wind::WindDataPtr( )
{
    return &m_WindData;
}

void Wind::UpdateWindData( )
{
    /* Copying data from one texture to another */
    //m_pD3DDevice->CopyResource(m_pWindTexStaging, m_pWindMap);
	m_pD3DDevice->CopyResource(m_pWindTexStaging, m_pHeightMap);
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

void Wind::MakeWindTex( float a_fElapsed, D3DXVECTOR3 a_vCamDir )
{    
    /*
    D3D10_MAPPED_TEXTURE2D MappedTexture;
    m_WindData.UpdateWindTex(MappedTexture, a_fElapsed, a_vCamDir);
    m_pWindTex->Map(D3D10CalcSubresource(0, 0, 1), D3D10_MAP_WRITE_DISCARD, 0, &MappedTexture);
//    m_WindData.UpdateWindTex(MappedTexture, a_fElapsed);
	WindCopy(MappedTexture);
	m_pWindTex->Unmap(D3D10CalcSubresource(0, 0, 1));
    */
    D3D10_MAPPED_TEXTURE2D MappedTexture;

    m_WindData.UpdateWindTex(MappedTexture, a_fElapsed, a_vCamDir);
    m_WindData.WindCopy(m_pWindTex, m_pD3DDevice);
}

void Wind::Update( float a_fElapsed, D3DXVECTOR3 a_vCamDir )
{
    m_fTime += a_fElapsed;
    m_pTimeESV->SetFloat(m_fTime);
    MakeWindTex(a_fElapsed, a_vCamDir);
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
