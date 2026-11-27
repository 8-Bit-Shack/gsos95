; ================================================================
; GS/OS95 Browser: Favoriten und eigene Startseite (Teil von web.s)
; gespeichert in *:System:GSOS95.NET: NPREF+26 Startseite (Laenge + max. 100),
; NPREF+128 + i*112: 8 Favoriten (Laenge + max. 111)
; about:fav  Liste   about:delN  Favorit N loeschen
; about:homeN  Favorit N als Startseite   about:nohome  Standard-Startseite
; Strg-D merkt die aktuelle Seite, Strg-B zeigt die Liste
; ================================================================
FAVN    = 8
FAVL    = 112
FAV0    = NPREF+128
HOMEP   = NPREF+26              ; bis 79 Zeichen
PROXP   = NPREF+106             ; Proxy "rechner:port" (bis 21 Zeichen)

        .pushseg
        .segment "BSS"
FVI:    .res 2
FVK:    .res 2
        .popseg

; Favorit A -> X = Adresse (Hauptbank)
fav_addr:
        and #$00FF
        sta FVK
        asl
        asl
        asl
        asl                     ; *16
        sta FVI
        asl
        asl
        asl                     ; *128
        sec
        sbc FVI                 ; *112
        clc
        adc #FAV0
        tax
        rts

; aktuelle Seite merken (Lesezeichen, XCODE4). Meldung in der Statuszeile
fav_add:
        JSLF xf_lz_add
        bcs @no
        WS ws_favok
        ldx #0
        jmp st_set
@no:    WS ws_favno
        ldx #0
        jmp st_set

; Text im Puffer bei A gleich Eintrag X? C=0 ja
fav_cmp:
        sta FVK
        tay
        BRD
        sta UN
        lda a:0,x
        and #$00FF
        cmp UN
        bne @n
@l:     lda UN
        beq @y
        inx
        iny
        BRD
        eor a:0,x
        and #$00FF
        bne @n
        dec UN
        bra @l
@y:     clc
        rts
@n:     sec
        rts

; Pufferzeichenkette A -> Eintrag X (Hauptbank)
fav_from:
        tay
        BRD
        sta UN
        inc UN
@l:     BRD
        sep #$20
        sta a:0,x
        rep #$20
        iny
        inx
        dec UN
        bne @l
        rts

; Eintrag X (Hauptbank) -> Puffer Y
fav_to: lda a:0,x
        and #$00FF
        sta UN
        inc UN
@l:     lda a:0,x
        BWR
        iny
        inx
        dec UN
        bne @l
        rts

; Eintraege ab A (Adresse) eins nach vorn schieben, letzten leeren
fav_shift:
        sta FVI
@l:     cmp #FAV0+FAVN*FAVL
        bcs @e
        tax
        sec
        sbc #FAVL
        sta FVK
        ldy #0
@c:     lda a:0,x
        phx
        ldx FVK
        sta a:0,x
        plx
        inc FVK
        inc FVK
        inx
        inx
        iny
        cpy #FAVL/2
        bcc @c
        lda FVI
        clc
        adc #FAVL
        sta FVI
        bra @l
@e:     lda #0
        sep #$20
        sta FAV0+(FAVN-1)*FAVL
        rep #$20
        rts

; Ziel fuer den Start-Knopf nach WB_TURL: eigene Startseite oder about:start
home_turl:
        jsr net_ptr0
        lda HOMEP
        and #$00FF
        beq @std
        ldx #HOMEP
        ldy #WB_TURL
        jmp fav_to
@std:   ldx #0
        ldy #WB_TURL
@c:     lda w_about,x
        BWR
        iny
        inx
        cpx #12
        bcc @c
        rts

; ---------- about:-Seiten ----------
; C=1: Startseite zeigen, C=0: Favoritenseite ist im Rohpuffer
about_cmd:
        ldy #WB_TURL
        BRD
        cmp #8
        bcs *+5
        jmp @std
        ldy #WB_TURL+7
        BRD
        ora #$20
        sta FVK
        ldy #WB_TURL+8
        BRD
        ora #$20
        sta UN
        ldy #WB_TURL+10         ; Nummer bei delN
        BRD
        sec
        sbc #'1'
        sta FVI
        lda FVK
        cmp #'b'                ; about:bbs... Mailbox-Telefonbuch
        bne @nb
        lda #0
        ldy #WB_TURL
        BRD
        cmp #10
        bcc @bc
        ldy #WB_TURL+10
        BRD
        ora #$20
