; ================================================================
; GS/OS95 Browser: Seitenfilter (Teil von net.s, XCODE3)
; 1) Software-Archiv: Verzeichnisseiten unter der Archiv-Adresse (Vorgabe:
;    Asimov-Spiegel) werden zu einer schlichten Liste - Ordner fett, Dateien
;    normal, oben ".." -, egal ob Apache, nginx oder eigene Listen-Seiten.
;    Ein Klick auf eine Datei laedt sie wie bisher nach *:Downloads.
; 2) Nachrichten: RSS- und Atom-Feeds (Inhaltstyp ...xml) werden zu einer
;    Seite: Name des Feeds, je Meldung Titel, Link, Text (gekuerzt), Datum.
; Eingabe HR..HE im Rohpuffer, Ausgabe ab WB_LIN (die Zeilentabelle entsteht
; erst in page_done), danach HR/HE auf die Ausgabe. Grosse Seiten kommen
; stueckweise (big_page); alle Zustaende bleiben dazwischen erhalten.
; Laeuft mit Datenbank = Hauptbank; Texte dieser Bank ueber LP2.
; about:archiv[=URL]  Archiv oeffnen / andere Adresse einstellen
; about:rss           Nachrichten-Seite mit einigen Feeds
; ================================================================
PF_OUT = WB_LIN
PF_LIM = WB_LIN+$2000
PF_CT  = $F5C0                  ; Name des Feeds (Laenge + 62), Pufferbank
PF_LH  = $F580                  ; letzter Archiv-Link (Laenge + 62)
ARCU   = NPREF+1828             ; Archiv-Adresse (Laengenbyte + bis 127)
RE_CT  = 1                      ; Elemente beim Feed
RE_T   = 2
RE_L   = 3
RE_D   = 4
RE_DT  = 5
RS_MAX = 200                    ; so viele Zeichen Text je Meldung

.macro TBL addr                ; Tabelle dieser Bank lesen (Datenbank = Hauptbank)
        phb
        phk
        plb
        lda addr,x
        plb
.endmacro

        .pushseg
        .segment "BSS"
PFM:    .res 2                  ; 0 aus, 1 Archiv, 2 Feed
PFX:    .res 2                  ; Inhaltstyp ist XML (Feed)
PFR:    .res 2
PFW:    .res 2
PFE:    .res 2                  ; Feed: Element, dessen Text gerade kommt
PFIT:   .res 2                  ; in einer Meldung
PFLK:   .res 2                  ; Link/Text/Datum der Meldung schon da
PFDS:   .res 2
PFDT:   .res 2
PFCD:   .res 2                  ; in CDATA
PFN:    .res 2
PFTG:   .res 2                  ; Text: in einem (entfernten) Tag
PFHD:   .res 2                  ; Kopf ausgegeben
PFDIR:  .res 2                  ; Archiv: Ende des Ordners in WB_TURL
PFRTE:  .res 2                  ;   hinter der Archiv-Adresse
PFTS:   .res 2                  ;   hinter "://"
PFTP:   .res 2                  ;   Anfang des Pfads
PFEND:  .res 2                  ; '>' des Tags
PFVS:   .res 2                  ; Attributwert
PFVE:   .res 2
PFNS:   .res 2                  ; Tag-Name
PFNE:   .res 2
PFCLO:  .res 2
PFSL:   .res 2
PFA:    .res 2
PFB:    .res 2
PFC:    .res 2
PFT:    .res 2
PFT2:   .res 2
PFT3:   .res 2
PFNL:   .res 2
FNLEN:    .res 2                  ; Dateiname: Laenge der Quelle [LP2]
FNB:    .res 42
        .popseg

; ---------- Einspruenge ----------
; Inhaltstyp (WB_CTYP) ist XML, aber kein XHTML -> Feed: C=1
xf_pf_ctype:
        stz PFX
        ldy #WB_CTYP
        BRD8
        beq @no
        clc
        adc #WB_CTYP+1
        sta PFT
        lda #pfs_xml
        jsr pf_has
        bcc @no
        lda #pfs_html
        jsr pf_has
        bcs @no
        lda #1
        sta PFX
        sec
        rtl
@no:    clc
        rtl

; vor dem Umwandeln einer Seite: Art bestimmen
xf_pf_init:
        stz PFM
        lda HCODE
        cmp #200
        bcc @r
        cmp #300
        bcs @r
        jsr net_ptr
        lda PFX
        beq @a
        lda #2
        sta PFM
        stz PFE
        stz PFIT
        stz PFCD
        stz PFTG
        stz PFHD
        rtl
@a:     lda HPLAIN
        bne @r
        jsr arc_match
        bcc @r
        lda #1
        sta PFM
        stz PFHD
        ldy #PF_LH
        lda #0
        BWR8
@r:     rtl

; HR..HE umwandeln (nur wenn PFM)
xf_pf_run:
        lda PFM
        bne @go
        rtl
@go:    jsr net_ptr
        lda HR
        sta PFR
        lda #PF_OUT
        sta PFW
        lda PFM
        cmp #2
        beq @f
        jsr arc_run
        bra @d
@f:     jsr rss_run
@d:     lda #PF_OUT
        sta HR
        lda PFW
        sta HE
        rtl

; ---------- Hilfen ----------
; LP2 = Text A dieser Bank
pf_kb:  sta LP2
        phk
        phk
        pla
        and #$00FF
        sta LP2+2
        rts
pf_lc:  cmp #'A'
        bcc @r
        cmp #'Z'+1
        bcs @r
        ora #$20
