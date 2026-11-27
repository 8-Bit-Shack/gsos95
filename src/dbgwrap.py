import signal, sys, bisect
src = open(sys.argv[1]).read()
g = {'__name__': '__main__'}
def h(*a):
    cpu = g['cpu']; lbl = g['lbl']; ADDR = g['ADDR']
    inv = sorted((v, k) for k, v in lbl.items())
    global CNT
    CNT = globals().get('CNT', 0) + 1
    if True:
        pc = (cpu.PBR << 16) | cpu.PC
        # Segment-Basis unbekannt: nur Hauptbank mappen
        off = pc - ADDR if (pc>>16)==(ADDR>>16) else pc & 0xFFFF
        i = bisect.bisect_right([v for v, k in inv], off) - 1
        print(hex(pc), inv[i][1] if i >= 0 else '?', hex(off - inv[i][0]) if i >= 0 else '', 'A', hex(cpu.A), 'X', hex(cpu.X), 'Y', hex(cpu.Y))
    if CNT > 15: import os; os._exit(1)
signal.signal(signal.SIGALRM, h); signal.setitimer(signal.ITIMER_REAL, int(sys.argv[2]), 0.7)
exec(compile(src, sys.argv[1], 'exec'), g)