@bc:    jmp bbs_cmd
@nb:    lda FVK
        cmp #'l'                ; about:lz... Lesezeichen (XCODE4)
        bne @nl
        JSLF xf_lz_cmd
        rts
@nl:    cmp #'w'                ; about:wetter
        bne @nw
        JSLF xf_wx_cmd
        rts
@nw:    cmp #'v'                ; about:verlauf
        bne @nv
        lda UN
        cmp #'e'
        bne @nv0
        JSLF xf_hist_page
        rts
@nv0:   lda FVK
@nv:    cmp #'f'
        bne *+5
        jmp @fav
        cmp #'d'
        bne @n1
        lda FVI                 ; loeschen
        cmp #FAVN
        bcc *+5
        jmp @fav
        jsr fav_addr
        txa
        clc
        adc #FAVL
        jsr fav_shift
        JSLF xf_net_save
        jmp @fav
@n1:    cmp #'h'
        bne @n2
        ldy #WB_TURL+11         ; homeN
        BRD
        sec
        sbc #'1'
        cmp #FAVN
        bcc *+5
        jmp @fav
        jsr fav_addr
        ldy #0
@hc:    lda a:0,x
        sta HOMEP,y
        inx
        inx
        iny
        iny
        cpy #80
        bcc @hc
        lda HOMEP               ; hoechstens 79 Zeichen
        and #$00FF
        cmp #80
        bcc @hs
        lda #0
        sep #$20
        sta HOMEP
        rep #$20
@hs:    JSLF xf_net_save
        bra @fav
@n2:    cmp #'p'                ; about:proxy=rechner:port
        bne @n3
        jsr px_set
        JSLF xf_net_save
        bra @fav
@n3:    cmp #'t'                ; about:tz=+1eu / -5us / 0 / off
        bne @n3n
        jsr tz_set
        JSLF xf_net_save
        stz NTPDONE             ; beim naechsten Laden neu stellen
        bra @fav
@n3n:   cmp #'v'                ; about:vt=farbe / about:vt=grau
        bne @n3v
        ldy #WB_TURL+10
        BRD
        ora #$20
        ldx #0
        cmp #'g'                ; grau / gray
        bne @vf
        ldx #$80
@vf:    stx UK
        sep #$20
        lda NPREF+2
        and #$7F
        ora UK
        sta NPREF+2
        rep #$20
        JSLF xf_net_save
        bra @fav
@n3v:   cmp #'n'
        beq *+5
        jmp about_more
        lda UN
        cmp #'e'                ; about:net - Netzwerkstatus
        bne @n3o
        jsr net_page
        clc
        rts
@n3o:   cmp #'o'
        bne @std
        lda #0
        sep #$20
        sta HOMEP
        rep #$20
        JSLF xf_net_save
@fav:   jsr fav_page
        clc
        rts
@std:   sec
        rts

; Netzwerkstatus (about:net) als HTML in den Rohpuffer: Karte, Adressen, HTTPS
net_page:
        JSLF xf_net_start       ; Netz einrichten, falls noch nicht geschehen
        jsr net_ptr0
        stz HO
        lda #w_fv1              ; <title>
        jsr raw_m
        WS ws_nstitle
        jsr raw_x
        lda #w_fv2              ; </title><h1>
        jsr raw_m
        WS ws_nstitle
        jsr raw_x
        lda #w_np1              ; </h1><p>
        jsr raw_m
        lda NSTATE
        bne @on
        WS ws_nsoff
        jsr raw_x
        jmp @https
@on:    WS ws_nscard
        jsr raw_x
        lda NCARD
        asl
        tax
        lda nd_cn,x
        ldy LANG
        beq @de
        lda nd_cn_en,x