@r:     rts
; Zeichen A ausgeben
pf_c:   ldy PFW
        cpy #PF_LIM
        bcs @r
        BWR8
        inc PFW
@r:     rts
; Text A (dieser Bank, Laengenbyte) ausgeben
pf_s:   jsr pf_kb
        lda [LP2]
        and #$00FF
        sta PFNL
        lda #1
        sta PFC
@l:     lda PFNL
        beq @r
        ldy PFC
        lda [LP2],y
        and #$00FF
        jsr pf_c
        inc PFC
        dec PFNL
        bra @l
@r:     rts
; Text DE A / EN X
pf_sl:  ldy LANG
        beq pf_s
        txa
        bra pf_s
; Puffer PFA..PFB unveraendert ausgeben
pf_raw: lda PFA
        cmp PFB
        bcs @r
        tay
        BRD8
        cmp #'"'
        beq @n
        jsr pf_c
@n:     inc PFA
        bra pf_raw
@r:     rts
; Puffer PFA..PFB mit %XX-Dekodierung als Text ausgeben
pf_dec: lda PFA
        cmp PFB
        bcs @r
        tay
        BRD8
        cmp #'%'
        bne @c
        lda PFA
        clc
        adc #3
        cmp PFB
        beq *+4
        bcs @c0
        iny
        BRD8
        jsr hexv
        asl
        asl
        asl
        asl
        sta PFC
        iny
        BRD8
        jsr hexv
        ora PFC
        inc PFA
        inc PFA
        bra @c
@c0:    lda #'%'
@c:     inc PFA
        cmp #' '
        bcc @q
        cmp #'<'
        beq @q
        cmp #'>'
        beq @q
        cmp #'&'
        beq @q
        cmp #'"'
        bne @o
@q:     lda #'?'
@o:     jsr pf_c
        bra pf_dec
@r:     rts
hexv:   and #$00FF
        cmp #'A'
        bcc @d
        and #$00DF
        sec
        sbc #'A'-10
        and #$000F
        rts
@d:     sec
        sbc #'0'
        and #$000F
        rts

; kommt Text A (dieser Bank, klein) in WB_CTYP+1..PFT vor? C=1 ja
pf_has: jsr pf_kb
        lda [LP2]
        and #$00FF
        sta PFNL
        lda #WB_CTYP+1
        sta PFA
@s:     lda PFA
        clc
        adc PFNL
        cmp PFT
        beq @t
        bcs @no
@t:     lda PFA
        sta PFB
        lda #1
        sta PFC
@m:     ldy PFB
        BRD8
        jsr pf_lc
        sta PFT2
        ldy PFC
        lda [LP2],y
        and #$00FF
        cmp PFT2
        bne @nx
        inc PFB
        inc PFC
        lda PFC
        dec a
        cmp PFNL
        bcc @m
        sec
        rts
@nx:    inc PFA
        bra @s
@no:    clc
        rts

; stimmt Text [LP2] (Laengenbyte) an Puffer-Stelle Y? C=1 ja (Y bleibt)
pf_at:  sty PFB
        lda #1
        sta PFC
        lda [LP2]
        and #$00FF
        sta PFNL
        clc
        adc PFB
        cmp HE
        beq @l
        bcs @no
@l:     lda PFC
        dec a
        cmp PFNL
        bcs @y
        ldy PFB
        BRD8
        sta PFT2
        ldy PFC
        lda [LP2],y
        and #$00FF
        cmp PFT2
        bne @no
        inc PFB
        inc PFC
        bra @l
@y:     lda PFB
        sec
        sbc PFNL
        tay
        sec
        rts
@no:    lda PFB
        sec
        sbc PFC
        inc a
        tay
        clc
        rts

; Ende des Tags ab Y suchen (Anfuehrungszeichen beachten) -> PFEND
pf_tend:
        stz PFC
@l:     cpy HE
        bcs @e
        BRD8
        ldx PFC
        bne @q
        cmp #'"'
        beq @o
        cmp #$27
        beq @o
        cmp #'>'
        beq @e
        iny
        bra @l
@o:     sta PFC
        iny
        bra @l
@q:     cmp PFC
        bne @q2
        stz PFC
@q2:    iny
        bra @l
@e:     sty PFEND
        rts

; Attribut [LP2] (klein, Laengenbyte) im Tag Y..PFEND -> PFVS/PFVE, C=1 gefunden
pf_attr:
        lda [LP2]
        and #$00FF
        sta PFNL
        sty PFA
@s:     lda PFA
        clc
        adc PFNL
        cmp PFEND
        bcc *+5
        jmp @no
        ldy PFA                 ; davor Leerraum
        dey
        BRD8
        cmp #' '+1
        bcc *+5
        jmp @nx
        lda PFA
        sta PFB
        lda #1
        sta PFC
@m:     ldy PFB
        BRD8
        jsr pf_lc
        sta PFT2
        ldy PFC
        lda [LP2],y
        and #$00FF
        cmp PFT2
        bne @nx
        inc PFB
        inc PFC
        lda PFC
        dec a
        cmp PFNL
        bcc @m
        ldy PFB                 ; Leerraum, '='
@w:     cpy PFEND
        bcs @nx
        BRD8
        cmp #'='
        beq @eq
        cmp #' '+1
        bcs @nx
        iny
        bra @w
@eq:    iny
@w2:    cpy PFEND
        bcs @nx
        BRD8
        cmp #' '+1
        bcs @v
        iny
        bra @w2
@v:     cmp #'"'
        beq @qv
        cmp #$27
        beq @qv
        sty PFVS                ; ohne Anfuehrungszeichen
