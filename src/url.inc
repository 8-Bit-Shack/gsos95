; ================================================================
; GS/OS95 Browser: Adressen zerlegen/zusammensetzen, HTTP-Anfrage und -Kopf
; (Teil von web.s, alle Zeichenketten im Puffer: Laengenbyte + Text)
; ================================================================
        .pushseg
        .segment "BSS"
US:     .res 2
UE:     .res 2
UP:     .res 2
UQ:     .res 2
UAQ:    .res 2
URD:     .res 2
UWR:     .res 2
UC:     .res 2
UX:     .res 2
UT_N:   .res 2
UT_P:   .res 2
UT_C:   .res 2
UDIR:   .res 2
WPORT:  .res 2
BS0:    .res 2
BE0:    .res 2
BH0:    .res 2
BP0:    .res 2
BQ0:    .res 2
BL0:    .res 2
        .popseg
        .pushseg
        .segment "RODATA"
u_about:  .byte 6,"about:"
u_http:   .byte 4,"http"
u_https:  .byte 5,"https"
u_httpc:  .byte 5,"http:"
u_httpsc: .byte 6,"https:"
u_pre:    .byte 7,"http://"
u_pres:   .byte 8,"https://"
u_preg:   .byte 9,"gopher://"
u_gopher: .byte 6,"gopher"
u_gopherc: .byte 7,"gopher:"
u_telnet: .byte 6,"telnet"
u_telnetc: .byte 7,"telnet:"
u_pret:   .byte 9,"telnet://"
u_irc:    .byte 3,"irc"
u_ircc:   .byte 4,"irc:"
u_prei:   .byte 6,"irc://"
u_ssh:    .byte 3,"ssh"
u_sshc:   .byte 4,"ssh:"
u_pressh: .byte 6,"ssh://"
u_ftp:    .byte 3,"ftp"
u_ftpc:   .byte 4,"ftp:"
u_preftp: .byte 6,"ftp://"
u_search: .byte u_search_e-u_search-1,"http://frogfind.com/?q="
u_search_e:
u_srw:    .byte u_srw_e-u_srw-1,"http://wiby.me/?q="
u_srw_e:
u_srd:    .byte u_srd_e-u_srd-1,"https://html.duckduckgo.com/html/?q="
u_srd_e:
u_stab:   .addr u_search, u_search, u_srw, u_srd
u_local:  .byte 9,"localhost"
h_http:   .byte 5,"http/"
h_loc:    .byte 9,"location:"
h_ctyp:   .byte 13,"content-type:"
h_te:     .byte 18,"transfer-encoding:"
c_html:   .byte 9,"text/html"
c_xhtml:  .byte 17,"application/xhtml"
c_text:   .byte 5,"text/"
w_start_n: .byte 5,"start"
r_get:    .byte 4,"GET "
r_host:   .byte r_host_e-r_host-1," HTTP/1.0",13,10,"Host: "
r_host_e:
r_rest:   .byte r_rest_e-r_rest-1,13,10,"User-Agent: Mozilla/2.0 (compatible; GSOS95/2.18; Apple IIgs)",13,10
          .byte "Accept: text/html, text/plain;q=0.8, */*;q=0.1",13,10
r_rest_e:
r_lang:   .byte r_lang_e-r_lang-1,"Accept-Language: de, en;q=0.5",13,10,"Connection: close",13,10
r_lang_e:
r_lang_en: .byte r_lange_e-r_lang_en-1,"Accept-Language: en, de;q=0.5",13,10,"Connection: close",13,10
r_lange_e:
r_post:   .byte 5,"POST "
r_ptype:  .byte r_ptype_e-r_ptype-1,"Content-Type: application/x-www-form-urlencoded",13,10,"Content-Length: "
r_ptype_e:
r_crlf:   .byte 2,13,10
        .popseg

; Kleinbuchstabe
lower:  cmp #'A'
        bcc @r
        cmp #'Z'+1
        bcs @r
        ora #$20
