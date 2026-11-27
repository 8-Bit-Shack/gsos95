; ================================================================
; GS/OS95 Browser: Lesezeichen mit Ordnern, Verlauf, Suchmaschine, Wetter
; (XCODE4, x4.s). Seiten werden als HTML in die Pufferbank geschrieben
; (ab WB_RAW, Laenge HO) wie die anderen about:-Seiten.
;   about:lz              Lesezeichen (Ordner), Formular "merken in Ordner"
;   about:lzadd?o=Ordner  zuletzt besuchte Seite in den Ordner
;   about:lzdel=N         Eintrag N loeschen
;   about:lzs=N           Suchmaschine 1 FrogFind, 2 wiby.me, 3 DuckDuckGo
;   about:verlauf         zuletzt besuchte Seiten
;   about:wetter[?ort=X]  Wetter (wttr.in) fuer den eingestellten Ort
; Strg-D / Stern merkt die aktuelle Seite im zuletzt benutzten Ordner.
; Gespeichert in *:System:GSOS95.LZ (Anzahl + 48 x 192 Bytes); beim ersten
; Mal werden die alten 8 Favoriten (GSOS95.NET) in "Favoriten" uebernommen.
; Suchmaschine: NPREF+1960, Wetter-Ort: NPREF+1961 (Laenge + 30).
; ================================================================
LZ_N    = 48                    ; Eintraege
LZ_SZ   = 192                   ; +0 Ordner (1+23), +24 Titel (1+47), +72 Adresse (1+119)
LZ_F    = 0
LZ_T    = 24
LZ_U    = 72
LZ_E0   = 2                     ; erster Eintrag im Griff
LZ_HIS  = $2500                 ; Verlauf: Anzahl + 20 x 168 (Titel 1+47, Adresse 1+119)
LZ_HN   = 20
LZ_HSZ  = 168
LZ_CUR  = $3300                 ; zuletzt benutzter Ordner (1+23)
LZ_LAST = $3320                 ; zuletzt besuchte Seite: Titel (1+47), Adresse (1+119)
LZ_SIZE = $3400
SRCHE   = NPREF+1960            ; Suchmaschine 0/1 FrogFind, 2 wiby, 3 DuckDuckGo
WXORT   = NPREF+1961            ; Wetter-Ort
        .pushseg
        .segment "BSS"
LZH:    .res 4                  ; Griff (0 = noch keiner)
LZP:    .res 4                  ; Zeiger
LZOK:   .res 2                  ; Datei gelesen
LZI:    .res 2
LZJ:    .res 2
LZK:    .res 2
LZN:    .res 2
LZT:    .res 2
LZT2:   .res 2
LZQ:    .res 2                  ; Lesestelle in WB_TURL
LZPB:   .res 24
LZO:    .res 2
LZC:    .res 2
        .popseg

; ---------- Speicher und Datei ----------
lz_mem: lda LZH
        ora LZH+2
        beq *+5
        jmp @ok
        pha
        pha
        pea $0000
        pea LZ_SIZE
        lda USERID
        pha
        pea $C018
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        pla
        sta LZH
        pla
        sta LZH+2
        bcc @g
        stz LZH
        stz LZH+2
        sec
        rts
@g:     lda LZH
        sta PDST
        lda LZH+2
        sta PDST+2
        ldy #2
        lda [PDST],y
        sta LZP+2
        lda [PDST]
        sta LZP
        jsr lz_dp               ; alles leer
        ldy #0
        lda #0
@z:     sta [PDST],y
        iny
        iny
        cpy #LZ_SIZE
        bcc @z
        ldy #LZ_CUR             ; Ordner "Favoriten"
        ldx #0
@f:     TBL lz_sfav
        sep #$20
        sta [PDST],y
        rep #$20
        iny
        inx
        cpx #10
        bcc @f
@ok:    lda LZOK
        bne @r
        inc LZOK
        jsr lz_load
@r:     clc
        rts
lz_sfav: .byte 9,"Favoriten"

lz_dp:  lda LZP
        sta PDST
        lda LZP+2
        sta PDST+2
        rts

lz_gs:  sta LZT
        lda MYBANK
        pha
        phx
        lda LZT
        pha
        jsl $E100B0
        rts