@u:     cpy PFEND
        bcs @ue
        BRD8
        cmp #' '+1
        bcc @ue
        iny
        bra @u
@ue:    sty PFVE
        sec
        rts
@qv:    sta PFC
        iny
        sty PFVS
@q:     cpy PFEND
        bcs @qe
        BRD8
        cmp PFC
        beq @qe
        iny
        bra @q
@qe:    sty PFVE
        sec
        rts
@nx:    inc PFA
        jmp @s
@no:    clc
        rts

; ================================================================
; Software-Archiv
; ================================================================
; LP2 = Archiv-Adresse (eingestellt oder eingebaut)
arc_root:
        lda ARCU
        and #$00FF
        beq @d
        lda #ARCU
        sta LP2
        lda MYBANK
        sta LP2+2
        rts
@d:     lda #arc_def
        jmp pf_kb

; Y hinter "://" im Puffer (ab Y bis PFT), sonst Y unveraendert
pf_sch: sty PFA
@l:     tya
        clc
        adc #3
        cmp PFT
        bcs @no
        BRD8
        cmp #':'
        bne @n
        iny
        BRD8
        cmp #'/'
        bne @n0
        iny
        BRD8
        cmp #'/'
        bne @n0
        iny
        rts
@n0:    dey
@n:     iny
        bra @l
@no:    ldy PFA
        rts

; WB_TURL unter der Archiv-Adresse? C=1 ja; PFTS/PFTP/PFRTE/PFDIR gesetzt
arc_match:
        jsr arc_root
        lda [LP2]
        and #$00FF
        sta PFT2                ; Laenge
        ldy #1                  ; "://" in der Archiv-Adresse
@r1:    tya
        clc
        adc #2
        cmp PFT2
        bcs @r0
        lda [LP2],y
        and #$00FF
        cmp #':'
        bne @r2
        iny
        lda [LP2],y
        and #$00FF
        cmp #'/'
        bne @r2
        iny
        lda [LP2],y
        and #$00FF
        cmp #'/'
        bne @r2
        iny
        bra @r3
@r2:    iny
        bra @r1
@r0:    ldy #1
@r3:    sty PFT3                ; Index in der Archiv-Adresse
        ldy #WB_TURL
        BRD8
        clc
        adc #WB_TURL+1
        sta PFT
        ldy #WB_TURL+1
        jsr pf_sch
        sty PFTS
@c:     lda PFT3                ; vergleichen (Gross/klein egal)
        cmp PFT2
        beq @c1
        bcs @hit
@c1:    cpy PFT
        bcs @no
        BRD8
        jsr pf_lc
        sta PFC
        phy
        ldy PFT3
        lda [LP2],y
        and #$00FF
        jsr pf_lc
        ply
        cmp PFC
        bne @no
        iny
        inc PFT3
        bra @c
@hit:   sty PFRTE
        ldy PFTS                ; Pfadanfang
@p:     cpy PFT
        bcs @pe
        BRD8
        cmp #'/'
        beq @pe
        iny
        bra @p
@pe:    sty PFTP
        ldy PFRTE               ; Ordner: bis hinter das letzte '/'
        sty PFDIR
@d:     cpy PFT
        bcs @de
        BRD8
        cmp #'?'
        beq @de
        cmp #'#'
        beq @de
        iny
        cmp #'/'
        bne @d
        sty PFDIR
        bra @d
@de:    sec
        rts
@no:    clc
        rts

; Kopf der Liste
arc_head:
        lda #pfs_t1             ; <title>
        jsr pf_s
        lda #pfs_arc
        ldx #pfs_arc_en
        jsr pf_sl
        lda #pfs_t2             ; </title><h1>
        jsr pf_s
        lda #pfs_arc
        ldx #pfs_arc_en
        jsr pf_sl
        lda #pfs_t3             ; </h1><p>
        jsr pf_s
        lda #pfs_ahint
        ldx #pfs_ahint_en
        jsr pf_sl
        lda #pfs_t4             ; </p><p><b>/
        jsr pf_s
        lda PFRTE
        sta PFA
        lda PFDIR
        sta PFB
        jsr pf_dec
        lda #pfs_t5             ; </b></p>
        jsr pf_s
        lda PFDIR
        cmp PFRTE
        beq @r
        lda #pfs_t6             ; <p><a href="../">
        jsr pf_s
        lda #pfs_up
        ldx #pfs_up_en
        jsr pf_sl
        lda #pfs_t7             ; </a></p>
        jsr pf_s
@r:     rts

arc_run:
        lda PFHD
        bne @l
        inc PFHD
        jsr arc_head
@l:     lda PFR
        cmp HE
        bcc *+3
        rts
        tay
        BRD8
        cmp #'<'
        bne @nx
        iny
        BRD8
        jsr pf_lc
        cmp #'a'
        bne @nx
        iny
        BRD8
        cmp #' '+1
        bcs @nx
        sty PFT3
        jsr pf_tend
        lda #pfs_href
        jsr pf_kb
        ldy PFT3
        jsr pf_attr
        bcc @sk
        jsr arc_link
@sk:    lda PFEND
        inc a
        sta PFR
        bra @l
@nx:    inc PFR
        bra @l

; Link PFVS..PFVE: Eintrag im aktuellen Ordner? Dann ausgeben
arc_link:
        lda PFVE
        sta PFT
        ldy PFVS
        jsr pf_sch
        cpy PFVS
        beq @ns
        ldx PFTS                ; mit Rechner: wie WB_TURL ab "://"
        jsr pf_pfx
        bcs *+5
        jmp @no
        bra @rest