@r:     rts

; Puffer ab Y beginnt (ohne Gross/Klein) mit Text X (Hauptbank)? C=0 ja, Y dahinter
pfx_ci: stx UT_P
        lda a:0,x
        and #$00FF
        sta UT_N
@l:     lda UT_N
        beq @y
        inc UT_P
        BRD
        jsr lower
        sta UT_C
        ldx UT_P
        lda a:0,x
        and #$00FF
        cmp UT_C
        bne @n
        iny
        dec UT_N
        bra @l
@y:     clc
        rts
@n:     sec
        rts

; ---------- Zielzeichenkette WB_TURL aufbauen ----------
tb_clr: ldy #WB_TURL
        lda #0
        BWR
        rts
tb_c:   sta UT_C
        phy
        ldy #WB_TURL
        BRD
        cmp #255
        bcs @r
        inc a
        BWR
        clc
        adc #WB_TURL
        tay
        lda UT_C
        BWR
@r:     ply
        rts
; Text X (Hauptbank) anhaengen
tb_m:   stx UT_P
        lda a:0,x
        and #$00FF
        sta UT_N
@l:     lda UT_N
        beq @r
        inc UT_P
        ldx UT_P
        lda a:0,x
        and #$00FF
        jsr tb_c
        dec UT_N
        bra @l
@r:     rts
; Pufferbytes von X bis Y (ausschliesslich) anhaengen; '#' beendet, Leerzeichen -> %20
tb_rng: stx URD
        sty UWR
@l:     lda URD
        cmp UWR
        bcs @r
        tay
        BRD
        cmp #'#'
        beq @r
        cmp #' '
        bne @c
        lda #'%'
        jsr tb_c
        lda #'2'
        jsr tb_c
        lda #'0'
@c:     jsr tb_c
        inc URD
        bra @l
@r:     rts

; Byte an WB_HOST / WB_PATH anhaengen
ps_c:   sta UT_C                ; X = Zeichenkette
        stx UT_P
        txy
        BRD
        cmp #250
        bcs @r
        inc a
        BWR
        clc
        adc UT_P
        tay
        lda UT_C
        BWR
@r:     rts

; ================= Adresse zerlegen =================
; WB_TURL -> USCH, WB_HOST, WPORT, WB_PATH; WB_TURL wird vereinheitlicht.
; C=1 wenn unbrauchbar
url_parse:
        ldx #WB_TURL            ; Kopie zum Lesen
        ldy #WB_REQ
        jsr bs_copy
        lda #WB_REQ+1
        sta US
        ldy #WB_REQ
        BRD
        clc
        adc #WB_REQ+1
        sta UE
@t1:    lda US                  ; Leerzeichen am Rand weg
        cmp UE
        bcs @bad
        tay
        BRD
        cmp #' '+1
        bcs @t2
        inc US
        bra @t1
@t2:    ldy UE
        dey
        BRD
        cmp #' '+1
        bcs @t3
        dec UE
        bra @t2
@t3:    ldy US
        ldx #u_about
        jsr pfx_ci
        bcs @nab
        lda #2
        sta USCH
        jsr tb_clr              ; about:xyz (klein geschrieben)
        stz UAQ                 ; nach '?' Gross/Klein behalten
@ac:    ldy US
        cpy UE
        bcs @ae
        BRD
        cmp #'?'
        bne @aq
        sta UAQ
@aq:    ldx UAQ
        bne @al
        cmp #'A'
        bcc @al
        cmp #'Z'+1
        bcs @al
        ora #$20
@al:    jsr tb_c
        inc US
        bra @ac
@ae:    clc
        rts
@bad:   sec
        rts
