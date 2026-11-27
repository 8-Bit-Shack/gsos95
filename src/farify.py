# Verlegt eine Quelldatei in die dritte Codebank (XCODE2):
#   Aufrufe nach draussen -> CALL (Sprungstellen tp_ in tramp.s),
#   Aufrufe von draussen  -> JSLF xf_name (Einstiege xf_ in der Datei).
import re, sys
INCL = [l.split('"')[1] for l in open('app.s') if l.strip().startswith('.include') and l.split('"')[1].endswith('.s')]
fn = sys.argv[1]
src = open(fn).read()
if '.segment "XCODE' in src: sys.exit('schon fern: ' + fn)
labs = set(re.findall(r'^([A-Za-z_]\w*):', src, re.M))
others = {f: open(f).read() for f in INCL + ['app.s'] if f != fn}
far = {f for f, s in others.items() if '.segment "XCODE' in s}
# 0) Datentabellen in die Hauptbank (RODATA), Code liest sie mit DBR = Hauptbank
DATA = re.compile(r'^\s*(?:[A-Za-z_]\w*:)?\s*\.(word|byte|res|addr|asciiz|dword|faraddr)\b')
lines = src.split('\n'); res = []; ind = False
for line in lines:
    isd = bool(DATA.match(line)) or (ind and (line.strip() == '' or line.strip().startswith(';')))
    if isd and not ind and DATA.match(line):
        res.append('        .pushseg\n        .segment "RODATA"'); ind = True
    elif ind and not isd:
        res.append('        .popseg'); ind = False
    res.append(line)
if ind: res.append('        .popseg')
src = '\n'.join(res)
# 1) Datei selbst
need_tp = set()
out = []
for line in src.split('\n'):
    m = re.match(r'^(\s*(?:[A-Za-z_@]\w*:)?\s*)(jsr|jmp)\s+([A-Za-z_]\w*)(\s*(?:;.*)?)$', line)
    if m and m.group(3) not in labs:
        need_tp.add(m.group(3))
        line = '%sCALL %s%s' % (m.group(1), m.group(3), m.group(4))
        if m.group(2) == 'jmp': line += '\n        rts'
    mb = re.match(r'^\s*(?:[A-Za-z_@]\w*:)?\s*(b(?:cc|cs|eq|ne|mi|pl|vc|vs|ra|rl))\s+([A-Za-z_]\w*)', line)
    if mb and mb.group(2) not in labs:
        sys.exit('Sprung nach draussen: ' + line)
    out.append(line)
body = '\n'.join(out)
# 2) Aufrufer
entries = set(); stubs = set()
for f, s in others.items():
    o = []
    for line in s.split('\n'):
        m = re.match(r'^(\s*(?:[A-Za-z_@]\w*:)?\s*)(jsr|jmp)\s+([A-Za-z_]\w*)(\s*(?:;.*)?)$', line)
        if m and m.group(3) in labs:
            entries.add(m.group(3))
            if f in far:
                sys.exit('jsr aus Fernbank auf %s in %s' % (m.group(3), f))
            line = '%sJSLF xf_%s%s' % (m.group(1), m.group(3), m.group(4))
            if m.group(2) == 'jmp': line += '\n        rts'
        mc = re.match(r'^(\s*(?:[A-Za-z_@]\w*:)?\s*)CALL\s+([A-Za-z_]\w*)(.*)$', line)
        if mc and mc.group(2) in labs and f != 'tramp.s':
            entries.add(mc.group(2))
            line = '%sJSLF xf_%s%s' % (mc.group(1), mc.group(2), mc.group(3))
        mt = re.match(r'^tp_([A-Za-z_]\w*):\s*jsr\s+(\w+)', line)
        if mt and mt.group(2) in labs:
            entries.add(mt.group(2))
            line = 'tp_%s: JSLF xf_%s' % (mt.group(1), mt.group(2))
        mb = re.match(r'^\s*(?:[A-Za-z_@]\w*:)?\s*(b(?:cc|cs|eq|ne|mi|pl|vc|vs|ra|rl))\s+([A-Za-z_]\w*)', line)
        if mb and mb.group(2) in labs:
            sys.exit('Sprung von %s in die Datei: %s' % (f, line))
        if re.search(r'\.(word|addr)\b', line) and any(w in labs for w in re.findall(r'\b([A-Za-z_]\w*)\b', line)):
            def rep(mm):
                w = mm.group(1)
                if w in labs:
                    entries.add(w); stubs.add(w); return w + '_m'
                return w
            head, _, tail = line.partition('.word') if '.word' in line else line.partition('.addr')
            line = head + ('.word' if '.word' in line else '.addr') + re.sub(r'\b([A-Za-z_]\w*)\b', rep, tail)
        for w in re.findall(r'[#(]\s*([A-Za-z_]\w*)', line):
            if w in labs and not line.strip().startswith(';'):
                print('WARNUNG Adressbezug von %s: %s' % (f, line.strip()))
        o.append(line)
    others[f] = '\n'.join(o)
# 3) Sprungstellen
tr = others['tramp.s']
have = set(re.findall(r'^tp_(\w+):', tr, re.M))
for n in sorted(need_tp - have):
    tr += '\ntp_%s: jsr %s\n        rtl' % (n, n)
for n in sorted(stubs):
    tr += '\n%s_m: JSLF xf_%s\n        rts' % (n, n)
others['tramp.s'] = tr
SEGN = sys.argv[2] if len(sys.argv) > 2 else '2'
hdr = '        .segment "XCODE%s"\nFARMODE .set %s\n' % (SEGN, SEGN)
wr = ''.join('xf_%s:\n        jsr %s\n        rtl\n' % (e, e) for e in sorted(entries))
body = hdr + wr + body + '\nFARMODE .set 0\n        .segment "CODE"\n'
open(fn, 'w').write(body)
for f, s in others.items(): open(f, 'w').write(s)
print(fn, 'Einstiege', sorted(entries), 'Sprungstellen neu', len(need_tp - have))