; Datei lesen; gibt es sie nicht: alte Favoriten uebernehmen
lz_load:
        lda #2
        sta LZPB
        lda #lz_path
        sta LZPB+4
        phk
        sep #$20
        pla
        rep #$20
        and #$00FF
        sta LZPB+6
        lda #$2010              ; Open
        ldx #LZPB
        jsr lz_gs
        bcs @mig
        lda LZPB+2
        sta LZT2
        lda #4                  ; Read: alles
        sta LZPB
        lda LZT2
        sta LZPB+2
        lda LZP
        sta LZPB+4
        lda LZP+2
        sta LZPB+6
        lda #LZ_E0+LZ_N*LZ_SZ
        sta LZPB+8
        stz LZPB+10
        lda #$2012
        ldx #LZPB
        jsr lz_gs
        lda #1
        sta LZPB
        lda LZT2
        sta LZPB+2
        lda #$2014
        ldx #LZPB
        jsr lz_gs
        rts
@mig:   stz LZI                 ; Favoriten (NPREF+128 + i*112, Laenge + Adresse)
@m:     lda LZI
        cmp #8
        bcs @r
        xba
        lsr                     ; *128
        sta LZT
        lda LZI
        asl
        asl
        asl
        asl                     ; *16
        eor #$FFFF
        sec
        adc LZT                 ; *112
        clc
        adc #NPREF+128
        sta LZK
        tax
        lda a:0,x
        and #$00FF
        beq @n
        cmp #120
        bcs @n
        jsr lz_newe             ; X = Eintrag (Offset) oder C=1
        bcs @r
        stx LZJ
        ldx LZK                 ; Adresse und (mangels Titel) Adresse als Titel
        lda LZJ
        clc
        adc #LZ_U
        ldy #119
        jsr lz_cpm
        ldx LZK
        lda LZJ
        clc
        adc #LZ_T
        ldy #47
        jsr lz_cpm
        lda LZJ                 ; Ordner "Favoriten"
        jsr lz_setf
@n:     inc LZI
        bra @m
@r:     rts

; Zeichenkette X (Hauptbank, Laenge + Text) nach Eintragsfeld A (hoechstens Y Zeichen)
lz_cpm: sta LZT
        sty LZT2
        lda a:0,x
        and #$00FF
        cmp LZT2
        bcc @k
        lda LZT2
@k:     sta LZN
        jsr lz_dp
        lda LZN
        ldy LZT
        sep #$20
        sta [PDST],y
        rep #$20
@l:     lda LZN
        beq @r
        inx
        iny
        lda a:0,x
        sep #$20
        sta [PDST],y
        rep #$20
        dec LZN
        bra @l
@r:     rts

; Ordner des Eintrags A = zuletzt benutzter Ordner
lz_setf:
        clc
        adc #LZ_F
        sta LZT
        jsr lz_dp
        ldy #LZ_CUR
        lda [PDST],y
        and #$00FF
        inc a
        sta LZN
        ldx #0
@l:     txa
        clc
        adc #LZ_CUR
        tay
        lda [PDST],y
        pha
        txa
        clc
        adc LZT
        tay
        pla
        sep #$20
        sta [PDST],y
        rep #$20
        inx
        cpx LZN
        bcc @l
        rts

; neuer Eintrag am Ende -> X = Offset. C=1 voll
lz_newe:
        jsr lz_dp
        lda [PDST]
        cmp #LZ_N
        bcs @r
        pha
        inc a
        sta [PDST]
        pla
        jsr lz_off
        tax
        clc
@r:     rts
; Offset von Eintrag A
lz_off: sta LZO
        asl
        clc
        adc LZO                 ; *3
        xba                     ; *768
        lsr
        lsr                     ; *192
        clc
        adc #LZ_E0
        rts