@de:    jsr raw_x
        lda NCARD               ; Adressen kennen Uthernet II, CoGS, Marinetti
        cmp #NC_FUJI
        beq @https
        cmp #NC_MODEM
        beq @https
        lda #w_np2              ; <br>
        jsr raw_m
        WS ws_nsip
        jsr raw_x
        ldx #0
        jsr np_ip
        lda #w_np2
        jsr raw_m
        WS ws_nsdns
        jsr raw_x
        ldx #12
        jsr np_ip
@https: lda #w_np2
        jsr raw_m
        lda NCAPS
        and #CAP_TLS
        beq @nt
        WS ws_nstls
        jsr raw_x
        bra @end
@nt:    lda PROXP
        and #$00FF
        beq @np
        WS ws_nsprox
        jsr raw_x
        lda #PROXP
        jsr raw_m
        bra @end
@np:    WS ws_nsnotls
        jsr raw_x
@end:   lda NSTATE              ; Verbindungstest: TCP zu frogfind.com:80, Zeit messen
        beq @set
        lda #w_np2
        jsr raw_m
        WS ws_nstest
        jsr raw_x
        jsr np_test
@set:   lda #w_fvb              ; </p><hr><p>
        jsr raw_m
        WS ws_nsset
        jsr raw_x
        WS ws_nsupd             ; Updates, Tastenkuerzel
        jmp raw_x

; about:keys (Hilfe) und about:update. C=0 Seite im Rohpuffer / BBSGO gesetzt
about_more:
        lda FVK
        cmp #'a'                ; about:archiv - Software-Archiv
        bne @a2
        JSLF xf_arc_cmd
        rts
@a2:    cmp #'r'                ; about:rss - Nachrichten
        bne @a3m
        JSLF xf_rss_cmd
        rts
@a3m:   cmp #'m'                ; about:msdos - Eingabeaufforderung
        bne @a3
        JSLF xf_dos_page
        lda #1
        sta DOSON
        lda WOBBS               ; aus dem Startmenue: bei EXIT schliessen
        cmp #5
        bne @m1
        sta DOSCL
@m1:    jsr vt_enter
        bcc @m2
        stz DOSON               ; kein Speicher: nur die Seite
        stz DOSCL
        clc
        rts
@m2:    JSLF xf_dos_start
        clc
        rts
@a3:    cmp #'u'
        bne @k
        jmp upd_cmd
@k:     cmp #'c'                ; about:cddb - Server fuer CD-Titelnamen
        bne @k2
        jmp cddb_cmd
@k2:    cmp #'k'
        bne @no
        jsr net_ptr0
        stz HO
        JSLF xf_keys_ptr        ; PDST = Seite (XCODE3)
        ldy #0
@l:     lda [PDST],y
        and #$00FF
        beq @e
        phy
        jsr raw_c
        ply
        iny
        bra @l
@e:     clc
        rts
@no:    sec
        rts

; ---------- about:cddb, about:cddb=rechner, about:cddbmail=name@rechner ----------
cddb_cmd:
        jsr net_ptr0
        ldy #WB_TURL
        BRD
        sta UVT                 ; Laenge der Adresse
        cmp #11
        bcc @page
        ldy #WB_TURL+11
        BRD
        cmp #'='
        bne @m
        ldx #CDB_SRV
        ldy #WB_TURL+12
        bra @set
@m:     ora #$20
        cmp #'m'
        bne @page
        lda UVT
        cmp #15
        bcc @page
        ldy #WB_TURL+15
        BRD
        cmp #'='
        bne @page
        ldx #CDB_ML
        ldy #WB_TURL+16
@set:   stx UVS
        tya                     ; Zeichen = Laenge + 1 - (Y - WB_TURL)
        sec
        sbc #WB_TURL
        eor #$FFFF
        sec
        adc UVT
        inc a
        bpl *+5
        lda #0
        cmp #64
        bcc *+5
        lda #63
        sep #$20
        sta a:0,x
        rep #$20
        sta UVT
@c:     lda UVT
        beq @sv
        BRD
        inx
        sep #$20
        sta a:0,x
        rep #$20
        iny
        dec UVT
        bra @c
