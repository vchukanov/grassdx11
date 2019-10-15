#pragma once

#include <vector>
#include "GrassPatch.h"

struct StaticPatchInfo
{    
    D3DXMATRIX mTransform;
    UINT       uMeshIndex;    
    StaticPatchInfo ( D3DXMATRIX a_mTransform, UINT a_uMeshIndex );
    StaticPatchInfo ( );
};

/** 
* GrassCollideStatic class
* Class for optimizing collision at long distance
*/
class GrassCollideStatic
{
private:
    /* Patches with transforms and additional info */
    std::vector< StaticPatchInfo > m_Instanses;
    UINT                           m_uInstanseStride;
    UINT                           m_uInstanseOffset;
    //UINT                           m_uNumCollidedPatchesPerMesh;
    GrassPatch                     m_GrassPatch;
    /* D3D variables */
    ID3D10Buffer                  *m_pInstansesBuffer;
    ID3D10InputLayout             *m_pInputLayout;
    ID3D10EffectPass              *m_pPass;
    ID3D10Device                  *m_pD3DDevice;

    //D3DXVECTOR3 GetInstPos     ( DWORD a_dwInstanseInd );    
    void GenInputLayout        ( );
    /* Loading transforms into slot 1, patch into slot 0 */
    void IASetVertexBuffers    ( );

public:
    GrassCollideStatic         ( GrassPatch &a_GrassPatch, ID3D10Effect *a_pEffect );
    ~GrassCollideStatic        ( );
    /* Taking patch for static collision with mesh a_iMeshInd */
    bool TakePatch             ( D3DXMATRIX &a_mTransform, UINT a_uMeshInd, DWORD a_dwIndex = -1 );
    void SetNumInstanses       ( DWORD a_dwNumInst );
    void GenTransformBuffer    ( );
    void UpdateTransformBuffer ( );
    /* Releasing patches out of Lod1 */
    //void Update         ( D3DXVECTOR3 a_vCamPos, float a_fLod0Dist, float a_fLod1Dist );
    void Render                ( );
};