lz_save:
        lda #3                  ; Create (gibt es sie schon: egal)
        sta LZPB
        lda #lz_path
        sta LZPB+2
        jsr @bk
        sta LZPB+4
        lda #$00C3
        sta LZPB+6
        lda #$0004              ; TXT
        sta LZPB+8
        lda #$2001
        ldx #LZPB
        jsr lz_gs
        lda #2                  ; Open
        sta LZPB
        lda #lz_path
        sta LZPB+4
        jsr @bk
        sta LZPB+6
        lda #$2010
        ldx #LZPB
        jsr lz_gs
        bcs @r
        lda LZPB+2
        sta LZT2
        lda #4                  ; Write
        sta LZPB
        lda LZT2
        sta LZPB+2
        lda LZP
        sta LZPB+4
        lda LZP+2
        sta LZPB+6
        lda #LZ_E0+LZ_N*LZ_SZ
        sta LZPB+8
        stz LZPB+10
        lda #$2013
        ldx #LZPB
        jsr lz_gs
        lda #1
        sta LZPB
        lda LZT2
        sta LZPB+2
        lda #$2014
        ldx #LZPB
        jsr lz_gs
@r:     rts
@bk:    phk
        sep #$20
        pla
        rep #$20
        and #$00FF
        rts
lz_path: .byte 18,0,"*:SYSTEM:GSOS95.LZ"

; ---------- Verlauf (aus his_push: Seite WB_BASE, Titel WB_TITLE) ----------
xf_hist_add:
        jsr lz_mem
        bcc *+5
        jmp @r
        lda NWB
        sta PSRC+2
        stz PSRC
        ldy #WB_BASE            ; about:-Seiten nicht
        lda [PSRC],y
        and #$00FF
        beq @r
        cmp #120
        bcs @r
        ldy #WB_BASE+1
        lda [PSRC],y
        ora #$2020
        cmp #'b'*256+'a'
        bne @go
        ldy #WB_BASE+6
        lda [PSRC],y
        and #$00FF
        cmp #':'
        beq @r
@go:    jsr lz_dp               ; zuletzt besuchte Seite merken
        lda #LZ_LAST
        jsr lz_tit
        ldy #WB_BASE
        lda #LZ_LAST+48
        jsr lz_cpn
        ldy #LZ_HIS             ; Verlauf: alle eins nach hinten, neu vorn
        lda [PDST],y
        cmp #LZ_HN
        bcs @full
        inc a
        sta [PDST],y
@full:  ldy #LZ_HIS+2+(LZ_HN-1)*LZ_HSZ-2
@sh:    lda [PDST],y
        pha
        tya
        clc
        adc #LZ_HSZ
        tay
        pla
        sta [PDST],y
        tya
        sec
        sbc #LZ_HSZ
        cmp #LZ_HIS+2
        beq @ins
        dec a
        dec a
        tay
        bra @sh
@ins:   ldx #0                  ; zuletzt besuchte Seite als ersten Eintrag
@c:     txa
        clc
        adc #LZ_LAST
        tay
        lda [PDST],y
        pha
        txa
        clc
        adc #LZ_HIS+2
        tay
        pla
        sta [PDST],y
        inx
        inx
        cpx #LZ_HSZ
        bcc @c
@r:     rtl

; Titel WB_TITLE (sonst Adresse) nach Feld A (1+47)
lz_tit: pha
        ldy #WB_TITLE
        lda [PSRC],y
        and #$00FF
        bne @t
        ldy #WB_BASE
@t:     pla
        ; weiter: lz_cpn mit 47 Zeichen
        ldx #47
        bra lz_cpx
; Zeichenkette in der Pufferbank (Offset Y) nach Feld A (1+119)
lz_cpn: ldx #119
lz_cpx: sta LZT
        stx LZT2
        lda [PSRC],y
        and #$00FF
        cmp LZT2
        bcc @k
        lda LZT2
@k:     sta LZN
        sty LZK
        ldy LZT
        sep #$20
        sta [PDST],y
        rep #$20
@l:     lda LZN
        beq @r
        inc LZK
        iny
        phy
        ldy LZK
        lda [PSRC],y
        ply
        sep #$20
        sta [PDST],y
        rep #$20
        dec LZN
        bra @l
@r:     rts

; ---------- Strg-D: aktuelle Seite merken (C=1: geht nicht) ----------
xf_lz_add:
        jsr lz_mem
        bcs @no
        lda NWB
        sta PSRC+2
        stz PSRC
        ldy #WB_BASE
        lda [PSRC],y
        and #$00FF
        beq @no
        cmp #120
        bcs @no
        jsr lz_newe
        bcs @no
        stx LZJ
        txa
        clc
        adc #LZ_T
        jsr lz_tit
        ldy #WB_BASE
        lda LZJ
        clc
        adc #LZ_U
        jsr lz_cpn
        lda LZJ
        jsr lz_setf
        jsr lz_save
        clc
        rtl
