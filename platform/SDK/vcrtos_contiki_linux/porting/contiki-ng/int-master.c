#include "contiki.h"
#include "sys/int-master.h"

#include <vcdrivers/cpu.h>
#include <vcrtos/cpu.h>

void int_master_enable(void)
{
    cpu_irq_enable();
}

int_master_status_t int_master_read_and_disable(void)
{
    return (int_master_status_t)cpu_irq_disable();
}

void int_master_status_set(int_master_status_t status)
{
    cpu_irq_restore((unsigned)status);
}

bool int_master_is_enable(void)
{
    return __get_PRIMASK() ? 1 : 0;
}
