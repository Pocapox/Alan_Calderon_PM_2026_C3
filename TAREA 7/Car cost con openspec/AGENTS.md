# AGENTS.md

## Project

C11 console app (`carcost.exe`) and Win32 desktop application (`carcost-gui.exe`)
that manage vehicles and compute trip cost in DOP. Units are fixed: DOP, fuel
price per liter, consumption in km/L.

## Layout

- `src/vehicle.*` — `Vehicle` model, validation, in-memory `VehicleStore`.
- `src/calculator.*` — pure cost functions; annual fixed costs are prorated by
  `km_per_year`, not by trip distance.
- `src/storage.*` — `vehicles.dat` load/save (pipe-delimited, atomic replace).
- `src/parse.*` — pure parsing functions for numbers and whitespace trimming.
- `src/utf8.*` — conversion utilities between UTF-8 and UTF-16 (wchar_t).
- `src/input.*` — stdin parsing and prompts; delegates pure parsing to `parse.*`.
- `src/main.c` — console menu plus CRUD and calculation flows (UTF-8 enabled).
- `src/gui_main.c` — Win32 desktop GUI (tabs, ListView, forms, trip calculation).
- `src/gui.rc`, `src/gui.manifest`, `src/no-manifest.specs` — Common Controls v6
  manifest, DPI awareness, and build specs.
- `tests/run_tests.c` — assert-style runner, no external test framework.

## Commands

Run from the repo root; the app reads and writes `vehicles.dat` in the current
working directory.

- `make` — build `carcost.exe` (GCC, C11, `-Wall -Wextra -Wpedantic`).
- `make gui` — build `carcost-gui.exe` (Win32 GUI, Common Controls v6).
- `make test` — build and run unit tests; must report `0 fallos`.
- `make clean` — remove binaries, objects, and test data.
- `.\carcost.exe` — interactive console menu.
- `.\carcost-gui.exe` — interactive graphical interface.

Requires MinGW-w64 GCC on `PATH` (`gcc --version` must work). A fresh terminal
may be needed after installing. Make is GNU Make 3.81 (GnuWin32); keep the
Makefile limited to basic rules and variables.

## Persistence

`vehicles.dat` has the header `carcost 1` and one pipe-delimited record per
line encoded in UTF-8. Both CLI and GUI share the same file in the working
directory. Invalid lines are skipped with a warning; an invalid header aborts
startup instead of overwriting the file. Saves write `vehicles.dat.tmp` and
replace the original, so do not hand-edit the file while the app is running.

## OpenSpec

Run `cmd /c "openspec ..."` from PowerShell. The `openspec.ps1` shim is blocked
by the execution policy on this machine, so the bare command fails.

## Maintaining this file

Update commands and layout here when they change; delete stale claims.
