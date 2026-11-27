# Prueft Code der zweiten Bank: jsr/jmp nur zu Labels derselben Datei (sonst CALL)
# Eingebundene .inc-Dateien zaehlen zur Datei.
import re, sys, glob
def expand(fn):
    s = open(fn).read()
    def inc(m):
        name = m.group(1)
        if name.endswith('.inc') and not name.startswith('assets'):
            return expand(name)
        return m.group(0)
    return re.sub(r'^\s*\.include\s+"([^"]+)"', inc, s, flags=re.M)
bad = 0
for fn in glob.glob('*.s'):
    s = expand(fn)
    if '.segment "XCODE"' not in s and '.segment "XCODE2"' not in s and '.segment "XCODE3"' not in s and '.segment "XCODE4"' not in s: continue
    labs = set(re.findall(r'^([A-Za-z_]\w*):', s, re.M))
    for i, line in enumerate(s.split('\n'), 1):
        m = re.match(r'\s*(?:\S+:\s*)?(jsr|jmp|jsl|jml)\s+([A-Za-z_]\w*)\b', line)
        if m and m.group(2) not in labs and not m.group(2).startswith('$'):
            print('%s:%d: %s %s liegt nicht in dieser Bank (CALL benutzen)' % (fn, i, m.group(1), m.group(2)))
            bad += 1
sys.exit(1 if bad else 0)