@ns:    BRD8
        cmp #'/'
        bne @rest               ; relativ
        ldx PFTP                ; absoluter Pfad
        jsr pf_pfx
        bcs *+5
        jmp @no
@rest:  sty PFT                 ; Rest: Name bzw. Name/
        cpy PFVE
        bcc *+5
        jmp @no
        BRD8
        cmp #'.'
        bne *+5
        jmp @no
        cmp #'?'
        bne *+5
        jmp @no
        cmp #'#'
        bne *+5
        jmp @no
        stz PFT2
@v:     cpy PFVE
        bcs @ok
        BRD8
        cmp #'?'
        beq @no
        cmp #'#'
        beq @no
        cmp #':'
        beq @no
        cmp #'&'
        beq @no
        iny
        cmp #'/'
        bne @v
        cpy PFVE
        bne @no
        inc PFT2                ; Ordner
@ok:    jsr arc_dup
        bcs @no
        lda #pfs_br
        jsr pf_s
        lda PFT2
        beq @f1
        lda #pfs_b
        jsr pf_s
@f1:    lda #pfs_ah
        jsr pf_s
        lda PFT
        sta PFA
        lda PFVE
        sta PFB
        jsr pf_raw
        lda #pfs_ac
        jsr pf_s
        lda PFT
        sta PFA
        lda PFVE
        sec
        sbc PFT2
        sta PFB
        jsr pf_dec
        lda PFT2
        beq @f2
        lda #'/'
        jsr pf_c
@f2:    lda #pfs_ae
        jsr pf_s
        lda PFT2
        beq @no
        lda #pfs_be
        jsr pf_s
@no:    rts

; Puffer ab X (WB_TURL) bis PFDIR gleich Puffer ab Y (Link)? C=1, Y = dahinter
pf_pfx: stx PFA
        sty PFB
@l:     lda PFA
        cmp PFDIR
        bcs @ok
        lda PFB
        cmp PFVE
        bcs @no
        ldy PFA
        BRD8
        jsr pf_lc
        sta PFC
        ldy PFB
        BRD8
        jsr pf_lc
        cmp PFC
        bne @no
        inc PFA
        inc PFB
        bra @l
@ok:    ldy PFB
        sec
        rts
@no:    clc
        rts

; derselbe Link wie eben (PFT..PFVE)? C=1; sonst merken
arc_dup:
        lda PFVE
        sec
        sbc PFT
        cmp #63
        bcc @l
        lda #62
@l:     sta PFNL
        ldy #PF_LH
        BRD8
        cmp PFNL
        bne @new
        lda PFT
        sta PFA
        lda #PF_LH+1
        sta PFB
        ldx PFNL
@c:     txa
        beq @same
        ldy PFA
        BRD8
        sta PFC
        ldy PFB
        BRD8
        cmp PFC
        bne @new
        inc PFA
        inc PFB
        dex
        bra @c
@same:  sec
        rts
@new:   ldy #PF_LH
        lda PFNL
        BWR8
        lda PFT
        sta PFA
        lda #PF_LH+1
        sta PFB
        ldx PFNL
@s:     txa
        beq @e
        ldy PFA
        BRD8
        ldy PFB
        BWR8
        inc PFA
        inc PFB
        dex
        bra @s
@e:     clc
        rts

; about:archiv[=URL]: Adresse merken, Archiv laden (BBSGO)
xf_arc_cmd:
        jsr net_ptr
        ldy #WB_TURL
        BRD8
        clc
        adc #WB_TURL+1
        sta PFT
        ldy #WB_TURL+7
@f:     cpy PFT
        bcs @use
        BRD8
        cmp #'='
        beq @set
        iny
        bra @f
@set:   iny
        sty PFA
        lda PFT
        sec
        sbc PFA
        cmp #128
        bcc @l
        lda #127
@l:     sta PFNL
        sep #$20
        sta ARCU
        rep #$20
        ldx #0
@c:     cpx PFNL
        bcs @sv
        BRD8
        sep #$20
        sta ARCU+1,x
        rep #$20
        iny
        inx
        bra @c
@sv:    jsr net_save
        jsr net_ptr
@use:   jsr arc_root
        lda [LP2]
        and #$00FF
        inc a
        sta PFNL
        ldy #0
@u:     lda [LP2],y
        phy
        pha
        tya
        clc
        adc #WB_TURL
        tay
        pla
        BWR8
        ply
        iny
        cpy PFNL
        bcc @u
        lda #1
        sta BBSGO
        clc
        rtl

; about:rss: Seite mit Feeds in den Rohpuffer (HO)
xf_rss_cmd:
        jsr net_ptr
        stz HPLAIN
        stz HO
        lda #rss_page
        ldx LANG
        beq @d
        lda #rss_page_en
@d:
xrc_copy:
        jsr pf_kb
        ldy #0
@l:     lda [LP2],y
        and #$00FF
        beq @e
        phy
        ldy HO
        BWR8
        inc HO
        ply
        iny
        bra @l
@e:     clc
        rtl

; ================================================================
; Nachrichten (RSS 0.9x/2.0, RDF, Atom)
; ================================================================
rss_run:
@l:     lda PFR
        cmp HE
        bcc *+3
        rts
        tay
        BRD8
        sta PFC
        lda PFCD
        beq @nc
        lda PFC                 ; in CDATA: bis "]]>" alles Text
        cmp #']'
        bne @cdc
        lda #pfs_cde
        jsr pf_kb
        ldy PFR
        jsr pf_at
        bcc @cdc
        stz PFCD
        lda PFR
        clc
        adc #3
        sta PFR
        bra @l
