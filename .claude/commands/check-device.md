---
description: Check Orange Pi device connectivity and UART port status
---

Check the status of the Orange Pi development target at `192.168.1.102`.

1. Ping the device: `ping -c 1 192.168.1.102`
2. If reachable, SSH in and check:
   - Available serial ports: `ssh orangepi@192.168.1.102 "ls /dev/ttyS* /dev/ttyUSB* 2>/dev/null"`
   - Whether the app process is running: `ssh orangepi@192.168.1.102 "pgrep -a app"`
   - Disk space: `ssh orangepi@192.168.1.102 "df -h ~/dynamic_ui"`
3. Report which UART devices are available. The app expects a 136-byte frame format (8-byte header + 128-byte body) from the serial sensor. The configured port is `/dev/ttyS1` at the baud rate set in `src/uart/port.c`.
4. If no UART device is found, suggest checking physical connections and kernel module status.