@no:    sec
        rtl

; ---------- HTML in die Pufferbank ----------
; Zeichen A
h_c:    phy
        ldy HO
        cpy #WB_TXT-16
        bcs @r
        sep #$20
        sta [PSRC],y
        rep #$20
        inc HO
@r:     ply
        rts
; Text A (diese Bank, mit 0 am Ende)
h_z:    jsr x4_kb
        ldy #0
@l:     lda [LP2],y
        and #$00FF
        beq @r
        jsr h_c
        iny
        bra @l
@r:     rts
; Text A (Deutsch) / X (Englisch)
h_zl:   ldy LANG
        beq @d
        txa
@d:     jmp h_z
; Zeichenkette (Laenge + Text) aus dem Griff (Offset A), Zeichen geschuetzt
h_lz:   sta LZK
        jsr lz_dp
        ldy LZK
        lda [PDST],y
        and #$00FF
        sta LZN
@l:     lda LZN
        beq @r
        inc LZK
        ldy LZK
        lda [PDST],y
        and #$00FF
        jsr h_esc
        dec LZN
        bra @l
@r:     rts
; Zeichen A mit HTML-Schutz
h_esc:  cmp #'<'
        beq @lt
        cmp #'&'
        beq @am
        cmp #'"'
        beq @qu
        jmp h_c
@lt:    lda #h_slt
        bra @s
@am:    lda #h_sam
        bra @s
@qu:    lda #h_squ
@s:     phy
        jsr h_z
        ply
        rts
h_slt:  .byte "&lt;",0
h_sam:  .byte "&amp;",0
h_squ:  .byte "&quot;",0
; Zahl A (0..999)
h_num:  ldx #0
@h:     cmp #100
        bcc @t
        sbc #100
        inx
        bra @h
@t:     stx LZT
        ldx #0
@tt:    cmp #10
        bcc @o
        sbc #10
        inx
        bra @tt
@o:     pha
        stx LZT2
        lda LZT
        beq @nt
        ora #'0'
        jsr h_c
        lda LZT2
        bra @t2
@nt:    lda LZT2
        beq @on
@t2:    ora #'0'
        jsr h_c
@on:    pla
        ora #'0'
        jmp h_c

h_begin:
        lda NWB
        sta PSRC+2
        stz PSRC
        stz HPLAIN
        stz HO
        rts

; ---------- about:lz... ----------
xf_lz_cmd:
        jsr h_begin
        jsr lz_mem
        bcc @m
        clc
        rtl
@m:     ldy #WB_TURL            ; Laenge der Adresse
        lda [PSRC],y
        and #$00FF
        sta LZQ
        cmp #10
        bcc @page               ; "about:lz"
        ldy #WB_TURL+9          ; about:lz?: a(dd), d(el), s(uche)
        lda [PSRC],y
        ora #$20
        and #$00FF
        cmp #'a'
        beq @add
        cmp #'d'
        beq @del
        cmp #'h'
        beq @home
        cmp #'s'
        bne @page
        ldy #WB_TURL+11         ; about:lzs=N
        lda [PSRC],y
        and #$000F
        cmp #4
        bcs @page
        sep #$20
        sta SRCHE
        rep #$20
        JSLF xf_net_save
        bra @page
@del:   ldy #WB_TURL+13         ; about:lzdel=N (1..48)
        jsr lz_dec
        dec a
        jsr lz_rm
        jsr lz_save
        bra @page
@home:  ldy #WB_TURL+14         ; about:lzhome=N: als Startseite
        jsr lz_dec
        dec a
        jsr lz_home
        bra @page
@add:   jsr lz_form             ; about:lzadd?o=Ordner
        jsr lz_save
@page:  jsr h_begin
        jmp lz_page

; Zahl ab WB_TURL-Offset Y -> A
lz_dec: stz LZT
@l:     lda [PSRC],y
        and #$00FF
        sec
        sbc #'0'
        cmp #10
        bcs @r
        sta LZT2
        lda LZT
        asl
        asl
        clc
        adc LZT
        asl
        clc
        adc LZT2
        sta LZT
        iny
        bra @l
