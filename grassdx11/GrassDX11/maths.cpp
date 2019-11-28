#include "maths.h"

maths::Line::Line (XMFLOAT3 &a_Dir, XMFLOAT3 &a_Point)
{
    m_Dir = a_Dir;
    m_Point = a_Point;
}

maths::Line::Line()
{
    m_Dir.z = 1.0;
}

XMFLOAT3& maths::Line::Dir()
{
    return m_Dir;
}

XMFLOAT3& maths::Line::Point()
{
    return m_Point;
}

maths::Plane::Plane()
{

}

maths::Plane::Plane(XMFLOAT3 a_Norm, float a_D)
{
    m_NandD.x = a_Norm.x;
    m_NandD.y = a_Norm.y;
    m_NandD.z = a_Norm.z;
    m_NandD.w = a_D;
}

void maths::Plane::SetD(float a_D)
{
    m_NandD.w = a_D;
}

float maths::Plane::GetD()
{
    return m_NandD.w;
}

void maths::Plane::SetN(XMFLOAT3 a_Norm)
{
    m_NandD.x = a_Norm.x;
    m_NandD.y = a_Norm.y;
    m_NandD.z = a_Norm.z;
}

void maths::Plane::SetN(float x, float y, float z)
{
    m_NandD.x = x;
    m_NandD.y = y;
    m_NandD.z = z;
}

XMFLOAT3 maths::Plane::GetN()
{
    return XMFLOAT3(m_NandD.x, m_NandD.y, m_NandD.z);
}

maths::AABox::AABox()
{

}

bool maths::AABox::Collide(AABox &a_Collider)
{
    {
        m_Min.x = com::maximum(m_Min.x, a_Collider.m_Min.x);
        m_Max.x = com::minimum(m_Max.x, a_Collider.m_Max.x);
        if (m_Min.x > m_Max.x)
            return false;
    }
	{
		m_Min.y = com::maximum(m_Min.y, a_Collider.m_Min.y);
		m_Max.y = com::minimum(m_Max.y, a_Collider.m_Max.y);
		if (m_Min.y > m_Max.y)
			return false;
	}
	{
		m_Min.z = com::maximum(m_Min.z, a_Collider.m_Min.z);
		m_Max.z = com::minimum(m_Max.z, a_Collider.m_Max.z);
		if (m_Min.z > m_Max.z)
			return false;
	}
    return true;
}

maths::AABox::AABox(XMFLOAT3 a_Min, XMFLOAT3 a_Max)
{
    m_Max = a_Max;
    m_Min = a_Min;
}

XMFLOAT3& maths::AABox::Min()
{
    return m_Min;
}

XMFLOAT3& maths::AABox::Max()
{
    return m_Max;
}

void copyVector3Values(XMFLOAT3 *a, float x, float y, float z)
{
    a->x = x;
    a->y = y;
    a->z = z;
}

void maths::AABox::GetPoints(PointArray *a_Res)
{
    copyVector3Values(&(*a_Res)[0],m_Min.x,m_Min.y,m_Min.z);//     7+------+6
    copyVector3Values(&(*a_Res)[1],m_Max.x,m_Min.y,m_Min.z);//     /|     /|
    copyVector3Values(&(*a_Res)[2],m_Max.x,m_Max.y,m_Min.z);//    / |    / |
    copyVector3Values(&(*a_Res)[3],m_Min.x,m_Max.y,m_Min.z);//   / 4+---/--+5
    copyVector3Values(&(*a_Res)[4],m_Min.x,m_Min.y,m_Max.z);// 3+------+2 /    y   z
    copyVector3Values(&(*a_Res)[5],m_Max.x,m_Min.y,m_Max.z);//  | /    | /     |  /
    copyVector3Values(&(*a_Res)[6],m_Max.x,m_Max.y,m_Max.z);//  |/     |/      |/
    copyVector3Values(&(*a_Res)[7],m_Min.x,m_Max.y,m_Max.z);// 0+------+1      *---x
}

