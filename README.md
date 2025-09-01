# Wardome (CircleMUD Fork)

Wardome is a legacy Multi-User Dungeon (MUD) server based on CircleMUD 3.0. This repository preserves and evolves the original codebase, with fixes and small features. It compiles and runs on Unix-like systems.

If you find issues, please open a GitHub issue with logs or repro steps.

## Quick Start
- Requirements: `gcc`, `make`, and a terminal client (e.g., `telnet`).
- Build:
  - `cd wdii/src && make` (produces `../bin/circle`)
- Run (from the game root so data paths resolve):
  - `cd wdii && ./bin/circle -q 2022`
  - Connect: `telnet localhost 2022`
- Logs:
  - `tail -f wdii/syslog` and `tail -f wdii/log/errors`

## Project Structure
- `wdii/src`: C sources, headers, and `Makefile`.
- `wdii/bin`: Build outputs (`circle`, tools). Git-ignored.
- `wdii/lib`: Game data and runtime files (world files, player data).
- `wdii/log` and `wdii/syslog`: Runtime logs.
- Scripts: `wdii/automud`, `wdii/automudgdb`, `wdii/autorun.sh` (some contain absolute paths—review before use).

## Development
- Match existing C style (K&R, tabs common, `lower_snake_case`). Keep builds warning-clean.
- Typical change flow: update `*.c`/`*.h` → `make` → run server → test via telnet.
- Debugging:
  - Quick: `gdb --args wdii/bin/circle -q 2022` then `run`
  - Scripted: `wdii/automudgdb` (operates from `wdii/` and logs to `gdblog`)

## Data and Logs
- Player files and logs are not versioned. `.gitignore` excludes `wdii/log/`, `wdii/syslog*`, and runtime data under `wdii/lib` (e.g., `pfiles/`, `plrobjs/`, `alias/`).

## Contributing
- See `AGENTS.md` for the contributor guide (build, style, testing, and PR expectations).

Português (resumo): o código do WD MUD compila e roda. Use `wdii/src/Makefile` para compilar e `wdii/bin/circle -q 2022` para executar. Registre bugs com logs (SYSERR) e passos para reproduzir.
