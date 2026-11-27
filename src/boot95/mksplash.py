# GS/OS95-Startbild nach Daniels Entwurf (GS.OS.9..00.PNG, 320x200, Echtfarben)
# -> *:System:GSOS95.SPL ($C1/$0000, 32768 Bytes: Bild, SCBs, 16 Paletten).
# START.GS.OS und GSOS95 lesen die Datei direkt nach $E1/2000.
# Aufbau:  Zeilen 0..140      Bild (verkleinert), darunter Wolken bis zum Rand
#                             (alles Palette 1), "Created by ..." und Build darauf
#          166..171           Ladebalken (320er-Modus, Palette 0: 0/5/10/15 fuer den
#                             Balken, sonst Wolkenfarben)
#          172..199           Dunst in einer Farbe (Palette 15): die Symbole
#                             der Systemerweiterungen beim Booten bleiben unsichtbar
import os, re, sys
import numpy as np
from PIL import Image
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, '..')
SRC = os.path.join(HERE, 'GS.OS.9..00.PNG')
BUILD = open(os.path.join(ROOT, 'BUILD')).read().strip()
BAR0, BAR1 = 166, 171
HAZE0 = 172                     # ab hier Dunst (Symbole der Systemerweiterungen)
PIC_H = 141                     # Hoehe des verkleinerten Bildes
FADE = 9                        # Zeilen Uebergang Bild -> Flaeche
BAND0 = PIC_H
NPAL = 1                        # eine Palette (1) fuer Bild, Flaeche und Schrift

def q4(c):                      # 8 Bit -> 4 Bit je Kanal
    return np.clip(np.round(np.asarray(c, float) / 17.0), 0, 15).astype(int)

MC = (228, 2, 316, 20)          # "Marcrosoft" oben rechts (x0, y0, x1, y1)

def compose():
    import cv2
    src = Image.open(SRC).convert('RGB')
    orig = np.asarray(src, np.uint8)
    # Schriftzug "Marcrosoft": dunkle Punkte im Rechteck
    lum = orig.astype(float) @ [0.299, 0.587, 0.114]
    mc = np.zeros((200, 320), np.uint8)
    x0, y0, x1, y1 = MC
    mc[y0:y1, x0:x1] = (lum[y0:y1, x0:x1] < 150)
    mc = cv2.dilate(mc, np.ones((3, 3), np.uint8))
    # fuer das verkleinerte Bild: Schriftzug wegretuschieren
    clean = cv2.inpaint(orig, mc, 5, cv2.INPAINT_TELEA)
    s = PIC_H / 200.0
    w = int(round(320 * s))
    small = np.asarray(Image.fromarray(clean).resize((w, PIC_H), Image.LANCZOS), float)
    out = orig[:PIC_H].astype(float).copy()       # Raender: Wolken in voller Groesse
    x0s = (320 - w) // 2
    m = np.ones(w)
    r = 14
    m[:r] = np.linspace(0, 1, r); m[-r:] = np.linspace(1, 0, r)
    out[:, x0s:x0s + w] = out[:, x0s:x0s + w] * (1 - m[None, :, None]) + small * m[None, :, None]
    # Schriftzug in Originalgroesse an seinen Platz
    sel = mc[:PIC_H].astype(bool)
    out[sel] = orig[:PIC_H][sel]
    # unten: Wolken des Originals bis zum Rand; der Schriftzug "GS/OS 95" wird
    # durch Wolken von rechts oben ersetzt (Poisson-Einblendung, ohne Kanten)
    # erst das Apfel-Logo (kraeftige Farben) unten wegretuschieren, damit beim
    # Einblenden keine Logofarben in die Wolken laufen
    o = orig.astype(float)
    sat = (o.max(2) - o.min(2)) / 255.0
    am = np.zeros((200, 320), np.uint8)
    am[90:150] = sat[90:150] > 0.25
    am = cv2.dilate(am, np.ones((7, 7), np.uint8))
    base = cv2.inpaint(clean, am, 9, cv2.INPAINT_TELEA)
    patch = cv2.resize(clean[34:114, 225:320], (216, 80), interpolation=cv2.INTER_CUBIC)
    low = cv2.seamlessClone(patch, base, 255 * np.ones(patch.shape[:2], np.uint8),
                            (172, 152), cv2.NORMAL_CLONE).astype(float)
    full = low.copy()
    full[:PIC_H] = out
    for i in range(FADE):                     # Uebergang verkleinertes Bild -> Wolken
        a = (i + 1) / (FADE + 1)
        y = PIC_H - FADE + i
        full[y] = out[y] * (1 - a) + low[y] * a
    # Flaechenfarbe (Balken innen, Ersatzbild ohne Datei): hellblau vom Bildrand
    band = q4(out[PIC_H - 6:].reshape(-1, 3).mean(0) * 0.97)
    return full, band

