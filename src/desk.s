; ================================================================
; GS/OS95: Schreibtischzubehoer (NDAs) und Kontrollfelder
; Startet bei Bedarf Window-, Menu-, Control-, Dialog- und Desk-Manager.
; Der Window Manager zeichnet den Schreibtisch ueber desk_cb, d.h. die
; komplette GS/OS95-Oberflaeche liegt "unter" den NDA-Fenstern.
; ================================================================
NDAMAX  = 12
NDANL   = 32                    ; Bytes pro Name (Laengenbyte + Text)

; ---------- Werkzeuge starten (einmalig). C=1 bei Fehler ----------
desk_init:
        lda NDAON
        beq @go
        clc
        rts
@go:    lda MYBANK
        pha
        pea toolstab
        TOOL $0E01              ; LoadTools
        bcc @lt
        sta ERRC
        sec
        rts
@lt:    ; 4 Seiten Direct Page in Bank 0
        pha
        pha
        pea $0000
        pea $0500
        lda USERID
        pha
        pea $C015
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        pla
        sta DSKH
        pla
        sta DSKH+2
        bcc @h
        sta ERRC
        sec
        rts
@h:     lda DSKH
        sta LP
        lda DSKH+2
        sta LP+2
        lda [LP]
        sta DSKDP
        jsr off_alloc           ; (veraendert A)
        lda DSKDP
        clc
        adc #$0400
        sta SCRDP               ; ab jetzt laufen Toolbox-Aufrufe (und NDAs) hier
        TOOL $0212              ; QDAuxStartUp
        lda USERID
        pha
        TOOL $020E              ; WindStartUp
        jsr desk_own
        lda USERID
        pha
        lda DSKDP
        pha
        TOOL $020F              ; MenuStartUp
        lda USERID
        pha
        lda DSKDP
        clc
        adc #$0100
        pha
        TOOL $0210              ; CtlStartUp
        lda USERID
        pha
        lda DSKDP
        clc
        adc #$0200
        pha
        TOOL $0214              ; LEStartUp
        lda USERID
        pha
        TOOL $0215              ; DialogStartUp
        TOOL $0216              ; ScrapStartUp
        lda USERID
        pha
        lda DSKDP
        clc
        adc #$0300
        pha
        TOOL $021B              ; FMStartUp
        TOOL $021C              ; ListStartUp
        TOOL $0205              ; DeskStartUp
        jsr desk_own            ; NDAs wie DeskMaker koennen den Schreibtisch umbiegen
        lda #1
        sta NDAON
        ; unsichtbares Apple-Menue, damit die Namen der NDAs bekannt werden
        pha
        pha
        lda MYBANK
        pha
        pea amenu
        TOOL $2D0F              ; NewMenu
        pla
        sta MNUH
        pla
        sta MNUH+2
        lda MNUH+2
        pha
        lda MNUH
        pha
        pea 0
        TOOL $0D0F              ; InsertMenu
        pea 1
        TOOL $1E05              ; FixAppleMenu
        jsr desk_names
        clc
        rts

toolstab: .word 10
        .word $05,0, $0E,0, $0F,0, $10,0, $12,0
        .word $14,0, $15,0, $16,0, $1B,0, $1C,0

; Namen aller NDAs nach ndanames (Pascal-Strings, nur druckbare Zeichen)
desk_names:
        pha
        TOOL $1B05              ; GetNumNDAs
        pla
        cmp #NDAMAX+1
        bcc @n
        lda #NDAMAX
@n:     sta NDAN
        stz NDAI
@l:     lda NDAI
        cmp NDAN
        bcc @g
        rts
@g:     pha
        pha
        lda NDAI
        inc a
        pha
        TOOL $250F              ; GetMItem
        pla
        sta LP
        pla
        sta LP+2
        ; Ziel: ndanames + NDAI*32
        lda NDAI
        asl
        asl
        asl
        asl
        asl
        clc
        adc #ndanames
        sta PB
        lda LP
        ora LP+2
        bne @s
        sep #$20
        lda #1
        sta (PB)
        ldy #1
        lda #'?'
        sta (PB),y
        rep #$20
        bra @nx
@s:     sep #$20
        lda [LP]
        cmp #NDANL-1
        bcc @ln
        lda #NDANL-1
