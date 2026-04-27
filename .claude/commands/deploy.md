---
description: Deploy the compiled binary and assets to the Orange Pi at 192.168.1.102
---

Deploy the application to the Orange Pi Lite at `192.168.1.102`.

Target user and paths on device:
- User: `orangepi`
- App binary: `~/dynamic_ui/app`
- Config: `~/dynamic_ui/config/`
- Assets: `~/dynamic_ui/assets/`

Steps:
1. Verify the device is reachable: `ping -c 1 192.168.1.102`
2. Copy the compiled binary: `scp build/app orangepi@192.168.1.102:~/dynamic_ui/app`
3. Copy config files: `scp -r config/ orangepi@192.168.1.102:~/dynamic_ui/config/`
4. Copy assets: `scp -r assets/ orangepi@192.168.1.102:~/dynamic_ui/assets/`
5. Confirm each transfer succeeded and report the final status.

If the `build/app` binary does not exist, suggest running `/build` first.
Note: this project is built on Windows but runs on Linux (Orange Pi). The binary produced by Meson on Windows will not run on ARM Linux — the binary must be cross-compiled or compiled on the device itself.
