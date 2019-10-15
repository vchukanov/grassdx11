#pragma once

#include "GrassPatch.h"
#include "PhysPatch.h"
#include "Mesh/Mesh.h"

struct GrassPatchExt
{
	bool inFirstLod;

    PhysPatch  Patch;
    D3DXMATRIX Transform;
    float      fLifeTime;
    bool       bIsDead;
    UINT       uMeshIndex;
	bool       isVisible;
    GrassPatchExt *pNext;
    GrassPatchExt *pPrev;
    GrassPatchExt ( GrassPatch *a_pPatch );
};

/* 20.07 
 * New members:
 * const PhysPatch& GetPatch(int)
 * bool  TakePatchFromOtherLod ( const PhysPatch &, bool);
 * UINT  GetMeshIndex   ( int a_iPatchIndex );
 */

/** 
* A Pool class
* Class for using physics sub-system
*/
class GrassPool
{
private:    
    /* Patches with transforms and additional info */
    GrassPatchExt             **m_pPatches;
    int                         m_iPatchCount;
    /* free-space list */
    GrassPatchExt              *m_pFirst;
    GrassPatchExt              *m_pLast;
    /* D3D variables */
    ID3D10InputLayout          *m_pPhysInputLayout;
    ID3D10InputLayout          *m_pAnimInputLayout;
    ID3D10EffectPass           *m_pRenderAnimPass;
    ID3D10EffectPass           *m_pRenderPhysPass;
    ID3D10EffectPass           *m_pLowGrassAnimPass;
    ID3D10EffectPass           *m_pLowGrassPhysPass;
    ID3D10EffectPass           *m_pShadowPass;
    ID3D10Device               *m_pD3DDevice;
    bool                        m_bUseLowGrass;

public:
    /** 
     * Pool c-tor
     * @param a_pD3DDevice is a direct3d 10 device pointer
     * @param a_pEffect is a direct3d effect pointer
     * @param a_pBasePatch is a grass patch for physics sub-system
     * @param a_iPatchCount is a number of patches in pool
     */
    GrassPool                 ( ID3D10Device *a_pD3DDevice, ID3D10InputLayout *a_pAnimInputLayout, ID3D10Effect *a_pEffect, 
        GrassPatch *a_pBasePatch, int a_iPatchCount, bool a_bUseLowGrass );

    /** 
    * Pool d-tor
    * Releases all resources
    */
    ~GrassPool                ( );

    /** 
    * @return the patch count (pool size)
    */
    int         GetPatchCount ( );

    /** 
    * Finds patch in pool by it's position
    * @param a_vPatchPos is the position
    * @return index of patch in the pool or -1 if it wasn't found
    */
    int         GetPatchIndex ( D3DXVECTOR3 a_vPatchPos );

    /** 
    * Finds patch in pool by it's index
    * @param a_iPatchIndex is the index
    * @return reference to PhysPatch
    */
    const PhysPatch& GetPatch ( int a_iPatchIndex );

	void SetPatchVisibility ( int a_iPatchIndex, bool isVisible );

    /** 
    * Assuming conversion from one lod to another
    * @param a_iPatchIndex is the index
    * @return reference to PhysPatch
    */
    bool TakePatchFromOtherLod ( const PhysPatch &a_SrcPatch, bool a_bLod0ToLod1, D3DXMATRIX a_mTransform, float a_fLifeTime, UINT a_uMeshIndex );

    /** 
    * Finds patch and delete it by position
    * @param a_vPatchPos is the position
    * @return true if patch was found and deleted, otherwise false
    */
    bool        FreePatch     ( D3DXVECTOR3 a_vPatchPos );

    /** 
    * Finds patch and delete it by index
    * @param a_iPatchIndex
    * @return true if patch was found and deleted, otherwise false
    */
    bool        FreePatch     ( int a_iPatchIndex );

    /** 
    * Taking patch from pool
    * @param a_mTransform is the position and orientation matrix
    * @param a_fLifeTime is the life time of the patch, when it'll "die", it'll be marked as "free"
    * @param a_uMeshIndex is the index of the mesh, which collided the patch
    * @return true if "free" patch was found, otherwise false
    */
    bool		TakePatch     ( D3DXMATRIX a_mTransform, float a_fLifeTime, UINT a_uMeshIndex );
	bool        TakePatch     ( D3DXMATRIX a_mTransform, bool a_bInFirstLod );

    /** 
    * Finds patch and returns it current lifetime by index
    * @param a_iPatchIndex
    * @return patch lifetime
    */
    float       GetLifeTime   ( int a_iPatchIndex );

    /** 
    * Finds patch and returns it current mesh index by index
    * @param a_iPatchIndex
    * @return Mesh Index
    */
    UINT       GetMeshIndex   ( int a_iPatchIndex );

    /** 
    * Finds patch and returns it current position by index
    * @param a_iPatchIndex
    * @return patch position
    */
    D3DXVECTOR3 GetPatchPos   ( int a_iPatchIndex );


    void    ClearDeadPatches( float a_fElapsedTime );
    /** 
    * Update pool function
    * @param a_fElapsedTime is elapsed time (in seconds) since last update call
    */
    void        Update        ( const float3 &viewPos, float physLodDst, float a_fElapsedTime, Mesh *a_pMeshes[], UINT a_uNumMeshes, const std::vector<GrassPropsUnified> &grassProps, const IndexMapData &indexMapData);

    /** 
    * Rendering patches using physics sub-system
    */
    void        Render        ( bool a_bShadowPass );
};