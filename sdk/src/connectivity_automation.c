#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "automation_common.h"
#include "../include/adb_client.h"

// Execute a shell command over the active on-device ADB session and return the raw output buffer
static int execute_adb_shell_cmd(AdbSession *session, const char *cmd, char *out_buf, uint32_t max_out_len) {
    if (!session || session->state != ADB_STATE_CONNECTED) {
        return AUTO_STATUS_ADB_DISCONNECTED;
    }

    // Open a dynamic shell channel specifically for this command
n    char shell_cmd[512];
    snprintf(shell_cmd, sizeof(shell_cmd), "shell:%s", cmd);

    if (adb_open_shell_channel(session, shell_cmd) < 0) {
        fprintf(stderr, "[Automation] Failed to open shell channel for: %s\n", cmd);
        return AUTO_STATUS_ERROR;
    }

    uint32_t total_bytes = 0;
    uint8_t temp_buf[2048];
    uint32_t bytes_read = 0;

    // Read the output stream until the channel is closed or EOF is hit
    while (adb_read_shell_data(session, temp_buf, sizeof(temp_buf), &bytes_read) == 0 && bytes_read > 0) {
        if (out_buf && total_bytes < max_out_len - 1) {
            uint32_t copy_len = (bytes_read < (max_out_len - 1 - total_bytes)) ? bytes_read : (max_out_len - 1 - total_bytes);
            memcpy(out_buf + total_bytes, temp_buf, copy_len);
            total_bytes += copy_len;
        }
    }

    if (out_buf) {
        out_buf[total_bytes] = '\0';
    }

    return AUTO_STATUS_OK;
}

// Check Bluetooth service status and enable Bluetooth if currently disabled
__attribute__((visibility("default")))
int auto_ensure_bluetooth_enabled(AutoContext *ctx) {
    if (!ctx || !ctx->adb_session_ptr) return AUTO_STATUS_ERROR;
    AdbSession *session = (AdbSession *)ctx->adb_session_ptr;

    char out_buf[128];
    int res = execute_adb_shell_cmd(session, CMD_BT_IS_ENABLED, out_buf, sizeof(out_buf));
    if (res != AUTO_STATUS_OK) return res;

    if (strstr(out_buf, "true") != NULL) {
        printf("[Automation] Bluetooth is already enabled.\n");
        return AUTO_STATUS_OK;
    }

    printf("[Automation] Enabling Bluetooth via on-device UID 2000...\n");
    res = execute_adb_shell_cmd(session, CMD_BT_ENABLE, out_buf, sizeof(out_buf));
    if (res != AUTO_STATUS_OK) return res;

    // Fast loop to wait for state change
    for (int i = 0; i < 10; ++i) {
        usleep(500000); // Wait 500ms
        execute_adb_shell_cmd(session, CMD_BT_IS_ENABLED, out_buf, sizeof(out_buf));
        if (strstr(out_buf, "true") != NULL) {
            return AUTO_STATUS_OK;
        }
    }

    return AUTO_STATUS_TIMEOUT;
}

// Programmatically initiate Bluetooth GATT pairing without root or popups
__attribute__((visibility("default")))
int auto_pair_bluetooth_device(AutoContext *ctx, const char *mac_address) {
    if (!ctx || !ctx->adb_session_ptr || !mac_address) return AUTO_STATUS_ERROR;
    AdbSession *session = (AdbSession *)ctx->adb_session_ptr;

