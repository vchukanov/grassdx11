#include "Wind.h"
#include "PhysMath.h"

#include <DDSTextureLoader.h>

#define NUM_SEGMENTS 4

#define TEX_W 32
#define TEX_H 32

static float SegLen        = 0.75f;
static float SegHard[3]    = {0.06f, 0.06f, 0.06f};
static float SegMass       = 0.01f;
static float ClearColor[4] = {0.0f, 0.0f, 1.0f, 0.0f};

static float3    fmW[TEX_W * TEX_H][NUM_SEGMENTS - 1];
static float3    fmA[TEX_W * TEX_H][NUM_SEGMENTS - 1];
static XMMATRIX  mT[TEX_W * TEX_H][NUM_SEGMENTS - 1];
static XMMATRIX  mR[TEX_W * TEX_H][NUM_SEGMENTS - 1];

static float fTexOffsets[2]     = {0.0f, 0.0f};
static float fTexOffsetKoefs[2] = {0.02f, 0.03f};
static int   Start              = 1;


WindData::WindData (void)
{
    pData = NULL;
    pWindMapData = NULL;
	
	for (int i = 0; i < TEX_H; i++)
        for (int j = 0; j < TEX_W; j++)
            for (int k = 0; k < NUM_SEGMENTS - 1; k++)
            {
				mT[i * TEX_W + j][k] = XMMatrixIdentity();
				mR[i * TEX_W + j][k] = XMMatrixIdentity();
            }
}


WindData::~WindData (void)
{
    if (pData)
        delete [] pData;
    if (pWindMapData)
        delete [] pWindMapData;
}


XMVECTOR WindData::GetValue (const XMVECTOR &a_vTexCoord, const float a_fWindTexTile) const
{
    float fX  = (getx(a_vTexCoord) * a_fWindTexTile);
    float fY  = (gety(a_vTexCoord) * a_fWindTexTile);
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
    XMFLOAT3 fLL = vWindData[uWidth * uLY + uLX];
    XMFLOAT3 fHL = vWindData[uWidth * uHY + uLX];
    XMFLOAT3 fLR = vWindData[uWidth * uLY + uHX];
    XMFLOAT3 fHR = vWindData[uWidth * uHY + uHX];

	XM_TO_V(fLL, v_fLL, 3);
	XM_TO_V(fHL, v_fHL, 3);
	XM_TO_V(fLR, v_fLR, 3);
	XM_TO_V(fHR, v_fHR, 3);

	return ((1.0f - fFracX) * v_fLL + fFracX * v_fLR) * (1.0f - fFracY) +
        fFracY * ((1.0f - fFracX) * v_fHL + fFracX * v_fHR);
}


XMVECTOR WindData::GetValueA (const XMVECTOR &a_vTexCoord, const float a_fWindTexTile, int a_iSegmentIndex) const
{
    float fX  = (getx(a_vTexCoord) * a_fWindTexTile);
    float fY  = (gety(a_vTexCoord) * a_fWindTexTile);

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

    const XMVECTOR &v_fLL = fmA[uWidth * uLY + uLX][a_iSegmentIndex];
    const XMVECTOR &v_fHL = fmA[uWidth * uHY + uLX][a_iSegmentIndex];
    const XMVECTOR &v_fLR = fmA[uWidth * uLY + uHX][a_iSegmentIndex];
    const XMVECTOR &v_fHR = fmA[uWidth * uHY + uHX][a_iSegmentIndex];

	return ((1.0f - fFracX) * v_fLL + fFracX * v_fLR) * (1.0f - fFracY) +
        fFracY * ((1.0f - fFracX) * v_fHL + fFracX * v_fHR);
}


float WindData::BiLinear (const XMVECTOR &a_vTexCoord)
{
	XMVECTOR vTexCoord = a_vTexCoord;

	if (getx(vTexCoord) < 0.0)
		setx(vTexCoord, getx(vTexCoord) - floorf(getx(vTexCoord)));
	if (gety(vTexCoord) < 0.0)
		sety(vTexCoord, gety(vTexCoord) - floorf(gety(vTexCoord)));

	float fX  = (getx(vTexCoord) * (fWidth - 1.0f));
    float fY  = (gety(vTexCoord) * (fHeight - 1.0f));
	
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
  
	return ( (1.0f - fFracX) * fLL + fFracX * fLR ) * (1.0f - fFracY) + 
        fFracY * ( (1.0f - fFracX) * fHL + fFracX * fHR );
}


