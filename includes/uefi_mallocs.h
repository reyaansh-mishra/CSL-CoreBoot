/* includes/uefi_mallocs.h */

#include <stddef.h>

void* uefi_alloc(size_t size);
void uefi_free(void* ptr);