BY_Y = BAND0 + 2               # "Created by Marc Golombeck"
BUILD_Y = BAND0 + 13           # "Build v..."
def pal14(bandc): return [bandc, 0x013, 0xFFF] + [bandc] * 13
def pal0(bandc): return [0x000, 0xFFF, 0x00A, bandc] * 4
def band_color():
    _, band = compose()
    return int(band[0]) << 8 | int(band[1]) << 4 | int(band[2])

def kmeans(px, k, it=12, seed=1):
    rng = np.random.default_rng(seed)
    if len(px) <= k:
        c = np.vstack([px, np.repeat(px[:1], k - len(px), 0)])
        return c
    c = px[rng.choice(len(px), k, replace=False)].copy()
    for _ in range(it):
        d = ((px[:, None, :] - c[None, :, :]) ** 2).sum(2)
        a = d.argmin(1)
        for j in range(k):
            sel = px[a == j]
            if len(sel): c[j] = sel.mean(0)
            else: c[j] = px[rng.integers(len(px))]
    return c

def pal_snap(c):
    return q4(c)

def row_err(row, pal8):
    d = ((row[:, None, :] - pal8[None, :, :]) ** 2).sum(2)
    return d.min(1).sum()

def seg_pal(img, rs, seed):
    """Eine Palette fuer alles bis zum Balken: Schwarz, Weiss, Flaeche, Schrift,
    5 Logofarben (kraeftige Punkte), 7 Himmel-/Wolkenfarben. Nur EINE Palette:
    GSSquared liest die Palette je Zeile in Echtzeit - wechselnde Paletten
    von Zeile zu Zeile zeigten dort flackernde Streifen."""
    px = img[rs].reshape(-1, 3)
    sat = (px.max(1) - px.min(1)) / 255.0
    rng = np.random.default_rng(seed)
    logo = px[sat > 0.38]
    sky = px[(sat <= 0.38) & (px.max(1) > 60)]
    parts = [np.array(FIXED, float)]
    nl = 5 if len(logo) > 200 else 0
    if nl:
        lp = logo[rng.choice(len(logo), min(4000, len(logo)), replace=False)]
        best = None
        for sd in range(12):                  # mehrere Starts: Logofarben nicht verschmelzen
            c = pal_snap(kmeans(lp, nl, seed=sd)) * 17.0
            e = ((lp[:, None, :] - c[None, :, :]) ** 2).sum(2).min(1).sum()
            if best is None or e < best[0]: best = (e, c)
        parts.append(best[1])
    sp = sky[rng.choice(len(sky), min(5000, len(sky)), replace=False)]
    parts.append(pal_snap(kmeans(sp, 16 - len(FIXED) - nl, seed=seed + 1)) * 17.0)
    pal = np.vstack(parts)
    err = sum(row_err(img[y], pal) for y in rs)
    return pal, err

def quantize(img, rows, npal):
    """Paletten fuer zusammenhaengende Zeilenbaender (wie beim Farbverlauf):
    jede Palette gilt fuer einen Block aufeinanderfolgender Zeilen, die Grenzen
    werden verschoben, bis der Fehler nicht mehr sinkt. Kein Hin- und Herspringen
    zwischen Paletten von Zeile zu Zeile."""
    rows = list(rows)
    n = len(rows)
    b = [round(i * n / npal) for i in range(npal + 1)]       # Grenzen
    cache = {}
    def seg(i):
        key = (b[i], b[i + 1])
        if key not in cache:
            cache[key] = seg_pal(img, rows[b[i]:b[i + 1]], 7 + b[i])
        return cache[key]
    for it in range(6):
        moved = False
        for k in range(1, npal):
            best = (seg(k - 1)[1] + seg(k)[1], b[k])
            orig = b[k]
            for d in (-4, -2, -1, 1, 2, 4):
                nb = orig + d
                if nb <= b[k - 1] + 2 or nb >= b[k + 1] - 2: continue
                b[k] = nb
                e = seg(k - 1)[1] + seg(k)[1]
                if e < best[0]: best = (e, nb)
            b[k] = best[1]
            if best[1] != orig: moved = True
        if not moved: break
    pals = [seg(i)[0] for i in range(npal)]
    assign = []
    for i in range(npal):
        assign += [i] * (b[i + 1] - b[i])
    return pals, assign