inline float2 Transform (float2 vec, float4 rot, float off)
{
	XMVECTOR rotxy = create(getx(rot), gety(rot));
	XMVECTOR rotzw = create(getz(rot), getw(rot));

    return create(XMVectorGetX(XMVector2Dot(vec, rotxy)), XMVectorGetX(XMVector2Dot(vec, rotzw)));
}


XMVECTOR WindData::GetWindValue (const XMVECTOR&a_vTexCoord, const float a_fWindTexTile, const float a_fWindStrength) const
{
    return GetValue(a_vTexCoord, a_fWindTexTile) * a_fWindStrength;
}


XMVECTOR WindData::GetWindValueA (const XMVECTOR &a_vTexCoord, const float a_fWindTexTile, const float a_fWindStrength, int a_iSegmentIndex) const
{
    return GetValueA(a_vTexCoord, a_fWindTexTile, a_iSegmentIndex) * a_fWindStrength;
}


void WindData::ConvertFrom (const D3D11_MAPPED_SUBRESOURCE &a_MappedTex, const D3D11_TEXTURE2D_DESC &a_TexDesc)
{
    float* pTexels = (float*)a_MappedTex.pData;
	if (pData == NULL)
    {
        pData = new XMFLOAT3[a_TexDesc.Height * a_TexDesc.Width];
        pWindMapData = new XMFLOAT4[a_TexDesc.Height * a_TexDesc.Width];
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
            pWindMapData[row * a_TexDesc.Width + col].x = 0.0f;
            pWindMapData[row * a_TexDesc.Width + col].y = 0.0f;
            pWindMapData[row * a_TexDesc.Width + col].z = 0.0f;
			pWindMapData[row * a_TexDesc.Width + col].w = pTexels[rowStart + colStart + 3];
        }
    }
}


void WindData::WindCopy (ID3D11Texture2D *a_pDestTex, ID3D11DeviceContext *a_pDeviceCtx)
{
    float row_pitch = 4 * sizeof(float) * TEX_W;
    static float pTexels[TEX_W * TEX_H * 4];
    float fMass = 0.23f;
    float fLseg = 0.6f;
    float3 fa;
    D3D11_BOX dest_region;
   
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

				float3 vec1 = MakeRotationVector(mR[row * TEX_W + col][segment]);
                float3 vec2 = MakeRotationVector(mT[row * TEX_W + col][segment]);
                float3 halfAxis = create(0.0f, SegLen * 0.5f, 0.0f);
				
                float3 mg = scale(SegMass, create(0, -9.8f, 0));
			
              	if (segment >= 1) {
					mg = XMVector3TransformCoord(mg, mT[row * TEX_W + col][segment - 1]);
				}
				mg = XMVector3Cross(halfAxis, mg);
				XMVECTOR w = SegHard[segment] * vec1 - mg;
                
                if (segment >= 1)
                {
                    XMMATRIX transposed = XMMatrixTranspose(mT[row * TEX_W + col][segment - 1]);
					w = XMVector3TransformCoord(w, transposed);
		        }

                pTexels[rowStart + colStart + 0] = XMVectorGetX(w);
                pTexels[rowStart + colStart + 1] = XMVectorGetY(w);
                pTexels[rowStart + colStart + 2] = XMVectorGetZ(w);
                pTexels[rowStart + colStart + 3] = 1.0f;

                fmA[row * TEX_W + col][segment] = create(XMVectorGetX(w), XMVectorGetY(w), XMVectorGetZ(w));
            }
        }

        a_pDeviceCtx->UpdateSubresource(a_pDestTex, 
            D3D11CalcSubresource(0, segment, 1), &dest_region, (const void *)pTexels, (UINT)row_pitch, 0);
    }
}


