---
description: Deploy the compiled binary and assets to the Raspberry Pi 3 at 192.168.1.55
---

Deploy the application to the Raspberry Pi 3 at `192.168.1.55`.

Target user and paths on device:
- User: `tus1808`
- App binary: `~/dynamic_ui/app`
- Config: `~/dynamic_ui/config/`
- Assets: `~/dynamic_ui/assets/`

Steps:
1. Verify the device is reachable: `ping -c 1 192.168.1.55`
2. Copy the compiled binary: `sshpass -p 1 scp build/app tus1808@192.168.1.55:~/dynamic_ui/app`
3. Copy config files: `sshpass -p 1 scp -r config/ tus1808@192.168.1.55:~/dynamic_ui/config/`
4. Copy assets: `sshpass -p 1 scp -r assets/ tus1808@192.168.1.55:~/dynamic_ui/assets/`
5. Confirm each transfer succeeded and report the final status.

If the `build/app` binary does not exist, suggest running `/build` first.
Note: this project is built on Windows but runs on Linux (Raspberry Pi). The binary produced by Meson on Windows will not run on ARM Linux — the binary must be cross-compiled or compiled on the device itself.