@ln:    sta (PB)
        rep #$20
        and #$00FF              ; oberes Byte loeschen, sonst kopiert TAX Unsinn
        tax
        sep #$20
        ldy #1
@c:     cpx #0
        beq @e
        lda [LP],y
        cmp #' '
        bcc @q
        cmp #$7F
        bcc @ok
@q:     lda #' '
@ok:    sta (PB),y
        iny
        dex
        bra @c
@e:     rep #$20
@nx:    inc NDAI
        jmp @l

; GS/OS95 zeichnet den Schreibtisch selbst (SetDeskPat mit Zeichenroutine)
desk_own:
        pha
        pha
        pea 5                   ; SetDeskPat
        lda MYBANK
        and #$00FF              ; $00 im obersten Byte = Zeichenroutine
        pha
        pea desk_cb
        TOOL $0C0E              ; Desktop
        pla
        pla
        rts

; ---------- Schreibtischzeichnen durch den Window Manager ----------
desk_cb:
        php
        rep #$30
        pha                     ; A, X, Y fuer den Window Manager erhalten
        phx
        phy
        phb
        phd
        phk
        plb
        lda MYDP
        tcd
        lda #1
        sta INCB
        lda KL                  ; immer die ganze Flaeche aufbauen
        pha
        lda KT
        pha
        lda KR
        pha
        lda KB
        pha
        jsr clip_full
        JSLF xf_set_palettes        ; NDAs koennen Paletten/SCBs veraendert haben
        jsr desktop_scb
        lda NDAWAS
        beq @dir
        jsr off_paint           ; NDA-Fenster offen: nur freie Flaeche kopieren
        bra @e
@dir:   jsr redraw_core
@e:     pla
        sta KB
        pla
        sta KR
        pla
        sta KT
        pla
        sta KL
        stz INCB
        pld
        plb
        ply
        plx
        pla
        plp
        rtl

; ---------- Zeichnen in einen Puffer + Kopie durch das Clipping ----------
off_alloc:
        lda OFFB
        beq @new
        rts
@new:   pha
        pha
        pea $0000
        pea 32000
        lda USERID
        pha
        pea $C010               ; fest, gesperrt, nicht ueber Bankgrenze
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        pla
        sta LP2
        pla
        sta LP2+2
        bcc @ok
        rts                     ; kein Speicher: direkt zeichnen (OFFB=0)
@ok:    ldy #2
        lda [LP2],y
        sta OFFB
        sta offloc+4
        lda [LP2]
        sta OFFA
        sta offloc+2
        rts

; Bildschirm (E1/2000) bzw. Puffer als Ziel aller Zeichenroutinen
scr_set:                        ; A = Startadresse, X = Bank
        stx PTR+2
        ldx #0
@l:     sta linetab,x
        clc
        adc #160
        inx
        inx
        cpx #400
        bcc @l
        rts
scr_off:
        lda OFFA
        ldx OFFB
        bra scr_set
scr_on: lda #$2000
        ldx #$00E1
        bra scr_set

off_paint:
        lda OFFB
        bne @p
        jmp redraw_core
@p:     jsr scr_off
        lda #1
        sta OFFON
        jsr redraw_core
        stz OFFON
        jsr scr_on
        lda MYBANK
        pha
        pea offloc
        lda MYBANK
        pha
        pea offrect
        pea 0
        pea 0
        pea 0                   ; srcCopy
        TOOL $D604              ; PPToPort (beachtet das Clipping des WM)
        rts

; Esc: vorderstes Zubehoer-Fenster schliessen
nda_esc:
        jsr front_nda
        bne @c
        rts
@c:     lda TMPK
        pha
        lda FNL
        pha
        TOOL $1C05              ; CloseNDAByWinPtr
        lda #1
        sta NEEDREF
        jmp nda_style

; vorderstes Fenster des Window Managers -> A|X (0 = keines)
front_nda:
        pha
        pha
        TOOL $150E              ; FrontWindow
        pla
        sta FNL
        plx
        stx TMPK
        ora TMPK
        rts

; ---------- in der Hauptschleife ----------
; vor der eigenen Ereignisbehandlung: C=1 wenn das Ereignis einem NDA gehoerte
desk_event:
        lda NDAON
        bne @on
        jsr nda_curoff          ; kein Zubehoer mehr: wieder unser Zeiger
        clc
        rts