@sv:    JSLF xf_net_save
@page:  stz HO
        lda #w_fv1              ; <title>
        jsr raw_m
        WS ws_cdtit
        jsr raw_x
        lda #w_fv2              ; </title><h1>
        jsr raw_m
        WS ws_cdtit
        jsr raw_x
        lda #w_np1              ; </h1><p>
        jsr raw_m
        WS ws_cdsrv
        jsr raw_x
        lda CDB_SRV
        and #$00FF
        beq @d1
        lda #CDB_SRV
        jsr raw_m
        bra @d2
@d1:    WS ws_cddef
        jsr raw_x
@d2:    jsr cdb_br
        WS ws_cdml
        jsr raw_x
        lda CDB_ML
        and #$00FF
        beq @d3
        lda #CDB_ML
        jsr raw_m
        bra @d4
@d3:    WS ws_cdnone
        jsr raw_x
@d4:    lda #w_fvb              ; </p><hr><p>
        jsr raw_m
        WS ws_cdhelp
        jsr raw_x
        WS ws_cdhelp2
        jsr raw_x
        clc
        rts
cdb_br: lda #'<'
        jsr raw_c
        lda #'b'
        jsr raw_c
        lda #'r'
        jsr raw_c
        lda #'>'
        jmp raw_c

; ---------- Update-Pruefung ----------
; about:update=URL speichert die Adresse der Versionsdatei (NPREF+UPDO),
; about:update laedt sie (BBSGO), upd_page wertet sie aus
UPDU = NPREF+1600               ; Laengenbyte + bis 95 Zeichen
upd_cmd:
        jsr net_ptr0
        ldy #WB_TURL
        BRD
        cmp #14
        bcc @use
        ldy #WB_TURL+13
        BRD
        cmp #'='
        bne @use
        ldy #WB_TURL            ; Laenge der neuen Adresse
        BRD
        sec
        sbc #13
        cmp #96
        bcc @l1
        lda #95
@l1:    sta UVT
        sep #$20
        sta UPDU
        rep #$20
        ldx #0
        ldy #WB_TURL+14
@c1:    cpx UVT
        bcs @sv
        BRD
        sep #$20
        sta UPDU+1,x
        rep #$20
        iny
        inx
        bra @c1
@sv:    JSLF xf_net_save
@use:   lda UPDU
        and #$00FF
        beq @def
        inc a                   ; Adresse -> WB_TURL
        sta UVT
        ldx #0
        ldy #WB_TURL
@c2:    lda UPDU,x
        BWR
        iny
        inx
        cpx UVT
        bcc @c2
        bra @go
@def:   lda #ws_updef           ; eingebaute Adresse (mkwebstr.py)
        sta PDST
        CBANK
        sta PDST+2
        lda [PDST]
        and #$00FF
        beq @none
        inc a
        sta UVT
        stz UVS
@c3:    ldy UVS
        lda [PDST],y
        pha
        tya
        clc
        adc #WB_TURL
        tay
        pla
        BWR
        inc UVS
        lda UVS
        cmp UVT
        bcc @c3
@go:    lda #1
        sta BBSGO
        sta UPDCHK
        clc
        rts
@none:  stz HO                  ; keine Adresse: Hinweis
        jsr upd_head
        lda #w_fvb
        jsr raw_m
        WS ws_upnourl
        jsr raw_x
        clc
        rts

; Kopf der Update-Seite: Titel, installierte Version
upd_head:
        lda #w_fv1              ; <title>
        jsr raw_m
        WS ws_uptitle
        jsr raw_x
        lda #w_fv2              ; </title><h1>
        jsr raw_m
        WS ws_uptitle
        jsr raw_x
        lda #w_np1              ; </h1><p>
        jsr raw_m
        WS ws_upinst
        jsr raw_x
        lda #ws_bld
        jmp raw_x

; Zeile ab A bis Zeilenende -> A = Ende, X = Anfang der naechsten Zeile
upd_eol:
        tay
@l:     cpy BEND
        bcs @e
        BRD
        cmp #13
        beq @e
        cmp #10
        beq @e
        iny
        bra @l
@e:     sty UVT
@s:     cpy BEND
        bcs @x
        BRD
        cmp #13
        beq @n
        cmp #10
        bne @x
@n:     iny
        bra @s
@x:     tyx
        lda UVT
        rts

; Pufferstueck A..X als Text in den Rohpuffer
upd_txt:
        stx UVT
        tay