@r:     lda LZT
        rts

; Adresse von Eintrag A als Startseite (HOMEP, hoechstens 79 Zeichen)
lz_home:
        sta LZI
        jsr lz_dp
        lda LZI
        cmp [PDST]
        bcs @r
        jsr lz_off
        clc
        adc #LZ_U
        tay
        lda [PDST],y
        and #$00FF
        cmp #80
        bcc @n
        lda #79
@n:     sta LZN
        sep #$20
        sta HOMEP
        rep #$20
        ldx #1
@c:     lda LZN
        beq @s
        iny
        lda [PDST],y
        sep #$20
        sta HOMEP,x
        rep #$20
        inx
        dec LZN
        bra @c
@s:     JSLF xf_net_save
@r:     rts

; Eintrag A entfernen
lz_rm:  sta LZI
        jsr lz_dp
        lda LZI
        cmp [PDST]
        bcs @r
        jsr lz_off
        tay                     ; Rest nach vorn
@l:     tya
        clc
        adc #LZ_SZ
        cmp #LZ_E0+LZ_N*LZ_SZ
        bcs @e
        phy
        tay
        lda [PDST],y
        ply
        sta [PDST],y
        iny
        iny
        bra @l
@e:     lda [PDST]
        dec a
        sta [PDST]
@r:     rts

; Formular ?o=Ordner: Ordner aus der Adresse (+ = Leerzeichen, %XX), dann merken
lz_form:
        ldy #WB_TURL+14         ; "about:lzadd?o=": '=' bei +14, Wert ab +15
        lda [PSRC],y
        and #$00FF
        cmp #'='
        beq @v
        rts
@v:     iny
        jsr lz_dp
        ldx #0                  ; Ordner nach LZ_CUR
@c:     tya
        sec
        sbc #WB_TURL
        cmp LZQ
        beq @c1
        bcs @e
@c1:    lda [PSRC],y
        and #$00FF
        iny
        cmp #'&'
        beq @e
        cmp #'+'
        bne @p
        lda #' '
@p:     cmp #'%'
        bne @s
        jsr lz_hex
        sta LZT
        jsr lz_hex
        sta LZT2
        lda LZT
        asl
        asl
        asl
        asl
        ora LZT2
@s:     cpx #23
        bcs @c
        phy
        pha
        txa
        clc
        adc #LZ_CUR+1
        tay
        pla
        sep #$20
        sta [PDST],y
        rep #$20
        ply
        inx
        bra @c
@e:     txa
        beq @r                  ; leer: alter Ordner bleibt
        ldy #LZ_CUR
        sep #$20
        sta [PDST],y
        rep #$20
        ldy #LZ_LAST+48         ; zuletzt besuchte Seite in diesen Ordner
        lda [PDST],y
        and #$00FF
        beq @r
        jsr lz_newe
        bcs @r
        stx LZJ
        ldx #0                  ; Titel und Adresse kopieren
@k:     txa
        clc
        adc #LZ_LAST
        tay
        lda [PDST],y
        pha
        txa
        clc
        adc LZJ
        adc #LZ_T
        tay
        pla
        sta [PDST],y
        inx
        inx
        cpx #168
        bcc @k
        lda LZJ
        jmp lz_setf
@r:     rts
; Hex-Ziffer an [PSRC],y -> A (0..15), Y weiter
lz_hex: lda [PSRC],y
        and #$00FF
        iny
        cmp #'A'
        bcc @d
        and #$000F
        clc
        adc #9
        and #$000F
        rts
@d:     and #$000F
        rts

lz_page:
        lda #lz_t1
        ldx #lz_t1_en
        jsr h_zl
        ; Formular: zuletzt besuchte Seite merken
        jsr lz_dp
        ldy #LZ_LAST+48
        lda [PDST],y
        and #$00FF
        beq @nl
        lda #lz_t2
        ldx #lz_t2_en
        jsr h_zl
        lda #LZ_LAST
        jsr h_lz
        lda #lz_t3
        ldx #lz_t3_en
        jsr h_zl
        lda #LZ_CUR
        jsr h_lz
        lda #lz_t4
        ldx #lz_t4_en
        jsr h_zl
