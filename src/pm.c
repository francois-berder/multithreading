#include "pm.h"

void __attribute__((weak)) pm_enter(void)
{
    __asm__ volatile ("wfi" ::: "memory");
}
