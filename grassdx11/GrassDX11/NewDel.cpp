#pragma once

/* Memory manager.
* Usage: include "memdebug.h" in all files, where you need to allocate memory. 
* Use new and delete operators only.
* Chukanov Vyacheslav, 02.08.2010
*/
#include "NewDel.h"
using namespace std;

struct NewCallData
{
    string sFileName;
    int    iLineNum;
    size_t Size;
    void  *Ptr;
};

static vector < NewCallData > NewCalls;
static ofstream oFS;

void *operator new (size_t BlockSize, const char *sFileName, int iLineNum)
{
    void *Ptr = malloc(BlockSize);
    NewCallData NDC;
    NDC.Ptr = Ptr;
    NDC.sFileName = string(sFileName);
    NDC.iLineNum  = iLineNum;
    NDC.Size      = BlockSize;
    NewCalls.push_back(NDC);
    return Ptr;
}

void *operator new [] (size_t BlockSize, const char *sFileName, int iLineNum)
{
    void *Ptr = malloc(BlockSize);
    NewCallData NDC;
    NDC.Ptr = Ptr;
    NDC.sFileName = string(sFileName);
    NDC.iLineNum  = iLineNum;
    NDC.Size      = BlockSize;
    NewCalls.push_back(NDC);
    return Ptr;
}

void operator delete (void *p)
{
    vector< NewCallData >::iterator it = NewCalls.begin();
    while (it != NewCalls.end())
    {
        if ((*it).Ptr == p)
        {
            NewCalls.erase(it);
            break;
        }
        ++it;
    }
    free(p);
}

void operator delete [] (void *p)
{
    vector< NewCallData >::iterator it = NewCalls.begin();
    while (it != NewCalls.end())
    {
        if ((*it).Ptr == p)
        {
            NewCalls.erase(it);
            break;
        }
        ++it;
    }
    free(p);
}

void operator delete (void *p, const char *, int)
{
    vector< NewCallData >::iterator it = NewCalls.begin();
    while (it != NewCalls.end())
    {
        if ((*it).Ptr == p)
        {
            NewCalls.erase(it);
            break;
        }
        ++it;
    }
    free(p);
}

void operator delete [] (void *p, const char *, int)
{
    vector< NewCallData >::iterator it = NewCalls.begin();
    while (it != NewCalls.end())
    {
        if ((*it).Ptr == p)
        {
            NewCalls.erase(it);
            break;
        }
        ++it;
    }
    free(p);
}

void PrintMemoryLeaks( wstring a_sFileName )
{
    oFS.open(a_sFileName.c_str());
    for (unsigned i = 0; i < NewCalls.size(); i++)
    {
        oFS << "Memory Leak Detected!" << endl; 
        oFS << "    " << NewCalls[i].sFileName << endl;
        oFS << "    at line " << NewCalls[i].iLineNum << endl;
        oFS << "    with block size " << NewCalls[i].Size << endl;
    }
    oFS.close();
}