@nl:    ; Ordner der Reihe nach: jeder Ordner, der noch nicht gezeigt wurde
        stz LZI
@fo:    jsr lz_dp
        lda LZI
        cmp [PDST]
        bcc *+5
        jmp @tail
        jsr @seen               ; Ordner schon vorher? dann weiter
        bcs @nf
        lda #lz_h2
        jsr h_z
        lda LZI
        jsr lz_off
        jsr h_lz
        lda #lz_h2e
        jsr h_z
        lda LZI                 ; alle Eintraege mit diesem Ordner
        sta LZJ
@en:    jsr lz_dp
        lda LZJ
        cmp [PDST]
        bcs @ee
        jsr @same
        bcs @ne
        lda #lz_li
        jsr h_z
        lda LZJ
        jsr lz_off
        clc
        adc #LZ_U
        jsr h_lz
        lda #lz_li2
        jsr h_z
        lda LZJ
        jsr lz_off
        clc
        adc #LZ_T
        jsr h_lz
        lda #lz_li3
        jsr h_z
        lda LZJ
        inc a
        jsr h_num
        lda #lz_li4
        jsr h_z
        lda LZJ
        inc a
        jsr h_num
        lda #lz_li5
        ldx #lz_li5_en
        jsr h_zl
@ne:    inc LZJ
        bra @en
@ee:    lda #lz_ul
        jsr h_z
@nf:    inc LZI
        jmp @fo
@tail:  jsr lz_dp
        lda [PDST]
        bne @t2
        lda #lz_empty
        ldx #lz_empty_en
        jsr h_zl
@t2:    lda #lz_t5              ; Verlauf, Suchmaschine
        ldx #lz_t5_en
        jsr h_zl
        lda SRCHE
        and #$0003
        bne @se
        inc a
@se:    sta LZT
        lda #1
        sta LZK
@sl:    lda LZK
        cmp LZT
        bne @sa
        lda #lz_bo
        jsr h_z
        bra @sn
@sa:    lda #lz_sa
        jsr h_z
        lda LZK
        ora #'0'
        jsr h_c
        lda #lz_sa2
        jsr h_z
@sn:    lda LZK
        dec a
        asl
        tax
        TBL lz_snames
        jsr h_z
        lda LZK
        cmp LZT
        bne @sc
        lda #lz_bc
        jsr h_z
        bra @sd
@sc:    lda #lz_ac
        jsr h_z
@sd:    lda #lz_bar
        jsr h_z
        inc LZK
        lda LZK
        cmp #4
        bcc @sl
        lda #lz_t6
        ldx #lz_t6_en
        jsr h_zl
        clc
        rtl
; Ordner von Eintrag LZI bei einem frueheren Eintrag? C=1 ja
@seen:  stz LZJ
@s1:    lda LZJ
        cmp LZI
        bcs @no
        jsr @same
        bcc @yes
        inc LZJ
        bra @s1
@yes:   sec
        rts
@no:    clc
        rts
; Ordner von LZJ gleich Ordner von LZI? C=0 ja
@same:  lda LZI
        jsr lz_off
        sta LZT
        lda LZJ
        jsr lz_off
        sta LZT2
        jsr lz_dp
        ldy LZT
        lda [PDST],y
        and #$00FF
        inc a
        sta LZN
        ldx #0
@sm:    txa
        clc
        adc LZT
        tay
        lda [PDST],y
        and #$00FF
        sta LZK
        txa
        clc
        adc LZT2
        tay
        lda [PDST],y
        and #$00FF
        cmp LZK
        bne @dif
        inx
        cpx LZN
        bcc @sm
        clc
        rts
@dif:   sec
        rts