bool maths::AABox::LastLineISect(XMFLOAT3 *a_Res, Line a_Line)
{
    const float l_fEps = 0.01f;
    float l_fCoef;
    XMVECTOR l_vISectPoint;

    if (!com::equal(a_Line.Dir().x, 0.0f, l_fEps))
    {
        if (a_Line.Dir().x > 0.0f)
        {
            l_fCoef = (m_Max.x - a_Line.Point().x) / a_Line.Dir().x;
        }
        else
        {
            l_fCoef = (m_Min.x - a_Line.Point().x) / a_Line.Dir().x;
        }
        // Point a_Line.Point + l_fCoef * a_Line.Dir lies on the BBox plane
		XM_TO_V(a_Line.Point(), p, 3);
		XM_TO_V(a_Line.Dir(), dir, 3);

        l_vISectPoint = p + XMVectorScale(dir, l_fCoef);

		V_TO_XM(l_vISectPoint, isecP, 3);

        if ((isecP.y >= m_Min.y) && (isecP.y <= m_Max.y) && (isecP.z >= m_Min.z) && (isecP.z <= m_Max.z))
        {
            *a_Res = isecP;
            return true;
        }         
    }

    if (!com::equal(a_Line.Dir().y, 0.0f, l_fEps))
    {
        if (a_Line.Dir().y > 0.0f)
        {
            l_fCoef = (m_Max.y - a_Line.Point().y) / a_Line.Dir().y;
        }
        else
        {
            l_fCoef = (m_Min.y - a_Line.Point().y) / a_Line.Dir().y;
        }
        // Point a_Line.Point + l_fCoef * a_Line.Dir lies on the BBox plane
		XM_TO_V(a_Line.Point(), p, 3);
		XM_TO_V(a_Line.Dir(), dir, 3);

		l_vISectPoint = p + XMVectorScale(dir, l_fCoef);

		V_TO_XM(l_vISectPoint, isecP, 3);
        if ((isecP.x >= m_Min.x) && (isecP.x <= m_Max.x) && (isecP.z >= m_Min.z) && (isecP.z <= m_Max.z))
        {
            *a_Res = isecP;
            return true;
        }         
    }

    if (!com::equal(a_Line.Dir().z, 0.0f, l_fEps))
    {
        if (a_Line.Dir().z > 0.0f)
        {
            l_fCoef = (m_Max.z - a_Line.Point().z) / a_Line.Dir().z;
        }
        else
        {
            l_fCoef = (m_Min.z - a_Line.Point().z) / a_Line.Dir().z;
        }
        // Point a_Line.Point + l_fCoef * a_Line.Dir lies on the BBox plane
		XM_TO_V(a_Line.Point(), p, 3);
		XM_TO_V(a_Line.Dir(), dir, 3);

		l_vISectPoint = p + XMVectorScale(dir, l_fCoef);

		V_TO_XM(l_vISectPoint, isecP, 3);
        if ((isecP.x >= m_Min.x) && (isecP.x <= m_Max.x) && (isecP.y >= m_Min.y) && (isecP.y <= m_Max.y))
        {
            *a_Res = isecP;
            return true;
        }         
    }
    return false;
}

void maths::PointArray::Transform (XMMATRIX &a_Mtx)
{
	M_TO_XM(a_Mtx, m);

    int i;
    XMFLOAT4 vOut;
    for (i = 0; i < m_ArraySize; ++i)
    {
        (m_Array[i]) = maths::operator * ((m_Array[i]),  m);
        //D3DXVec3Transform(&vOut, &m_Array[i], &a_Mtx);
        /*m_Array[i].x = vOut.x;
        m_Array[i].y = vOut.y;
        m_Array[i].z = vOut.z;*/
    }
}

void maths::PointArray::CalcAABBox(AABox *a_Res)
{
    const int VEC_SIZE = 3;
    int i, j;
    a_Res->Min() =  m_Array[0];
    a_Res->Max() =  m_Array[0];

    for (i = 0; i < m_ArraySize; ++i)
    {
		 {
		    if (m_Array[i].x < a_Res->Min().x)
		    {
		        a_Res->Min().x = m_Array[i].x;
		    }
		    else if (m_Array[i].x > a_Res->Max().x)
		    {
		        a_Res->Max().x = m_Array[i].x;
		    }
		 }
		 {
			 if (m_Array[i].y < a_Res->Min().y)
			 {
				 a_Res->Min().y = m_Array[i].y;
			 }
			 else if (m_Array[i].y > a_Res->Max().y)
			 {
				 a_Res->Max().y = m_Array[i].y;
			 }
		 }
		 {
			 if (m_Array[i].z < a_Res->Min().z)
			 {
				 a_Res->Min().z = m_Array[i].z;
			 }
			 else if (m_Array[i].z > a_Res->Max().z)
			 {
				 a_Res->Max().z = m_Array[i].z;
			 }
		 }
    }
}

void maths::PolyPointArray::ToPointArray(maths::PointArray *a_Res)
{
    int j, i;
    a_Res->Empty();
    for (i = 0; i < m_ArraySize; ++i)
    {
        
        for (j = 0; j < m_Array[i].GetSize(); ++j)
        {
            a_Res->AppendObj(& (m_Array[i][j]));
        }
    }    
}