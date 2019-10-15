/* common functions and values */
#pragma once

class Mesh;

#include <vector>
#include <string>
#include "DXUT.h"
#include <d3d10.h>
#include <d3dx10.h>

#define SIGN(x) ((x) > 0 ? 1 : -1)

/**
*/
#define NUM_SEGMENTS 4
#define NUM_BLADEPTS 5


#define float4 D3DXVECTOR4
#define float3 D3DXVECTOR3
#define float2 D3DXVECTOR2
#define INVALID_DIST 1e9
#define NO_VALUE   -1
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
const float g_fMeter = 1.0f;
const float g_fEps = 0.001f;
const float g_fPrecision = 0.01f;
const float g_fDeg2Rad = (float)M_PI / 180.0f;
const int   g_iInvPrecision = 100;

using namespace std;

inline bool D3DXVec3Alike(D3DXVECTOR3 a_vVec1, D3DXVECTOR3 a_vVec2)
{
    return (fabs(a_vVec1.x - a_vVec2.x) < g_fEps) && 
        (fabs(a_vVec1.y - a_vVec2.y) < g_fEps) &&
        (fabs(a_vVec1.z - a_vVec2.z) < g_fEps);
}

inline float fRand( float a_fLeft, float a_fRight )
{
    if (a_fRight < a_fLeft)
        return 0.0f;
    /* Random value in (0, 1) */
    float fRandom = (rand() % g_iInvPrecision) * g_fPrecision;

    return (a_fRight - a_fLeft) * fRandom + a_fLeft;
}

inline float fRand( float a_fMax )
{
    return fRand(0.0f, a_fMax);
}

inline float SignedfRand( float a_fMax )
{
    return fRand(-a_fMax, a_fMax);
}

inline HRESULT D3DXLoadTextureArray(ID3D10Device* a_pD3DDevice, std::vector< std::wstring > a_sTexNames, 
                            ID3D10Texture2D** a_ppTex2D, ID3D10ShaderResourceView** a_ppSRV)
{
    HRESULT hr = S_OK;
    D3D10_TEXTURE2D_DESC desc;
    ZeroMemory( &desc, sizeof(D3D10_TEXTURE2D_DESC) );
    size_t i;

    //WCHAR str[MAX_PATH];
    size_t iNumTextures = a_sTexNames.size();
    if (iNumTextures == 0)
    {
        *a_ppTex2D = NULL;
        *a_ppSRV   = NULL;
        return hr;
    }
    for( i = 0; i < iNumTextures; i++)
    {
        //V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, szTextureNames[i] ) );

        ID3D10Resource *pRes = NULL;
        D3DX10_IMAGE_LOAD_INFO loadInfo;
        ZeroMemory( &loadInfo, sizeof( D3DX10_IMAGE_LOAD_INFO ) );
        loadInfo.Width = D3DX_FROM_FILE;
        loadInfo.Height  = D3DX_FROM_FILE;
        loadInfo.Depth  = D3DX_FROM_FILE;
        loadInfo.FirstMipLevel = 0;
        loadInfo.MipLevels = D3DX_FROM_FILE;
        loadInfo.Usage = D3D10_USAGE_STAGING;
        loadInfo.BindFlags = 0;
        loadInfo.CpuAccessFlags = D3D10_CPU_ACCESS_WRITE | D3D10_CPU_ACCESS_READ;
        loadInfo.MiscFlags = 0;
        loadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        loadInfo.Filter = D3DX10_FILTER_NONE;
        loadInfo.MipFilter = D3DX10_FILTER_NONE;

        V_RETURN(D3DX10CreateTextureFromFile( a_pD3DDevice, a_sTexNames[i].c_str(), &loadInfo, NULL, &pRes, &hr ));
        if( pRes )
        {
            ID3D10Texture2D* pTemp;
            pRes->QueryInterface( __uuidof( ID3D10Texture2D ), (LPVOID*)&pTemp );
            pTemp->GetDesc( &desc );

            D3D10_MAPPED_TEXTURE2D mappedTex2D;
            if(DXGI_FORMAT_R8G8B8A8_UNORM != desc.Format)   //make sure we're R8G8B8A8
                return false;

            if(desc.MipLevels > 4)
                desc.MipLevels -= 4;
            //desc.MipLevels = 1;
            if(!(*a_ppTex2D))
            {
                desc.Usage = D3D10_USAGE_DEFAULT;
                desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
                desc.CPUAccessFlags = 0;
                desc.ArraySize = (UINT)iNumTextures;
                V_RETURN(a_pD3DDevice->CreateTexture2D( &desc, NULL, a_ppTex2D));
            }

            for(UINT iMip=0; iMip < desc.MipLevels; iMip++)
            {
                pTemp->Map( iMip, D3D10_MAP_READ, 0, &mappedTex2D );

                a_pD3DDevice->UpdateSubresource( (*a_ppTex2D), 
                    D3D10CalcSubresource( iMip, (UINT)i, desc.MipLevels ),
                    NULL,
                    mappedTex2D.pData,
                    mappedTex2D.RowPitch,
                    0 );

                pTemp->Unmap( iMip );
            }

            SAFE_RELEASE( pRes );
            SAFE_RELEASE( pTemp );
        }
        else
        {
            return false;
        }
    }

    D3D10_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    ZeroMemory( &SRVDesc, sizeof(SRVDesc) );
    SRVDesc.Format = desc.Format;
    SRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2DARRAY;
    SRVDesc.Texture2DArray.MipLevels = desc.MipLevels;
    SRVDesc.Texture2DArray.ArraySize = (UINT)iNumTextures;
    V_RETURN(a_pD3DDevice->CreateShaderResourceView( *a_ppTex2D, &SRVDesc, a_ppSRV ));

    return hr;
}

