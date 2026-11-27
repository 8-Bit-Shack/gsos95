#!/bin/sh
# Startbild komplett neu bauen (nach Aenderung von BUILD): START.GS.OS, GSOS95.SPL, SPLASH95, BOOT95, PRODOS
set -e
cd "$(dirname "$0")"
CC=${CC65_BIN:-/home/claude/ext/cc65/bin}
(cd boot95 && python3 gen.py >/dev/null && $CC/ca65 boot95.s -o boot95.o && $CC/ld65 -C boot95.cfg boot95.o -o boot95.bin \
  && $CC/ca65 early.s -o early.o && $CC/ld65 -C early.cfg early.o -o early.bin)
python3 boot95/patch.py sys61/START.GS.OS.61 sys/START.GS.OS.95
python3 boot95/mksplash.py
python3 init95/mkspl.py
(cd boot95x && $CC/ca65 boot95x.s -o boot95x.o && $CC/ld65 -C boot95x.cfg boot95x.o -o BOOT95)
python3 tools/patch_prodos.py sys61/PRODOS.61 sys/PRODOS.95
