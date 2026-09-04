#include <stdio.h>
#include <stdlib.h>
#include "ipc_common.h"

extern int execute_hardware_command(int subsystem, int command, const uint8_t *payload, uint32_t payload_len, uint8_t *out_buffer, uint32_t *out_len);
extern const char *get_client_library_version();

int main() {
    printf("[Test] Native Client version: %s\n", get_client_library_version());

    uint8_t buffer[256];
    uint32_t len = sizeof(buffer);

    printf("[Test] Triggering WiFi Scan request via client_bridge...\n");
    int status = execute_hardware_command(SUBSYSTEM_WIFI, CMD_WIFI_START_SCAN, NULL, 0, buffer, &len);

    if (status == STATUS_OK) {
        if (len < sizeof(buffer)) {
            buffer[len] = '\0';
        } else {
            buffer[sizeof(buffer)-1] = '\0';
        }
        printf("[Test] Transaction Successful! Response payload: \"%s\"\n", (char *)buffer);
    } else {
        printf("[Test] Execution failed with status: %d (Is the wifi_svc daemon running?)\n", status);
    }

    return 0;
}
