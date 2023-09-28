#include "../os.h"
#include <stdint.h>
#define MAX_REGIONS 20

int total = 0;
struct mem_region {
    uint64_t start;
    uint64_t used;
    uint64_t end;
    uint8_t present;
};

struct mem_region regions[MAX_REGIONS];
static int use = 0;

void manager_init(EFI_MEMORY_DESCRIPTOR* desc) {
    if (total + 1 == MAX_REGIONS) {
        print("Only 20 memory regions can be supported at once");
        BS->Exit(IM, EFI_SUCCESS, 0, NULL);
        return;
    }
    regions[total].start = desc->PhysicalStart;
    regions[total].end = desc->NumberOfPages * EFI_PAGE_SIZE;
    regions[total].used = 0;
    if (total == 0)
        regions[total].present = 1;
    regions[total].present = 0;
    total++;
}

void* malloc(uint64_t sz) {
    if ((regions[use].end == regions[use].used) || (regions[use].used + sz >= regions[use].end)) {
        if (use + 1 == total) {
            print("Out of memory");
            return NULL;
        }
        use++;
    }

    uint64_t offset = regions[use].used;
    regions[use].used += sz;
    return ((void*)(regions[use].start) + offset);
}

int cmp (const CHAR16 *s1, const CHAR16 *s2)
{
  wchar_t c1, c2;
  do
   {
      c1 = *s1++;
      c2 = *s2++;
      if (c1 == L'\0' || c2 == L'\0')
        break;
  } while (c1 == c2);
  return c1 < c2 ? -1 : 1;
}