float3 GetWaveW (XMVECTOR vCamDir)
{
	float fT;
	float3 waveW = create(1.0f, 1.0f, 0.0f);
	sety(vCamDir, 0.0f);;
	waveW = normalize(waveW);

	if ((getx(vCamDir) < 0.0f) && getz(vCamDir) < 0.7071f)
	{
		if (getz(vCamDir) > 0.3827)
		{
			fT = (0.7071f - getz(vCamDir)) / (0.7071f - 0.3827f);
			setx(waveW, 1.0f - fT);
		}
		else
		{
			if (getz(vCamDir) > 0.0f)
			{
				fT = (0.3827f - getz(vCamDir)) / 0.3827f;
				setx(waveW, 0.0f);
				setz(waveW, fT);
			}
			else
			{
				if (getz(vCamDir) > -0.3827f)
				{
					setx(waveW, 0.0f);;
					fT = (0.3827f + getz(vCamDir)) / 0.3827f;
					sety(waveW, fT);;
					setz(waveW, 1.0f);;
				}
				else
				{
					if (getz(vCamDir) > -0.7071f)
					{
						fT = (0.7071f + getz(vCamDir)) / (0.7071f - 0.3827f);
						setx(waveW, 1.0f - fT);;
						sety(waveW, 0.0f);;
						setz(waveW, 1.0f);;
					}
					else
					{
						if (getz(vCamDir) > -0.9239f)
						{
							fT = (0.9239f + getz(vCamDir)) / (0.9239f - 0.7071f);
							setx(waveW, 1.0f);;
							sety(waveW, 0.0f);;
							setz(waveW, fT);;
						}
						else 
						{
							fT = (1.0f + getz(vCamDir)) / (1.0f - 0.9239f);
							sety(waveW, 1.0f - fT);;
						}
					}
				}
			}
		}
	}
	return waveW;
}


static void CalcTR (float3x3 &Tres, float3x3 &Rres, float3x3 &T, float3x3 &T_1, float3x3 &R, float3 &psi)
{	
	Rres = XMMatrixMultiply(R, MakeRotationMatrix(psi));
	Tres = XMMatrixMultiply(T_1, Rres);
}


static XMVECTOR GetDw (XMMATRIX &T, XMMATRIX&R, XMVECTOR &fw, XMVECTOR &sum, XMVECTOR &w, float invJ, float segmentHeight, float Hardness, int j )
{
	XMVECTOR localSum, g, m_f;
	XMVECTOR halfAxis(create(0.0f, segmentHeight * 0.5f, 0.0f));
	
	localSum = XMVector3TransformCoord(sum, T);
	m_f = XMVector3Cross(halfAxis, localSum);
    g = Hardness * MakeRotationVector(R);

    return (m_f - g) * invJ;
}


static float3 GetVel (float3x3 &T, float3 &w, float segmentHeight)
{
	float3 halfAxis = create(0.0f, segmentHeight *0.5f, 0.0f), r;
	float3x3 transposed_t;
	transposed_t = XMMatrixTranspose(T);
    r = XMVector3Cross(w, halfAxis);
    r = XMVector3TransformCoord(r, transposed_t);
    return r;
}


static int   uCount      = 0;
static int   State       = 0;
static float maxW        = 0.0f;
static float TimeCont[4] = { 4.0f, 1.0f, 5.0f, 1.0f };
static float CurTime	 = 0.f;
static float Bound		 = TimeCont[0];