def dither(img, rowpal):
    """Floyd-Steinberg, je Zeile mit der Palette der Zeile (rowpal[y] = 16x3 oder None)"""
    work = img.copy()
    idx = np.zeros((200, 320), int)
    for y in range(200):
        pl = rowpal[y]
        if pl is None: continue
        for x in range(320):
            o = work[y, x]
            d = ((pl - o) ** 2).sum(1)
            j = int(d.argmin()); idx[y, x] = j
            e = np.clip((o - pl[j]) * 0.8, -28, 28)
            if x + 1 < 320: work[y, x + 1] += e * 7 / 16
            if y + 1 < 200 and rowpal[y + 1] is not None:
                if x > 0: work[y + 1, x - 1] += e * 3 / 16
                work[y + 1, x] += e * 5 / 16
                if x + 1 < 320: work[y + 1, x + 1] += e * 1 / 16
    return idx

def spr(name):
    txt = open(os.path.join(ROOT, 'assets_spr.inc')).read()
    m = re.search(r'^%s:\n((?:\s*\.byte .*\n)+)' % re.escape(name), txt, re.M)
    vals = []
    for l in m.group(1).strip().split('\n'):
        vals += [int(x.strip()[1:], 16) for x in l.split('.byte')[1].split(',')]
    return vals

