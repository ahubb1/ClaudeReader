# Store catalog

ClaudeReader lists these stores in **Settings → Stores → Browse catalog** and installs them with one click.
A store is a Lua script; see the `README.txt` of the stores folder (Settings → Stores → Open folder) for how to write one.

To add a store to the catalog:

1. Add its script here, named after its `SOURCE.id` (for example `xkcd.lua`).
2. Add an entry to [`index.json`](index.json):
   - `id`: the `SOURCE.id` of the script
   - `name`: the name shown in the catalog
   - `version`: the `SOURCE.version` of the script; raise both when the script changes, so that ClaudeReader offers the update
   - `description`: one line
   - `url`: the script, relative to `index.json` (or a full `https://` address)

To test a catalog before publishing it, start ClaudeReader with the environment variable
`CLAUDEREADER_STORE_CATALOG` set to the address of your `index.json`.
