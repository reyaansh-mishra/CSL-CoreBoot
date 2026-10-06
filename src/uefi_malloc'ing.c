/* src/uefi_malloc'ing.c */

#include <stddef.h>
#include <Uefi.h>
#include <csl/core.h>
#include <csl/utils.h>
#include <csl/terminal.h>
#include <csl/system.h>
#include <memory.h>

void* uefi_alloc(size_t size) {
    EFI_STATUS status;
    EFI_PHYSICAL_ADDRESS address = 0;

    status = efi.SystemTable->BootServices->AllocatePages(
        AllocateAnyPages,
        EfiLoaderData,
        size/PAGE_SIZE,
        &address
    );

    if (EFI_ERROR(status)) {
        ERR("PAGE ALLOCATION FAILED. ERR: %p\n", status);
        return NULL;
    }
    memset((void *)address, 0, size);
    return (void*)address;

};

void uefi_free(void* ptr) {
    // if (ptr == NULL) {
    //     return;
    // }

    // EFI_STATUS status = efi.SystemTable->BootServices->FreePool(ptr);

    // if (EFI_ERROR(status)) {
    //     ERR("FREE FAILED. ERR: %p\n", status);
    //     SYSTEM_HALT();
    // }
};
