; ================================================================
; SPLASH95 - Temporaerer Systemzusatz (Typ $B7) in *:System:System.Setup
; Setzt das Startbild *:System:GSOS95.SPL in das Bootbild ein, sobald GS/OS
; bereit ist (START.GS.OS selbst darf GS/OS noch nicht aufrufen).
; Das Bootbild von START.GS.OS ist schon der untere Teil des Startbilds
; (Flaeche, "Created by", Build, Balken); oben ist es noch leer.
; Ablauf ohne Flackern:
;   1. Datei ganz in einen Speicherblock lesen (nichts auf dem Schirm aendert sich)
;   2. Paletten 1..13 setzen - die benutzt das Bootbild nicht
;   3. Zeilen 0..BAND0-1 einzeln kopieren (Punkte, dann SCB): das Bild baut
;      sich von oben nach unten auf, ohne falsche Farben.
; Balkenzeilen, Flaeche und Paletten 0/14/15 sind in beiden Bildern gleich.
; ================================================================
        .setcpu "65816"
        .a16
        .i16
BAND0 = 166                     ; Zeilen 0..165 (bis zum Balken) kommen aus der Datei

; Direct Page von GS/OS fuer Zusaetze: eigene Zeiger in 2 Langworten
SRC = $F0
DST = $F4

        .segment "CODE"
entry:  php
        rep #$30
        phb
        phk
        plb
        phd
        sta uid
        phk
        phk
        pla
        and #$00FF
        sta bk
        ; Bootbild noch zu sehen? (Balkenzeilen im 640er-Modus, unterer Rahmen schwarz)
        lda f:$E19DA6           ; Balkenzeilen mit Palette 0 (640er oder 320er)
        and #$007F
        beq *+5
        jmp done
        lda f:$E12000+171*160+40
        beq *+5
        jmp done
        lda f:$E12000+167*160+30 ; linker Balkenrahmen: $0F leer, $05 gefuellt
        and #$00FF
        bne *+5
        jmp done
        and #$00F0
        beq *+5
        jmp done
        lda f:$E19DFE           ; Bild schon da (SYSTEM:PHOENIX)? Dann nichts zu tun
        cmp #$3539
        bne *+5
        jmp done
        ; Speicherblock 32 KB
        pha
        pha
        pea $0000
        pea $8000
        lda uid
        pha
        pea $C000               ; fest, gesperrt
        pea $0000
        pea $0000
        ldx #$0902              ; NewHandle
        jsl $E10000
        pla
        sta hnd
        pla
        sta hnd+2
        bcc *+5
        jmp done
        tdc                     ; eigene Zeiger in der Direct Page von GS/OS
        sta dpsave
        lda hnd                 ; Griff -> Adresse
        sta SRC
        lda hnd+2
        sta SRC+2
        ldy #2
        lda [SRC],y
        sta rpb+6
        sta bufb
        lda [SRC]
        sta rpb+4
        sta bufa
        ; Datei lesen
        lda #path
        sta opb+4
        lda bk
        sta opb+6
        ldx #opb
        lda #$2010              ; Open
        jsr gs
        bcc *+5
        jmp free
        lda opb+2
        sta rpb+2
        sta cpb+2
        ldx #rpb                ; ganze Datei
        lda #$2012
        jsr gs
        php
        ldx #cpb
        lda #$2014              ; Close
        jsr gs
        plp
        bcc *+5
        jmp free
        lda rpb+12
        cmp #$8000
        beq *+5
        jmp free
        ; Paletten 1..13 (Bootbild benutzt sie nicht)
        lda bufa
        clc
        adc #32256+32
        sta SRC
        lda bufb
        adc #0
        sta SRC+2
        lda #$9E20
        sta DST
        lda #$00E1
        sta DST+2
        ldy #13*32-2
@pl:    lda [SRC],y
        sta [DST],y
        dey
        dey
        bpl @pl
        ; Zeilen 0..BAND0-1: erst die Punkte, dann der SCB
        stz row
@rw:    lda row
        xba                     ; *256
        lsr                     ; *128
        sta tmp
        lsr
        lsr                     ; *32
        clc
        adc tmp                 ; *160
        sta tmp
        clc
        adc bufa
        sta SRC
        lda bufb
        adc #0
        sta SRC+2
        lda tmp
        clc
        adc #$2000
        sta DST
        ldy #158
@px:    lda [SRC],y
        sta [DST],y
        dey
        dey
        bpl @px
        lda bufa                ; SCB der Zeile
        clc
        adc #32000
        sta SRC
        lda bufb
        adc #0
        sta SRC+2
        ldy row
        sep #$20
        .a8
        lda [SRC],y
        tyx
        sta f:$E19D00,x
        rep #$20
        .a16
        inc row
        lda row
        cmp #BAND0
        bcc @rw
free:   lda hnd+2
        pha
        lda hnd
        pha
        ldx #$1002              ; DisposeHandle
        jsl $E10000
done:   pld
        plb
        plp
        clc
        rtl

; GS/OS-Aufruf A mit Parameterblock X (in dieser Bank)
gs:     sta tmp
        lda bk
        pha
        phx
        lda tmp
        pha
        jsl $E100B0
        rts

uid:    .word 0
bk:     .word 0
tmp:    .word 0
row:    .word 0
hnd:    .dword 0
bufa:   .word 0
bufb:   .word 0
dpsave: .word 0
opb:    .word 2, 0
        .dword 0                ; Pfad (zur Laufzeit)
rpb:    .word 4, 0
        .dword 0                ; Puffer (zur Laufzeit)
        .word $8000, 0          ; Laenge 32 KB
        .dword 0                ; gelesen
cpb:    .word 1, 0
path:   .word 19
        .byte "*:System:GSOS95.SPL"
