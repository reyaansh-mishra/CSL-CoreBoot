/* includes/payload-includes/payload.h */

#pragma once
#include <csl/mmu.h>
#include <csl/core.h>
#include <csl/mem_map.h>

struct PAYLOAD_BOOT_INFO {
    uint8_t     version;
    uintptr_t   ImageBase_phy;
    uintptr_t   ImageBase_virt;
    size_t      ImageSize;

    UEFI_MEMORY_MAP uefi_memory_map;

    char*       BootArgs;
} __attribute__((packed));
extern struct PAYLOAD_BOOT_INFO   boot_info;

extern uint8_t      remap_addrs_count;
extern uintptr_t    payload_virtual_entry;
extern uint64_t     payload_reloc_physically;
extern bool         drop_to_el1;

/* FUNCTIONS */
void payload_main(struct PAYLOAD_BOOT_INFO boot_struct);