@l:     cpy UVT
        bcs @r
        BRD
        cmp #'<'
        bne @c
        lda #'('
@c:     phy
        jsr raw_c
        ply
        iny
        bra @l
@r:     rts

; Versionsdatei HEND..BEND (Zeile 1 Build, 2 Name, 3 Link) -> HTML dahinter,
; HEND/BEND zeigen danach auf die neue Seite
upd_page:
        stz UPDCHK
        stz HPLAIN
        jsr net_ptr0
        lda HEND
        ldx #0
@ln:    sta UVL,x               ; drei Zeilen suchen
        phx
        jsr upd_eol
        sta UVT
        txa
        plx
        pha
        lda UVT
        sta UVL+2,x
        pla
        inx
        inx
        inx
        inx
        cpx #12
        bcc @ln
        stz UVN                 ; Build a.b.c aus Zeile 1
        stz UVN+2
        stz UVN+4
        ldx #0
        ldy UVL
@p:     cpy UVL+2
        bcs @pe
        BRD
        cmp #'.'
        bne @d
        inx
        inx
        cpx #6
        bcs @pe
        bra @nx
@d:     sec
        sbc #'0'
        cmp #10
        bcs @nx
        sta UVS
        lda UVN,x
        asl
        sta UVT
        asl
        asl
        clc
        adc UVT
        clc
        adc UVS
        sta UVN,x
@nx:    iny
        bra @p
@pe:    lda BEND                ; Seite hinter die Datei schreiben
        clc
        adc #16
        sta HO
        sta UVS
        jsr upd_head
        lda #w_np2              ; <br>
        jsr raw_m
        WS ws_upavail
        jsr raw_x
        lda UVN
        bne @ok
        lda #w_fvb
        jsr raw_m
        WS ws_upbad
        jsr raw_x
        jmp @fin
@ok:    lda UVL                 ; Zeile 1 und 2
        ldx UVL+2
        jsr upd_txt
        lda #' '
        jsr raw_c
        lda UVL+4
        ldx UVL+6
        jsr upd_txt
        lda #w_fvb              ; </p><hr><p>
        jsr raw_m
        lda UVN                 ; neuer als UPD_B1.UPD_B2.UPD_B3?
        cmp #UPD_B1
        bne @cmp
        lda UVN+2
        cmp #UPD_B2
        bne @cmp
        lda UVN+4
        cmp #UPD_B3
        beq @cur
@cmp:   bcc @cur
        WS ws_upnew
        jsr raw_x
        lda UVL+8               ; Link aus Zeile 3
        cmp UVL+10
        bcs @how
        WS ws_upa1
        jsr raw_x
        lda UVL+8
        ldx UVL+10
        jsr upd_txt
        WS ws_upa2
        jsr raw_x
        lda UVL+8
        ldx UVL+10
        jsr upd_txt
        WS ws_upa3
        jsr raw_x
@how:   WS ws_uphow
        jsr raw_x
        bra @fin
@cur:   WS ws_upcur
        jsr raw_x
@fin:   lda #w_fvb              ; Quelle: Adresse der Datei
        jsr raw_m
        WS ws_upsrc
        jsr raw_x
        ldx #WB_URL
        jsr raw_b
        lda UVS
        sta HEND
        lda HO
        sta BEND
        rts

; Verbindung zu frogfind.com:80 auf- und abbauen, Dauer in ms anhaengen
np_test:
        jsr net_ptr0
        ldx #0
        ldy #WB_HOST
@c:     lda w_npt,x
        BWR
        iny
        inx
        cpx #13
        bcc @c
        JSLF xf_net_tick
        sta FVI
        lda NCAPS
        and #CAP_NAME
        bne @o
        JSLF xf_net_resolve
        bcs @f
@o:     lda #80
        sta DPORT
        stz NTLS
        JSLF xf_tcp_open
        bcs @f
        JSLF xf_net_tick
        sec
        sbc FVI
        sta FVK
        JSLF xf_tcp_close
        WS ws_nsok
        jsr raw_x
        lda FVK                 ; Ticks * 17 = ms (1/60 s)
        asl
        asl
        asl
        asl
        clc
        adc FVK
        jsr raw_num0
        WS ws_nsms
        jmp raw_x