@cdc:   inc PFR
        lda PFC
        jsr rs_char
        bra @l
@nc:    lda PFC
        cmp #'<'
        bne @am
        jsr rs_tag
        bra @l
@am:    cmp #'&'
        bne @ch
        lda PFE
        cmp #RE_D
        bne @ch
        jsr rs_ent
        bcs @l
@ch:    inc PFR
        lda PFC
        jsr rs_char
        bra @l

; Zeichen A im aktuellen Element
rs_char:
        sta PFC
        ldx PFE
        bne @go
        rts
@go:    cpx #RE_CT
        bne @n1
        ldy #PF_CT              ; Name des Feeds sammeln
        BRD8
        cmp #62
        bcs @r
        inc a
        BWR8
        clc
        adc #PF_CT
        tay
        lda PFC
        cmp #' '
        bcs @ct
        lda #' '
@ct:    BWR8
@r:     rts
@n1:    cpx #RE_L
        bne @n2
        lda PFC                 ; Link: ohne Leerraum und '"'
        cmp #' '+1
        bcc @r
        cmp #'"'
        beq @r
        jmp pf_c
@n2:    cpx #RE_D
        beq @d
        lda PFC                 ; Titel, Datum
        cmp #'<'
        bne @o
        lda #'('
@o:     jmp pf_c
@d:     lda PFTG                ; Text: Tags weg, kuerzen
        beq @d1
        lda PFC
        cmp #'>'
        bne @r
        stz PFTG
        rts
@d1:    lda PFC
        cmp #'<'
        bne @d2
        inc PFTG
        rts
@d2:    lda PFN
        cmp #RS_MAX
        bcc @d3
        bne @r
        inc PFN
        lda #pfs_ell
        jmp pf_s
@d3:    inc PFN
        lda PFC
        cmp #' '
        bcs @o
        lda #' '
        jmp pf_c

; '&' im Text: &lt; &gt; &amp; &quot; aufloesen. C=1 erledigt
rs_ent: ldx #0
@l:     TBL rs_etab
        and #$00FF
        beq @no
        sta PFT
        phx
        txa
        clc
        adc #rs_etab
        jsr pf_kb
        ldy PFR
        jsr pf_at
        plx
        bcs @hit
        txa
        sec
        adc PFT
        inc a
        tax
        bra @l
@hit:   lda PFT
        clc
        adc PFR
        sta PFR
        txa
        sec
        adc PFT
        tax
        TBL rs_etab
        and #$00FF
        jsr rs_char
        sec
        rts
@no:    clc
        rts

; Tag ab PFR
rs_tag: lda #pfs_cdo            ; <![CDATA[
        jsr pf_kb
        ldy PFR
        jsr pf_at
        bcc @c2
        lda #1
        sta PFCD
        lda PFR
        clc
        adc #9
        sta PFR
        rts
@c2:    iny                     ; <! und <? ueberspringen (Kommentare bis -->)
        BRD8
        cmp #'?'
        beq @sk
        cmp #'!'
        bne @nm
        iny
        BRD8
        cmp #'-'
        bne @sk
@cm:    iny                     ; Kommentar
        cpy HE
        bcs @ce
        BRD8
        cmp #'>'
        bne @cm
        dey
        BRD8
        iny
        cmp #'-'
        bne @cm
@ce:    iny
        sty PFR
        rts
@sk:    jsr pf_tend
        lda PFEND
        inc a
        sta PFR
        rts
@nm:    stz PFCLO
        cmp #'/'
        bne @n0
        inc PFCLO
        iny
@n0:    sty PFNS
@n1:    cpy HE
        bcs @n2
        BRD8
        cmp #':'                ; Namensraum weg (dc:date -> date)
        bne @n3
        iny
        sty PFNS
        bra @n1
@n3:    cmp #'-'
        beq @n4
        cmp #'0'
        bcc @n2
        cmp #'z'+1
        bcs @n2
        cmp #'9'+1
        bcc @n4
        cmp #'A'
        bcc @n2
@n4:    iny
        bra @n1
@n2:    sty PFNE
        jsr pf_tend
        stz PFSL
        ldy PFEND
        dey
        BRD8
        cmp #'/'
        bne @n5
        inc PFSL
@n5:    jsr rs_name
        jsr rs_act
        lda PFEND
        inc a
        sta PFR
        rts

; Tag-Name PFNS..PFNE -> A: 0 andere, 1 Meldung, 2 Titel, 3 Link, 4 Text, 5 Datum, 6 Inhalt
rs_name:
        lda PFNE
        sec
        sbc PFNS
        sta PFNL
        ldx #0
@l:     TBL rs_ntab
        and #$00FF
        beq @r
        sta PFC
        cmp PFNL
        bne @nx
        stx PFT
        ldy PFNS
@c:     BRD8
        jsr pf_lc
        sta PFT2
        TBL rs_ntab+1
        and #$00FF
        cmp PFT2
        bne @nx0
        inx
        iny
        cpy PFNE
        bcc @c
        TBL rs_ntab+1
        and #$00FF
        rts
@nx0:   ldx PFT
        TBL rs_ntab
        and #$00FF
        sta PFC
@nx:    txa
        sec
        adc PFC
        inc a
        tax
        bra @l
@r:     rts

; Tag-Art A behandeln
rs_act: sta PFT3
        cmp #0
        bne @g
        rts
