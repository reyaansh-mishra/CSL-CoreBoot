/* src/bootstrappr.c */

#include <csl/utils.h>
#include <csl/mmu.h>
#include <csl/aarch64.h>
#include <csl/stack.h>
#include <csl/bootstrappr.h>
#include <csl/terminal.h>
#include <csl/stack.h>
#include <csl/system.h>

#include <vectors.h>

#include <payload.h>

#undef INFO
#undef ERR
#define INFO(fmt, ...)  printf("[CSL] <bootstrappr>: " fmt, ##__VA_ARGS__)
#define ERR(fmt, ...)   printf("[ERR] [CSL] <bootstrappr>: " fmt, ##__VA_ARGS__)

void setup_tables();
struct PAYLOAD_BOOT_INFO boot_info;

static void setup_bootinfo() {
    boot_info.ImageBase_phy     = efi.csl_base_phy;
    boot_info.ImageBase_virt    = efi.csl_base_virt;
    boot_info.ImageSize         = efi.csl_size;    
};

/*
 * Start bootstrapping Payload's requirements now that CSL is alive
 */

void bootstrappr(UEFI_MEMORY_MAP mem_info) {   /* Bootstrappr is used to bootstrap the PAYLOAD, not CSL. */
    size_t      itr __attribute__((unused));
    itr = 0;
    uint8_t*    entry       = (uint8_t*)mem_info.memory_map;
    uint8_t*    end         = entry + mem_info.memory_map_size; // memory_map_size should be total bytes here

    while (entry < end) {
        entry += mem_info.descriptor_size;
        itr++;
    }

    // bool debug_waiting = 1;
    // INFO("Text At: %p\n", efi.csl_base_phy + 0x1000);
    // INFO("Waiting for Debugger to set debug_waiting == 0....\n");
    // while (debug_waiting) {
    //     __asm__ volatile("yield");
    // }

    boot_info.version           = PAYLOAD_BOOT_INFO_VERSION;
    boot_info.uefi_memory_map   = mem_info;

    if (!payload_reloc_physically)  {
        payload_reloc_physically = efi.csl_base_phy;
        INFO("Payload Phy Reloc was ZERO\n");
    }
    if (!payload_virtual_entry) {
        payload_virtual_entry = efi.csl_base_phy;
        INFO("Payload Virt Reloc was ZERO\n");
    }

    INFO("SETUP STACK && START MMU WORK\n");
    // add_virtual_mapping(    // Ident Map the memory map itself, because we need it
    //     (uintptr_t)mem_info.memory_map,
    //     (uintptr_t)mem_info.memory_map,
    //     mem_info.memory_map_size,
    //     READ_ONLY
    // );
    install_exception_vectors((uintptr_t)&vector_table);
    enable_stack((uintptr_t)setup_mmu);
    
    INFO("MMU ON!\n");
    enable_mmu();
    csl_continue_if_needed();
};

[[noreturn]] void csl_continue_if_needed()
{
    // install_exception_vectors(); // Reinstall because VBARS have changed
    INFO("Setting Up boot_info...\n");
    setup_bootinfo();

    payload_main(boot_info);

    ERR("PAYLOAD RETURNED! EXITING!\n");
    SYSTEM_POWEROFF();
    __builtin_unreachable();
};