@on:    TOOL $1905              ; SystemTask
        jsr nda_cursor
        lda EVREC
        cmp #1
        beq @md
        cmp #3                  ; Taste / Tastenwiederholung
        beq @key
        cmp #5
        beq @key
        clc
        rts
@key:   jmp nda_keyev
@md:    pha
        lda MYBANK
        pha
        pea fwptr
        lda EVREC+12
        pha
        lda EVREC+10
        pha
        TOOL $170E              ; FindWindow
        pla
        sta FWCODE
        bmi @sys
        lda NDAFOC              ; Klick auf GS/OS95: Tasten wieder fuer uns
        beq @same
        stz NDAFOC
        JSLF xf_foc_paint           ; Titelleiste/Taskleiste wieder aktiv zeichnen
@same:  clc
        rts
@sys:   lda NDAFOC              ; Klick in ein Zubehoer: Tasten gehen dorthin
        bne @foc
        lda #1
        sta NDAFOC
        JSLF xf_foc_paint           ; eigene Fenster grau (inaktiv) zeichnen
@foc:
.if WDEF_ON
        JSLF xf_wdef_click      ; X-Knopf im Win95-Rahmen: selbst behandeln
        bcc @std
        jsr nda_style
        sec
        rts
@std:
.endif
        jsr cur_hide
        pea $0000
        lda EVPTR
        pha
        lda fwptr+2
        pha
        lda fwptr
        pha
        lda FWCODE
        pha
        TOOL $1705              ; SystemClick
        jsr nda_qdcur           ; QuickDraw-Zeiger passend ein-/ausschalten
        jsr nda_style
        sec
        rts

; Taste an das Zubehoer weitergeben, wenn es zuletzt angeklickt wurde.
; Esc (schliessen) und Apfel-Kuerzel bleiben bei GS/OS95. C=1: verbraucht
nda_keyev:
        lda NDAFOC
        beq @no
        lda MENU
        ora DROP
        bne @no
        lda EVREC+14            ; Apfeltaste: Bearbeiten-Kuerzel ans Zubehoer
        and #$0100
        beq @nk
        jmp nda_editkey
@nk:    lda EVREC+2
        and #$00FF
        cmp #$1B
        beq @no
        jsr front_nda
        beq @nf
        pha                     ; Ergebnis
        lda EVREC               ; Ereignis wie es kam
        pha
        lda EVREC+4
        pha
        lda EVREC+2
        pha
        lda EVREC+8
        pha
        lda EVREC+6
        pha
        lda EVREC+12
        pha
        lda EVREC+10
        pha
        lda EVREC+14
        pha
        TOOL $1A05              ; SystemEvent
        pla                     ; auch nicht beanspruchte Tasten (Pfeile)
        sec                     ; nicht an GS/OS95: sonst zeichnet der
        rts                     ; Explorer durch das Zubehoer-Fenster
@nf:    stz NDAFOC              ; kein Zubehoer mehr offen
@no:    clc
        rts

; Apfel+Z/X/C/V im angeklickten Zubehoer: Widerrufen/Ausschneiden/Kopieren/Einfuegen
nda_editkey:
        lda EVREC               ; Tastenwiederholung: nicht mehrfach einfuegen
        cmp #5
        bne @nrep
        sec
        rts
@nrep:  lda EVREC+2
        and #$00DF              ; Grossbuchstabe
        ldx #1
        cmp #'Z'
        beq @go
        inx
        cmp #'X'
        beq @go
        inx
        cmp #'C'
        beq @go
        inx
        cmp #'V'
        bne @no
@go:    phx
        jsr front_nda
        plx
        cmp #0
        beq @no
        pha                     ; Ergebnis
        phx                     ; editType 1..4
        TOOL $1805              ; SystemEdit
        pla
        beq @no
        sec
        rts
@no:    clc
        rts

; ---------- Mauszeiger ueber Zubehoer-Fenstern ----------
; Ueber einem Zubehoer zeigt QuickDraw den Zeiger (es weiss, wann das
; Zubehoer zeichnet), sonst zeichnet GS/OS95 seinen eigenen.
nda_cursor:
        lda MENU                ; offene Menues liegen ueber allem: unser Zeiger
        ora DROP
        bne @own
        pha
        lda MYBANK
        pha
        pea fwptr2
        lda EVREC+12
        pha
        lda EVREC+10
        pha
        TOOL $170E              ; FindWindow
        pla
        bpl @own
        lda CURNDA
        bne @r
        jsr cur_hide
        lda #1
        sta CURNDA
        TOOL $CA04              ; InitCursor: Pfeil, sichtbar
