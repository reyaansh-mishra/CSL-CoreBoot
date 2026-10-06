#include <csl/core.h>
#include <csl/mem_map.h>

#define PAYLOAD_BOOT_INFO_VERSION   1

void bootstrappr(UEFI_MEMORY_MAP mem_info);
[[noreturn]] void csl_continue_if_needed();

extern size_t number_of_pages;
