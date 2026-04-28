---
description: Check Raspberry Pi 3 device connectivity and UART port status
---

Check the status of the Raspberry Pi 3 development target at `192.168.1.55`.

1. Ping the device: `ping -c 1 192.168.1.55`
2. If reachable, SSH in and check:
   - Available serial ports: `sshpass -p 1 ssh tus1808@192.168.1.55 "ls /dev/ttyS* /dev/ttyUSB* 2>/dev/null"`
   - Whether the app process is running: `sshpass -p 1 ssh tus1808@192.168.1.55 "pgrep -a app"`
   - Disk space: `sshpass -p 1 ssh tus1808@192.168.1.55 "df -h ~/dynamic_ui"`
3. Report which UART devices are available. The app expects a 136-byte frame format (8-byte header + 128-byte body) from the serial sensor. The configured port is `/dev/ttyS1` at the baud rate set in `src/uart/port.c`.
4. If no UART device is found, suggest checking physical connections and kernel module status.