@f:     WS ws_nsfail
        jmp raw_x
; Adresse NCUR+X als a.b.c.d
np_ip:  stx FVI
        stz FVK
@l:     ldx FVI
        lda NCUR,x
        and #$00FF
        jsr raw_num0
        inc FVI
        inc FVK
        lda FVK
        cmp #4
        bcs @r
        lda #'.'
        jsr raw_c
        bra @l
@r:     rts

; Favoritenseite als HTML in den Rohpuffer
fav_page:
        jsr net_ptr0
        ldx #0                  ; Adresse wird about:fav
        ldy #WB_TURL
@c:     lda w_favu,x
        BWR
        iny
        inx
        cpx #10
        bcc @c
        ldx #WB_TURL
        ldy #WB_URL
        jsr bs_copy
        stz HO
        lda #w_fv1              ; <title>
        jsr raw_m
        WS ws_fav
        jsr raw_x
        lda #w_fv2              ; </title><h1>
        jsr raw_m
        WS ws_fav
        jsr raw_x
        lda #w_fv3              ; </h1><ul>
        jsr raw_m
        stz FVI
        stz UK
@l:     lda FVI
        jsr fav_addr
        lda a:0,x
        and #$00FF
        bne *+5
        jmp @nx
        inc UK
        phx
        lda #w_fv4              ; <li><a href="
        jsr raw_m
        pla
        pha
        jsr raw_m
        lda #w_fv5              ; ">
        jsr raw_m
        pla
        jsr raw_m
        lda #w_fv6              ; </a> - <a href="about:home
        jsr raw_m
        jsr fv_num
        lda #w_fv5
        jsr raw_m
        WS ws_fhome
        jsr raw_x
        lda #w_fv7              ; </a> <a href="about:del
        jsr raw_m
        jsr fv_num
        lda #w_fv5
        jsr raw_m
        WS ws_fdel
        jsr raw_x
        lda #w_fv8              ; </a>
        jsr raw_m
@nx:    inc FVI
        lda FVI
        cmp #FAVN
        bcs *+5
        jmp @l
        lda UK
        bne @hv
        lda #w_fv9              ; <li>
        jsr raw_m
        WS ws_fnone
        jsr raw_x
@hv:    lda #w_fva              ; </ul><p>
        jsr raw_m
        WS ws_fhelp
        jsr raw_x
        lda #w_fvb              ; <hr><p>
        jsr raw_m
        WS ws_fhis
        jsr raw_x
        lda HOMEP
        and #$00FF
        beq @nh
        lda #HOMEP
        jsr raw_m
        lda #w_fvc              ; <br><a href="about:nohome">
        jsr raw_m
        WS ws_fstd
        jsr raw_x
        lda #w_fv8
        jsr raw_m
        bra @px
@nh:    WS ws_fdef
        jsr raw_x
@px:    lda #w_fvb              ; Proxy
        jsr raw_m
        WS ws_fprox
        jsr raw_x
        lda PROXP
        and #$00FF
        beq @np
        lda #PROXP
        jmp raw_m
@np:    WS ws_fnopx
        jsr raw_x
        lda #w_fvb              ; Telnet-Terminal
        jsr raw_m
        WS ws_fvt
        jsr raw_x
        lda NPREF+2
        and #$0080
        bne @vg
        WS ws_fvtc
        bra @vc
@vg:    WS ws_fvtg
@vc:    jsr raw_x
        lda #w_fvb              ; Zeitzone
        jsr raw_m
        WS ws_ftz
        jsr raw_x
        lda NPREF+3
        and #$00FF
        sta FVK
        and #6
        cmp #6
        bne @tz
        WS ws_ftzoff            ; aus
        jmp raw_x
@tz:    lda #'U'
        jsr raw_c
        lda #'T'
        jsr raw_c
        lda #'C'
        jsr raw_c
        lda FVK
        lsr
        lsr
        lsr
        bne *+5
        lda #17
        sec
        sbc #16
        pha
        lda #'+'
        plx
        phx
        bpl @pl
        lda #'-'
@pl:    jsr raw_c
        pla
        bpl @ab
        eor #$FFFF
        inc a
