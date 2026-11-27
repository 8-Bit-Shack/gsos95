set -e
CC=${CC65_BIN:-/home/claude/ext/cc65/bin}
python3 assets.py >/dev/null
$CC/ca65 -g -l main.lst main.s -o main.o
$CC/ld65 -C gs95.cfg main.o -o gs95.bin -m gs95.map -Ln gs95.lbl
python3 mkimg.py
