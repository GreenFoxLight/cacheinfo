/* Prints out cache information obtained by CPUID (EAX = 0x04)
 *
 * Compile using:
 *  gcc -o cacheinfo cacheinfo.c
 * 
 * Tested on macOS using clang-900.0.39.2 
 */

#include <stdio.h>

static inline void
cpuid(unsigned int ax, unsigned int cx, unsigned int out[4]) {
    __asm__ volatile ( 
            "cpuid"
            : "=a" (out[0]), "=b" (out[1]), "=c" (out[2]), "=d" (out[3])
            : "a" (ax), "c" (cx)
            );
}

#define GET_CACHE_INFO 0x04
#define WRITEBACKINVALIDATE 0x01
#define INCLUSIVENESS 0x02
#define COMPLEXMAPPING 0x04

int
query_cache_info(unsigned int level) {
    unsigned int regs[4];
    cpuid(GET_CACHE_INFO, level, regs);

    unsigned int type = regs[0] & 0x3;
    const char *typestr = "";
    if (type == 0) return 0; /* no more caches */
    if (type == 1)
        typestr = "Data";
    if (type == 2)
        typestr = "Instruction";
    if (type == 3)
        typestr = "Unified";
    unsigned int ebx = regs[1];
    unsigned int ways = ((ebx >> 22) & 0x3ff) + 1;
    unsigned int partitions = ((ebx >> 12) & 0x3ff) + 1;
    unsigned int line_size = (ebx & 0x3ff) + 1;
    unsigned int sets = regs[2] + 1;
    unsigned int size = ways * partitions * line_size * sets; 
    unsigned int features = regs[3];
    printf("Cache %u - %s\n\t", level, typestr);
    printf("Ways: %u\n\t", ways);
    printf("Partitions: %u\n\t", partitions);
    printf("Line Size: %u\n\t", line_size);
    printf("Sets: %u\n\t", sets);
    printf("Size: %u", size);
    if (size > (1024 * 1024))
        printf(" (%u MiB)\n", size / (1024 * 1024));
    else if (size > 1024)
        printf(" (%u KiB)\n", size / 1024);
    else 
        printf("\n");
    if (features & WRITEBACKINVALIDATE)
        printf("WBINVD/INVD is not guaranteed to affect lower level caches of non-originating threads sharing this cache.\n");
    else
        printf("WBINVD/INVD from threads sharing this cache acts upon lower level caches, affecting non-originating threads sharing this cache.\n");
    if (features & INCLUSIVENESS)
        printf("This cache is inclusive of lower cache levels.\n");
    else
        printf("This cache is not inclusive of lower cache levels.\n");
    if (features & COMPLEXMAPPING)
        printf("This is not a direct mapped cache.\n");
    else
        printf("This is a direct mapped cache.\n");
    return 1;
}

int
main() {
    unsigned int level;
    for (level = 0; query_cache_info(level); ++level) ;

    return 0;
}
