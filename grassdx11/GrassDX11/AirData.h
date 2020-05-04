#pragma once

#include "includes.h"

class AirData {
public:
   AirData (ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceCtx)
   {
      data = new XMFLOAT3[dataSize];  // FlowW * FlowH * segments count
      dev = pD3DDevice;
      devcon = pD3DDeviceCtx;

      D3D11_TEXTURE2D_DESC textureDesc;
      ZeroMemory(&textureDesc, sizeof(textureDesc));
      textureDesc.Width = 512; // FlowW
      textureDesc.Height = 512; // FlowH
      textureDesc.MipLevels = 1;
      textureDesc.ArraySize = 3;
      textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
      textureDesc.SampleDesc.Count = 1;
      textureDesc.Usage = D3D11_USAGE_STAGING;
      textureDesc.BindFlags = 0;
      textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
      textureDesc.MiscFlags = 0;

      // Create the render target textures.
      dev->CreateTexture2D(&textureDesc, NULL, &texture);
   }


   ~AirData (void) {
      delete[] data;
   }

   void Update (ID3D11Texture2D* newTexture) {
      D3D11_MAPPED_SUBRESOURCE mappedSubResource;

      devcon->CopyResource(texture, newTexture);

      HRESULT mapResult = devcon->Map(texture, 0, D3D11_MAP_READ, 0, &mappedSubResource);
      if (mapResult != S_OK) {
         assert(false);
      }

      memcpy(data, mappedSubResource.pData, dataSize * sizeof(XMFLOAT3));
      devcon->Unmap(texture, 0);
   }

   XMVECTOR GetAirValue (const XMVECTOR& a_vTexCoord) const
   {
      float fX = getx(a_vTexCoord);
      float fY = gety(a_vTexCoord);
      /* bilinear interpolation... */
      fX = (fX - floorf(fX)) * 512.f - 0.5f;
      fY = (fY - floorf(fY)) * 512.f - 0.5f;
      float fFracX = fX - floorf(fX);
      float fFracY = fY - floorf(fY);
      UINT uLX = UINT(fX);
      UINT uHX = uLX + 1;
      UINT uLY = UINT(fY);
      UINT uHY = uLY + 1;
      if (uHX > 512 - 1)
         uHX = 511;
      if (uHY > 512 - 1)
         uHY = 511;

      XMFLOAT3 fLL = data[512 * uLY + uLX];
      XMFLOAT3 fHL = data[512 * uHY + uLX];
      XMFLOAT3 fLR = data[512 * uLY + uHX];
      XMFLOAT3 fHR = data[512 * uHY + uHX];

      XM_TO_V(fLL, v_fLL, 3);
      XM_TO_V(fHL, v_fHL, 3);
      XM_TO_V(fLR, v_fLR, 3);
      XM_TO_V(fHR, v_fHR, 3);

      return ((1.0f - fFracX) * v_fLL + fFracX * v_fLR) * (1.0f - fFracY) +
         fFracY * ((1.0f - fFracX) * v_fHL + fFracX * v_fHR);
   }


   /*XMVECTOR GetAirValueA (const XMVECTOR& a_vTexCoord, int a_iSegmentIndex) const
   {
      float fX = getx(a_vTexCoord);
      float fY = gety(a_vTexCoord);

      fX = (fX - floorf(fX)) * 512.f - 0.5f;
      fY = (fY - floorf(fY)) * 512.f - 0.5f;
      float fFracX = fX - floorf(fX);
      float fFracY = fY - floorf(fY);
      UINT uLX = UINT(fX);
      UINT uHX = uLX + 1;
      UINT uLY = UINT(fY);
      UINT uHY = uLY + 1;
      if (uHX > 512 - 1)
         uHX = 511;
      if (uHY > 512 - 1)
         uHY = 511;

      const XMVECTOR& v_fLL = fmA[512 * uLY + uLX][a_iSegmentIndex];
      const XMVECTOR& v_fHL = fmA[512 * uHY + uLX][a_iSegmentIndex];
      const XMVECTOR& v_fLR = fmA[512 * uLY + uHX][a_iSegmentIndex];
      const XMVECTOR& v_fHR = fmA[512 * uHY + uHX][a_iSegmentIndex];

      return ((1.0f - fFracX) * v_fLL + fFracX * v_fLR) * (1.0f - fFracY) +
         fFracY * ((1.0f - fFracX) * v_fHL + fFracX * v_fHR);
   }

   void Copy (void) 
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
         for (UINT row = 0; row < TEX_H; row++)
         {
            UINT rowStart = (UINT)(row * row_pitch / sizeof(float));

            for (UINT col = 0; col < TEX_W; col++)
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

         a_pDeviceCtx->UpdateSubresource(a_pDestTex, D3D11CalcSubresource(0, segment, 1), &dest_region, (const void*)pTexels, (UINT)row_pitch, 0);
      }
   }
   */
private:
   XMFLOAT3* data;
   ID3D11Device* dev;
   ID3D11DeviceContext* devcon;
   ID3D11Texture2D* texture;

   const int dataSize = 512 * 512 * 3;
};