@r:     rts
@own:   jmp nda_curoff
; zurueck zu unserem Zeiger
nda_curoff:
        lda CURNDA
        beq @r
        TOOL $9004              ; HideCursor
        stz CURNDA
        jsr cur_show
@r:     rts
; nach Aufrufen, die den QuickDraw-Zeiger veraendern koennen
nda_qdcur:
        lda CURNDA
        beq @h
        TOOL $CA04              ; InitCursor
        rts
@h:     TOOL $9004              ; HideCursor
        rts

; ---------- NDA-Fenster im Win95-Stil: blaue Titelleiste, weisse Schrift ----------
NS_TITLE = $01EF                ; inaktiv: Leiste dunkelgrau, Titel hellgrau; aktiv weiss
NS_TBAR  = $00DD                ; einfarbig dunkelblau
nda_style:
.if WDEF_ON
        JSLF xf_wdef_sync
.endif
        ;       ; Eintraege geschlossener Fenster freigeben
        pha
        pha
        TOOL $520E              ; GetFirstWindow
        pla
        sta NSW
        pla
        sta NSW+2
@l:     lda NSW
        ora NSW+2
        bne @w
        rts
@w:     lda MYBANK
        pha
        pea nscur
        lda NSW+2
        pha
        lda NSW
        pha
        TOOL $100E              ; GetFrameColor
        lda nscur+4
        cmp #NS_TBAR
        beq @next
        lda nstab+4
        cmp #NS_TBAR
        beq @have
        ldx #8                  ; Vorlage: Werte des ersten Fensters
@c:     lda nscur,x
        sta nstab,x
        dex
        dex
        bpl @c
        lda #NS_TITLE
        sta nstab+2
        lda #NS_TBAR
        sta nstab+4
@have:  lda MYBANK
        pha
        pea nstab
        lda NSW+2
        pha
        lda NSW
        pha
        TOOL $0F0E              ; SetFrameColor
@next:
.if WDEF_ON
        JSLF xf_wdef_install    ; Win95-Rahmen (eigene defProc)
.endif
        pha
        pha
        lda NSW+2
        pha
        lda NSW
        pha
        TOOL $2A0E              ; GetNextWindow
        pla
        sta NSW
        pla
        sta NSW+2
        jmp @l

; nach der eigenen Behandlung: NDA-Fenster wieder obenauf zeichnen
desk_after:
        lda NDAON
        beq @r
        lda NDAWAS
        beq @nw
        dec NSTC                ; ab und zu neue NDA-Fenster umfaerben
        bpl @ns
        lda #15
        sta NSTC
        jsr nda_style
        lda TNNDA               ; Fenster dazugekommen/geschlossen: Taskleiste neu
        pha
        jsr nda_list
        pla
        cmp TNNDA
        beq @ns
        lda #1
        sta NEEDREF
@ns:
        jsr front_nda
        bne @nw
        stz NDAWAS              ; letztes NDA-Fenster wurde geschlossen
        stz NDAFOC
        stz NEEDREF
        jsr cur_hide
        JSLF xf_set_palettes
        jsr desktop_scb
        jsr redraw_core
        jmp cur_show
@nw:    lda NEEDREF
        beq @r
        lda INCB
        bne @r
        stz NEEDREF
        jsr front_nda
        beq @r
        jmp desk_refresh
@r:     rts

; Bildschirm in richtiger Reihenfolge neu aufbauen: NDA-Fenster ausblenden,
; GS/OS95 zeichnen, Fenster von hinten nach vorn wieder einblenden
desk_refresh:
        jsr cur_hide
        pea $0000
        pea $0000
        TOOL $390E              ; RefreshDesktop(nil): Schreibtisch, dann Fenster
        jmp cur_show

; von redraw_all: wenn NDA-Fenster offen sind, danach neu darueberzeichnen
desk_mark:
        lda NDAON
        beq @r
        lda INCB
        bne @r
        lda #1
        sta NEEDREF
@r:     rts