void WindData::UpdateWindTex (D3D11_MAPPED_SUBRESOURCE &a_MappedTex, float a_fElapsed, XMVECTOR a_vCamDir)
{
//    const GrassPropsUnified &props = grassProps[0];
		if (a_fElapsed >= 0.1f)
		a_fElapsed = 0.1f;

	uCount++;
	if (uCount>1500)
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
	float2 vTexCoord = create(0, 0, 0);
    float *pTexels = (float*)a_MappedTex.pData;
    XMFLOAT2 vUV;
    XMFLOAT4 vWind;
	float2 fPixHeight;
    float3 fw, fa, fw1, fa1;
	float d = powf(0.98f, a_fElapsed * 0.01f);
	if (d > 0.9998f) d = 0.9998f;
	float fDt = 0.05f;
	float fMass = 0.245f * 0.8f;//0.23f;
	float fLseg = 0.75f;//0.6f;
	float fJ_1 = 0.75f *1.0f / (0.33f*SegMass*SegLen*SegLen);
    float4 vRotate45  = create(0.7071f, 0.7071f, -0.7071f, 0.7071f);
    float4 vRotate315 = create(0.7071f, -0.7071f, 0.7071f, 0.7071f);

    float3 vWaveW = GetWaveW(a_vCamDir);
	for (int i = 0; i < 2; i++)
    {
        fTexOffsets[i] -= a_fElapsed * fTexOffsetKoefs[i] * *pWindSpeed;
    }
	
	float fT = 0.5f + 0.5f*sinf(4.0f*fTexOffsets[0]);
	float fT1 = 0.5f + 0.5f*sinf(7.5f*fTexOffsets[0]);
	float height[3];

	for( UINT row = 0; row < uHeight; row++ )
//	for( UINT row = 0; row < 1; row++ )
    {
        UINT rowStart = row * a_MappedTex.RowPitch / sizeof(float);
        sety(vTexCoord, float(row) / (fHeight - 1.0f));;

        for( UINT col = 0; col < uWidth; col++ )
//        for( UINT col = 0; col < 1; col++ )
        {
            UINT colStart = col * 4;//RGBA
            setx(vTexCoord, float(col) / (fWidth - 1.0f));;
			float2 vUV = create(0, 0);
			
			height[0]=height[1]=height[2]=0.0f;
			
			if (getx(vWaveW) > 0.001f)
			{
				setx(vUV, getx(vTexCoord));;
				sety(vUV, gety(vTexCoord));;
				setx(vUV, getx(vUV) + 1.5f * fTexOffsets[0]);
				height[0] = BiLinear(vUV);
			}            
			if (gety(vWaveW) > 0.001)
			{
	            vUV = 0.7071f * 2.0f * Transform(vTexCoord, vRotate45, fTexOffsets[1]);
				setx(vUV, getx(vUV) + fTexOffsets[1]);
			    height[1] = BiLinear(vUV);
			}
			if (getz(vWaveW) > 0.001f)
			{
				setx(vUV, gety(vTexCoord));;
				sety(vUV, getx(-vTexCoord));;
				setx(vUV, getx(vUV) + 1.4f * fTexOffsets[0]);
				height[2] = BiLinear(vUV);
			}            
            
			fPixHeight = (0.9f*fT + 0.1f) * (create(1.0f, 0.0f) * (1.5f - fT1) * height[0] + create(1.0f, 1.0f) * (0.5f + fT1) * height[1]
												+ create(0.0f, 1.0f) * (1.5f - fT1) * height[2]);

			float fDamp;
			setx(vUV, getx(vTexCoord));;
			sety(vUV, gety(vTexCoord));;
			setx(vUV, getx(vUV) + 1.f*fTexOffsets[0]);
			fDamp = BiLinear(vUV);
			fDamp = fDamp * (1.f - fTFull) + fTFull; 

			fPixHeight *= 6.f * fDamp;
			float fPixHmax = 6.f;
			float fLen = sqrt(getx(fPixHeight) * getx(fPixHeight) + gety(fPixHeight) * gety(fPixHeight) + 0.0001f); 
			if ( fLen > fPixHmax ) fPixHeight = fPixHmax * fPixHeight / fLen;

            float3 windBase, wind, vZ, v, r;
			windBase = create(getx(fPixHeight), 0.0f, gety(fPixHeight));
			vZ = create(0.0f, 0.0f, 0.0f);
			float3 halfAxis = create(0.0f, SegLen *0.5f, 0.0f);
			for (int segment = 0; segment < NUM_SEGMENTS - 1; segment++)
            {                
                UINT ind = row * uWidth + col;
                float3 psi;
                float3 g = create(0.0f, -9.8f, 0.0f);

				r = GetVel(mT[ind][segment], fmW[ind][segment], SegLen);
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
					T_1 = XMMatrixIdentity();

                CalcTR(Tres, Rres, mT[ind][segment], T_1, mR[ind][segment], psi);

				r = GetVel(Tres, w_, SegLen);
				v = vZ + r;
				wind = 0.02f * (windBase - v);

//                sum = g * SegMass * Tres[5] + wind;
				sum = g * SegMass + wind;
                float3 Dw_ = GetDw(Tres, Rres, w_, sum, wind, fJ_1, SegLen, h, segment);
                fmW[ind][segment] += 0.5f * a_fElapsed * (Dw + Dw_); 
                fmW[ind][segment] *= d;
                psi = a_fElapsed * fmW[ind][segment];
                
                // Rotate segment
                mR[ind][segment] = XMMatrixMultiply(mR[ind][segment], MakeRotationMatrix(psi));
                mT[ind][segment] = XMMatrixMultiply(T_1, mR[ind][segment]);

				r = GetVel(mT[ind][segment], fmW[ind][segment], SegLen);
				vZ = vZ + 2.f * r;
			}

			vWindData[row * uWidth + col].x = getx(fPixHeight);
			vWindData[row * uWidth + col].y = 0.0f;
			vWindData[row * uWidth + col].z = getz(fPixHeight);
  
		}
    }

}

