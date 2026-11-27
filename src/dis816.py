import sys
d=open(sys.argv[1],'rb').read(); base=int(sys.argv[2],16)
names = "BRK ORA COP ORA TSB ORA ASL ORA PHP ORA ASL PHD TSB ORA ASL ORA BPL ORA ORA ORA TRB ORA ASL ORA CLC ORA INC TCS TRB ORA ASL ORA JSR AND JSL AND BIT AND ROL AND PLP AND ROL PLD BIT AND ROL AND BMI AND AND AND BIT AND ROL AND SEC AND DEC TSC BIT AND ROL AND RTI EOR WDM EOR MVP EOR LSR EOR PHA EOR LSR PHK JMP EOR LSR EOR BVC EOR EOR EOR MVN EOR LSR EOR CLI EOR PHY TCD JML EOR LSR EOR RTS ADC PER ADC STZ ADC ROR ADC PLA ADC ROR RTL JMP ADC ROR ADC BVS ADC ADC ADC STZ ADC ROR ADC SEI ADC PLY TDC JMP ADC ROR ADC BRA STA BRL STA STY STA STX STA DEY BIT TXA PHB STY STA STX STA BCC STA STA STA STY STA STX STA TYA STA TXS TXY STZ STA STZ STA LDY LDA LDX LDA LDY LDA LDX LDA TAY LDA TAX PLB LDY LDA LDX LDA BCS LDA LDA LDA LDY LDA LDX LDA CLV LDA TSX TYX LDY LDA LDX LDA CPY CMP REP CMP CPY CMP DEC CMP INY CMP DEX WAI CPY CMP DEC CMP BNE CMP CMP CMP PEI CMP DEC CMP CLD CMP PHX STP JML CMP DEC CMP CPX SBC SEP SBC CPX SBC INC SBC INX SBC NOP XBA CPX SBC INC SBC BEQ SBC SBC SBC PEA SBC INC SBC SED SBC PLX XCE JSR SBC INC SBC".split()
# Modus je Opcode (Standardtabelle)
M = {}
def setm(ops, mode):
    for o in ops: M[o]=mode
for col,mode in [(0x01,'(d,x)'),(0x03,'d,s'),(0x05,'d'),(0x07,'[d]'),(0x09,'#M'),(0x0D,'a'),(0x0F,'al'),(0x11,'(d),y'),(0x12,'(d)'),(0x13,'(d,s),y'),(0x15,'d,x'),(0x17,'[d],y'),(0x19,'a,y'),(0x1D,'a,x'),(0x1F,'al,x')]:
    setm([col+0x20*k for k in range(8)], mode)
for o in range(256):
    if o not in M:
        M[o]='i'
special = {0x00:'#8',0x02:'#8',0x04:'d',0x06:'d',0x0C:'a',0x0E:'a',0x14:'d',0x16:'d,x',0x1C:'a',0x1E:'a,x',0x10:'r',0x30:'r',0x50:'r',0x70:'r',0x90:'r',0xB0:'r',0xD0:'r',0xF0:'r',0x80:'r',0x82:'rl',0x20:'a',0x22:'al',0x24:'d',0x2C:'a',0x34:'d,x',0x3C:'a,x',0x89:'#M',0x26:'d',0x2E:'a',0x36:'d,x',0x3E:'a,x',0x44:'mv',0x54:'mv',0x46:'d',0x4E:'a',0x56:'d,x',0x5E:'a,x',0x4C:'a',0x5C:'al',0x62:'rl',0x64:'d',0x66:'d',0x6E:'a',0x74:'d,x',0x76:'d,x',0x7E:'a,x',0x6C:'(a)',0x7C:'(a,x)',0x84:'d',0x86:'d',0x8C:'a',0x8E:'a',0x94:'d,x',0x96:'d,y',0x9C:'a',0x9E:'a,x',0xA0:'#X',0xA2:'#X',0xA4:'d',0xA6:'d',0xAC:'a',0xAE:'a',0xB4:'d,x',0xB6:'d,y',0xBC:'a,x',0xBE:'a,y',0xC0:'#X',0xC2:'#8',0xC4:'d',0xC6:'d',0xCC:'a',0xCE:'a',0xD4:'d',0xD6:'d,x',0xDC:'(a)',0xDE:'a,x',0xE0:'#X',0xE2:'#8',0xE4:'d',0xE6:'d',0xEC:'a',0xEE:'a',0xF4:'a',0xF6:'d,x',0xFC:'(a,x)',0xFE:'a,x',0x0A:'i',0x2A:'i',0x4A:'i',0x6A:'i'}
M.update(special)
SZ={'i':1,'#8':2,'d':2,'(d,x)':2,'d,s':2,'[d]':2,'(d),y':2,'(d)':2,'(d,s),y':2,'d,x':2,'d,y':2,'[d],y':2,'a':3,'a,x':3,'a,y':3,'(a)':3,'(a,x)':3,'al':4,'al,x':4,'r':2,'rl':3,'mv':3}
def run(start,end,m=0,x=0):
    pc=start
    while pc<end:
        o=pc-base; op=d[o]; md=M[op]
        if md=='#M': sz=2 if m else 3
        elif md=='#X': sz=2 if x else 3
        else: sz=SZ[md]
        b=d[o:o+sz]; n=names[op]
        if n=='REP':
            if b[1]&0x20: m=0
            if b[1]&0x10: x=0
        if n=='SEP':
            if b[1]&0x20: m=1
            if b[1]&0x10: x=1
        arg=int.from_bytes(b[1:],'little') if sz>1 else None
        if md=='r': arg=(pc+2+(b[1]-256 if b[1]>127 else b[1]))&0xFFFF
        if md=='rl': arg=(pc+3+int.from_bytes(b[1:],'little',signed=True))&0xFFFF
        a='' if arg is None else ('$%X'%arg)
        print('%04X: %-12s %s %s %s'%(pc,b.hex(' '),n,md.replace('#M','#').replace('#X','#').replace('#8','#'),a))
        pc+=sz
run(int(sys.argv[3],16),int(sys.argv[4],16),int(sys.argv[5]),int(sys.argv[6]))
