#pragma once
/* Memory manager.
* Usage: include "memdebug.h" in all files, where you need to allocate memory. 
* Use new and delete operators only.
* Chukanov Vyacheslav, 02.08.2010
*/
#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <new>

void *operator new (size_t BlockSize, const char *sFileName, int iLineNum);

void *operator new [] (size_t BlockSize, const char *sFileName, int iLineNum);

void operator delete (void *p);

void operator delete [] (void *p);

void operator delete (void *p, const char *, int);

void operator delete [] (void *p, const char *, int);

void PrintMemoryLeaks( std::wstring a_sFileName );
