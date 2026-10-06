#include <csl/utils.h>
#include <csl/terminal.h>
#include <payload.h>

void payload_main(struct PAYLOAD_BOOT_INFO __attribute__((unused)) boot_struct) {
    INFO("Payload RAN!");
};
