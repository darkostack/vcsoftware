#include <stdio.h>

#include <vcrtos/instance.h>
#include <vcrtos/thread.h>

int main(void)
{
    void *instance = instance_get();

    while (1)
    {
        thread_sleep(instance);
    }

    return 0;
}