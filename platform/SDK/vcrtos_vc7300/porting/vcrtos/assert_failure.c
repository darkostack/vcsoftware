#include <stdio.h>
#include <vcrtos/assert.h>

void vcassert_failure(const char *file, unsigned line)
{
    printf("%s:%u => ASSERT FAILED\r\n", file, line);
    while (1);
}
