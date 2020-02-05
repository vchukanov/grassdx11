#include "DXUT.h"
#include "DXUTcamera.h"
#include "Terrain.h"
#include "Mesh.h"

class LandscapeCamera : public CFirstPersonCamera
{
public:
   LandscapeCamera (float a_fHeight, Terrain* const a_pTerrain, float a_fHeightScale, float a_fGrassRadius)
      :
      m_fDefaultHeight(a_fHeight), m_pTerrain(a_pTerrain),
      m_fHeightScale(a_fHeightScale), m_fGrassRadius(a_fGrassRadius)
   {}

   virtual void FrameMove (FLOAT fElapsedTime);

private:
   Terrain* const m_pTerrain;
   float m_fHeightScale;
   float m_fGrassRadius;

   float m_fDefaultHeight;
};

