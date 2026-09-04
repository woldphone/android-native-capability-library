#ifndef NACL_USB_SUBSYSTEM_H
#define NACL_USB_SUBSYSTEM_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define USB_ENDPOINT_IN  0x80
#define USB_ENDPOINT_OUT 0x00

typedef enum {
    USB_STATUS_OK = 0,
    USB_ERROR_PERMISSION_DENIED = -1,
    USB_ERROR_INVALID_FD = -2,
    USB_ERROR_TRANSFER_FAILED = -3,
    USB_ERROR_TIMEOUT = -4
} UsbStatus;

typedef struct {
    int file_descriptor;
    uint8_t interface_number;
    uint8_t endpoint_in;
    uint8_t endpoint_out;
    uint32_t max_packet_size;
} UsbConfig;

// Callback signature triggered when a raw asynchronous bulk transfer completes
typedef void (*UsbTransferCallback)(const uint8_t *buffer, size_t size, void *user_data);

/**
 * @brief Claims a dynamically duplicated file descriptor and configures endpoints.
 */
int usb_claim_device(const UsbConfig *config);

/**
 * @brief Starts an asynchronous bulk transfer polling loop on a background thread.
 */
int usb_start_async_poll(UsbTransferCallback callback, void *user_data);

/**
 * @brief Transmits a raw payload buffer over the configured OUT bulk endpoint.
 */
int usb_write_bulk(const uint8_t *data, size_t size, uint32_t timeout_ms);

/**
 * @brief Terminates the background polling thread and releases claimed interfaces.
 */
void usb_release_device(void);

#ifdef __cplusplus
}
#endif

#endif // NACL_USB_SUBSYSTEM_H