@g:     lda PFCLO
        beq @op
        lda PFT3                ; schliessendes Tag
        cmp #1
        bne @c2
        lda PFIT
        bne *+5
        jmp @r
        jsr rs_cls
        stz PFIT
        lda #pfs_pe
        jmp pf_s
@c2:    jmp rs_cls
@op:    lda PFT3
        cmp #1
        bne @o2
        jsr rs_cls              ; Meldung beginnt
        lda #1
        sta PFIT
        stz PFLK
        stz PFDS
        stz PFDT
        lda #pfs_p
        jmp pf_s
@o2:    cmp #2
        bne @o3
        lda PFE
        beq *+5
        jmp @r
        lda PFIT
        beq @ct
        lda #RE_T
        sta PFE
        lda #pfs_b
        jsr pf_s
        brl @sl
@ct:    lda PFHD                ; Name des Feeds (erster Titel)
        beq *+5
        jmp @r
        lda #RE_CT
        sta PFE
        ldy #PF_CT
        lda #0
        BWR8
        brl @sl
@o3:    cmp #3
        bne @o4
        lda PFIT
        beq @r
        lda PFLK
        bne @r
        lda PFE
        bne @r
        lda #pfs_href           ; Atom: <link href=".." rel="alternate"/>
        jsr pf_kb
        ldy PFNE
        jsr pf_attr
        bcc @rl
        lda PFVS
        sta PFT
        lda PFVE
        sta PFT3
        lda #pfs_rel
        jsr pf_kb
        ldy PFNE
        jsr pf_attr
        bcc @al
        ldy PFVS
        BRD8
        jsr pf_lc
        cmp #'a'
        bne @r
@al:    lda #1
        sta PFLK
        lda #pfs_ah
        jsr pf_s
        lda PFT
        sta PFA
        lda PFT3
        sta PFB
        jsr pf_raw
        lda #pfs_ac
        jsr pf_s
        jmp rs_more
@rl:    lda PFSL                ; RSS: <link>URL</link>
        bne @r
        lda #RE_L
        sta PFE
        lda #pfs_ah
        jmp pf_s
@r:     rts
@o4:    cmp #5
        beq @dt
        lda PFIT                ; Text (description/summary/content)
        beq @r
        lda PFDS
        bne @r
        lda PFE
        bne @r
        inc PFDS
        lda #RE_D
        sta PFE
        stz PFN
        stz PFTG
@sl:    lda PFSL
        beq @r
        jmp rs_cls
@dt:    lda PFIT
        beq @r
        lda PFDT
        bne @r
        lda PFE
        bne @r
        inc PFDT
        lda #RE_DT
        sta PFE
        lda #pfs_br
        jsr pf_s
        bra @sl

; aktuelles Element abschliessen
rs_cls: lda PFE
        bne @g
        rts
@g:     stz PFE
        cmp #RE_CT
        bne @n1
        inc PFHD
        lda #pfs_t1             ; <title>Name</title><h1>Name</h1>
        jsr pf_s
        jsr rs_ctn
        lda #pfs_t2
        jsr pf_s
        jsr rs_ctn
        lda #pfs_h1e
        jmp pf_s
@n1:    cmp #RE_T
        bne @n2
        lda #pfs_be
        jsr pf_s
        lda #pfs_br
        jmp pf_s
@n2:    cmp #RE_L
        bne @n3
        lda #1
        sta PFLK
        lda #pfs_ac
        jsr pf_s
        jmp rs_more
@n3:    cmp #RE_D
        beq @n4
        cmp #RE_DT
        bne @r
@n4:    stz PFTG
        lda #pfs_br
        jmp pf_s
@r:     rts
rs_more:
        lda #pfs_more
        ldx #pfs_more_en
        jsr pf_sl
        lda #pfs_ae
        jsr pf_s
        lda #pfs_br
        jmp pf_s
; Name des Feeds ausgeben
rs_ctn: ldy #PF_CT
        BRD8
        clc
        adc #PF_CT+1
        sta PFB
        lda #PF_CT+1
        sta PFA
@l:     lda PFA
        cmp PFB
        bcs @r
        tay
        BRD8
        cmp #'<'
        bne @o
        lda #'('
@o:     jsr pf_c
        inc PFA
        bra @l
@r:     rts

; ================================================================
; Dateiname fuer *:Downloads aus [LP2] (FNLEN Zeichen) -> DLPATH, DLNN
; %XX aufgeloest, Trennzeichen werden '.', hoechstens 15 Zeichen (die
; Endung bleibt), beginnt mit einem Buchstaben
; ================================================================
xf_fname:
        jsr fn_make
        rtl
fn_make:
        stz PFA                 ; Zeichen in FNB
        stz PFB                 ; Quelle
@l:     lda PFB
        cmp FNLEN
        bcs @e1
        tay
        lda [LP2],y
        and #$00FF
        inc PFB
        cmp #'%'
        bne @m
        lda PFB
        inc a
        cmp FNLEN
        bcs @m0
        iny
        lda [LP2],y
        jsr hexv
        asl
        asl
        asl
        asl
        sta PFC
        iny
        lda [LP2],y
        jsr hexv
        ora PFC
        inc PFB
        inc PFB
        bra @m
@m0:    lda #'.'
@m:     jsr fn_map
        beq @l
        cmp #'.'
        bne @put
        ldx PFA                 ; kein Punkt vorn, keine zwei hintereinander
        beq @l
        dex
        lda FNB,x
        and #$00FF
        cmp #'.'
        beq @l
        lda #'.'
@put:   ldx PFA
        cpx #40
        bcs @l
        sep #$20
        sta FNB,x
        rep #$20
        inc PFA
        bra @l
