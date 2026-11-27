# Ordner kopieren (rekursiv) ueber Kopieren/Einfuegen
import sys; sys.path.insert(0,'.')
import t3
t3.FS[':GSOS95:SPIELE'] = [('SHANGHAI',0xB3),('CRYSTAL.QUEST',0xB3),('LEVELS',0x0F)]
t3.FS[':GSOS95:SPIELE:LEVELS'] = [('L1',0x06),('L2',0x06)]
t3.FILES[':GSOS95:SPIELE:SHANGHAI']=b'S'*3000
t3.FILES[':GSOS95:SPIELE:LEVELS:L1']=b'1'*700
src=open('t49.py').read().split("p1=path()")[0].replace("key(0x15); key(13)                  # erstes Laufwerk oeffnen\n","")
exec(src)
key(0x15); key(0x15); key(13)        # zweites Laufwerk GSOS95
print('in', path())
# SPIELE ist der dritte Eintrag (GSOS95, LIESMICH, SPIELE)
key(0x15); key(0x15); key(0x15)
key(3)                               # Strg+C
key(0x7F)                            # Delete: nach oben (Arbeitsplatz)
print('in', path())
key(0x15); key(13)                   # SYSTEM.DISK
print('in', path())
key(22)                              # Strg+V
for i in range(5): settle()
render(cpu,'t64.png',3)
fs=t3.FS
ok = (':SYSTEM.DISK:SPIELE' in fs and ':SYSTEM.DISK:SPIELE:LEVELS' in fs
      and t3.FILES.get(':SYSTEM.DISK:SPIELE:SHANGHAI')==b'S'*3000
      and t3.FILES.get(':SYSTEM.DISK:SPIELE:LEVELS:L1')==b'1'*700)
print('kopiert:', [k for k in fs if 'SYSTEM.DISK:SPIELE' in k], fs.get(':SYSTEM.DISK:SPIELE'))
print('OK' if ok else 'FAIL Ordner kopieren')