Wind::Wind (ID3D11Device *a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx)
{
    m_fTime = 0.0f;
    m_pD3DDevice = a_pD3DDevice;
	m_pD3DDeviceCtx = a_pD3DDeviceCtx;

	/* Loading effect */
	ID3DBlob* pErrorBlob = nullptr;
	D3DX11CompileEffectFromFile(L"Shaders/WindEffect.fx",
		0,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		0,
		0,
		m_pD3DDevice,
		&m_pWindEffect,
		&pErrorBlob);

	if (pErrorBlob)
	{
		OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
		pErrorBlob->Release();
	}

	m_pHeightMapPass = m_pWindEffect->GetTechniqueByIndex(0)->GetPassByName("HeighttexToHeightmap");
    m_pWindMapPass   = m_pWindEffect->GetTechniqueByIndex(0)->GetPassByName("HeightmapToWindmap");
    m_pWindTexPass   = m_pWindEffect->GetTechniqueByIndex(0)->GetPassByName("WindmapToWindtex");

    m_uVertexStride = sizeof(QuadVertex);
    m_uVertexOffset = 0;
    CreateVertexBuffer();
    CreateInputLayout();

	m_pHeightTexESRV = m_pWindEffect->GetVariableByName("g_txHeightTex")->AsShaderResource();
	CreateDDSTextureFromFile(m_pD3DDevice, L"resources/Wind.dds", nullptr, &m_pHeightTexSRV);
	m_pHeightTexESRV->SetResource(m_pHeightTexSRV);

    m_pWindSpeedESV = m_pWindEffect->GetVariableByName( "g_fWindSpeed" )->AsScalar();
    m_pTimeESV      = m_pWindEffect->GetVariableByName( "g_fTime"      )->AsScalar();
    m_pWindBias     = m_pWindEffect->GetVariableByName( "g_fWindBias"  )->AsScalar();
    m_pWindScale    = m_pWindEffect->GetVariableByName( "g_fWindScale" )->AsScalar();
    m_pTimeESV->SetFloat(m_fTime);

#pragma region Height Map Creation
	D3D11_TEXTURE2D_DESC HeightMapDesc;
    ID3D11Resource *pRes;
    ID3D11Texture2D *pHeightTex;
    m_pHeightTexSRV->GetResource(&pRes);
    pRes->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pHeightTex);
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
	HeightMapDesc.Usage = D3D11_USAGE_DEFAULT;
	HeightMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	m_pD3DDevice->CreateTexture2D(&HeightMapDesc, NULL, &m_pHeightMap);

	/* Creating Render Target View for Height Map */
	D3D11_RENDER_TARGET_VIEW_DESC HeightMapRTVDesc;
	ZeroMemory( &HeightMapRTVDesc, sizeof(HeightMapRTVDesc) );
	HeightMapRTVDesc.Format = HeightMapDesc.Format;
	HeightMapRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	HeightMapRTVDesc.Texture2D.MipSlice = 0;
	m_pD3DDevice->CreateRenderTargetView(m_pHeightMap, &HeightMapRTVDesc, &m_pHeightMapRTV);

	/* Creating Shader Resource View for Height Map */
	D3D11_SHADER_RESOURCE_VIEW_DESC HeightMapSRVDesc;
	ZeroMemory( &HeightMapSRVDesc, sizeof(HeightMapSRVDesc) );
	HeightMapSRVDesc.Format = HeightMapDesc.Format;
	HeightMapSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	HeightMapSRVDesc.Texture2D.MostDetailedMip = 0;
	HeightMapSRVDesc.Texture2D.MipLevels = 1;
	m_pD3DDevice->CreateShaderResourceView(m_pHeightMap, &HeightMapSRVDesc, &m_pHeightMapSRV);
