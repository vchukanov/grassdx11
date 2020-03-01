#include "DebugWindow.h"
#include "includes.h"

DebugWindow::DebugWindow(ID3D11Device* device, int screenWidth, int screenHeight, ID3D11ShaderResourceView *pSRV, float scale)
{
   m_pDevice = device;
   m_fScale = scale;

   // Store the screen size.
   m_screenWidth = screenWidth;
   m_screenHeight = screenHeight;

   // Getting desc of texture
   ID3D11Texture2D* pTextureInterface = 0;
   ID3D11Resource* res;
   pSRV->GetResource(&res);
   res->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
   D3D11_TEXTURE2D_DESC desc;
   pTextureInterface->GetDesc(&desc);
   SAFE_RELEASE(pTextureInterface);

   // Store the size in pixels that this bitmap should be rendered at.
   m_bitmapWidth = desc.Width * (int)floor(scale);
   m_bitmapHeight = desc.Height * (int)floor(scale);

   // Initialize the previous rendering position to negative one.
   m_previousPosX = -1;
   m_previousPosY = -1;

   // Initialize the vertex and index buffers.
   InitializeBuffers(device);

   
   /* Getting technique from fx */
   ID3DBlob* pErrorBlob = nullptr;
   D3DX11CompileEffectFromFile(L"Shaders/DebugWindow.fx",
      0,
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      0,
      0,
      device,
      &m_pEffect,
      &pErrorBlob);

   if (pErrorBlob)
   {
      OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
      pErrorBlob->Release();
   }

   ID3DX11EffectTechnique* pTechnique = m_pEffect->GetTechniqueByIndex(0);
   m_pPass = pTechnique->GetPassByName("P0");
   
   m_pTexESRV = m_pEffect->GetVariableByName("g_texture")->AsShaderResource();
   m_pTexESRV->SetResource(pSRV);

   m_pArraySlice = m_pEffect->GetVariableByName("g_iArraySlice")->AsScalar();
   m_pArraySlice->SetInt(1);

   m_fScaleESV = m_pEffect->GetVariableByName("g_fScale")->AsScalar();
   m_fScaleESV->SetFloat(m_fScale);

   m_pOrthoEMV = m_pEffect->GetVariableByName("g_mOrtho")->AsMatrix();
   m_pTransformEMV = m_pEffect->GetVariableByName("g_mWorld")->AsMatrix();

   CreateInputLayout();
}


DebugWindow::~DebugWindow(void)
{
   SAFE_RELEASE(m_indexBuffer);
   SAFE_RELEASE(m_vertexBuffer);
   SAFE_RELEASE(m_pPass)
   SAFE_RELEASE(m_pEffect);
   SAFE_RELEASE(m_pInputLayout);
}


bool DebugWindow::Render (ID3D11DeviceContext* deviceContext, int positionX, int positionY)
{
   if (!m_bNeedRender) {
      return true;
   }

   bool result;

   // Re-build the dynamic vertex buffer for rendering to possibly a different location on the screen.
   result = UpdateBuffers(deviceContext, positionX, positionY);
   if (!result)
   {
      return false;
   }

   // Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
   RenderBuffers(deviceContext);

   return true;
}


void DebugWindow::SetOrthoMtx (float4x4& a_mViewProj)
{
   m_pOrthoEMV->SetMatrix((float*)& a_mViewProj);
}


void DebugWindow::SetWorldMtx (float4x4& a_mWorld)
{
   m_pTransformEMV->SetMatrix((float*)& a_mWorld);
}


void DebugWindow::ToggleSlice (void)
{
   m_iCurSlice = ++m_iCurSlice % (NUM_SEGMENTS - 1); 
   m_pArraySlice->SetInt(m_iCurSlice);
}


int DebugWindow::GetIndexCount()
{
   return m_indexCount;
}