lz_snames: .addr lz_s1, lz_s2, lz_s3
lz_s1:  .byte "FrogFind",0
lz_s2:  .byte "wiby.me",0
lz_s3:  .byte "DuckDuckGo (https)",0
lz_bo:  .byte "<b>",0
lz_bc:  .byte "</b>",0
lz_sa:  .byte "<a href=",$22,"about:lzs=",0
lz_sa2: .byte $22,">",0
lz_ac:  .byte "</a>",0
lz_bar: .byte " &nbsp; ",0
lz_h2:  .byte "<h2>",0
lz_h2e: .byte "</h2><ul>",0
lz_ul:  .byte "</ul>",0
lz_li:  .byte "<li><a href=",$22,0
lz_li2: .byte $22,">",0
lz_li3: .byte "</a> &nbsp; <a href=",$22,"about:lzdel=",0
lz_li4: .byte $22,">[x]</a> &nbsp; <a href=",$22,"about:lzhome=",0
lz_li5: .byte $22,">[Startseite]</a>",0
lz_li5_en: .byte $22,">[Home]</a>",0
lz_t1:  .byte "<title>Lesezeichen</title><h1>Lesezeichen</h1>",0
lz_t1_en: .byte "<title>Bookmarks</title><h1>Bookmarks</h1>",0
lz_t2:  .byte "<form action=",$22,"about:lzadd",$22,"><p>Zuletzt besucht: <b>",0
lz_t2_en: .byte "<form action=",$22,"about:lzadd",$22,"><p>Last visited: <b>",0
lz_t3:  .byte "</b><br>merken im Ordner <input name=",$22,"o",$22," value=",$22,0
lz_t3_en: .byte "</b><br>keep in folder <input name=",$22,"o",$22," value=",$22,0
lz_t4:  .byte $22,"> <input type=",$22,"submit",$22," value=",$22,"Merken",$22,"></p></form><hr>",0
lz_t4_en: .byte $22,"> <input type=",$22,"submit",$22," value=",$22,"Add",$22,"></p></form><hr>",0
lz_empty: .byte "<p>Noch keine Lesezeichen. Strg-D oder der Stern merkt die angezeigte Seite.</p>",0
lz_empty_en: .byte "<p>No bookmarks yet. Ctrl-D or the star keeps the page you are looking at.</p>",0
lz_t5:  .byte "<hr><p><a href=",$22,"about:verlauf",$22,">Verlauf</a> &nbsp; <a href=",$22,"about:wetter",$22,">Wetter</a></p><h2>Suchmaschine</h2><p>Suchbegriffe in der Adresszeile gehen an: ",0
lz_t5_en: .byte "<hr><p><a href=",$22,"about:verlauf",$22,">History</a> &nbsp; <a href=",$22,"about:wetter",$22,">Weather</a></p><h2>Search engine</h2><p>Words typed in the address bar go to: ",0
lz_t6:  .byte "</p><p>(DuckDuckGo braucht die CoGS- oder FujiNet-Karte.)</p>",0
lz_t6_en: .byte "</p><p>(DuckDuckGo needs the CoGS or FujiNet card.)</p>",0

; ---------- about:verlauf ----------
xf_hist_page:
        jsr h_begin
        jsr lz_mem
        bcc @m
        clc
        rtl
@m:     lda #lz_v1
        ldx #lz_v1_en
        jsr h_zl
        jsr lz_dp
        ldy #LZ_HIS
        lda [PDST],y
        sta LZC
        bne @go
        lda #lz_v3
        ldx #lz_v3_en
        jsr h_zl
        clc
        rtl
@go:    stz LZI
@l:     lda LZI
        cmp LZC
        bcs @e
        lda LZI                 ; Eintrag: LZ_HIS+2 + i*168
        ldx #LZ_HSZ
        jsr d4_mul
        clc
        adc #LZ_HIS+2
        sta LZJ
        lda #lz_li
        jsr h_z
        lda LZJ
        clc
        adc #48
        jsr h_lz
        lda #lz_li2
        jsr h_z
        lda LZJ
        jsr h_lz
        lda #lz_v2
        jsr h_z
        inc LZI
        bra @l
@e:     lda #lz_ul
        jsr h_z
        clc
        rtl
lz_v1:  .byte "<title>Verlauf</title><h1>Verlauf</h1><p>Zuletzt besuchte Seiten (seit dem Start):</p><ul>",0
lz_v1_en: .byte "<title>History</title><h1>History</h1><p>Pages visited since start-up:</p><ul>",0
lz_v2:  .byte "</a></li>",0
lz_v3:  .byte "</ul><p>Noch keine Seiten besucht.</p>",0
lz_v3_en: .byte "</ul><p>No pages visited yet.</p>",0