#pragma endregion

	m_pHeightMapESRV = m_pWindEffect->GetVariableByName("g_txHeightMap")->AsShaderResource();
	m_pHeightMapESRV->SetResource(m_pHeightMapSRV);
 
    ID3DX11EffectVectorVariable *pPixSize = m_pWindEffect->GetVariableByName("g_vPixSize")->AsVector();
    XMFLOAT2 vPixSize(1.0f / (float)m_uViewPortWidth, 1.0f / (float)m_uViewPortHeight); // 1/256
    pPixSize->SetFloatVector((float *) &vPixSize);

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
    m_WindTexStagingDesc.Usage = D3D11_USAGE_DEFAULT;//D3D11_USAGE_DYNAMIC;
    m_WindTexStagingDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;//D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    m_pD3DDevice->CreateTexture2D(&m_WindTexStagingDesc, NULL, &m_pWindTex);
    
    /* Creating Shader Resource View for Wind Tex */
    D3D11_SHADER_RESOURCE_VIEW_DESC WindTexSRVDesc;
    ZeroMemory( &WindTexSRVDesc, sizeof(WindTexSRVDesc) );
    WindTexSRVDesc.Format = m_WindTexStagingDesc.Format;
    WindTexSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    WindTexSRVDesc.Texture2DArray.MostDetailedMip = 0;
    WindTexSRVDesc.Texture2DArray.ArraySize = NUM_SEGMENTS - 1;
    WindTexSRVDesc.Texture2DArray.FirstArraySlice = 0;
    WindTexSRVDesc.Texture2DArray.MipLevels = 1;
    m_pD3DDevice->CreateShaderResourceView(m_pWindTex, &WindTexSRVDesc, &m_pWindTexSRV);

    /* Creating texture for reading on CPU */
    m_WindTexStagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    m_WindTexStagingDesc.BindFlags      = 0;
    m_WindTexStagingDesc.Usage          = D3D11_USAGE_STAGING;
    m_WindTexStagingDesc.ArraySize      = 1;
    m_pD3DDevice->CreateTexture2D(&m_WindTexStagingDesc, 0, &m_pWindTexStaging);
#pragma endregion

    /*m_pWindTexESRV = a_pGrassEffect->GetVariableByName("g_txWindTex")->AsShaderResource();
    m_pWindTexESRV->SetResource(m_pWindTexSRV);*/

#pragma region depth stencil texture
    m_pDepthTex = NULL;
    D3D11_TEXTURE2D_DESC dsDesc;
    dsDesc.Width = m_uViewPortWidth;
    dsDesc.Height = m_uViewPortHeight;
    dsDesc.MipLevels = 1;
    dsDesc.ArraySize = 1;
    dsDesc.Format = DXGI_FORMAT_D32_FLOAT;//R32_FLOAT ?
    dsDesc.SampleDesc.Count = 1;
    dsDesc.SampleDesc.Quality = 0;
    dsDesc.Usage = D3D11_USAGE_DEFAULT;
    dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    dsDesc.CPUAccessFlags = 0;
    dsDesc.MiscFlags = 0;
    m_pD3DDevice->CreateTexture2D( &dsDesc, NULL, &m_pDepthTex );

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC DescDS;
    ZeroMemory( &DescDS, sizeof(DescDS) );
    DescDS.Format = dsDesc.Format;
    DescDS.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    m_pD3DDevice->CreateDepthStencilView( m_pDepthTex, &DescDS, &m_pDSV );    