bool DebugWindow::InitializeBuffers (ID3D11Device* device)
{
   VertexType    *vertices;
   unsigned long *indices;
   D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
   D3D11_SUBRESOURCE_DATA vertexData, indexData;
   HRESULT result;
   int i;

   // Set the number of vertices in the vertex array.
   m_vertexCount = 6;

   // Set the number of indices in the index array.
   m_indexCount = m_vertexCount;

   // Create the vertex array.
   vertices = new VertexType[m_vertexCount];
   if (!vertices)
   {
      return false;
   }

   // Create the index array.
   indices = new unsigned long[m_indexCount];
   if (!indices)
   {
      return false;
   }

   // Initialize vertex array to zeros at first.
   memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

   // Load the index array with data.
   for (i = 0; i < m_indexCount; i++)
   {
      indices[i] = i;
   }

   // Set up the description of the static vertex buffer.
   vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
   vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
   vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
   vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
   vertexBufferDesc.MiscFlags = 0;
   vertexBufferDesc.StructureByteStride = 0;

   // Give the subresource structure a pointer to the vertex data.
   vertexData.pSysMem = vertices;
   vertexData.SysMemPitch = 0;
   vertexData.SysMemSlicePitch = 0;

   // Now create the vertex buffer.
   result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
   if (FAILED(result))
   {
      return false;
   }

   // Set up the description of the static index buffer.
   indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
   indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
   indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
   indexBufferDesc.CPUAccessFlags = 0;
   indexBufferDesc.MiscFlags = 0;
   indexBufferDesc.StructureByteStride = 0;

   // Give the subresource structure a pointer to the index data.
   indexData.pSysMem = indices;
   indexData.SysMemPitch = 0;
   indexData.SysMemSlicePitch = 0;

   // Create the index buffer.
   result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
   if (FAILED(result))
   {
      return false;
   }

   // Release the arrays now that the vertex and index buffers have been created and loaded.
   delete[] vertices;
   vertices = 0;

   delete[] indices;
   indices = 0;

   return true;
}



bool DebugWindow::UpdateBuffers (ID3D11DeviceContext* deviceContext, int positionX, int positionY)
{
   float left, right, top, bottom;
   VertexType* vertices;
   D3D11_MAPPED_SUBRESOURCE mappedResource;
   VertexType* verticesPtr;
   HRESULT result;


   // If the position we are rendering this bitmap to has not changed then don't update the vertex buffer since it
   // currently has the correct parameters.
   if ((positionX == m_previousPosX) && (positionY == m_previousPosY))
   {
      return true;
   }

   // If it has changed then update the position it is being rendered to.
   m_previousPosX = positionX;
   m_previousPosY = positionY;

   // Calculate the screen coordinates of the left side of the bitmap.
   left = (float)((m_screenWidth / 2) * -1) + (float)positionX;

   // Calculate the screen coordinates of the right side of the bitmap.
   right = left + (float)m_bitmapWidth;

   // Calculate the screen coordinates of the top of the bitmap.
   top = (float)(m_screenHeight / 2) - (float)positionY;

   // Calculate the screen coordinates of the bottom of the bitmap.
   bottom = top - (float)m_bitmapHeight;


   // Create the vertex array.
   vertices = new VertexType[m_vertexCount];
   if (!vertices)
   {
      return false;
   }

   // Load the vertex array with data.
   // First triangle.
   vertices[0].position = XMFLOAT3(left, top, 0.0f);  // Top left.
   vertices[0].texture = XMFLOAT2(0.0f, 0.0f);

   vertices[1].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
   vertices[1].texture = XMFLOAT2(1.0f, 1.0f);

   vertices[2].position = XMFLOAT3(left, bottom, 0.0f);  // Bottom left.
   vertices[2].texture = XMFLOAT2(0.0f, 1.0f);

   // Second triangle.
   vertices[3].position = XMFLOAT3(left, top, 0.0f);  // Top left.
   vertices[3].texture = XMFLOAT2(0.0f, 0.0f);

   vertices[4].position = XMFLOAT3(right, top, 0.0f);  // Top right.
   vertices[4].texture = XMFLOAT2(1.0f, 0.0f);

   vertices[5].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
   vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

   // Lock the vertex buffer so it can be written to.
   result = deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
   if (FAILED(result))
   {
      return false;
   }

   // Get a pointer to the data in the vertex buffer.
   verticesPtr = (VertexType*)mappedResource.pData;

   // Copy the data into the vertex buffer.
   memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

   // Unlock the vertex buffer.
   deviceContext->Unmap(m_vertexBuffer, 0);

   // Release the vertex array as it is no longer needed.
   delete[] vertices;
   vertices = nullptr;

   return true;
}


void DebugWindow::CreateInputLayout (void)
{
   D3D11_INPUT_ELEMENT_DESC InputDesc[] =
   {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0},
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
   };
   D3DX11_PASS_DESC PassDesc;
   m_pPass->GetDesc(&PassDesc);
   int InputElementsCount = sizeof(InputDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);
   m_pDevice->CreateInputLayout(InputDesc, InputElementsCount,
      PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize,
      &m_pInputLayout);
}

void DebugWindow::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
   unsigned int stride;
   unsigned int offset;

   // Set vertex buffer stride and offset.
   stride = sizeof(VertexType);
   offset = 0;

   deviceContext->IASetInputLayout(m_pInputLayout);
   deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
   deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
   deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
   m_pPass->Apply(0, deviceContext);
   deviceContext->DrawIndexed(m_indexCount, 0, 0);

   return;
}
