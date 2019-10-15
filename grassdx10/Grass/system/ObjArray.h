#pragma once

template <class Obj, int MaxSize> 
class ObjArray
{
protected:
    Obj   *m_Array;
    int m_ArraySize;
    int m_MaxSize;

public:
    ObjArray          (const ObjArray<Obj, MaxSize>& a_Src)
    {
        int i;
        m_MaxSize = MaxSize;
        m_ArraySize = a_Src.m_ArraySize;
        m_Array = new Obj[MaxSize];
        for (i = 0; i < m_ArraySize; ++i)
        {
            m_Array[i] = a_Src.m_Array[i];
        }
    }
    ObjArray          ()
    {
        m_MaxSize = MaxSize;
        m_Array = new Obj[MaxSize];
        m_ArraySize = 0;
    }
    virtual ~ObjArray         ()
    {
        if (m_Array != 0)
        {
            delete [] m_Array;
        }
    }

    void  SetSize    (int a_ArraySize)
    { 
        m_ArraySize = a_ArraySize;
    }

    int GetSize    ()
    {
        return m_ArraySize;
    }

    void  Empty      ()
    {
        m_ArraySize = 0;
    }

    bool AppendObj   (Obj *a_Obj)
    {
        if (m_ArraySize == MaxSize)
            return false;
        m_Array[m_ArraySize] = *a_Obj;
        m_ArraySize++;
        return true;
    }

    Obj&   operator [](int a_Ind)
    {
        if (a_Ind > m_ArraySize)
            a_Ind = m_ArraySize - 1;

        return m_Array[a_Ind];
    }

    ObjArray<Obj, MaxSize>& operator = (ObjArray<Obj, MaxSize> &a_ObjArray)
    {
        SetSize(a_ObjArray.m_ArraySize);
        int i;
        for (i = 0; i < m_ArraySize; ++i)
        {
            m_Array[i] = a_ObjArray.m_Array[i];
        }
        return *this;
    }
};