#pragma endregion

	MakeHeightMap();
	
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

    SAFE_RELEASE(m_pWindTex);
    SAFE_RELEASE(m_pWindTexStaging);

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

}

void Wind::SetWindScale( float a_fScale )
{

}

const WindData *Wind::WindDataPtr( )
{
    return &m_WindData;
}

void Wind::UpdateWindData( )
{
    /* Copying data from one texture to another */
    m_pD3DDeviceCtx->CopyResource(m_pWindTexStaging, m_pHeightMap);
	D3D11_MAPPED_SUBRESOURCE MappedTexture;
	m_pD3DDeviceCtx->Map(m_pWindTexStaging, D3D11CalcSubresource(0, 0, 1), D3D11_MAP_READ, 0, &MappedTexture);
    m_WindData.ConvertFrom(MappedTexture, m_WindTexStagingDesc);
	m_pD3DDeviceCtx->Unmap(m_pWindTexStaging, D3D11CalcSubresource(0, 0, 1));
}

void Wind::MakeHeightMap( )
{
	/* Saving render targets */
	ID3D11RenderTargetView *pOrigRT;
	ID3D11DepthStencilView *pOrigDS;
	D3D11_VIEWPORT         OrigViewPort[1];
	D3D11_VIEWPORT         ViewPort[1];
	UINT                   NumV = 1;
	m_pD3DDeviceCtx->RSGetViewports(&NumV, OrigViewPort);
	ViewPort[0] = OrigViewPort[0];
	ViewPort[0].Height = m_uViewPortHeight;
	ViewPort[0].Width  = m_uViewPortWidth;
	m_pD3DDeviceCtx->RSSetViewports(1, ViewPort);

	m_pD3DDeviceCtx->OMGetRenderTargets( 1, &pOrigRT, &pOrigDS );
	/* Setting up WindMap and depth stencil */    

	m_pD3DDeviceCtx->ClearRenderTargetView( m_pHeightMapRTV, ClearColor );
	m_pD3DDeviceCtx->OMSetRenderTargets(1, &m_pHeightMapRTV, NULL);
	/* Executing rendering */
	m_pD3DDeviceCtx->IASetInputLayout(m_pInputLayout);
	m_pD3DDeviceCtx->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
	m_pD3DDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pHeightMapPass->Apply(0, m_pD3DDeviceCtx);
	m_pD3DDeviceCtx->Draw(4, 0);

	/* Reverting changes */
	m_pD3DDeviceCtx->OMSetRenderTargets(1, &pOrigRT, pOrigDS);
	m_pD3DDeviceCtx->RSSetViewports(NumV, OrigViewPort);

	SAFE_RELEASE( pOrigRT );
	SAFE_RELEASE( pOrigDS );
}

void Wind::MakeWindMap( )
{
    m_pWindMapESRV->SetResource(NULL);
    /* Saving render targets */
    ID3D11RenderTargetView *pOrigRT;
    ID3D11DepthStencilView *pOrigDS;
    D3D11_VIEWPORT         OrigViewPort[1];
    D3D11_VIEWPORT         ViewPort[1];
    UINT                   NumV = 1;
    
	m_pD3DDeviceCtx->RSGetViewports(&NumV, OrigViewPort);
    ViewPort[0] = OrigViewPort[0];
    ViewPort[0].Height = m_uViewPortHeight;
    ViewPort[0].Width  = m_uViewPortWidth;
    m_pD3DDeviceCtx->RSSetViewports(1, ViewPort);

    m_pD3DDeviceCtx->OMGetRenderTargets( 1, &pOrigRT, &pOrigDS );
    /* Setting up WindMap and depth stencil */    
    
    m_pD3DDeviceCtx->ClearRenderTargetView( m_pWindMapRTV, ClearColor );
    //m_pD3DDevice->ClearDepthStencilView( m_pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
    m_pD3DDeviceCtx->OMSetRenderTargets(1, &m_pWindMapRTV, NULL);
    /* Executing rendering */
    m_pD3DDeviceCtx->IASetInputLayout(m_pInputLayout);
    m_pD3DDeviceCtx->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_uVertexStride, &m_uVertexOffset);
    m_pD3DDeviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_pWindMapPass->Apply(0, m_pD3DDeviceCtx);
    m_pD3DDeviceCtx->Draw(4, 0);

    /* Reverting changes */
    m_pD3DDeviceCtx->OMSetRenderTargets(1, &pOrigRT, pOrigDS);
    m_pD3DDeviceCtx->RSSetViewports(NumV, OrigViewPort);
    m_pWindMapESRV->SetResource(m_pWindMapSRV);
    
    SAFE_RELEASE( pOrigRT );
    SAFE_RELEASE( pOrigDS );
}