inline HRESULT D3DXLoadCubemap(ID3D10Device* a_pD3DDevice, const std::wstring a_sTexNames[6], 
                                    ID3D10ShaderResourceView** a_ppSRV)
{
    HRESULT hr = S_OK;
    D3D10_TEXTURE2D_DESC desc;
    ZeroMemory( &desc, sizeof(D3D10_TEXTURE2D_DESC) );
    //ID3D10Texture2D** ppTex2D = NULL;
    ID3D10Texture2D* pTex2D = NULL;
    int i;    
    
    for( i = 0; i < 6; i++)
    {
        ID3D10Resource *pRes = NULL;
        D3DX10_IMAGE_LOAD_INFO loadInfo;
        ZeroMemory( &loadInfo, sizeof( D3DX10_IMAGE_LOAD_INFO ) );
        loadInfo.Width          = D3DX_FROM_FILE;
        loadInfo.Height         = D3DX_FROM_FILE;
        loadInfo.Depth          = D3DX_FROM_FILE;
        loadInfo.FirstMipLevel  = 0;
        loadInfo.MipLevels      = D3DX_FROM_FILE;
        loadInfo.Usage          = D3D10_USAGE_STAGING;
        loadInfo.BindFlags      = 0;
        loadInfo.CpuAccessFlags = D3D10_CPU_ACCESS_WRITE | D3D10_CPU_ACCESS_READ;
        loadInfo.MiscFlags      = 0;
        loadInfo.Format         = DXGI_FORMAT_R8G8B8A8_UNORM;
        loadInfo.Filter         = D3DX10_FILTER_NONE;
        loadInfo.MipFilter      = D3DX10_FILTER_NONE;

        V_RETURN(D3DX10CreateTextureFromFile( a_pD3DDevice, a_sTexNames[i].c_str(), &loadInfo, NULL, &pRes, &hr ));
        if( pRes )
        {
            ID3D10Texture2D* pTemp;
            pRes->QueryInterface( __uuidof( ID3D10Texture2D ), (LPVOID*)&pTemp );
            pTemp->GetDesc( &desc );

            D3D10_MAPPED_TEXTURE2D mappedTex2D;
            if(DXGI_FORMAT_R8G8B8A8_UNORM != desc.Format)   //make sure we're R8G8B8A8
                return false;

            if(desc.MipLevels > 4)
                desc.MipLevels -= 4;
            if(!(pTex2D))
            {
                desc.Usage          = D3D10_USAGE_DEFAULT;
                desc.BindFlags      = D3D10_BIND_SHADER_RESOURCE;
                desc.CPUAccessFlags = 0;
                desc.ArraySize      = 6;
                desc.MiscFlags      = D3D10_RESOURCE_MISC_TEXTURECUBE;
                V_RETURN(a_pD3DDevice->CreateTexture2D( &desc, NULL, &pTex2D));
            }

            for(UINT iMip = 0; iMip < desc.MipLevels; iMip++)
            {
                pTemp->Map( iMip, D3D10_MAP_READ, 0, &mappedTex2D );

                a_pD3DDevice->UpdateSubresource( (pTex2D), 
                    D3D10CalcSubresource( iMip, i, desc.MipLevels ),
                    NULL,
                    mappedTex2D.pData,
                    mappedTex2D.RowPitch,
                    0 );

                pTemp->Unmap( iMip );
            }
            

            SAFE_RELEASE( pRes );
            SAFE_RELEASE( pTemp );
        }
        else
        {
            return false;
        }
    }

    D3D10_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    ZeroMemory( &SRVDesc, sizeof(SRVDesc) );
    SRVDesc.Format = desc.Format;
    SRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURECUBE;
    SRVDesc.TextureCube.MipLevels = 1;
    SRVDesc.TextureCube.MostDetailedMip = 0;
    V_RETURN(a_pD3DDevice->CreateShaderResourceView( pTex2D, &SRVDesc, a_ppSRV ));
    SAFE_RELEASE(pTex2D);

    return hr;
}

class CID3D10Include: public ID3D10Include
{
public:
    STDMETHOD(Open)(D3D10_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
    {
        // Read the file content
        FILE *pInFile = NULL; 
        fopen_s(&pInFile, pFileName, "rb");        
        if (pInFile == NULL)
            return E_FAIL;

        // Get the file size
        fseek(pInFile, 0, SEEK_END);
        UINT uBufSize = ftell(pInFile);
        fseek(pInFile, 0, SEEK_SET);

        // Get the file content
        char *buffer = new char[uBufSize];
        fread(buffer, 1, uBufSize, pInFile);
        fclose(pInFile);

        // Save the file data into ppData and the size into pBytes.
        *ppData = buffer;
        *pBytes = UINT(uBufSize);
        return S_OK;
    }

    STDMETHOD(Close)( LPCVOID pData )
    {
        char *pBuffer = (char *)pData;
        delete [] pBuffer;
        return S_OK;
    }
};

static CID3D10Include g_D3D10Include;