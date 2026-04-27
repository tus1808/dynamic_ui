---
description: Generate a SHA-256 hash for the editor mode password to store in config/app.json
---

Generate a SHA-256 hash for a new editor mode password.

1. Ask the user for the plaintext password they want to set (if not provided as an argument).
2. Compute the SHA-256 hash using Python: `python3 -c "import hashlib, sys; print(hashlib.sha256(sys.argv[1].encode()).hexdigest())" "<password>"`
3. Show the resulting hash to the user.
4. Instruct the user to update `config/app.json` by replacing the value of the `editor_password` field with this new hash.

Do NOT write the hash directly to `config/app.json` without the user's explicit confirmation — that file also contains other application settings.
