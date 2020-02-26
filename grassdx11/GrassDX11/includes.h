#pragma once

#include <d3dx11effect.h>
#include <string>

#include <DDSTextureLoader.h>
#include <DirectXTex.h>

#include "DXUT.h"
#include "PhysMath.h"

#include "memdebug.h"

using namespace DirectX;

#define NUM_SEGMENTS 4
#define NUM_BLADEPTS 5

#define M_PI         3.14159265358979323846
#define INVALID_DIST 1e9
#define NO_VALUE     -1

#define XM_TO_V(xm, v, ending) \
XMVECTOR v = XMLoadFloat##ending(&xm);

#define V_TO_XM(v, xm, ending) \
XMFLOAT##3 xm; \
XMStoreFloat##ending(&xm, v)

#define XM_TO_M(xm, m) \
XMMATRIX m = XMLoadFloat4x4(&xm);

#define M_TO_XM(m, xm) \
XMFLOAT4X4 xm; \
XMStoreFloat4x4(&xm, m)

const float g_fPrecision    = 0.01f;
const int   g_iInvPrecision = 100;
const float g_fEps          = 0.001f;

inline bool XMVector3Alike (XMVECTOR a_vVec1, XMVECTOR a_vVec2)
{
   return (fabs(getx(a_vVec1) - getx(a_vVec2)) < g_fEps) &&
      (fabs(gety(a_vVec1) - gety(a_vVec2)) < g_fEps) &&
      (fabs(getz(a_vVec1) - getz(a_vVec2)) < g_fEps);
}


inline float fRand (float a_fLeft, float a_fRight)
{
   if (a_fRight < a_fLeft)
      return 0.0f;
   /* Random value in (0, 1) */
   float fRandom = (rand() % g_iInvPrecision) * g_fPrecision;

   return (a_fRight - a_fLeft) * fRandom + a_fLeft;
}


inline float fRand (float a_fMax)
{
   return fRand(0.0f, a_fMax);
}


inline float SignedfRand (float a_fMax)
{
   return fRand (-a_fMax, a_fMax);
}


inline HRESULT D3DXLoadTextureArray (ID3D11Device* a_pD3DDevice, ID3D11DeviceContext* a_pD3DDeviceCtx, std::vector< std::wstring > a_sTexNames,
   ID3D11Texture2D** a_ppTex2D, ID3D11ShaderResourceView** a_ppSRV)
{
   HRESULT hr = S_OK;
   D3D11_TEXTURE2D_DESC desc;
   ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
   size_t i;

   //WCHAR str[MAX_PATH];
   size_t iNumTextures = a_sTexNames.size();
   if (iNumTextures == 0)
   {
      *a_ppTex2D = NULL;
      *a_ppSRV = NULL;
      return hr;
   }
   for (i = 0; i < iNumTextures; i++)
   {
      ID3D11Resource* pRes = NULL;
      V_RETURN(CreateDDSTextureFromFileEx(a_pD3DDevice, a_sTexNames[i].c_str(), 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ, 0, 0, &pRes, nullptr));
      if (pRes)
      {
         ID3D11Texture2D* pTemp;
         pRes->QueryInterface(__uuidof(ID3D11Texture2D), (LPVOID*)& pTemp);
         pTemp->GetDesc(&desc);

         if (DXGI_FORMAT_B8G8R8A8_UNORM != desc.Format)   //make sure we're R8G8B8A8
            return false;

         if (desc.MipLevels > 4)
            desc.MipLevels -= 4;
         
         if (!(*a_ppTex2D))
         {
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = 0;
            desc.ArraySize = (UINT)iNumTextures;
            V_RETURN(a_pD3DDevice->CreateTexture2D(&desc, NULL, a_ppTex2D));
         }

         D3D11_MAPPED_SUBRESOURCE mappedTex2D;
         for (UINT iMip = 0; iMip < desc.MipLevels; iMip++)
         {
            a_pD3DDeviceCtx->Map(pTemp, iMip, D3D11_MAP_READ, 0, &mappedTex2D);

            a_pD3DDeviceCtx->UpdateSubresource((*a_ppTex2D),
               D3D11CalcSubresource(iMip, (UINT)i, desc.MipLevels),
               NULL,
               mappedTex2D.pData,
               mappedTex2D.RowPitch,
               0);

            a_pD3DDeviceCtx->Unmap(pTemp, iMip);
         }

         SAFE_RELEASE(pRes);
         SAFE_RELEASE(pTemp);
      }
      else
      {
         return false;
      }
   }

   D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
   ZeroMemory(&SRVDesc, sizeof(SRVDesc));
   SRVDesc.Format = desc.Format;
   SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
   SRVDesc.Texture2DArray.MipLevels = desc.MipLevels;
   SRVDesc.Texture2DArray.ArraySize = (UINT)iNumTextures;
   V_RETURN(a_pD3DDevice->CreateShaderResourceView(*a_ppTex2D, &SRVDesc, a_ppSRV));

   return hr;
}
