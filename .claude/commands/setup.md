---
description: First-time Meson setup or reconfigure after meson.build changes
---

Set up or reconfigure the Meson build directory.

1. If `build/` does not exist, run: `meson setup build`
2. If `build/` already exists and you are reconfiguring (e.g. after changes to `meson.build`), run: `meson build --reconfigure`
3. Confirm that `build/compile_commands.json` was generated — this is used by clangd for code indexing.
4. Report the result. If there are dependency errors (missing libgtk-3-dev, libjson-glib-dev, etc.), remind the user that those must be installed on the **target Raspberry Pi 3 device**, not the Windows dev machine.
