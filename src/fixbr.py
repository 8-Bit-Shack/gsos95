# Lange Spruenge: "bxx ziel" mit Bereichsfehler -> Gegenbedingung + jmp (wiederholt bis fehlerfrei)
import re, subprocess, sys
INV = {'bcc': 'bcs', 'bcs': 'bcc', 'beq': 'bne', 'bne': 'beq', 'bmi': 'bpl', 'bpl': 'bmi', 'bvc': 'bvs', 'bvs': 'bvc'}
CC = __import__('os').environ.get('CC65_BIN', '/home/claude/ext/cc65/bin').rstrip('/') + '/ca65'
for it in range(30):
    r = subprocess.run([CC, '-g', 'app.s', '-o', '/tmp/x.o'], capture_output=True, text=True)
    errs = re.findall(r'^(\S+):(\d+): Error: Range error', r.stderr, re.M)
    if not errs:
        print('ok nach', it, 'Runden'); sys.exit(0)
    byf = {}
    for f, l in errs: byf.setdefault(f, set()).add(int(l))
    for f, ls in byf.items():
        lines = open(f).read().split('\n')
        for l in sorted(ls, reverse=True):
            s = lines[l - 1]
            m = re.match(r'^(\s*(?:[@A-Za-z_]\w*:\s*)?)(b(?:cc|cs|eq|ne|mi|pl|vc|vs|ra))\s+(\S+)(.*)$', s)
            if not m:
                print('nicht umsetzbar:', f, l, s); sys.exit(1)
            pre, op, tgt, rest = m.groups()
            if op == 'bra':
                lines[l - 1] = '%sjmp %s%s' % (pre, tgt, rest)
            else:
                lines[l - 1] = '%s%s *+5%s\n        jmp %s' % (pre, INV[op], rest, tgt)
        open(f, 'w').write('\n'.join(lines))
print('gibt nicht auf'); sys.exit(1)
