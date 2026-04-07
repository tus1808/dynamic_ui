# Project: dynamic_ui

## Device: Orange Pi Lite + OS: Armbian 26.2 (XFCE)

### Install dependencies:
__sudo apt install -y build-essential meson ninja-build pkg-config libgtk-3-dev libjson-glib-dev__

### Build and run:
<!-- For next the first time build -->
_meson setup build_
<!-- For the next time build -->
_meson build --reconfigure_
_meson compile -C build_
_cd /build_
_./app_