@e1:    ldx PFA                 ; Punkt am Ende weg
        beq @def
        dex
        lda FNB,x
        and #$00FF
        cmp #'.'
        bne @e2
        stx PFA
        bra @e1
@e2:    lda FNB                 ; muss mit einem Buchstaben anfangen
        and #$00DF
        cmp #'A'
        bcc @f
        cmp #'Z'+1
        bcc @tr
@f:     ldx PFA
        cpx #40
        bcc @f1
        dex
@f1:    lda FNB-1,x
        sep #$20
        sta FNB,x
        rep #$20
        dex
        bne @f1
        sep #$20
        lda #'F'
        sta FNB
        rep #$20
        lda PFA
        cmp #40
        bcs @tr
        inc PFA
        bra @tr
@def:   ldx #6
@dc:    TBL fn_dfl
        sta FNB,x
        dex
        dex
        bpl @dc
        lda #8
        sta PFA
@tr:    lda PFA                 ; auf 15 kuerzen, Endung behalten
        cmp #16
        bcs *+5
        jmp @mk
        ldx PFA                 ; letzter Punkt
@p:     dex
        beq @cut
        lda FNB,x
        and #$00FF
        cmp #'.'
        bne @p
        stx PFC                 ; Endung FNB[PFC..PFA)
        lda PFA
        sec
        sbc PFC
        cmp #6
        bcs @cut
        sta PFT                 ; Laenge der Endung
        lda #15
        sec
        sbc PFT
        tay                     ; Ziel
        ldx PFC
@cp:    cpx PFA
        bcs @cpe
        lda FNB,x
        sep #$20
        sta FNB,y
        rep #$20
        inx
        iny
        bra @cp
@cpe:   lda #15
        sec
        sbc PFT
        tax                     ; kein ".." an der Nahtstelle
        dex
        lda FNB,x
        and #$00FF
        cmp #'.'
        bne @cut
        lda FNB+1,x
        and #$00FF
        cmp #'.'
        bne @cut
        ldy #14                 ; ein Zeichen nach vorn schieben
@sh:    lda FNB+1,x
        sep #$20
        sta FNB,x
        rep #$20
        inx
        cpx #14
        bcc @sh
        lda #14
        sta PFA
        bra @mk
@cut:   lda #15
        sta PFA
@mk:    ldx #10                 ; "*:Downloads:" + Name
@mp:    TBL fn_pfx
        sta DLPATH+2,x
        dex
        dex
        bpl @mp
        ldx #0
@mn:    cpx PFA
        bcs @md
        lda FNB,x
        sep #$20
        sta DLPATH+14,x
        rep #$20
        inx
        bra @mn
@md:    lda PFA
        sta DLNN
        clc
        adc #12
        sta DLPATH
        rts
; Zeichen fuer ProDOS-Namen: Buchstabe/Ziffer/Punkt bleiben, Trenner -> '.', sonst 0
fn_map: cmp #'0'
        bcc @sep
        cmp #'9'+1
        bcc @y
        cmp #'A'
        bcc @sep
        cmp #'Z'+1
        bcc @y
        cmp #'a'
        bcc @sep
        cmp #'z'+1
        bcc @y
        cmp #$80
        bcs @no
@sep:   cmp #' '
        bcc @no
        lda #'.'
@y:     ora #0
        rts
@no:    lda #0
        rts

; ---------- Texte ----------
arc_def:   .byte arc_def_e-arc_def-1,"http://mirrors.apple2.org.za/ftp.apple.asimov.net/"
arc_def_e:
fn_pfx:    .byte "*:Downloads:"
fn_dfl:    .byte "DOWNLOAD"
pfs_xml:   .byte 3,"xml"
pfs_html:  .byte 4,"html"
pfs_href:  .byte 4,"href"
pfs_rel:   .byte 3,"rel"
pfs_cdo:   .byte 9,"<![CDATA["
pfs_cde:   .byte 3,"]]>"
pfs_t1:    .byte 7,"<title>"
pfs_t2:    .byte 12,"</title><h1>"
pfs_t3:    .byte 8,"</h1><p>"
pfs_h1e:   .byte 5,"</h1>"
pfs_t4:    .byte 11,"</p><p><b>/"
pfs_t5:    .byte 8,"</b></p>"
pfs_t6:    .byte pfs_t6e-pfs_t6-1,"<p><a href=",$22,"../",$22,">"
pfs_t6e:
pfs_t7:    .byte 8,"</a></p>"
pfs_br:    .byte 4,"<br>"
pfs_b:     .byte 3,"<b>"
pfs_be:    .byte 4,"</b>"
pfs_p:     .byte 3,"<p>"
pfs_pe:    .byte 4,"</p>"
pfs_ah:    .byte 9,"<a href=",$22
pfs_ac:    .byte 2,$22,">"
pfs_ae:    .byte 4,"</a>"
pfs_ell:   .byte 3,"..."
pfs_arc:   .byte pfs_arc_e-pfs_arc-1,"Software-Archiv"
pfs_arc_e:
pfs_arc_en: .byte pfs_arc_en_e-pfs_arc_en-1,"Software Archive"
pfs_arc_en_e:
pfs_ahint: .byte pfs_ahint_e-pfs_ahint-1,"Ordner &ouml;ffnen mit einem Klick, Dateien kommen nach *:Downloads (Disk-Images dann mit DiskMaker auf Diskette). Anderer Spiegel: about:archiv=Adresse"
pfs_ahint_e:
pfs_ahint_en: .byte pfs_ahint_en_e-pfs_ahint_en-1,"Click a folder to open it; files go to *:Downloads (write disk images to a disk with DiskMaker). Other mirror: about:archiv=address"
pfs_ahint_en_e:
pfs_up:    .byte pfs_up_e-pfs_up-1,".. (&uuml;bergeordneter Ordner)"
pfs_up_e:
pfs_up_en: .byte pfs_up_en_e-pfs_up_en-1,".. (parent folder)"
pfs_up_en_e:
pfs_more:  .byte pfs_more_e-pfs_more-1,"Weiterlesen"
pfs_more_e:
pfs_more_en: .byte pfs_more_en_e-pfs_more_en-1,"Read more"
pfs_more_en_e:
; Entitaeten im Feed-Text: Laenge, Name, Zeichen
rs_etab:   .byte 4,"&lt;",'<'
           .byte 4,"&gt;",'>'
           .byte 5,"&amp;",'&'
           .byte 6,"&quot;",$22
           .byte 0
