; ================================================================
; GS/OS95: Startklang, Bildschirmschoner (Sternenfeld)
; liegt in der zweiten Code-Bank (XCODE), Einstieg ueber xf_*
; ================================================================
        .segment "XCODE"
FARMODE .set 1
xf_boot_chime:  jsr boot_chime
                rtl
xf_snd_down:    jsr snd_down
                rtl
xf_idle_reset:  jsr idle_reset
                rtl
xf_idle_check:  jsr idle_check
                rtl
xf_snd_up:      jsr snd_up
                rtl

; ---------- Startklang ueber den Free-Form-Synthesizer ----------
; nur in den ersten 3 Minuten nach dem Einschalten (nicht nach jedem Programmende)
boot_chime:
        pha
        pha
        TOOL $2503              ; GetTick
        pla
        sta TMP
        pla
        bne @r
        lda TMP
        cmp #10800
        bcs @r
        jmp snd_start
@r:     rts

snd_start:
        lda SNDOFF
        beq @go
        rts
@go:    jsr snd_up
        bcs @r
        lda #snd_wave           ; liegt im Grafiksegment
        sta sndpb
        lda ABANK
        sta sndpb+2
        lda #SNDPAGES
        sta sndpb+4
        lda #214                ; 32*11025/1645 = 11 kHz
        sta sndpb+6
        lda #$8000              ; DOC-RAM ab $8000
        sta sndpb+8
        lda #3                  ; Puffer 2 KB
        sta sndpb+10
        stz sndpb+12
        stz sndpb+14
        lda #200
        sta sndpb+16
        pea $0101               ; Generator 1, Free-Form
        lda MYBANK
        pha
        pea sndpb
        TOOL $0E08              ; FFStartSound
@r:     rts

; Sound Tool bereitstellen (laden und starten, falls noetig). C=1 Fehler
snd_up:
        pha
        TOOL $0608              ; SoundToolStatus
        pla
        bne @ok
        pea $0008               ; Sound Tool laden
        pea $0000
        TOOL $0F01              ; LoadOneTool
        bcs @e
        pha                     ; Direct Page (1 Seite, Bank 0, seitenbuendig)
        pha
        pea $0000
        pea $0100
        lda USERID
        pha
        pea $C005
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        pla
        sta HND
        pla
        sta HND+2
        bcs @e
        lda [HND]
        pha
        TOOL $0208              ; SoundStartUp
        bcs @e
        lda #1
        sta SNDMINE
@ok:    clc
        rts
@e:     sec
        rts

snd_down:
        lda SNDMINE
        beq @r
        stz SNDMINE
        TOOL $0308              ; SoundShutDown
@r:     rts

; ---------- Leerlauf messen ----------
; Benutzer hat etwas getan
idle_reset:
        pha
        pha
        TOOL $2503              ; GetTick
        pla
        sta IDLET
        pla
        sta IDLET+2
        rts

; im Hauptprogramm aufgerufen: Schoner starten, wenn lange nichts passiert
idle_check:
        lda SSMIN
        beq @r
        lda NDAWAS
        ora MENU
        ora DROP
        bne @r
        pha
        pha
        TOOL $2503              ; GetTick
        pla
        sec
        sbc IDLET
        sta TMP
        pla
        sbc IDLET+2
        sta TMP2                ; vergangene Ticks (60 pro Sekunde)
        lda SSMIN
        ldx #3600
        CALL gmul                ; Grenze (bis 18 Minuten passen in 16 Bit)
        ldx TMP2
        bne @go
        cmp TMP
        beq @go
        bcs @r
@go:    lda SSALT               ; abwechselnd Linienspiel und Sternenfeld
        eor #1
        sta SSALT
        beq @s1
        JSLF xf_ss_myst
        bra @s2
@s1:    jsr saver
@s2:    jsr idle_reset
@r:     rts

; ---------- Sternenfeld ----------
NSTAR = 24
saver:
        CALL cur_hide
        CALL org0
        CALL clip_full
        lda #320
        sta CLIPR
        FILL 0,0,320,200,BLACK
        ldx #NSTAR*2-2
@i:     phx
        jsr star_new
        plx
        dex
        dex
        bpl @i
        lda EVREC+12
        sta SVX
        lda EVREC+10
        sta SVY
@loop:  CALL waitvbl
        ldx #NSTAR*2-2
@s:     stx STK
        ; alten Punkt loeschen
        lda #BLACK
        CALL setcolor
        ldx STK
        jsr star_xy
        bcs @n1
        CALL setpixel_raw
@n1:    ldx STK
        lda stard,x             ; Abstand wachsen lassen (schneller nach aussen)
        lsr
        lsr
        lsr
        clc
        adc #2
        adc stard,x
        sta stard,x
        jsr star_xy
        bcc @in
        ldx STK
        jsr star_new
        bra @nx
@in:    phx
        phy
        ldx STK
        lda stard,x
        cmp #$0C00
        lda #GRAY
        bcc @c
        lda #WHITE
@c:     CALL setcolor
        ply
        plx
        CALL setpixel_raw
@nx:    ldx STK
        dex
        dex
        bpl @s
        ; Ende bei Taste, Klick oder Mausbewegung
        pha
        pea $002A
        pea $0000
        lda EVPTR
        pha
        TOOL $0A06              ; GetNextEvent
        pla
        bne @end
        lda EVREC+12
        cmp SVX
        bne @end
        lda EVREC+10
        cmp SVY
        bne @end
        jmp @loop
@end:   CALL orgr
        CALL redraw_all
        CALL cur_show
        rts

; Stern X neu: zufaellige Richtung, Abstand klein
star_new:
        jsr rnd
        and #$01FF
        sec
        sbc #$0100
        sta stardx,x
        jsr rnd
        and #$01FF
        sec
        sbc #$0100
        sta stardy,x
        lda #$0100
        sta stard,x
        rts

; Bildschirmposition von Stern X -> X/Y, C=1 ausserhalb
star_xy:
        lda stard,x
        xba
        and #$00FF
        sta SVD                 ; Abstand (ganzzahlig)
        lda stardx,x
        jsr svmul
        clc
        adc #160
        cmp #320
        bcs @out
        pha
        lda stardy,x
        jsr svmul
        clc
        adc #93
        cmp #186
        bcs @out2
        tay
        plx
        clc
        rts
@out2:  pla
@out:   sec
        rts

; A (Richtung -256..255) * SVD / 64 -> A (vorzeichenbehaftet)
svmul:  phx
        sta SVA
        bpl @p
        eor #$FFFF
        inc a
@p:     ldx SVD
        CALL gmul
        lsr
        lsr
        lsr
        lsr
        lsr
        lsr
        ldx SVA
        bpl @r
        eor #$FFFF
        inc a
@r:     plx
        rts

; 16-Bit-Zufallszahl (Xorshift)
rnd:    lda RNDS
        bne @n
        lda #$ACE1
@n:     asl
        eor RNDS
        sta RNDS
        lsr
        lsr
        lsr
        eor RNDS
        sta RNDS
        xba
        asl
        eor RNDS
        sta RNDS
        rts

FARMODE .set 0
        .segment "CODE"
