#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "native_internal.h"

void vcstdio_init(void *arg)
{
    (void) arg;
}

ssize_t vcstdio_write(const void *buffer, size_t len)
{
    return real_write(STDOUT_FILENO, buffer, len);
}

ssize_t vcstdio_read(void *buffer, size_t max_len)
{
    return real_read(STDIN_FILENO, buffer, max_len);
}

int vcstdio_read_available(void)
{
    return 0;
}
