#include <stdint.h>
#include <stdio.h>

#include <vcrtos/instance.h>
#include <vcrtos/mutex.h>

mutex_t main_mutex;

int main(void)
{
    printf("main function entry\r\n");

    instance_t *instance = instance_get();

    mutex_init(instance, &main_mutex);

    while(1)
    {
        mutex_lock(&main_mutex);
    }

    return 0;
}
