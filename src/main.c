/* src/main.c */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <csl/terminal.h>
#include <csl/utils.h>
#include <csl/aarch64.h>
#include <csl/system.h>
#include <csl/bootstrappr.h>
#include <csl/memory.h>
#include <csl/csl_lib.h>

#include <uefi_mallocs.h>

#include <Uefi/Uefi.h>
#include <Protocol/LoadedImage.h>

// Loaded Image Protocol GUID
EFI_GUID gEfiLoadedImageProtocolGuid = 
    { 0x5B1B31A1, 0x9562, 0x11D2, { 0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B } };

// Simple Text Output Protocol GUID
EFI_GUID gEfiSimpleTextOutProtocolGuid = 
    { 0xD35EE3B1, 0x5775, 0x11D1, { 0x9A, 0x60, 0x00, 0x80, 0xC7, 0x3C, 0x37, 0x19 } };

bool            pls_use_malloc_now;
uintptr_t       payload_virtual_entry;
uint64_t        payload_reloc_physically;


EFI_CONTEXT efi;
bool drop_to_el1 = false;

static EFI_STATUS EFIAPI csl_main(void)
{ /* Actually run CSL */

    payload_reloc_physically = round_down(payload_reloc_physically, PAGE_SIZE);

    INFO("CSL v%s,\n\tRevision Desc: \"%s\"\n", CSL_VERSION, CSL_VERSION_DESC);
    INFO("BASE = %p, SIZE = %d\n", efi.csl_base_phy, efi.csl_size);

    INFO("!!! STARTING CORE CSL APPLICATION !!!\n");
    UEFI_MEMORY_MAP mem_map = getMemMap();
    INFO("Boots\n");
    bootstrappr(mem_map);
    return EFI_SUCCESS;
};

/**
 * Entry point.
 * Use:     Get a minimal CSL runtime up before Payload sets its configs up.
 */

EFI_STATUS EFIAPI csl_bootstrap(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{  /* Setup Core CSL UEFI Runtime */
    mask_interrupts();

    if (get_current_el() != 2) {
        not_in_el2();
    };

    pls_use_malloc_now          = false;

    efi.ImageHandle             = ImageHandle;
    efi.SystemTable             = SystemTable;
    efi.BootServices            = efi.SystemTable->BootServices;
    payload_virtual_entry       = 0;
    payload_reloc_physically    = 0;

    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;

    EFI_STATUS status = efi.BootServices->OpenProtocol(efi.ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&LoadedImage, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(status)) { return status; };

    efi.csl_base_phy    = (uintptr_t)LoadedImage->ImageBase;
    efi.csl_size        = round_up(LoadedImage->ImageSize, PAGE_SIZE);

    set_uefi_malloc(uefi_alloc);
    set_free_malloc(uefi_free);

    // terminal_clear();
    // int err = payload_init();

    // if (err != EFI_SUCCESS) {
    //     ERR("CSL_BOOT_STUB: Unable to conitnue, Err: %lu\n", err);
    //     return err;
    // };

    csl_main();

    SYSTEM_HALT();
    return EFI_SUCCESS;
};

void exception_dump(uint64_t esr, uint64_t far, uint64_t elr, uint64_t spsr, uint64_t which /*, uint64_t* regs*/) {
    printf("\n\n");
    ERR("Unhandled Exception Caught!\n");
    printf("ESR: %p\n", esr);
    printf("FAR: %p\n", far);
    printf("ELR: %p\n", elr);
    printf("SPSR: %p\n", spsr);
    printf("WHICH: %lu\n", which);

    // Decode ESR for fast debugging
    uint32_t ec = (esr >> 26) & 0x3F;
    if (ec == 0x25 || ec == 0x24) { // Data aborts
        printf("Type: Data Abort on ");
        printf((esr & (1 << 6)) ? "WRITE\n" : "READ\n");
    };

    // Freeze here
    while (1) {
        __asm__ volatile("wfi");
    };
};
