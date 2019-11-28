#pragma once

#include <vector>
#include <algorithm>
#include "includes.h"
#include "GrassPatch.h"

struct XMMATRIXEXT
{
	XMFLOAT4X4 mValue;
	UINT       uOnEdge;
	float      fCamDist;
	XMMATRIXEXT (void) {}
	XMMATRIXEXT (const XMFLOAT4X4& a_mValue, const float a_fCamDist)
	{
		mValue = a_mValue;
		uOnEdge = 0;
		fCamDist = a_fCamDist;
	}
};

class GrassLod
{
private:
	ID3D11Device			*m_pD3DDevice;
	ID3D11DeviceContext		*m_pD3DDeviceCtx;
	GrassPatch				*m_pGrassPatch;
	std::vector<XMMATRIXEXT> m_Transforms;
	UINT                     m_TransformsStride;
	UINT                     m_TransformsOffset;
	ID3D11Buffer			*m_pTransformsBuffer;

public:
	GrassLod  (GrassPatch* a_pPatch);
	~GrassLod (void);
	
	void  SetTransformsCount (DWORD a_dwCount);
	DWORD GetTransformsCount (void);
	/* Number of vertices in patch */
	
	DWORD		VerticesCount	      (void);
	XMFLOAT4X4& GetTransform	      (DWORD a_dwIndex);
	void		AddTransform	      (XMFLOAT4X4& a_mTransform, float a_fCamDist, bool a_bIsCornerPatch, int a_iIndex = -1);
	void		GenTransformBuffer    (void);
	void		UpdateTransformBuffer (void);
	
	/* Loading transforms into slot 1, patch into slot 0 */
	void IASetVertexBuffers (void);
};