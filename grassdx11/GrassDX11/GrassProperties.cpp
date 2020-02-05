#include "GrassProperties.h"
#include "PhysMath.h"

#include <fstream>

void ReadToCoord (XMVECTOR& v, DWORD coord, std::ifstream& f)
{
   float val;
   f >> val;
   setcoord(v, coord, val);
}

void GrassProps1::Read (std::ifstream& a_ifIn)
{
   ReadToCoord(vHardnessSegment, 0, a_ifIn);
   ReadToCoord(vHardnessSegment, 1, a_ifIn);
   ReadToCoord(vHardnessSegment, 2, a_ifIn);
   ReadToCoord(vMassSegment, 0, a_ifIn);
   ReadToCoord(vMassSegment, 1, a_ifIn);
   ReadToCoord(vMassSegment, 2, a_ifIn);
   ReadToCoord(vSizes, 0, a_ifIn);
   ReadToCoord(vSizes, 1, a_ifIn);
   ReadToCoord(vColor, 0, a_ifIn);
   ReadToCoord(vColor, 1, a_ifIn);
   ReadToCoord(vColor, 2, a_ifIn);
   ReadToCoord(vColor, 3, a_ifIn);
   vColor = create(1.0, 1.0, 1.0, 1.0);
   a_ifIn >> uTexIndex;
}


void GrassProps3::Read (std::ifstream& a_ifIn)
{
   ReadToCoord(vHardnessSegment, 0, a_ifIn);
   ReadToCoord(vHardnessSegment, 1, a_ifIn);
   ReadToCoord(vHardnessSegment, 2, a_ifIn);
   ReadToCoord(vMassSegment, 0, a_ifIn);
   ReadToCoord(vMassSegment, 1, a_ifIn);
   ReadToCoord(vMassSegment, 2, a_ifIn);
   ReadToCoord(vSizes, 0, a_ifIn);
   ReadToCoord(vSizes, 1, a_ifIn);
   a_ifIn >> uTexIndex;
   a_ifIn >> uTopTexIndex;
}