; ---------- vor dem Start eines Programms ----------
desk_down:
        lda NDAON
        bne @go
        rts
@go:    TOOL $1D05              ; CloseAllNDAs
        TOOL $0305              ; DeskShutDown
        TOOL $031C              ; ListShutDown
        TOOL $031B              ; FMShutDown
        TOOL $0316              ; ScrapShutDown
        TOOL $0315              ; DialogShutDown
        TOOL $0314              ; LEShutDown
        TOOL $0310              ; CtlShutDown
        TOOL $030F              ; MenuShutDown
        TOOL $030E              ; WindShutDown
        TOOL $0312              ; QDAuxShutDown
        lda DSKH+2
        pha
        lda DSKH
        pha
        lda MYDP
        sta SCRDP
        TOOL $1002              ; DisposeHandle
        stz NDAON
        rts

; ---------- Fenster "Zubehoer" ----------
open_access:
        jsr desk_init
        bcc @ok
        lda #4
        jmp show_msg
@ok:    lda #$FFFF
        sta ACCSEL
        lda #14
        jsr win_open
        jmp redraw_all

ACCY = WY+40
ACCRH = 12
ACCROWS = 7
wc_access:
        ICON WX+14,WY+24,i_sys
        TEXT WX+40,WY+28,t_acc1,BLACK
        SUNKEN WX+8,ACCY-2,WW-16,WH-ACCY+WY-30
        FILL WX+10,ACCY,WW-20,WH-ACCY+WY-34,WHITE
        lda NDAN
        bne @l0
        TEXT WX+14,ACCY+4,t_acc0,BLACK
        jmp @btn
@l0:    stz NDAI
        lda #ACCY+3
        sta IY
@l:     lda NDAI
        cmp NDAN
        bcc *+5
        jmp @btn
        cmp #ACCROWS
        bcs @btn
        ; kleines Symbol (Raute) und Name
        lda #WX+14
        sta RX
        lda IY
        clc
        adc #2
        sta RY
        lda #4
        sta RW
        sta RH
        lda #NAVY
        jsr setcolor
        jsr fillrect
        lda #WX+24
        sta SX
        lda IY
        sta SY
        lda NDAI
        asl
        asl
        asl
        asl
        asl
        tax
        clc
        adc #ndanames+1
        sta STRP
        lda ndanames,x
        and #$00FF
        sta STRN
        lda NDAI
        cmp ACCSEL
        bne @ns
        lda #WX+22
        sta RX
        lda IY
        dec a
        sta RY
        lda #WW-34
        sta RW
        lda #10
        sta RH
        lda #NAVY
        jsr setcolor
        jsr fillrect
        lda #WHITE
        bra @sc
@ns:    lda #BLACK
@sc:    jsr setcolor
        jsr drawstr
        lda IY
        clc
        adc #ACCRH
        sta IY
        inc NDAI
        jmp @l
@btn:   BUTTON WX+WW-72,WY+WH-26,62,15
        TEXTC WX+WW-41,WY+WH-22,t_abbr,BLACK
        rts

access_click:
        HIT WX+WW-72,WY+WH-26,62,15
        bcc @n
        jmp close_win
@n:     HIT WX+10,ACCY,WW-20,ACCROWS*ACCRH
        bcs @in
        rts
@in:    jsr lmy
        sec
        sbc #ACCY
        ldx #0
@dv:    cmp #ACCRH
        bcc @dd
        sbc #ACCRH
        inx
        bra @dv
@dd:    txa
        cmp NDAN
        bcc access_go
        rts

; Tasten im Zubehoer-Fenster: Pfeile, Eingabe, Escape
access_key:
        lda KEYC
        cmp #$1B
        bne @k1
        jmp close_win
@k1:    cmp #$0A
        bne @k2
        lda ACCSEL
        inc a
        cmp NDAN
        bcc @set
        cmp #ACCROWS
        bcc @set
        lda #0
        bra @set
@k2:    cmp #$0B
        bne @k3
        lda ACCSEL
        dec a
        bpl @set
        lda NDAN
        dec a
        bmi @r
        bra @set
@k3:    cmp #$0D
        bne @r
        lda ACCSEL
        bmi @r
        cmp NDAN
        bcs @r
        bra access_go
@set:   cmp NDAN
        bcs @r
        sta ACCSEL
        jmp redraw_win
