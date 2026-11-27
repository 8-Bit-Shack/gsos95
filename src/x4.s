; ================================================================
; GS/OS95: fuenfte Codebank (XCODE4, Segment 5 "CODE4")
; Platz fuer groessere Programme. Aufrufe von aussen mit JSLF xf_...,
; Hauptbank-Routinen mit CALL (Sprungstellen tp_ in tramp.s).
; Enthaelt: MS-DOS-Eingabeaufforderung (dos.inc), SSH ueber FujiNet (ssh.inc),
;           Bildschirmschoner Linienspiel (ssv.inc), Paint (paint.inc),
;           Bluescreen Wahl-Apfel-C (bsod.inc)
; ================================================================
        .segment "XCODE4"
FARMODE .set 4

; ---------- kleine Helfer dieser Bank ----------
x4_put: JSLF xf_vt_put          ; Zeichen A ins Vollbild-Terminal
        rts
x4_cur: JSLF xf_vt_cur          ; Cursor an (A=1) / aus (A=0)
        rts
x4_nl:  lda #13
        jsr x4_put
        lda #10
        jmp x4_put
x4_kb:  sta LP2                 ; LP2 = A in dieser Bank
        phk
        phk
        pla
        and #$00FF
        sta LP2+2
        rts

; Fensterarten 27 (Datum/Uhrzeit) und 28 (Editor)
xf_x4_draw:
        lda WIN
        cmp #28
        beq @e
        jsr xf_cal_draw
        rtl
@e:     jsr xf_ed_draw
        rtl
xf_x4_click:
        lda WIN
        cmp #28
        beq @e
        jmp xf_cal_click
@e:     jmp xf_ed_click
xf_x4_key:
        lda WIN
        cmp #28
        beq @e
        jmp xf_cal_key
@e:     jmp xf_ed_key

        .include "dos.inc"
        .include "ssh.inc"
        .include "ssv.inc"
        .include "paint.inc"
        .include "bsod.inc"
        .include "lz.inc"
        .include "ftp.inc"
        .include "cal.inc"
        .include "ed.inc"
        .include "deskpat.s"

FARMODE .set 0
        .segment "CODE"
