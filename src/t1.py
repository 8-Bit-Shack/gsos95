import sys, time; sys.path.insert(0,'.')
from emu import *
labels = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('gs95.lbl')}
cpu = boot('GSOS95.2mg')
t=time.time()
ok = run(cpu, 3_000_000, stop_at=labels['mainloop'])
print('mainloop', ok, cpu.icount, 'blocks', cpu.blockreads, '%.1fs'%(time.time()-t))
render(cpu, 'shot_desktop.png', 3)
