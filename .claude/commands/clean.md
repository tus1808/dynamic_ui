---
description: Remove the Meson build directory to start fresh
---

Clean the project build artifacts.

1. Confirm with the user before deleting, since this removes all compiled objects and the `build/` directory.
2. Run: `rm -rf build/`
3. Remind the user to run `/setup` followed by `/build` to rebuild from scratch.

Only do a clean build if:
- The build is in a broken state that recompilation can't fix
- `meson.build` dependencies changed significantly
- The user explicitly requests a clean build
