MEMORY
{
    flash       :   ORIGIN = 0x08000000, LENGTH = 512K
    sram        :   ORIGIN = 0x20000000, LENGTH = 256K
}

/* Include main link script. Note: it will be searched in -L paths. */
INCLUDE cortex.ld