@ab:    cmp #10
        bcc @one
        sbc #10
        pha
        lda #'1'
        jsr raw_c
        pla
@one:   clc
        adc #'0'
        jsr raw_c
        lda FVK
        and #6
        beq @eu
        cmp #4
        bne @r
        lda #' '
        jsr raw_c
        lda #'U'
        jsr raw_c
        lda #'S'
        jmp raw_c
@eu:    lda #' '
        jsr raw_c
        lda #'E'
        jsr raw_c
        lda #'U'
        jmp raw_c
@r:     rts

; about:tz=WERT (ab WB_TURL+10) -> NPREF+3 Bit 1..7
tz_set: ldy #WB_TURL
        BRD
        clc
        adc #WB_TURL+1
        sta FVK                 ; Ende
        ldy #WB_TURL+10
        jsr tz_ch
        cmp #'o'
        bne @nm
        lda #6                  ; aus (Stunden bleiben)
        sta UN
        lda NPREF+3
        and #$F9
        ora UN
        bra @st
@nm:    stz UN                  ; Vorzeichen
        cmp #'+'
        beq @sg
        cmp #'-'
        bne @d0
        inc UN
@sg:    iny
        jsr tz_ch
@d0:    stz FVI                 ; Stunden
@d:     sec
        sbc #'0'
        cmp #10
        bcs @dn
        pha
        lda FVI
        asl
        asl
        adc FVI
        asl
        sta FVI
        pla
        adc FVI
        sta FVI
        iny
        jsr tz_ch
        bra @d
@dn:    clc                     ; zurueck zum Buchstaben
        adc #'0'
        ldx #2                  ; Regel: e = EU, u = US, sonst keine
        cmp #'e'
        bne @nu
        ldx #0
@nu:    cmp #'u'
        bne @nr
        ldx #4
@nr:    stx UK
        lda FVI
        cmp #15
        bcs @x
        ldx UN
        beq @pos
        eor #$FFFF
        inc a
@pos:   clc
        adc #16
        asl
        asl
        asl
        ora UK
        sta UK
        lda NPREF+3
        and #1
        ora UK
@st:    sep #$20
        sta NPREF+3
        rep #$20
@x:     rts
; Zeichen an Y (klein), 0 hinter dem Ende
tz_ch:  cpy FVK
        bcc @c
        lda #0
        rts
@c:     BRD
        ora #$20
        rts
fv_num: lda FVI
        clc
        adc #'1'
        jmp raw_c

        .pushseg
        .segment "RODATA"
w_favu: .byte w_favu_e-w_favu-1,"about:fav"
w_favu_e:
w_fv1: .byte w_fv1_e-w_fv1-1,"<title>"
w_fv1_e:
w_fv2: .byte w_fv2_e-w_fv2-1,"</title><h1>"
w_fv2_e:
w_fv3: .byte w_fv3_e-w_fv3-1,"</h1><ul>"
w_fv3_e:
w_fv4: .byte w_fv4_e-w_fv4-1,"<li><a href=",$22
w_fv4_e:
w_fv5: .byte w_fv5_e-w_fv5-1,$22,">"
w_fv5_e:
w_fv6: .byte w_fv6_e-w_fv6-1,"</a> &middot; <a href=",$22,"about:home"
w_fv6_e:
w_fv7: .byte w_fv7_e-w_fv7-1,"</a> &middot; <a href=",$22,"about:del"
w_fv7_e:
w_fv8: .byte w_fv8_e-w_fv8-1,"</a>"
w_fv8_e:
w_fv9: .byte w_fv9_e-w_fv9-1,"<li>"
w_fv9_e:
w_fva: .byte w_fva_e-w_fva-1,"</ul><p>"
w_fva_e:
w_fvb: .byte w_fvb_e-w_fvb-1,"</p><hr><p>"
w_fvb_e:
w_fvc: .byte w_fvc_e-w_fvc-1,"<br><a href=",$22,"about:nohome",$22,">"
w_fvc_e:
w_np1: .byte w_np1_e-w_np1-1,"</h1><p>"
w_np1_e:
w_np2: .byte w_np2_e-w_np2-1,"<br>"
w_np2_e:
w_npt: .byte 12,"frogfind.com"
        .popseg