; ---------- Suchbegriffe: Anfang der Adresse -> A (Hauptbank-Text fuer tb_m) ----------
; (url.inc fragt SRCHE selbst ab; hier nichts)

; ---------- about:wetter ----------
; ?ort=X speichert den Ort. Mit Ort: weiter zu wttr.in (ueber BBSGO/WB_TURL)
xf_wx_cmd:
        jsr h_begin
        ldy #WB_TURL
        lda [PSRC],y
        and #$00FF
        sta LZQ
        cmp #17                 ; "about:wetter?ort=" + mindestens 1 Zeichen
        bcc @noq
        ldy #WB_TURL+13
        lda [PSRC],y
        and #$00FF
        cmp #'?'
        bne @noq
        ldy #WB_TURL+18         ; Ort speichern (Laenge + bis 30)
        ldx #0
@c:     tya
        sec
        sbc #WB_TURL
        cmp LZQ
        beq @ce1
        bcs @ce
@ce1:   lda [PSRC],y
        and #$00FF
        iny
        cmp #'&'
        beq @ce
        cmp #'%'                ; %XX
        bne @nh
        jsr lz_hex
        asl
        asl
        asl
        asl
        sta LZT
        jsr lz_hex
        ora LZT
@nh:    cpx #30
        bcs @c
        sep #$20
        sta WXORT+1,x
        rep #$20
        inx
        bra @c
@ce:    txa
        sep #$20
        sta WXORT
        rep #$20
        JSLF xf_net_save
@noq:   lda WXORT
        and #$00FF
        bne @go
        jmp @form
@go:    lda #wx_u1              ; WB_TURL = "http(s)://wttr.in/Ort?0T&lang=de"
        ldx NCAPS
        txa
        and #2                  ; CAP_TLS
        beq @h
        lda #wx_u1s
        bra @h2
@h:     lda #wx_u1
@h2:    jsr x4_kb
        ldx #WB_TURL+1
        ldy #0
@u:     lda [LP2],y
        and #$00FF
        beq @ue
        phy
        txy
        sep #$20
        sta [PSRC],y
        rep #$20
        ply
        inx
        iny
        bra @u
@ue:    lda WXORT               ; Ort (Leerzeichen als +)
        and #$00FF
        sta LZN
        ldy #0
@o:     cpy LZN
        bcs @oe
        lda WXORT+1,y
        and #$00FF
        cmp #' '
        bne @o2
        lda #'+'
@o2:    phy
        txy
        sep #$20
        sta [PSRC],y
        rep #$20
        ply
        inx
        iny
        bra @o
@oe:    lda #wx_u2
        ldy LANG
        beq @l
        lda #wx_u2e
@l:     jsr x4_kb
        ldy #0
@u2:    lda [LP2],y
        and #$00FF
        beq @u3
        phy
        txy
        sep #$20
        sta [PSRC],y
        rep #$20
        ply
        inx
        iny
        bra @u2
@u3:    txa
        sec
        sbc #WB_TURL+1
        ldy #WB_TURL
        sep #$20
        sta [PSRC],y
        rep #$20
        lda #1
        sta BBSGO
        clc
        rtl
@form:  lda #wx_f
        ldx #wx_f_en
        jsr h_zl
        clc
        rtl
wx_u1:  .byte "http://wttr.in/",0
wx_u1s: .byte "https://wttr.in/",0
wx_u2:  .byte "?0T&lang=de",0
wx_u2e: .byte "?0T",0
wx_f:   .byte "<title>Wetter</title><h1>Wetter</h1><form action=",$22,"about:wetter",$22,"><p>F&uuml;r welchen Ort? <input name=",$22,"ort",$22,"> <input type=",$22,"submit",$22," value=",$22,"Anzeigen",$22,"></p></form><p>Das Wetter kommt von wttr.in. Den Ort &auml;ndern: about:wetter?ort=Ortsname</p>",0
wx_f_en: .byte "<title>Weather</title><h1>Weather</h1><form action=",$22,"about:wetter",$22,"><p>Which town? <input name=",$22,"ort",$22,"> <input type=",$22,"submit",$22," value=",$22,"Show",$22,"></p></form><p>The weather comes from wttr.in. To change the town: about:wetter?ort=name</p>",0
