#!/system/bin/sh
# verify_daemons.sh - Diagnostic Utility for Android Native Capability Library
# To be executed under UID 2000 (Shell) or Root (UID 0) via ADB

export PATH=/system/bin:/system/xbin:/data/local/tmp:$PATH

echo "======================================================================"
echo "    ANDROID NATIVE DAEMON SUITE: HIGH-PERFORMANCE DIAGNOSTIC TOOL     "
echo "======================================================================"
echo "Current Execution Context:"
echo "  - User ID (UID): $(id -u)"
echo "  - Group ID (GID): $(id -g)"
echo "  - SELinux Domain: $(getcon)"
echo "  - Local Time: $(date)"
echo "======================================================================"

SOCKET_DIR="/data/local/tmp/sdk/sockets"
DAEMONS="wifi_svc sensors_svc bluetooth_svc"

# 1. Audit Daemon Process States
echo "\n[STEP 1] Auditing Background Native Daemon Processes..."
for daemon in $DAEMONS; do
    PID=$(pgrep -f "$daemon")
    if [ -n "$PID" ]; then
        echo "  [✔] $daemon is RUNNING (PID: $PID)"
        # Print CPU, memory, and thread counts from procfs
        THREADS=$(cat /proc/$PID/status | grep "Threads" | awk '{print $2}')
        RSS=$(cat /proc/$PID/status | grep "VmRSS" | awk '{print $2 " " $3}')
        echo "      -> Threads: $THREADS | Resident Memory (RSS): $RSS"
        echo "      -> SELinux Context: $(ps -Z $PID | awk '{print $1}')"
    else
        echo "  [✘] $daemon is NOT RUNNING"
    fi
done

# 2. Verify Unix Domain Socket Existence and Permissions
echo "\n[STEP 2] Auditing Unix Domain Sockets in $SOCKET_DIR..."
if [ -d "$SOCKET_DIR" ]; then
    echo "  [✔] Socket directory exists: $SOCKET_DIR"
    ls -la "$SOCKET_DIR" | grep ".sock" | while read -r line; do
        echo "      -> $line"
    done
else
    echo "  [✘] Socket directory DOES NOT EXIST. Sockets have not been initialized."
fi

# 3. Check Socket Directory Permissions (Must be 777 for app sandbox access)
echo "\n[STEP 3] Verifying App Sandbox Accessibility Permissions..."
DIR_PERM=$(stat -c "%a" "$SOCKET_DIR" 2>/dev/null || stat -f "%p" "$SOCKET_DIR" 2>/dev/null)
echo "  - Directory permissions: $DIR_PERM"
if [ "$DIR_PERM" = "777" ] || [ "$DIR_PERM" = "40777" ]; then
    echo "  [✔] SUCCESS: Socket directory is wide-open (777). App sandbox can connect."
else
    echo "  [⚠] WARNING: Directory permissions are restrictive. Sandboxed applications may hit EACCES (Permission Denied)."
fi

# 4. Binary Packet Sniffing (Transaction Header Validation)
echo "\n[STEP 4] Auditing Real-time Socket Intercepts (Validating Magic Headers)..."
for sock in wifi.sock sensors.sock bluetooth.sock; do
    SOCK_PATH="$SOCKET_DIR/$sock"
    if [ -S "$SOCK_PATH" ]; then
        echo "  - Sniffing socket connection: $sock"
        # We attempt a non-blocking poll using toybox netcat if available
        if command -v nc >/dev/null 2>&1; then
            # Fire a Ping packet (Magic: 0x4E41434C, Payload: 0)
            # Layout: Magic (4B), TxID (4B), Subsystem (2B), Command (2B), Status (4B), PayloadLen (4B)
            # Binary hex: 43 41 41 4E 01 00 00 00 00 00 64 00 00 00 00 00 00 00 00 00
            HEX_PING="4c41414e01000000000064000000000000000000"
            echo -n "$HEX_PING" | xxd -r -p | nc -U -w 1 "$SOCK_PATH" > /data/local/tmp/sock_resp.bin 2>/dev/null

            if [ -s /data/local/tmp/sock_resp.bin ]; then
                RESP_HEX=$(xxd -p /data/local/tmp/sock_resp.bin | head -n 1)
                echo "    [✔] Response received!"
                echo "        -> Sent Ping Hex: $HEX_PING"
                echo "        -> Rcvd Resp Hex: $RESP_HEX"
                # Check magic header match "NACL" (little-endian: 4c 41 41 4e)
                if echo "$RESP_HEX" | grep -q "^4c41414e"; then
                    echo "        -> Magic Header Verified: OK (0x4E41434C)"
                else
                    echo "        -> [⚠] Malformed Response: Magic header mismatch!"
                fi
            else
                echo "    [⚠] Socket opened but returned no response data (Service is likely idle or non-blocking)."
            fi
            rm -f /data/local/tmp/sock_resp.bin
        else
            echo "    [⚠] Diagnostic skip: 'nc' (Netcat) utility not found on device shell."
        fi
    else
        echo "  [✘] Socket $sock is not active."
    fi
done

# 5. Native Binder Diagnostics
echo "\n[STEP 5] Auditing Native System Service Binder Handles..."
echo "  - Querying service manager registrations for Bluetooth and Wi-Fi:"
for svc in bluetooth wifi sensor; do
    if service list | grep -q -E "\b$svc\b"; then
        echo "    [✔] Binder Service '$svc' is active in AOSP ServiceManager."
    else
        echo "    [✘] Binder Service '$svc' is NOT registered!"
    fi
done

echo "\n======================================================================"
echo "    DIAGNOSTIC PIPELINE RUN COMPLETE                                 "
echo "======================================================================"
