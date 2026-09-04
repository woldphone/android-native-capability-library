#ifndef NATIVE_TELEPHONY_COMMON_H
#define NATIVE_TELEPHONY_COMMON_H

#include <stdint.h>

#pragma pack(push, 1)

// Unified Cell Tower Metric Packet (Packed)
typedef struct {
    uint8_t  type;              // Maps to CellularRadioTech (LTE=12, NR/5G=19)
    uint8_t  status;            // Maps to CellConnStatus (Primary=1, Secondary=2)
    int32_t  dbm;               // General signal strength (RSSI) in dBm
    int32_t  rsrp;              // LTE/5G Reference Signal Received Power (dBm)
    int32_t  rsrq;              // LTE/5G Reference Signal Received Quality (dB)
    int32_t  rssnr;             // LTE/5G Signal-to-Noise Ratio (dB)
    int32_t  asu;               // Arbitrary Strength Unit

    // Identity Parameters (Precise coordinates for diagnostics)
    int32_t  mcc;               // Mobile Country Code (2-3 digits)
    int32_t  mnc;               // Mobile Network Code (2-3 digits)
    int32_t  lac_or_tac;        // Location Area Code (GSM) or Tracking Area Code (LTE/5G)
    int32_t  cid_or_ci;         // Cell Identity
    int32_t  pci_or_psc;        // Physical Cell ID (LTE/5G) or Primary Scrambling Code (UMTS)
    int32_t  earfcn_or_nrarfcn; // Absolute Radio Frequency Channel Number
} CellTowerMetric;

#pragma pack(pop)

#endif // NATIVE_TELEPHONY_COMMON_H
