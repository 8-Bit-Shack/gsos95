# Nachgebildetes Marinetti (TCP/IP-Toolset $36) fuer die Tests im 65816-Simulator.
# Gegenstellen aus dem Test-"Internet" (w5100sim.Net) ueber netconn.Conn.
import struct
from netconn import Conn

class Marinetti:
    def __init__(s, net, connected=True, can_connect=True, started=True):
        s.net = net
        s.started = started
        s.connected = connected
        s.can_connect = can_connect
        s.ip = bytes([192, 168, 1, 77])
        s.dnr = None               # (adresse, name, poll-zaehler)
        s.conns = {}               # ipid -> dict
        s.next = 1
        s.log = []
    def rdlong(s, cpu, a): return cpu.rd16(a) | cpu.rd16(a + 2) << 16
    def call(s, cpu, x, sp, res):
        f = x >> 8
        s.log.append(f)
        if f == 0x06: return res(0, [1 if s.started else 0, 0])
        if f == 0x02: s.started = True; return 0
        if f == 0x09: return res(0, [1 if s.connected else 0, 0])
        if f == 0x12:
            s.log.append('connect')
            if s.can_connect: s.connected = True
            return 4
        if f == 0x0F: return res(0, list(s.ip))
        if f == 0x1C:
            p = s.rdlong(cpu, sp)
            cpu.mem[p:p + 8] = bytes([10, 0, 0, 53, 0, 0, 0, 0]); return 4
        if f == 0x22: s.poll(cpu); return 0
        if f == 0x21:
            dnr = s.rdlong(cpu, sp); np = s.rdlong(cpu, sp + 4)
            name = bytes(cpu.mem[np + 1:np + 1 + cpu.mem[np]]).decode()
            cpu.wr16(dnr, 0)
            s.dnr = [dnr, name, 3]
            s.net.log.append(('mdnr', name)); return 8
        if f == 0x20: s.dnr = None; return 4
        if f == 0x23:
            port = cpu.rd16(sp + 4); ip = '.'.join(str(b) for b in cpu.mem[sp + 6:sp + 10])
            ipid = s.next; s.next += 1
            s.conns[ipid] = dict(ip=ip, port=port, st=0, c=None, wait=0)
            s.net.log.append(('mlogin', ip, port))
            return res(12, [ipid, 0])
        if f == 0x2C:
            ipid = cpu.rd16(sp); c = s.conns.get(ipid)
            if c is None: return res(2, [0x0E, 0])
            c['st'] = 2; c['wait'] = 4
            return res(2, [0, 0])
        if f == 0x2D:
            ln = s.rdlong(cpu, sp + 4); p = s.rdlong(cpu, sp + 8); ipid = cpu.rd16(sp + 12)
            c = s.conns.get(ipid)
            if c is None or c['c'] is None: return res(14, [0x0E, 0])
            c['c'].send(bytes(cpu.mem[p:p + ln]))
            return res(14, [0, 0])
        if f == 0x2E:
            rr = s.rdlong(cpu, sp); ln = s.rdlong(cpu, sp + 4); p = s.rdlong(cpu, sp + 8)
            ipid = cpu.rd16(sp + 14); c = s.conns.get(ipid)
            assert cpu.rd16(sp + 12) == 0, 'nur buffType 0'
            d = c['c'].take(min(ln, 700)) if c and c['c'] else b''
            cpu.mem[p:p + len(d)] = d
            cpu.mem[rr:rr + 14] = struct.pack('<IIHHH', len(d), 0, 1 if c and c['c'] and c['c'].out else 0, 0, 0)
            return res(16, [0, 0])
        if f in (0x2F, 0x30):
            ipid = cpu.rd16(sp); c = s.conns.get(ipid)
            if c: c['st'] = 10 if f == 0x2F else 0; c['c'] = None
            s.net.log.append(('mclose', ipid))
            return res(2, [0, 0])
        if f == 0x31:
            p = s.rdlong(cpu, sp); ipid = cpu.rd16(sp + 4); c = s.conns.get(ipid)
            if c is None: return res(6, [0x0E, 0])
            q = len(c['c'].out) if c['c'] else 0
            st = c['st']
            if st == 4 and c['c'] and c['c'].done(): st = 7
            cpu.mem[p:p + 22] = struct.pack('<HHIIIHHH', st, 0, 0, q, 0, c['port'], 0, 0)
            return res(6, [0, 0])
        if f == 0x24:
            s.conns.pop(cpu.rd16(sp), None); return 2
        raise Exception('Marinetti-Aufruf %04X' % x)
    def poll(s, cpu):
        if s.dnr:
            s.dnr[2] -= 1
            if s.dnr[2] <= 0:
                a, name, _ = s.dnr; ip = s.net.dns.get(name)
                if ip: cpu.wr16(a, 1); cpu.mem[a + 2:a + 6] = bytes(int(x) for x in ip.split('.'))
                else: cpu.wr16(a, 3)
                s.dnr = None
        for c in s.conns.values():
            if c['st'] == 2:
                c['wait'] -= 1
                if c['wait'] <= 0:
                    if Conn.exists(s.net, c['ip'], c['port']):
                        c['c'] = Conn(s.net, c['ip'], c['port']); c['st'] = 4
                        s.net.log.append(('mconnect', c['ip'], c['port']))
                    else: c['st'] = 0
