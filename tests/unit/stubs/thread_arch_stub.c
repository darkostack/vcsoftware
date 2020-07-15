#include "test-helper.h"

#include <mtos/thread.h>

char *mtThreadArchStackInit(mtThreadHandlerFunc aFunction, void *aArg, void *aStackStart, int aStackSize)
{
    (void)aFunction;
    (void)aArg;
    (void)aStackSize;
    return (char*)aStackStart;
}

void mtThreadArchStackPrint(void)
{

}

int mtThreadArchStackUsage(void)
{
    return 0;
}

void *mtThreadArchIsrStackPointer(void)
{
    return NULL;
}

void *mtThreadArchStackStart(void)
{
    return NULL;
}
