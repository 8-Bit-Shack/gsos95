; SFTEST - kleines Testprogramm fuer die Oeffnen/Sichern-Dialoge
; O = SFGetFile2, S = SFPutFile2, Q = Ende. Ergebnis steht unten.
        .setcpu "65816"
        .a16
        .i16
        .segment "CODE"
start:  phk
        plb
        rep #$30
        phk
        phk
        pla
        and #$00FF
        sta bank
        sta ssp+2
        ldx #$0201              ; TLStartUp
        jsl $E10000
        pha
        ldx #$0202              ; MMStartUp
        jsl $E10000
        pla
        sta myid
        pha
        pha
        lda myid
        pha
        pea $0000
        lda bank
        pha
        pea ssrec
        ldx #$1801              ; StartUpTools
        jsl $E10000
        pla
        sta ssref
        pla
        sta ssref+2
        pea $0000
        pea $0000
        ldx #$390E              ; RefreshDesktop
        jsl $E10000
        ldx #$CA04              ; InitCursor
        jsl $E10000
        lda #.loword(t_help)
        ldy #20
        jsr line
loop:   pha
        pea $000A               ; Maus runter, Taste
        lda bank
        pha
        pea evrec
        ldx #$0A06              ; GetNextEvent
        jsl $E10000
        pla
        beq loop
        lda evrec
        cmp #1
        bne @key
        lda evrec+10            ; Klick: oben Oeffnen, unten Sichern
        cmp #100
        bcs @sv
        jsr doopen
        bra loop
@sv:    jsr dosave
        bra loop
@key:   lda evrec+2
        and #$00DF
        cmp #'O'
        bne @1
        jsr doopen
        bra loop
@1:     cmp #'S'
        bne @2
        jsr dosave
        bra loop
@2:     cmp #'Q'
        bne loop
        ; Ende
        lda ssref+2
        pha
        lda ssref
        pha
        pea $0000
        ldx #$1901              ; ShutDownTools(0, ref)
        jsl $E10000
        lda myid
        pha
        ldx #$0302              ; MMShutDown
        jsl $E10000
        ldx #$0301              ; TLShutDown
        jsl $E10000
        lda bank
        pha
        pea quitpb
        pea $2029               ; Quit
        jsl $E100B0
        brk $00

newreply:
        ldx #18
@c:     stz reply,x
        dex
        dex
        bpl @c
        lda #3
        sta reply+8
        sta reply+14
        rts

doopen: lda #.loword(t_call)
        ldy #150
        jsr line
        jsr newreply
        pea 40                  ; x
        pea 30                  ; y
        pea $0000               ; Prompt: Zeiger
        lda bank
        pha
        pea t_pget
        pea $0000               ; kein Filter
        pea $0000
        pea $0000               ; keine Typliste
        pea $0000
        lda bank
        pha
        pea reply
        ldx #$0E17              ; SFGetFile2
        jsl $E10000
        sta err
        jmp show

dosave: lda #.loword(t_call)
        ldy #150
        jsr line
        jsr newreply
        pea 40
        pea 30
        pea $0000
        lda bank
        pha
        pea t_pput
        pea $0000               ; Vorgabename: Zeiger
        lda bank
        pha
        pea orig
        lda bank
        pha
        pea reply
        ldx #$0F17              ; SFPutFile2
        jsl $E10000
        sta err
        jmp show

; Ergebnis anzeigen
show:   lda err
        jsr hex4
        lda reply
        beq @c
        lda #.loword(t_ok)
        bra @s
@c:     lda #.loword(t_can)
@s:     ldy #150
        jsr line
        lda reply
        beq @x
        ; Pfad aus dem Handle (GS/OS-Klasse-1-Ergebnis)
        lda reply+16
        sta $00
        lda reply+18
        sta $02
        ldy #2
        lda [$00],y
        tax
        lda [$00]
        sta $04
        stx $06
        pea 10
        pea 165
        ldx #$3A04              ; MoveTo
        jsl $E10000
        lda $06
        pha
        lda $04
        clc
        adc #4
        pha
        ldy #2
        lda [$04],y
        pha
        ldx #$A704              ; DrawText
        jsl $E10000
        ; Typ/Zusatztyp
        lda reply+2
        jsr hex4
        lda #.loword(t_typ)
        ldy #180
        jsr line
        ; Handles freigeben
        lda reply+12
        pha
        lda reply+10
        pha
        ldx #$1002
        jsl $E10000
        lda reply+18
        pha
        lda reply+16
        pha
        ldx #$1002
        jsl $E10000
@x:     rts

; Zeile Y loeschen und Pascal-String A (danach hexbuf) zeichnen
line:   sta $08
        sty $0A
        lda $0A
        sec
        sbc #10
        sta rect
        lda $0A
        clc
        adc #3
        sta rect+4
        pha
        ldx #$200E              ; GetWMgrPort
        pha
        jsl $E10000
        ldx #$1B04              ; SetPort
        jsl $E10000
        lda bank
        pha
        pea rect
        ldx #$5504              ; EraseRect
        jsl $E10000
        pea 10
        lda $0A
        pha
        ldx #$3A04
        jsl $E10000
        lda bank
        pha
        lda $08
        pha
        ldx #$A504              ; DrawString
        jsl $E10000
        lda bank
        pha
        pea hexbuf
        ldx #$A504
        jsl $E10000
        rts

hex4:   ldx #4
@l:     pha
        and #$000F
        tay
        lda hexd,y
        sep #$20
        .a8
        sta hexbuf+2,x
        dex
        rep #$20
        .a16
        pla
        lsr
        lsr
        lsr
        lsr
        cpx #1
        bcs @l
        rts

myid:   .word 0
bank:   .word 0
err:    .word 0
ssref:  .word 0, 0
ssp:    .word 0, 0
evrec:  .res 16
reply:  .res 20
rect:   .word 0, 0, 0, 640
quitpb: .word 0
hexd:   .byte "0123456789ABCDEF"
hexbuf: .byte .strlen(" $0000"), " $0000"
orig:   .word 8
        .byte "TEST.TXT"
t_help: .byte .strlen("SFTEST: Klick oben/O = Oeffnen, unten/S = Sichern, Q = Ende"), "SFTEST: Klick oben/O = Oeffnen, unten/S = Sichern, Q = Ende"
t_call: .byte .strlen("Aufruf..."), "Aufruf..." 
t_pget: .byte .strlen("Datei zum Oeffnen waehlen"), "Datei zum Oeffnen waehlen"
t_pput: .byte .strlen("Datei sichern als..."), "Datei sichern als..."
t_ok:   .byte .strlen("Ergebnis: OK, Fehler  "), "Ergebnis: OK, Fehler  "
t_can:  .byte .strlen("Ergebnis: Abbruch, Fehler   "), "Ergebnis: Abbruch, Fehler   "
t_typ:  .byte .strlen("Typ   "), "Typ   "
ssrec:  .word 0                 ; Flags
        .word $0080             ; 640er Modus
        .word 0                 ; Ressourcendatei
        .word 0, 0              ; DP-Handle (wird geliefert)
        .word 14
        .word 3, 0, 4, 0, 5, 0, 6, 0, 14, 0, 15, 0, 16, 0, 18, 0
        .word 20, 0, 21, 0, 22, 0, 23, 0, 27, 0, 28, 0
