exec(open('t19.py').read().split("print(names(':SYSTEM.DISK'))")[0])
h0,v0,h1,v1=260,76,108,49
cpu.events += [(0,h0,v0,0)]; settle()
cpu.events += [(1,h0,v0,0)]+[(0,h0+(h1-h0)*i//8, v0+(v1-v0)*i//8,0) for i in range(1,9)]
for k in range(6):
    run(cpu, 500_000)
    pc=(cpu.PBR<<16|cpu.PC)-ADDR
    near=min(((v,k2) for k2,v in lbl.items() if v<=pc and not k2.startswith('@')), default=None)
    print(k, hex(pc), [kk for kk,vv in lbl.items() if vv==max(v for v in lbl.values() if v<=pc)][:2], 'events left', len(cpu.events))
render(cpu,'g5.png',3)
