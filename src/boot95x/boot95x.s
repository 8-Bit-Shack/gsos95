; ================================================================
; GS/OS95: SYSTEM:BOOT95 - Startbild sofort beim Einschalten
; PRODOS laedt diese Datei nach $01/1000 und ruft sie mit JSL $01/1004
; auf, bevor START.GS.OS geladen wird (Kopf "PHX1", ab $1007 die
; Dateilaenge). Hier: GSOS95.SPL (Punkte, SCBs, Paletten) direkt in den
; Super-Hires-Speicher kopieren, Marke "95" setzen, Super-Hires einschalten.
; START.GS.OS loescht dann nichts mehr; der Ladebalken laeuft darueber.
; ================================================================
        .setcpu "65816"
        .segment "CODE"
MAGIC = $3539
        .byte "PHX1"
        brl start               ; $1004
        .word fend-$1000        ; $1007: Dateilaenge
start:  php
        phb
        rep #$30
        .i16
        sep #$20
        .a8
        lda f:$E0C029           ; waehrend des Kopierens: Super-Hires aus
        and #$7F
        sta f:$E0C029
        rep #$20
        .a16
        ldx #.loword(pic)       ; Bild nach $E1/2000-$E1/9FFF
        ldy #$2000
        lda #$7FFF
        mvn #$01,#$E1
        lda #MAGIC
        sta f:$E19DFE
        sep #$20
        .a8
        lda f:$E0C029           ; Super-Hires an (linear)
        ora #$C0
        sta f:$E0C029
        rep #$20
        .a16
        plb
        plp
        rtl
pic:    .incbin "../sys/GSOS95.SPL"
fend:
