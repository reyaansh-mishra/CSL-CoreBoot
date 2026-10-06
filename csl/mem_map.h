/* includes/csl/mem_map.h */

#pragma once
#include <csl/core.h>
#include <stdbool.h>

struct MemMapprInfo {
    EFI_MEMORY_DESCRIPTOR*  memory_map;
    UINTN                   memory_map_size;
    UINTN                   map_key;
    UINTN                   descriptor_size;
    UINT32                  descriptor_version;

    bool active;
};
typedef struct MemMapprInfo UEFI_MEMORY_MAP;
