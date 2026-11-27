# Zu weite relative Spruenge (ca65 "Range error") in lange Formen umbauen
import re, subprocess, sys
INV = {'bcc':'bcs','bcs':'bcc','beq':'bne','bne':'beq','bmi':'bpl','bpl':'bmi','bvc':'bvs','bvs':'bvc'}
for it in range(30):
    r = subprocess.run([__import__('os').environ.get('CC65_BIN', '/home/claude/ext/cc65/bin').rstrip('/') + '/ca65','-g','app.s','-o','/tmp/fx.o'], capture_output=True, text=True)
    errs = [l for l in (r.stdout+r.stderr).splitlines() if 'Range error' in l]
    if not errs: print('ok', it); break
    done = set()
    for l in errs:
        f, n = l.split(':')[0], int(l.split(':')[1])
        if (f, n) in done: continue
        done.add((f, n))
        L = open(f).read().split('\n')
        m = re.match(r'^(\s*(?:@?\w+:)?\s*)(b\w\w)\s+(\S+)(.*)$', L[n-1])
        if not m: print('?', f, n, L[n-1]); continue
        pre, op, tgt, rest = m.groups()
        if op == 'bra': L[n-1] = '%sbrl %s%s' % (pre, tgt, rest)
        elif op in INV: L[n-1] = '%s%s *+5\n        jmp %s%s' % (pre, INV[op], tgt, rest)
        else: print('?', f, n, L[n-1]); continue
        open(f, 'w').write('\n'.join(L))
        print('fix', f, n, op, tgt)
        break   # Zeilennummern verschieben sich: einzeln
