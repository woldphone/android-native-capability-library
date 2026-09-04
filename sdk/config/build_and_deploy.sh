#!/bin/sh
# Grant read/write access to the socket file so sandboxed applications can read/write
if [ -n "$IPC_SOCKET_WIFI" ]; then
  chmod 0777 "$IPC_SOCKET_WIFI" || true
fi