@r:     rts

access_go:
        inc a
        pha
        jsr win_pop
        pla
; NDA Nummer A (ab 1) oeffnen
nda_open:
        sta NDAI
        sta NDAOP
        jsr redraw_all
        lda #1                  ; schon waehrend OpenNDA nur durch das Clipping zeichnen
        sta NDAWAS
        pha
        lda NDAI
        pha
        TOOL $1505              ; OpenNDA
        pla
        lda #1
        sta NDAFOC              ; neues Zubehoer ist vorne: Tasten dorthin
        sta NDAWAS
        sta NEEDREF
        jsr nda_qdcur
        jsr desk_own
        jsr nda_list            ; Fensterliste fuer die Taskleiste
        jsr nda_map             ; Fenster -> Zubehoer-Nummer merken (Taskleiste)
        jsr nda_style
        JSLF xf_foc_paint           ; eigene Titelleiste grau
        lda #1
        sta NEEDREF
        rts

; ---------- Zubehoer-Fenster fuer die Taskleiste ----------
; Fensterzeiger nach ndawl, Anzahl TNNDA (hoechstens 4)
nda_list:
        lda INCB                ; in der Zeichenroutine des Window Managers keine
        beq @m                  ; Toolbox-Aufrufe: gemerkte Liste benutzen
        rts
@m:     stz TNNDA
        lda NDAON
        bne @go
        rts
@go:    pha
        pha
        TOOL $520E              ; GetFirstWindow
        pla
        sta NLW
        pla
        sta NLW+2
@l:     lda NLW
        ora NLW+2
        beq @r
        lda TNNDA
        cmp #4
        bcs @r
        asl
        asl
        tax
        lda NLW
        sta ndawl,x
        lda NLW+2
        sta ndawl+2,x
        inc TNNDA
        pha
        pha
        lda NLW+2
        pha
        lda NLW
        pha
        TOOL $2A0E              ; GetNextWindow
        pla
        sta NLW
        pla
        sta NLW+2
        bra @l
@r:     rts

; Titel des Zubehoer-Fensters A -> STRP/STRN (Name aus dem Apple-Menue)
nda_title:
        asl
        asl
        tax
        lda ndawl,x
        sta TMP
        lda ndawl+2,x
        sta TMP2
; Name zum Fensterzeiger TMP/TMP2 -> STRP/STRN
nda_name:
        ldx #0
@l:     lda ndamap,x
        cmp TMP
        bne @n
        lda ndamap+2,x
        cmp TMP2
        bne @n
        lda ndamap+4,x          ; Zubehoer-Nummer (ab 1)
        dec a
        asl
        asl
        asl
        asl
        asl
        tax
        clc
        adc #ndanames+1
        sta STRP
        lda ndanames,x
        and #$00FF
        sta STRN
        rts
@n:     txa
        clc
        adc #6
        tax
        cpx #6*4
        bcc @l
        lda #ndaunk             ; unbekannt
        sta STRP
        lda #3
        sta STRN
        rts

; vorderstes Fenster nach OpenNDA dem Zubehoer NDAI zuordnen
nda_map:
        pha
        pha
        TOOL $150E              ; FrontWindow
        pla
        sta TMP
        pla
        sta TMP2
        ora TMP
        beq @r
        ldx #0                  ; gleicher Zeiger oder freier Platz
@l:     lda ndamap,x
        cmp TMP
        bne @a
        lda ndamap+2,x
        cmp TMP2
        beq @set
@a:     lda ndamap,x
        ora ndamap+2,x
        beq @set
        txa
        clc
        adc #6
        tax
        cpx #6*4
        bcc @l
        ldx #0                  ; voll: ersten Eintrag ueberschreiben
@set:   lda TMP
        sta ndamap,x
        lda TMP2
        sta ndamap+2,x
        lda NDAOP
        sta ndamap+4,x
@r:     rts
ndaunk: .byte "NDA"

; Zubehoer-Fenster A nach vorn holen
nda_select:
        cmp TNNDA
        bcs @r
        asl
        asl
        tax
        lda ndawl+2,x
        pha
        lda ndawl,x
        pha
        TOOL $110E              ; SelectWindow
        jsr nda_style
        lda #1
        sta NDAWAS
        sta NEEDREF
@r:     rts
