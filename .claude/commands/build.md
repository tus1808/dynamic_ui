---
description: Compile the dynamic_ui GTK3/C application using Meson
---

Compile the project with Meson. Run these steps in order:

1. Check if `build/` directory exists. If it does not, run `meson setup build` first.
2. Run `meson compile -C build` to compile.
3. Report any compiler errors clearly, pointing to the relevant source file and line number.
4. On success, confirm the binary is at `build/app`.

If `meson.build` was recently changed, suggest running `/setup` to reconfigure before recompiling.
