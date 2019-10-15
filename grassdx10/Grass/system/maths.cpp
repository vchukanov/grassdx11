#include "maths.h"

maths::Line::Line(D3DXVECTOR3 &a_Dir, D3DXVECTOR3 &a_Point)
{
    m_Dir = a_Dir;
    m_Point = a_Point;
}

maths::Line::Line()
{
    m_Dir.z = 1.0;
}

D3DXVECTOR3& maths::Line::Dir()
{
    return m_Dir;
}

D3DXVECTOR3& maths::Line::Point()
{
    return m_Point;
}

maths::Plane::Plane()
{

}

maths::Plane::Plane(D3DXVECTOR3 a_Norm, float a_D)
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

void maths::Plane::SetN(D3DXVECTOR3 a_Norm)
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

D3DXVECTOR3 maths::Plane::GetN()
{
    return D3DXVECTOR3(m_NandD.x, m_NandD.y, m_NandD.z);
}

maths::AABox::AABox()
{

}

bool maths::AABox::Collide(AABox &a_Collider)
{
    const int VEC_SIZE = 3;
    int i;
    for (i = 0; i < VEC_SIZE; ++i)
    {
        m_Min[i] = com::maximum(m_Min[i], a_Collider.m_Min[i]);
        m_Max[i] = com::minimum(m_Max[i], a_Collider.m_Max[i]);
        if (m_Min[i] > m_Max[i])
            return false;
    }
    return true;
}

maths::AABox::AABox(D3DXVECTOR3 a_Min, D3DXVECTOR3 a_Max)
{
    m_Max = a_Max;
    m_Min = a_Min;
}

D3DXVECTOR3& maths::AABox::Min()
{
    return m_Min;
}

D3DXVECTOR3& maths::AABox::Max()
{
    return m_Max;
}

void copyVector3Values(D3DXVECTOR3 *a, float x, float y, float z)
{
    a->x = x;
    a->y = y;
    a->z = z;
}

void maths::AABox::GetPoints(PointArray *a_Res)
{
    copyVector3Values(&(*a_Res)[0],m_Min[0],m_Min[1],m_Min[2]);//     7+------+6
    copyVector3Values(&(*a_Res)[1],m_Max[0],m_Min[1],m_Min[2]);//     /|     /|
    copyVector3Values(&(*a_Res)[2],m_Max[0],m_Max[1],m_Min[2]);//    / |    / |
    copyVector3Values(&(*a_Res)[3],m_Min[0],m_Max[1],m_Min[2]);//   / 4+---/--+5
    copyVector3Values(&(*a_Res)[4],m_Min[0],m_Min[1],m_Max[2]);// 3+------+2 /    y   z
    copyVector3Values(&(*a_Res)[5],m_Max[0],m_Min[1],m_Max[2]);//  | /    | /     |  /
    copyVector3Values(&(*a_Res)[6],m_Max[0],m_Max[1],m_Max[2]);//  |/     |/      |/
    copyVector3Values(&(*a_Res)[7],m_Min[0],m_Max[1],m_Max[2]);// 0+------+1      *---x
}

bool maths::AABox::LastLineISect(D3DXVECTOR3 *a_Res, Line a_Line)
{
    const float l_fEps = 0.01f;
    float l_fCoef;
    D3DXVECTOR3 l_vISectPoint;

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

        l_vISectPoint = a_Line.Point() + l_fCoef * a_Line.Dir();
        if ((l_vISectPoint.y >= m_Min.y) && (l_vISectPoint.y <= m_Max.y) && (l_vISectPoint.z >= m_Min.z) && (l_vISectPoint.z <= m_Max.z))
        {
            *a_Res = l_vISectPoint;
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

        l_vISectPoint = a_Line.Point() + l_fCoef * a_Line.Dir();
        if ((l_vISectPoint.x >= m_Min.x) && (l_vISectPoint.x <= m_Max.x) && (l_vISectPoint.z >= m_Min.z) && (l_vISectPoint.z <= m_Max.z))
        {
            *a_Res = l_vISectPoint;
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

        l_vISectPoint = a_Line.Point() + l_fCoef * a_Line.Dir();
        if ((l_vISectPoint.x >= m_Min.x) && (l_vISectPoint.x <= m_Max.x) && (l_vISectPoint.y >= m_Min.y) && (l_vISectPoint.y <= m_Max.y))
        {
            *a_Res = l_vISectPoint;
            return true;
        }         
    }
    return false;
}

void maths::PointArray::Transform(D3DXMATRIX &a_Mtx)
{
    int i;
    D3DXVECTOR4 vOut;
    for (i = 0; i < m_ArraySize; ++i)
    {
        (m_Array[i]) = maths::operator * ((m_Array[i]),  a_Mtx);
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
        for (j = 0; j < VEC_SIZE; ++j)
        {
            if (m_Array[i][j] < a_Res->Min()[j])
            {
                a_Res->Min()[j] = m_Array[i][j];
            }
            else if (m_Array[i][j] > a_Res->Max()[j])
            {
                a_Res->Max()[j] = m_Array[i][j];
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