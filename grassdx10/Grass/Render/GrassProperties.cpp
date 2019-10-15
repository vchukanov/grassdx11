#include "GrassProperties.h"

void GrassProps1::Read( ifstream &a_ifIn )
{
    a_ifIn >> vHardnessSegment.x; 
    a_ifIn >> vHardnessSegment.y;
    a_ifIn >> vHardnessSegment.z;
    a_ifIn >> vMassSegment.x; 
    a_ifIn >> vMassSegment.y;
    a_ifIn >> vMassSegment.z;
    a_ifIn >> vSizes.x;
    a_ifIn >> vSizes.y;
    a_ifIn >> vColor.x; 
    a_ifIn >> vColor.y;
    a_ifIn >> vColor.z;
    a_ifIn >> vColor.w;
    vColor = D3DXVECTOR4(1.0, 1.0, 1.0, 1.0);
    a_ifIn >> uTexIndex;
}

void GrassProps3::Read( ifstream &a_ifIn )
{
    a_ifIn >> vHardnessSegment.x; 
    a_ifIn >> vHardnessSegment.y;
    a_ifIn >> vHardnessSegment.z;
    a_ifIn >> vMassSegment.x; 
    a_ifIn >> vMassSegment.y;
    a_ifIn >> vMassSegment.z;
    a_ifIn >> vSizes.x;
    a_ifIn >> vSizes.y;
    a_ifIn >> uTexIndex;
    a_ifIn >> uTopTexIndex;
}