; Tag-Namen: Laenge, Name (klein), Art
rs_ntab:   .byte 4,"item",1
           .byte 5,"entry",1
           .byte 5,"title",2
           .byte 4,"link",3
           .byte 11,"description",4
           .byte 7,"summary",4
           .byte 7,"content",6
           .byte 7,"pubdate",5
           .byte 7,"updated",5
           .byte 9,"published",5
           .byte 4,"date",5
           .byte 0
rss_page:
        .byte "<title>Nachrichten</title><h1>Nachrichten</h1>"
        .byte "<p>Ein Klick auf einen Feed zeigt seine Meldungen. Eigene Feeds: einfach die Adresse (RSS oder Atom) eintippen; als Favorit merken mit Strg-D.</p>"
        .byte "<h2>Apple II</h2><ul>"
        .byte "<li><a href=",$22,"https://www.a2central.com/feed/",$22,">A2Central</a>"
        .byte "<li><a href=",$22,"https://www.callapple.org/feed/",$22,">Call-A.P.P.L.E.</a>"
        .byte "<li><a href=",$22,"https://www.kansasfest.org/feed/",$22,">KansasFest</a>"
        .byte "</ul><h2>Deutschland</h2><ul>"
        .byte "<li><a href=",$22,"https://www.tagesschau.de/index~rss2.xml",$22,">tagesschau.de</a>"
        .byte "<li><a href=",$22,"https://www.heise.de/rss/heise-atom.xml",$22,">heise online</a>"
        .byte "</ul><h2>International</h2><ul>"
        .byte "<li><a href=",$22,"https://hnrss.org/frontpage",$22,">Hacker News</a>"
        .byte "</ul><h2>Ohne https</h2><ul>"
        .byte "<li><a href=",$22,"http://feeds.bbci.co.uk/news/rss.xml",$22,">BBC News</a>"
        .byte "<li><a href=",$22,"http://rss.slashdot.org/Slashdot/slashdotMain",$22,">Slashdot</a>"
        .byte "<li><a href=",$22,"http://www.osnews.com/files/recent.xml",$22,">OSnews</a>"
        .byte "<li><a href=",$22,"http://feeds.feedburner.com/TechTinkering",$22,">TechTinkering (Retro)</a>"
        .byte "</ul><p>Ohne HTTPS-f&auml;hige Karte (CoGS, FujiNet) braucht https einen Proxy (about:proxy).</p>",0
rss_page_en:
        .byte "<title>News</title><h1>News</h1>"
        .byte "<p>Click a feed to see its headlines. Your own feeds: just type the address (RSS or Atom); bookmark it with Ctrl-D.</p>"
        .byte "<h2>Apple II</h2><ul>"
        .byte "<li><a href=",$22,"https://www.a2central.com/feed/",$22,">A2Central</a>"
        .byte "<li><a href=",$22,"https://www.callapple.org/feed/",$22,">Call-A.P.P.L.E.</a>"
        .byte "<li><a href=",$22,"https://www.kansasfest.org/feed/",$22,">KansasFest</a>"
        .byte "</ul><h2>Germany</h2><ul>"
        .byte "<li><a href=",$22,"https://www.tagesschau.de/index~rss2.xml",$22,">tagesschau.de</a>"
        .byte "<li><a href=",$22,"https://www.heise.de/rss/heise-atom.xml",$22,">heise online</a>"
        .byte "</ul><h2>International</h2><ul>"
        .byte "<li><a href=",$22,"https://hnrss.org/frontpage",$22,">Hacker News</a>"
        .byte "</ul><h2>Without https</h2><ul>"
        .byte "<li><a href=",$22,"http://feeds.bbci.co.uk/news/rss.xml",$22,">BBC News</a>"
        .byte "<li><a href=",$22,"http://rss.slashdot.org/Slashdot/slashdotMain",$22,">Slashdot</a>"
        .byte "<li><a href=",$22,"http://www.osnews.com/files/recent.xml",$22,">OSnews</a>"
        .byte "<li><a href=",$22,"http://feeds.feedburner.com/TechTinkering",$22,">TechTinkering (Retro)</a>"
        .byte "</ul><p>Without an HTTPS-capable card (CoGS, FujiNet), https needs a proxy (about:proxy).</p>",0

; Seite fuer about:msdos (nach dem Beenden der Eingabeaufforderung sichtbar, dos.inc)
xf_dos_page:
        jsr net_ptr
        stz HPLAIN
        stz HO
        lda #dos_pg
        ldx LANG
        beq @d
        lda #dos_pg_en
@d:     jmp xrc_copy
        .include "dospg.inc"