void Wind::MakeWindTex (float a_fElapsed, XMVECTOR a_vCamDir)
{    
	/*
	D3D11_MAPPED_SURESOURCE MappedTexture;
	m_WindData.UpdateWindTex(MappedTexture, a_fElapsed, a_vCamDir);
	m_pWindTex->Map(D3D10CalcSubresource(0, 0, 1), D3D10_MAP_WRITE_DISCARD, 0, &MappedTexture);
	//    m_WindData.UpdateWindTex(MappedTexture, a_fElapsed);
	WindCopy(MappedTexture);
	m_pWindTex->Unmap(D3D10CalcSubresource(0, 0, 1));
	*/

	D3D11_MAPPED_SUBRESOURCE MappedTexture;
	
    m_WindData.UpdateWindTex(MappedTexture, a_fElapsed, a_vCamDir);
    m_WindData.WindCopy(m_pWindTex, m_pD3DDeviceCtx);
}

void Wind::Update (float a_fElapsed, XMVECTOR a_vCamDir)
{
    m_fTime += a_fElapsed;
    m_pTimeESV->SetFloat(m_fTime);
    MakeWindTex(a_fElapsed, a_vCamDir);
}


void Wind::CreateVertexBuffer (void)
{
    /* Initializing vertices */
    QuadVertex Vertices[4];
    Vertices[0].vPos = XMFLOAT3(-1.0f, -1.0f, 0.1f);
    Vertices[0].vTexCoord = XMFLOAT2(0.0f, 0.0f);

    Vertices[1].vPos = XMFLOAT3(1.0f, -1.0f, 0.1f);
    Vertices[1].vTexCoord = XMFLOAT2(1.0f, 0.0f);

    Vertices[2].vPos = XMFLOAT3(-1.0f, 1.0f, 0.1f);
    Vertices[2].vTexCoord = XMFLOAT2(0.0f, 1.0f);

    Vertices[3].vPos = XMFLOAT3(1.0f, 1.0f, 0.1f);
    Vertices[3].vTexCoord = XMFLOAT2(1.0f, 1.0f);
   
	/* Initializing buffer */
    D3D11_BUFFER_DESC BufferDesc = 
    {
        4 * sizeof(QuadVertex),
        D3D11_USAGE_DEFAULT,
        D3D11_BIND_VERTEX_BUFFER,
        0, 0
    };
    D3D11_SUBRESOURCE_DATA BufferInitData;
    BufferInitData.pSysMem = Vertices;
    m_pD3DDevice->CreateBuffer(&BufferDesc, &BufferInitData, &m_pVertexBuffer);
}


void Wind::CreateInputLayout (void)
{
    D3D11_INPUT_ELEMENT_DESC InputDesc[] = 
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    D3DX11_PASS_DESC PassDesc;
    m_pWindMapPass->GetDesc(&PassDesc);
    int InputElementsCount = sizeof(InputDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);
    m_pD3DDevice->CreateInputLayout(InputDesc, InputElementsCount, 
        PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, 
        &m_pInputLayout);   
}


ID3D11ShaderResourceView *Wind::GetMap (void)
{
    return m_pWindTexSRV;
}
