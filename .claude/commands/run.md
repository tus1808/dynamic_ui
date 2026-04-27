---
description: Start the dynamic_ui application on the Orange Pi via SSH
---

Run the application on the Orange Pi at `192.168.1.102` over SSH.

1. SSH into the device: `ssh orangepi@192.168.1.102`
2. Change to the app directory and start it: `cd ~/dynamic_ui && ./app`
3. If the app fails to start, check for:
   - Missing UART device (`/dev/ttyS1` or similar) — the app auto-detects the serial port
   - Missing GTK display (DISPLAY env var) — may need `export DISPLAY=:0` for a connected screen
   - Missing config files (`config/app.json`, `config/layout.json`)
4. Show any stderr output to help diagnose startup issues.