def stamp(idx, name, cx, y, col):
    s = spr(name); w, h = s[0], s[1]; bpr = (w + 7) // 8
    x0 = cx - w // 2
    for r in range(h):
        for c in range(w):
            if (s[2 + r * bpr + c // 8] >> (7 - c % 8)) & 1:
                idx[y + r, x0 + c] = col

FIXED = []
I_BAND, I_TEXT = 3, 4
I_SPARE = 1                     # Eintrag 1 bleibt unbenutzt (wird beim Booten in GSSquared ueberschrieben)
def build():
    img, band = compose()
    FIXED[:] = [[0, 0, 0], list(band * 17.0), [255, 255, 255], list(band * 17.0), [0, 17, 51]]   # 1 = Platzhalter, $013 = Schrift
    # unten (ab HAZE0): Dunst in einer Farbe (Palette 15). Dort
    # zeichnet GS/OS beim Laden die Symbole der Systemerweiterungen - so bleiben
    # sie praktisch unsichtbar. Darueber laufen die Wolken weich in den Dunst aus.
    hm = img[HAZE0:].reshape(-1, 3).mean(0)
    haze = [q4(hm) * 17.0] * 2                # eine Farbe: Symbole ganz unsichtbar
    for y in range(HAZE0 - 44, 200):
        t = min(1.0, (y - (HAZE0 - 44)) / 44.0)
        a = (t * t * (3 - 2 * t)) * 0.95
        img[y] = img[y] * (1 - a) + hm * a
    photo_rows = [y for y in range(HAZE0) if not BAR0 <= y <= BAR1]
    pals, assign = quantize(img, photo_rows, NPAL)
    rowpal = [None] * 200
    scb = [0] * 200
    for i, y in enumerate(photo_rows):
        dp = pals[assign[i]].copy()
        dp[I_SPARE] = [9999, 9999, 9999]      # beim Rastern nie Eintrag 1 waehlen
        rowpal[y] = dp; scb[y] = 1 + assign[i]
    bandc = int(band[0]) << 8 | int(band[1]) << 4 | int(band[2])
    # Palette 14: Flaeche (0), Schrift dunkelblau (1), Schatten/heller Rand (2)
    p14 = pal14(bandc)
    p15 = [bandc] * 16
    # Balkenzeilen: 320er-Modus, Palette 0. Der Balken (START.GS.OS, GSOS95) schreibt
    # 640er-Muster aus reinen Halbbytes $0/$5/$A/$F: Eintrag 0 schwarz, 5 weiss,
    # 10 blau, 15 Flaeche - sieht damit aus wie im 640er-Modus. Die uebrigen 12
    # Eintraege sind Wolkenfarben fuer die Zeilen links und rechts vom Balken.
    brow = img[BAR0:BAR1 + 1].reshape(-1, 3)
    rng = np.random.default_rng(3)
    cl = pal_snap(kmeans(brow[rng.choice(len(brow), 1500, replace=False)], 12, seed=3)) * 17.0
    p0f = np.zeros((16, 3)); free = [i for i in range(16) if i not in (0, 5, 10, 15)]
    p0f[free] = cl
    p0f[0] = [0, 0, 0]; p0f[5] = [255, 255, 255]; p0f[10] = [0, 0, 170]; p0f[15] = q4(band * 17.0 * 0.72) * 17.0   # Balken innen etwas dunkler: weisse Fuellung hebt sich ab
    dp = p0f.copy(); dp[0] = dp[10] = [9999, 9999, 9999]    # Wolken nie schwarz/blau
    for y in range(BAR0, BAR1 + 1):
        rowpal[y] = dp; scb[y] = 0
    hp = np.array([haze[i % 2] for i in range(16)])
    for y in range(HAZE0, 200):
        rowpal[y] = hp; scb[y] = 15
    idx = dither(img, rowpal)
    idx[BAR0:BAR1 + 1, 60:262] = 15          # Balken innen: Eintrag 15 (Rahmen zeichnet START.GS.OS)
    # "Created by" und Build: dunkelblau mit hellem Rand auf den Wolken
    for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1)):
        stamp(idx, 't_by', 160 + dx, BY_Y + dy, 2)
        stamp(idx, 't_build', 160 + dx, BUILD_Y + dy, 2)
    stamp(idx, 't_by', 160, BY_Y, I_TEXT)
    stamp(idx, 't_build', 160, BUILD_Y, I_TEXT)
    p0 = [(int(c[0]) // 17) << 8 | (int(c[1]) // 17) << 4 | int(c[2]) // 17 for c in p0f]
    pal12 = [p0]
    for p in pals:
        pal12.append([(int(c[0]) // 17) << 8 | (int(c[1]) // 17) << 4 | int(c[2]) // 17 for c in p])
    pal12 += [[bandc] * 16] * (14 - len(pal12))
    p15 = [(int(c[0]) // 17) << 8 | (int(c[1]) // 17) << 4 | int(c[2]) // 17 for c in hp]
    pal12 += [p14, p15]
    assert len(pal12) == 16
    pix = bytearray(32000)
    for y in range(200):
        for x in range(0, 320, 2):
            pix[y * 160 + x // 2] = (idx[y, x] & 15) << 4 | (idx[y, x + 1] & 15)
    data = bytes(pix) + bytes(scb) + bytes(56) + b''.join(
        b''.join(c.to_bytes(2, 'little') for c in p) for p in pal12)
    assert len(data) == 32768
    return data

def render(data, fn, scale=3):
    im = Image.new('RGB', (320, 200))
    px = im.load()
    for y in range(200):
        s = data[32000 + y]; p = s & 15
        pal = [int.from_bytes(data[32256 + p * 32 + 2 * i:32258 + p * 32 + 2 * i], 'little') for i in range(16)]
        for x in range(320):
            if s & 0x80:            # 640er: 2 Punkte je Bildpunkt, hier linker Punkt
                b = data[y * 160 + x // 2]
                v = (b >> (6 - 2 * ((x & 1) * 2))) & 3
                c = pal[(v + 8) if (x & 1) == 0 else v + 12]
            else:
                v = (data[y * 160 + x // 2] >> (0 if x & 1 else 4)) & 15
                c = pal[v]
            px[x, y] = (((c >> 8) & 15) * 17, ((c >> 4) & 15) * 17, (c & 15) * 17)
    im.resize((320 * scale, 200 * scale), Image.NEAREST).save(fn)

def bake_frame(data):
    """Balkenrahmen so einzeichnen, wie ihn START.GS.OS zeichnet ($77E6, System 6.1)"""
    sys.path.insert(0, ROOT)
    from emu import CPU, run
    d = open(os.path.join(ROOT, 'sys', 'START.GS.OS.95'), 'rb').read()
    c = CPU(None); c.mem[0x6800:0x6800 + len(d)] = d
    c.mem[0xE12000:0xE1A000] = data
    drv = [0x20, 0xE6, 0x77, 0x80, 0xFE]
    c.mem[0x0300:0x0300 + len(drv)] = bytes(drv)
    c.E = 0; c.M = 0; c.XF = 0; c.D = 0x0800; c.S = 0x01FF; c.DBR = 0; c.PBR = 0; c.PC = 0x0300
    c.mem[0x0802] = 0xE1
    run(c, 5_000_000, stop_at=0x0303)
    return bytes(c.mem[0xE12000:0xE1A000])

if __name__ == '__main__':
    d = bake_frame(build())
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, 'sys', 'GSOS95.SPL')
    open(out, 'wb').write(d)
    if len(sys.argv) > 2: render(d, sys.argv[2])
    print('GSOS95.SPL', len(d), 'Build', BUILD)
