set -e
CC=${CC65_BIN:-/home/claude/ext/cc65/bin}
python3 farcheck.py
python3 assets.py >/dev/null
python3 split_assets.py >/dev/null
$CC/ca65 -g -l app.lst app.s -o app.o
python3 mkomf2.py
