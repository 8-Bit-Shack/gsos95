# Teilt assets.inc in Tabellen (Codesegment) und Sprites (Asset-Segment)
TAB = {'palettes', 'splash_scb', 'splash_fill', 'note_tab', 'note_tab_en', 'font_tab', 'font_ext', 'digits', 'gmask'}
tab, spr = [], []
cur = tab
for line in open('assets.inc'):
    s = line.rstrip('\n')
    if s.endswith(':') and not s.startswith(' '):
        cur = tab if s[:-1] in TAB else spr
        cur.append(s)
    elif '=' in s and not s.startswith(' '):
        tab.append(s)
    else:
        cur.append(s)
open('assets_tab.inc', 'w').write('\n'.join(tab) + '\n')
open('assets_spr.inc', 'w').write('\n'.join(spr) + '\n')
print(len(tab), len(spr))
