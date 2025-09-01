# Repository Guidelines

## Project Structure & Module Organization
- `wdii/src`: C source, headers, and Makefiles. Builds the game server.
- `wdii/bin`: Build outputs (e.g., `circle`). Ignored by Git.
- `wdii/lib`: Game data/world files used at runtime (do not edit casually).
- `wdii/log` and `wdii/syslog`: Runtime logs. Useful for debugging.
- Scripts: `wdii/automud`, `wdii/automudgdb`, `wdii/autorun.sh` (may require path fixes before use).

## Build, Test, and Development Commands
- Prereqs: `gcc`, `make` on a Unix-like system.
- Configure (if needed): `cd wdii/src && ./configure`
- Build: `cd wdii/src && make` (outputs `../bin/circle`)
- Clean: `cd wdii/src && make clean`
- Run locally (from game root so data paths resolve):
  - `cd wdii && ./bin/circle -q 2022`
  - Connect: `telnet localhost 2022`
- Logs: `tail -f wdii/syslog` and `tail -f wdii/log/errors`

## Coding Style & Naming Conventions
- Language: C (CircleMUD codebase). Keep `gcc -Wall` warnings clean.
- Indentation/Braces: Match existing files (K&R style; tabs common here).
- Naming: `lower_snake_case` for functions/vars; headers `*.h`, sources `*.c`.
- Includes: Prefer local headers first (e.g., `conf.h`, `sysdep.h`), then system.
- Touch only relevant modules; follow existing patterns (e.g., `act.*.c`, `*edit.c`).

## Testing Guidelines
- No formal unit tests; rely on compile + runtime checks.
- Smoke test: build, start server, create a new character, exercise movement, combat, save/load.
- Watch for `SYSERR` in `wdii/syslog` and `wdii/log/errors`.
- Debugging: `gdb wdii/bin/circle` or use `wdii/automudgdb` after fixing hardcoded paths.

## Commit & Pull Request Guidelines
- Commits: small, focused, imperative subject (e.g., "fix: prevent null room crash"). Link issues (`#123`).
- PRs: clear description, reproduction steps, affected areas/files, logs (`SYSERR`) or telnet transcript if relevant, and rollback notes.
- Target branch: default branch unless specified. Avoid committing runtime data (`wdii/lib` player files, logs).

## Security & Configuration Tips
- Run on non-privileged ports in dev (e.g., `2022`).
- Do not commit logs or player/account data. Update `.gitignore` if needed.
- Some scripts contain absolute paths—review before using in production.