@nab:   ldy US                  ; "://" suchen (vor / ? #)
@f:     cpy UE
        bcc *+5
        jmp @nosch
        BRD
        cmp #'/'
        bne *+5
        jmp @nosch
        cmp #'?'
        bne *+5
        jmp @nosch
        cmp #'#'
        bne *+5
        jmp @nosch
        cmp #':'
        beq @col
        iny
        bra @f
@col:   sty UP
        iny
        BRD
        cmp #'/'
        beq *+5
        jmp @nosch
        iny
        BRD
        cmp #'/'
        beq *+5
        jmp @nosch
        lda UP                  ; Schema pruefen
        sec
        sbc US
        cmp #4
        bne @s5
        ldy US
        ldx #u_http
        jsr pfx_ci
        bcs @bad
        stz USCH
        brl @hs
@s5:    cmp #5
        bne @s6
        ldy US
        ldx #u_https
        jsr pfx_ci
        bcs @bad
        lda #1
        sta USCH
        bra @hs
@s6:    cmp #6
        bne @s3
        ldy US
        ldx #u_gopher
        jsr pfx_ci
        bcs @s6t
        lda #3
        sta USCH
        bra @hs
@s6t:   ldy US
        ldx #u_telnet
        jsr pfx_ci
        bcc *+5
        jmp @bad
        lda #4
        sta USCH
        bra @hs
@s3:    cmp #3
        beq *+5
        jmp @bad
        ldy US
        ldx #u_irc
        jsr pfx_ci
        bcs @s3s
        lda #5
        bra @s3e
@s3s:   ldy US                  ; ssh:// (nur FujiNet)
        ldx #u_ssh
        jsr pfx_ci
        bcs @s3f
        lda #6
        bra @s3e
@s3f:   ldy US                  ; ftp://
        ldx #u_ftp
        jsr pfx_ci
        bcc *+5
        jmp @bad
        lda #7
@s3e:   sta USCH
@hs:    lda UP
        clc
        adc #3
        sta US
        jmp url_host
@nosch: ; kein Schema: Suchbegriff oder Rechnername?
        stz UC                  ; Punkte
        stz UX                  ; Leerzeichen
        ldy US
@sc:    cpy UE
        bcs @sd
        BRD
        cmp #'.'
        bne @sc1
        inc UC
@sc1:   cmp #' '
        bne @sc2
        inc UX
@sc2:   iny
        bra @sc
@sd:    lda UX
        bne url_search
        lda UC
        bne @http
        ldy US
        ldx #u_local
        jsr pfx_ci
        bcs url_search
@http:  stz USCH
        jmp url_host

; Suchbegriff -> FrogFind
url_search:
        jsr tb_clr
        lda SRCHE               ; Suchmaschine (about:lz): 1 FrogFind, 2 wiby, 3 DuckDuckGo
        and #$0003
        asl
        tax
        lda u_stab,x
        tax
        jsr tb_m
        ldy US
@l:     cpy UE
        bcs @d
        BRD
        sta UC
        phy
        lda UC
        cmp #' '
        bne @n1
        lda #'+'
        jsr tb_c
        bra @nx
@n1:    jsr url_safe
        bcc @hex
        lda UC
        jsr tb_c
        bra @nx
@hex:   lda #'%'
        jsr tb_c
        lda UC
        lsr
        lsr
        lsr
        lsr
        jsr u_hexd
        jsr tb_c
        lda UC
        and #$000F
        jsr u_hexd
        jsr tb_c
@nx:    ply
        iny
        bra @l
@d:     jmp url_parse           ; jetzt mit Schema

; A unveraendert uebernehmen? C=1 ja
url_safe:
        cmp #'0'
        bcc @s
        cmp #'9'+1
        bcc @y
        cmp #'A'
        bcc @n
        cmp #'Z'+1
        bcc @y
        cmp #'a'
        bcc @s
        cmp #'z'+1
        bcc @y
@s:     cmp #'-'
        beq @y
        cmp #'.'
        beq @y
        cmp #'_'
        beq @y
@n:     clc
        rts
@y:     sec
        rts
u_hexd:   cmp #10
        bcc @d
        adc #'A'-11
        rts
@d:     adc #'0'
        rts

; ab US: Rechner[:Port][Pfad] -> WB_HOST, WPORT, WB_PATH, WB_TURL
url_host:
        jsr tb_clr
        ldx #u_pre
        lda USCH
        beq @p
        ldx #u_pres
        cmp #3
        bne @p4
        ldx #u_preg
@p4:    cmp #4
        bne @p5
        ldx #u_pret
@p5:    cmp #5
        bne @p6
        ldx #u_prei
@p6:    cmp #6
        bne @p7
        ldx #u_pressh
@p7:    cmp #7
        bne @p
        ldx #u_preftp
@p:     jsr tb_m
        ldy #WB_HOST
        lda #0
        BWR
        ldy #WB_PATH
        BWR
        ldy US
@h:     cpy UE
        bcs @he
        BRD
        cmp #'/'
        beq @he
        cmp #':'
        beq @he
        cmp #'?'
        beq @he
        cmp #'#'
        beq @he
        jsr lower
        phy
        pha
        ldx #WB_HOST
        jsr ps_c
        pla
        jsr tb_c
        ply
        iny
        bra @h
@he:    sty UP
        ldy #WB_HOST
        BRD
        bne @hok
        sec
        rts
@hok:   lda #80
        ldx USCH
        beq @pd
        lda #443
        cpx #3
        bne @pd4
        lda #70
@pd4:   cpx #4
        bne @pd5
        lda #23
@pd5:   cpx #5
        bne @pd6
        lda #6667
@pd6:   cpx #6
        bne @pd7
        lda #22
@pd7:   cpx #7
        bne @pd
        lda #21
@pd:    sta WPORT
        ldy UP
        cpy UE
        bcs @path
        BRD
        cmp #':'
        bne @path
        lda #':'
        jsr tb_c
        stz WPORT
        iny
@pn:    cpy UE
        bcs @pe
        BRD
        sec
        sbc #'0'
        cmp #10
        bcs @pe
        sta UC
        lda WPORT
        asl
        asl
        clc
        adc WPORT
        asl
        clc
        adc UC
        sta WPORT
        BRD
        jsr tb_c
        iny
        bra @pn
@pe:    sty UP
@path:  ldy UP                  ; Pfad bis '#'
        cpy UE
        bcs @root
        BRD
        cmp #'#'
        beq @root
        cmp #'/'
        beq @pl
        lda #'/'                ; "?..." ohne Schraegstrich
        ldx #WB_PATH
        jsr ps_c
        lda #'/'
        jsr tb_c
@pl:    ldy UP
@pc:    cpy UE
        bcs @done
        BRD
        cmp #'#'
        beq @done
        sta UC
        phy
        cmp #' '
        bne @pc1
        lda #'%'
        jsr @both
        lda #'2'
        jsr @both
        lda #'0'
        bra @pc2
@pc1:   lda UC
@pc2:   jsr @both
        ply
        iny
        bra @pc
@root:  lda #'/'
        jsr @both
@done:  clc
        rts
@both:  pha
        ldx #WB_PATH
        jsr ps_c
        pla
        jmp tb_c

; ================= Link relativ zur Seite =================
; WB_HREF relativ zu WB_BASE -> WB_TURL. C=1 wenn nicht moeglich
url_join:
        ldy #WB_HREF
        BRD
        clc
        adc #WB_HREF+1
        sta UE
        lda #WB_HREF+1
        sta US
@t:     lda US                  ; Leerzeichen vorn weg
        cmp UE
        bcs @same
        tay
        BRD
        cmp #' '+1
        bcs @t1
        inc US
        bra @t
@same:  ldx #WB_BASE
        ldy #WB_TURL
        jsr bs_copy
        clc
        rts
@t1:    sta UC                  ; erstes Zeichen
        cmp #'#'
        bne @sch
        sec                     ; Sprung innerhalb der Seite: nicht unterstuetzt
        rts
@sch:   ldy US                  ; Schema?
@s1:    cpy UE
        bcs @rel
        BRD
        cmp #':'
        beq @hasc
        jsr url_safe
        bcc @rel
        iny
        bra @s1
@hasc:  cpy US
        beq @rel
        ldy US
        ldx #u_httpc
        jsr pfx_ci
        bcc @abs
        ldy US
        ldx #u_httpsc
        jsr pfx_ci
        bcc @abs
        ldy US
        ldx #u_gopherc
        jsr pfx_ci
        bcc @abs
        ldy US
        ldx #u_telnetc
        jsr pfx_ci
        bcc @abs
        ldy US
        ldx #u_ircc
        jsr pfx_ci
        bcc @abs
        ldy US
        ldx #u_sshc
        jsr pfx_ci
        bcc @abs
        ldy US
        ldx #u_ftpc
        jsr pfx_ci
        bcc @abs
        ldy US
        ldx #u_about            ; (Formulare des Telefonbuchs)
        jsr pfx_ci
        bcc @abs
        sec                     ; mailto:, javascript: ...
        rts
@abs:   jsr tb_clr
        ldx US
        ldy UE
        jsr tb_rng
        clc
        rts
@rel:   jsr base_parts
        jsr tb_clr
        lda UC
        cmp #'/'
        bne @r2
        ldy US
        iny
        cpy UE
        bcs @r1
        BRD
        cmp #'/'
        bne @r1
        ldx BS0                 ; "//rechner/..." -> Schema der Seite
        lda BH0
        dec a
        dec a
        tay
        jsr tb_rng
        bra @add
@r1:    ldx BS0                 ; "/pfad" -> Ursprung + Pfad
        ldy BP0
        jsr tb_rng
        bra @add
@r2:    cmp #'?'
        bne @r3
        ldx BS0
        ldy BQ0
        jsr tb_rng
        bra @add
@r3:    ldx BS0                 ; relativ: Ordner der Seite
        ldy BL0
        jsr tb_rng
        lda BL0                 ; Seite ohne Pfad: "/" ergaenzen
        cmp BP0
        bne @add
        lda #'/'
        jsr tb_c
@add:   ldx US
        ldy UE
        jsr tb_rng
        jsr url_dots
        clc
        rts

; WB_BASE zerlegen: BS0 Anfang, BH0 hinter "://", BP0 Pfadanfang,
; BQ0 Ende des Pfads (? oder #), BL0 hinter dem letzten '/' im Pfad
base_parts:
        lda #WB_BASE+1
        sta BS0
        ldy #WB_BASE
        BRD
        clc
        adc #WB_BASE+1
        sta BE0
        lda BS0
        sta BH0
        ldy BS0
@f:     cpy BE0
        bcs @nf
        BRD
        cmp #':'
        bne @nx
        iny
        BRD
        cmp #'/'
        bne @f
        iny
        BRD
        cmp #'/'
        bne @f
        iny
        sty BH0
        bra @nf
@nx:    iny
        bra @f
@nf:    ldy BH0                 ; Pfadanfang
@p:     cpy BE0
        bcs @pe
        BRD
        cmp #'/'
        beq @pe
        iny
        bra @p
@pe:    sty BP0
        sty BL0
@q:     cpy BE0
        bcs @qe
        BRD
        cmp #'?'
        beq @qe
        cmp #'#'
        beq @qe
        cmp #'/'
        bne @q1
        sty BL0
        inc BL0
@q1:    iny
        bra @q
@qe:    sty BQ0
        rts

; "./" und "../" im Pfad von WB_TURL aufloesen
url_dots:
        ldy #WB_TURL
        BRD
        clc
        adc #WB_TURL+1
        sta UE
        ldy #WB_TURL+1          ; hinter "://" den ersten '/'
@a:     cpy UE
        bcs @rr
        BRD
        cmp #':'
        beq @c
        iny
        bra @a
@c:     iny
        iny
        iny
@b:     cpy UE
        bcs @rr
        BRD
        cmp #'/'
        beq @g
        iny
        bra @b
@rr:    rts
@g:     sty UP                  ; Pfadanfang
        sty URD
        sty UWR
@q:     cpy UE                  ; Ende des Pfads
        bcs @qe
        BRD
        cmp #'?'
        beq @qe
        iny
        bra @q
@qe:    sty UQ
        stz UDIR
@seg:   lda URD                  ; URD steht auf '/'
        cmp UQ
        bcc *+5
        jmp @fin
        tay
        iny
        BRD                     ; "/." ?
        cmp #'.'
        bne @copy
        iny
        cpy UQ
        beq @dot
        BRD
        cmp #'/'
        beq @dot
        cmp #'.'
        bne @copy
        iny
        cpy UQ
        beq @dd
        BRD
        cmp #'/'
        bne @copy
@dd:    sty URD                  ; "/.." -> letzten Teil streichen
        lda #1
        sta UDIR
@back:  lda UWR
        cmp UP
        beq @seg
        dec UWR
        ldy UWR
        BRD
        cmp #'/'
        bne @back
        bra @seg
@dot:   sty URD
        lda #1
        sta UDIR
        bra @seg
@copy:  stz UDIR
        ldy URD                  ; '/' und den Teil bis zum naechsten '/'
        BRD
        ldy UWR
        BWR
        inc UWR
        inc URD
@cl:    lda URD
        cmp UQ
        bcc *+5
        jmp @seg
        tay
        BRD
        cmp #'/'
        bne *+5
        jmp @seg
        ldy UWR
        BWR
        inc UWR
        inc URD
        bra @cl
@fin:   lda UDIR
        bne @sl
        lda UWR
        cmp UP
        bne @rest
@sl:    ldy UWR
        lda #'/'
        BWR
        inc UWR
@rest:  lda UQ                  ; Abfrage dahinter verschieben
        cmp UE
        bcs @len
        tay
        BRD
        ldy UWR
        BWR
        inc UWR
        inc UQ
        bra @rest
@len:   lda UWR
        sec
        sbc #WB_TURL+1
        ldy #WB_TURL
        BWR
        rts

; ================= HTTP-Anfrage =================
; nach WB_REQ; WPTR/WCNT fuer tcp_write. C=1 wenn zu lang
rq_m:   stx UT_P                ; Text X (Hauptbank)
        lda a:0,x
        and #$00FF
        sta UT_N
@l:     lda UT_N
        beq @r
        inc UT_P
        ldx UT_P
        lda a:0,x
        jsr rq_c
        dec UT_N
        bra @l
@r:     rts
rq_b:   stx UT_P                ; Zeichenkette X im Puffer
        txy
        BRD
        sta UT_N
@l:     lda UT_N
        beq @r
        inc UT_P
        ldy UT_P
        BRD
        jsr rq_c
        dec UT_N
        bra @l
@r:     rts
rq_c:   ldy UWR
        cpy #WB_URL-1
        bcs @r
        BWR
        inc UWR
@r:     rts

req_build:
        lda USCH                ; Gopher: nur der Selektor
        cmp #3
        bne @ht
        jmp gop_req
@ht:    lda #WB_REQ
        sta UWR
        ldx #r_get
        lda WPOST
        beq @g
        ldx #r_post
@g:     jsr rq_m
        lda WPROXY              ; ueber Proxy: ganze Adresse
        beq @pp
        ldx #u_pres
        jsr rq_m
        ldx #WB_HOST
        jsr rq_b
        lda WPORT
        cmp #443
        beq @pp
        lda #':'
        jsr rq_c
        lda WPORT
        jsr rq_dec
@pp:    ldx #WB_PATH
        jsr rq_b
        ldx #r_host
        jsr rq_m
        ldx #WB_HOST
        jsr rq_b
        lda WPORT
        cmp #80
        beq @np
        cmp #443
        bne @hp
        lda USCH
        cmp #1
        beq @np
@hp:    lda WPORT
        lda #':'
        jsr rq_c
        lda WPORT
        jsr rq_dec
@np:    ldx #r_rest
        jsr rq_m
        ldx #r_lang
        lda LANG
        beq @l
        ldx #r_lang_en
@l:     jsr rq_m
        jsr ck_req              ; Cookies dieses Rechners
        lda WPOST               ; POST: Typ, Laenge, Leerzeile, Inhalt
        beq @e
        ldx #r_ptype
        jsr rq_m
        ldy #WB_HREF
        BRD
        jsr rq_dec
        ldx #r_crlf
        jsr rq_m
        ldx #r_crlf
        jsr rq_m
        ldx #WB_HREF
        jsr rq_b
        bra @ln
@e:     ldx #r_crlf
        jsr rq_m
@ln:    lda UWR
        cmp #WB_URL-1
        bcs @bad
        sec
        sbc #WB_REQ
        sta WCNT
        lda #WB_REQ
        sta WPTR
        clc
        rts
@bad:   sec
        rts
; Zahl A dezimal anhaengen (bis 65535)
rq_dec: sta UC
        stz UX                  ; schon Ziffern?
        ldx #0
@d:     lda rq_pow,x
        sta UT_C
        ldy #0
        lda UC
@s:     cmp UT_C
        bcc @o
        sbc UT_C
        iny
        bra @s
@o:     sta UC
        tya
        bne @w
        lda UX
        bne @w0
        cpx #8
        bne @nx
@w0:    lda #0
@w:     clc
        adc #'0'
        phx
        jsr rq_c
        plx
        inc UX
@nx:    inx
        inx
        cpx #10
        bcc @d
        rts
        .pushseg
        .segment "RODATA"
rq_pow: .word 10000,1000,100,10,1
        .popseg

; ================= Antwortkopf =================
hdr_parse:
        stz PFX
        lda #200
        sta HCODE
        stz HEND
        lda RAWEND
        sta BEND
        stz HCHUNK
        stz HLOC
        lda #1
        sta HTYPOK
        stz HPLAIN
        ldy #WB_CTYP
        lda #0
        BWR
        ldy #0
        ldx #h_http
        jsr pfx_ci
        bcc @st
        rts                     ; HTTP/0.9: alles ist Inhalt
@st:    ldy #0                  ; Statuszahl hinter dem ersten Leerzeichen
@s1:    cpy RAWEND
        bcc *+5
        jmp @end
        BRD
        cmp #' '
        beq @s2
        cmp #$0A
        beq @s2
        iny
        bra @s1
@s2:    iny
        stz HCODE
        ldx #3
@s3:    BRD
        sec
        sbc #'0'
        cmp #10
        bcs @sn
        sta UC
        lda HCODE
        asl
        asl
        clc
        adc HCODE
        asl
        clc
        adc UC
        sta HCODE
        iny
        dex
        bne @s3
@sn:    jsr hd_eol
@ln:    cpy RAWEND
        bcc *+5
        jmp @end
        BRD
        cmp #$0D
        bne @l1
        iny
        BRD
        cmp #$0A
        bne @l1b
        iny
@l1b:   sty HEND
        jmp @ty
@l1:    cmp #$0A
        bne @hdr
        iny
        sty HEND
        jmp @ty
@hdr:   sty UK
        ldx #h_loc
        jsr pfx_ci
        bcs @h2
        ldx #WB_HREF
        lda #190
        jsr hd_val
        lda #1
        sta HLOC
        bra @nx
@h2:    ldy UK
        ldx #h_ctyp
        jsr pfx_ci
        bcs @h2b
        ldx #WB_CTYP
        lda #30
        jsr hd_val
        bra @nx
@h2b:   ldy UK                  ; Set-Cookie: merken (cookie.inc)
        ldx #h_setc
        jsr pfx_ci
        bcs @h3
        ldy UK
        jsr ck_set
        bra @nx
@h3:    ldy UK
        ldx #h_te
        jsr pfx_ci
        bcs @nx
@te:    cpy RAWEND              ; "chunked"?
        bcs @nx
        BRD
        cmp #$0A
        beq @nx
        jsr lower
        cmp #'c'
        bne @te1
        iny
        BRD
        jsr lower
        cmp #'h'
        bne @te
        lda #1
        sta HCHUNK
        bra @nx
@te1:   iny
        bra @te
@nx:    ldy UK
        jsr hd_eol
        jmp @ln
@end:   lda RAWEND
        sta HEND
@ty:    JSLF xf_pf_ctype        ; RSS/Atom (xml): als Seite umbauen
        bcs @r
        ldy #WB_CTYP            ; Inhaltstyp einordnen
        BRD
        beq @r
        ldy #WB_CTYP+1
        ldx #c_html
        jsr pfx_ci
        bcc @r
        ldy #WB_CTYP+1
        ldx #c_xhtml
        jsr pfx_ci
        bcc @r
        ldy #WB_CTYP+1
        ldx #c_text
        jsr pfx_ci
        bcs @no
        lda #1
        sta HPLAIN
@r:     rts
@no:    stz HTYPOK
        rts

; Y hinter das naechste Zeilenende
hd_eol: cpy RAWEND
        bcs @r
        BRD
        iny
        cmp #$0A
        bne hd_eol
@r:     rts

; Wert ab Y (Leerzeichen vorn weg, bis Zeilenende) nach Zeichenkette X, hoechstens A Zeichen
hd_val: sta UT_N
        stx UT_P
        txy
        lda #0
        BWR
        ldy UK
@s:     cpy RAWEND              ; ':' suchen, dann Leerzeichen weg
        bcs @r
        BRD
        iny
        cmp #':'
        bne @s
@sp:    cpy RAWEND
        bcs @r
        BRD
        cmp #' '
        beq @n
        cmp #$09
        bne @c
@n:     iny
        bra @sp
@c:     cpy RAWEND
        bcs @r
        BRD
        cmp #$0D
        beq @r
        cmp #$0A
        beq @r
        sta UT_C
        phy
        ldy UT_P
        BRD
        cmp UT_N
        bcs @full
        inc a
        BWR
        clc
        adc UT_P
        tay
        lda UT_C
        BWR
@full:  ply
        iny
        bra @c
@r:     rts

; ================= "chunked" zusammenfuegen =================
dechunk:
        lda HEND
        sta URD
        sta UWR
@l:     lda URD
        cmp RAWEND
        bcc *+5
        jmp @e
        stz UC                  ; Groesse (hex)
        tay
@h:     cpy RAWEND
        bcc *+5
        jmp @e
        BRD
        jsr lower
        sec
        sbc #'0'
        cmp #10
        bcc @hd
        sbc #'a'-'0'-10
        cmp #10
        bcc @hx
        cmp #16
        bcs @he
@hd:    sta UX
        lda UC
        asl
        asl
        asl
        asl
        ora UX
        sta UC
        iny
        bra @h
@hx:    bra @he
@he:    jsr hd_eol
        sty URD
        lda UC
        beq @e
@c:     lda UC
        beq @cr
        lda URD
        cmp RAWEND
        bcs @e
        tay
        BRD
        ldy UWR
        BWR
        inc UWR
        inc URD
        dec UC
        bra @c
@cr:    ldy URD                  ; CR LF nach dem Stueck
        cpy RAWEND
        bcs @e
        BRD
        cmp #$0D
        bne @lf
        iny
        BRD
@lf:    cmp #$0A
        bne @nl
        iny
@nl:    sty URD
        jmp @l
@e:     lda UWR
        sta BEND
        rts
