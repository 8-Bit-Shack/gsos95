; ================================================================
; GS/OS95: Startbild sofort beim Start von START.GS.OS
; Wird hinter START.GS.OS angehaengt ($9B79) und ganz am Anfang per JSR
; aufgerufen. Hat PHOENIX das Bild schon gezeigt (Marke "95"), passiert
; nichts. Sonst laedt der Datei-Lader von PRODOS ($2000-Vektor 4, derselbe,
; mit dem START.GS.OS danach GS.OS laedt) *:System:GSOS95.SPL direkt nach
; $E1/2000 und schaltet Super-Hires ein - noch bevor der Balken anlaeuft.
; Danach wird der Speicher hier nicht mehr gebraucht.
; ================================================================
        .setcpu "65816"
        .segment "CODE"
MAGIC = $3539
        .a16
        .i16
early:  php
        rep #$30
        phb
        phk
        plb
        lda f:$E19DFE
        cmp #MAGIC
        beq @done
        pha
        pha
        pha
        pha
        pea 0
        pea path
        pea $00E1
        pea $2000
        ldx #4
        jsr ($2000,x)
        pla
        pla
        pla
        pla
        bcs @done
        lda #MAGIC
        sta f:$E19DFE
        sep #$20
        .a8
        lda f:$E0C029           ; Super-Hires an (linear)
        ora #$C0
        sta f:$E0C029
        rep #$20
        .a16
@done:  plb
        plp
        lda #0                  ; ersetzter Befehl am Einsprung
        sta $7CF2
        rts
path:   .word path_e-path-2
        .byte "SYSTEM:GSOS95.SPL"
path_e:
