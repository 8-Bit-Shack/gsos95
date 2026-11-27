; ================================================================
; GS/OS95: FujiNet am SmartPort (Teil von net.s)
; Die FujiNet meldet sich als SmartPort-Geraete; eines davon heisst
; "NETWORK" (Typ $11). Aufrufe ueber die SmartPort-Firmware mit FWEntry
; (Misc Tool $2403) aus einem kleinen Block in Bank 0:
;   +$000 Nutzlast (512 Bytes, bei CONTROL: Laenge (2) + Daten)
;   +$220 Befehlsliste, +$230 "JSR Firmware / DFB Befehl / DW Liste / RTS"
; Netzwerkbefehle (CONTROL/STATUS-Code): 'O' oeffnen (Modus, Uebersetzung,
; "N:TCP://rechner:port/"), 'C' schliessen, 'S' Status (verfuegbar (2),
; verbunden, Fehler), 'M' HTTP-Kanal; Daten mit READ/WRITE.
; Netzwerk-Einheit 1 (Befehlslisten mit 4 bzw. 5 Parametern).
; http, Telnet, Gopher, IRC: "N:TCP://" - die Seite wird wie bei den
; anderen Karten selbst angefragt. https: "N:HTTPS://..." - die Karte
; verschluesselt; GS/OS95 baut aus der fertigen HTTP-Anfrage (tcp_write)
; Adresse und Methode und setzt vor die Antwort einen Kopf.
; ================================================================
        .pushseg
        .segment "BSS"
FNBLK:  .res 2                  ; Adresse des Blocks in Bank 0 (0 = noch keiner)
FNUNIT: .res 2                  ; SmartPort-Einheit des NETWORK-Geraets (0 = keine)
FNSLOT: .res 2
FNDISP: .res 2                  ; Firmware-Einsprung ($Cn00 + [$CnFF] + 3)
FNCNT:  .res 2                  ; uebertragene Bytes
FNERR:  .res 2                  ; Fehler der Firmware
FNMODE: .res 2                  ; 0 TCP, 1 https (Anfrage noch offen)
FNFTP:  .res 2                  ; naechstes fn_open: 1 FTP-Datei, 2 FTP-Verzeichnis
FNAUX2: .res 2
FNHDR:  .res 2                  ; 1 = vor die Antwort einen HTTP-Kopf setzen
FNPORT: .res 2
FNAV:   .res 2                  ; Status: Bytes verfuegbar
FNCON:  .res 2                  ; Status: verbunden
FNST:   .res 2                  ; Status: Fehlercode (1 = gut, 136 = Ende)
FNI:    .res 2
FNJ:    .res 2
FNK:    .res 2
FNN:    .res 2
FNPOST: .res 2
FNCT:   .res 48                 ; Inhaltstyp (Laenge + Text)
FNDN:   .res 2
FNE:    .res 2                  ; Ende der https-Anfrage
FNDV:   .res 2
FNDG:   .res 2
FNDS:   .res 2
        .popseg

FN_PAY = $000
FN_CL  = $220
FN_JS  = $230
FN_UNIT = 1                     ; Netzwerk-Einheit (N1:)
FNS_OK  = 1
FNS_EOF = 136
FNS_404 = 170

; ---------- Bank-0-Block ----------
; Byte A nach Block+X
fn_put0:
        pha
        txa
        clc
        adc FNBLK
        tax
        pla
        sep #$20
        sta f:$000000,x
        rep #$20
        rts
; Byte Block+X -> A
fn_get0:
        txa
        clc
        adc FNBLK
        tax
        lda f:$000000,x
        and #$00FF
        rts
; Wort Block+X -> A
fn_get0w:
        txa
        clc
        adc FNBLK
        tax
        lda f:$000000,x
        rts

; Block anlegen (einmal). C=1 kein Speicher
fn_mem: lda FNBLK
        beq @g
        clc
        rts
@g:     pha
        pha
        pea $0000
        pea $0400               ; 1 KB
        lda USERID
        pha
        pea $C015               ; fest, gesperrt, Bank 0, Seitenanfang
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        pla
        sta HND
        pla
        sta HND+2
        bcs @f
        lda [HND]
        sta FNBLK
        clc
        rts
@f:     lda #NE_MEM
        sta NERR
        sec
        rts

; Einsprung FNDISP in den Aufrufblock schreiben
fn_stub:
        lda #$20                ; JSR
        ldx #FN_JS
        jsr fn_put0
        lda FNDISP
        ldx #FN_JS+1
        jsr fn_put0
        lda FNDISP+1
        ldx #FN_JS+2
        jsr fn_put0
        lda FNBLK               ; Befehlsliste
        clc
        adc #FN_CL
        pha
        ldx #FN_JS+4
        jsr fn_put0
        pla
        xba
        ldx #FN_JS+5
        jsr fn_put0
        lda #$60                ; RTS
        ldx #FN_JS+6
        jmp fn_put0

; Befehlsliste: A = Anzahl Parameter, X = Einheit; Puffer = Nutzlast
fn_cl:  phx
        ldx #FN_CL
        jsr fn_put0
        pla
        ldx #FN_CL+1
        jsr fn_put0
        lda FNBLK
        ldx #FN_CL+2
        jsr fn_put0
        lda FNBLK
        xba
        ldx #FN_CL+3
        jmp fn_put0

; SmartPort-Befehl A ausfuehren. C=1 bei Fehler (FNERR), FNCNT = Bytes
fn_call:
        ldx #FN_JS+3
        jsr fn_put0
        pea $0000               ; Ergebnisse: Y, X, A, P
        pea $0000
        pea $0000
        pea $0000
        pea $0000               ; A, X, Y beim Einsprung
        pea $0000
        pea $0000
        lda FNBLK
        clc
        adc #FN_JS
        pha
        TOOL $2403              ; FWEntry
        pla                     ; Y (Anzahl hoch)
        and #$00FF
        xba
        sta FNCNT
        pla                     ; X (Anzahl tief)
        and #$00FF
        ora FNCNT
        sta FNCNT
        pla                     ; A: Fehler
        and #$00FF
        sta FNERR
        pla                     ; P
        lda FNERR
        cmp #1                  ; C=1 bei Fehler
        rts

; STATUS: A = Code, X = Einheit (nur Laufwerks-/Geraetedaten, ohne Netzwerk-Einheit)
fn_stat:
        pha
        lda #3
        jsr fn_cl
        pla
        ldx #FN_CL+4
        jsr fn_put0
        lda #0                  ; SP STATUS
        jmp fn_call

; Netzwerk-STATUS 'S' -> FNAV, FNCON, FNST. C=1 bei Fehler
fn_nstat:
        lda #4
        ldx FNUNIT
        jsr fn_cl
        lda #'S'
        ldx #FN_CL+4
        jsr fn_put0
        lda #FN_UNIT
        ldx #FN_CL+5
        jsr fn_put0
        lda #0
        jsr fn_call
        bcs @r
        ldx #FN_PAY
        jsr fn_get0w
        sta FNAV
        ldx #FN_PAY+2
        jsr fn_get0
        sta FNCON
        ldx #FN_PAY+3
        jsr fn_get0
        sta FNST
        clc
@r:     rts

; Netzwerk-CONTROL A (Nutzlast mit Laenge schon gefuellt). C=1 bei Fehler
fn_nctl:
        pha
        lda #4
        ldx FNUNIT
        jsr fn_cl
        pla
        ldx #FN_CL+4
        jsr fn_put0
        lda #FN_UNIT
        ldx #FN_CL+5
        jsr fn_put0
        lda #4                  ; SP CONTROL
        jmp fn_call

; CONTROL A mit zwei Bytes 0, X (z.B. 'M' HTTP-Kanal)
fn_nctl2:
        pha
        phx
        lda #2
        ldx #FN_PAY
        jsr fn_put0
        lda #0
        ldx #FN_PAY+1
        jsr fn_put0
        lda #0
        ldx #FN_PAY+2
        jsr fn_put0
        pla
        ldx #FN_PAY+3
        jsr fn_put0
        pla
        jmp fn_nctl

; READ/WRITE (A = 8 / 9), Laenge in FNN. C=1 bei Fehler
fn_rw:  pha
        lda #5
        ldx FNUNIT
        jsr fn_cl
        lda FNN
        ldx #FN_CL+4
        jsr fn_put0
        lda FNN+1
        ldx #FN_CL+5
        jsr fn_put0
        lda #FN_UNIT
        ldx #FN_CL+6
        jsr fn_put0
        pla
        jmp fn_call

; ---------- FujiNet suchen: SmartPort-Karten, Geraet vom Typ $11 ----------
; C=0 gefunden (FNUNIT)
fn_probe:
        lda FNUNIT
        beq @s
        clc
        rts
@s:     lda #$C700
        sta FNSLOT
@sl:    ldx FNSLOT
        sep #$20
        lda f:$000001,x
        cmp #$20
        beq *+5
        jmp @nx
        lda f:$000003,x
        beq *+5
        jmp @nx
        lda f:$000005,x
        cmp #$03
        bne @nx
        lda f:$000007,x
        bne @nx
        lda f:$0000FF,x         ; ProDOS-Einsprung; SmartPort = +3
        rep #$20
        and #$00FF
        clc
        adc #3
        adc FNSLOT
        sta FNDISP
        jsr fn_mem
        bcs @no
        jsr fn_stub
        lda #0                  ; Anzahl der Geraete
        ldx #0
        jsr fn_stat
        bcs @nx16
        ldx #FN_PAY
        jsr fn_get0
        beq @nx16
        cmp #9
        bcc @c
        lda #8
@c:     sta FNJ
        lda #1
        sta FNI
@dv:    lda #3                  ; DIB: Typ an +21
        ldx FNI
        jsr fn_stat
        bcs @dn
        ldx #FN_PAY+21
        jsr fn_get0
        cmp #$11
        bne @dn
        lda FNI
        sta FNUNIT
        stz FNMODE
        stz FNHDR
        clc
        rts
@dn:    inc FNI
        lda FNI
        cmp FNJ
        bcc @dv
        beq @dv
        bra @nx16
@nx:    rep #$20
@nx16:  lda FNSLOT
        sec
        sbc #$0100
        sta FNSLOT
        cmp #$C100
        bcc *+5
        jmp @sl
@no:    sec
        rts

; Karte bereit: Adresse kennt nur die FujiNet selbst
fn_start:
        ldx #14
@c:     stz NCUR,x
        dex
        dex
        bpl @c
        lda #1
        sta NDHOK
        stz FNMODE
        stz FNHDR
        clc
        rts

; ---------- Texte dieser Codebank ----------
; Pascal-String A (diese Bank) an die Nutzlast ab FNI anhaengen
fn_kpay:
        sta FNK
        lda PDST
        pha
        lda PDST+2
        pha
        lda FNK
        sta PDST
        sep #$20
        phk
        pla
        rep #$20
        and #$00FF
        sta PDST+2
        lda [PDST]
        and #$00FF
        sta FNK
        ldy #1
@l:     lda FNK
        beq @r
        lda [PDST],y
        ldx FNI
        jsr fn_put0
        inc FNI
        iny
        dec FNK
        bra @l
@r:     pla
        sta PDST+2
        pla
        sta PDST
        rts

; Rechnername WB_HOST an die Nutzlast ab FNI
fn_host:
        jsr net_ptr
        ldy #WB_HOST
        lda [PSRC],y
        and #$00FF
        sta FNK
        iny
@l:     lda FNK
        beq @r
        lda [PSRC],y
        ldx FNI
        jsr fn_put0
        inc FNI
        iny
        dec FNK
        bra @l
@r:     rts

; Zahl A dezimal an die Nutzlast ab FNI
fn_dec: sta FNDN
        lda #10000
        sta FNDV
        stz FNDS                ; schon eine Ziffer?
@d:     lda #'0'
        sta FNDG
@s:     lda FNDN
        cmp FNDV
        bcc @o
        sec
        sbc FNDV
        sta FNDN
        inc FNDG
        bra @s
@o:     lda FNDG
        cmp #'0'
        bne @w
        lda FNDS
        bne @w
        lda FNDV
        cmp #1
        bne @n
@w:     lda FNDG
        ldx FNI
        jsr fn_put0
        inc FNI
        inc FNDS
@n:     lda FNDV                ; naechste Stelle: FNDV / 10
        ldx #0
@q:     cmp #10
        bcc @qe
        sec
        sbc #10
        inx
        bra @q
@qe:    stx FNDV
        lda FNDV
        bne @d
        rts

; OPEN mit Modus A, Adresse schon ab Nutzlast+4 (FNI = Ende). C=1 bei Fehler
fn_nopen:
        ldx #FN_PAY+2
        jsr fn_put0
        lda FNAUX2              ; 0: keine Zeilenende-Uebersetzung
        ldx #FN_PAY+3
        jsr fn_put0
        lda #0                  ; Ende der Adresse
        ldx FNI
        jsr fn_put0
        inc FNI
        lda FNI                 ; Laenge der Daten
        sec
        sbc #2
        ldx #FN_PAY
        jsr fn_put0
        lda #0
        ldx #FN_PAY+1
        jsr fn_put0
        lda #'O'
        jmp fn_nctl

; CONTROL X mit Text A (Hauptbank: Laenge (2) + Zeichen) als Nutzlast
; (wie bei 'O': Laenge (2), Zeichen, 0)
fn_cred:
        stx FNJ
        sta FNK
        lda #FN_PAY+2
        sta FNI
        ldx FNK
        lda a:0,x
        sta FNDG
        stz FNDS
@l:     lda FNDS
        cmp FNDG
        bcs @e
        clc
        adc FNK
        tax
        lda a:2,x
        and #$00FF
        ldx FNI
        jsr fn_put0
        inc FNI
        inc FNDS
        bra @l
@e:     lda #0
        ldx FNI
        jsr fn_put0
        inc FNI
        lda FNI
        sec
        sbc #FN_PAY+2
        ldx #FN_PAY
        jsr fn_put0
        lda #0
        ldx #FN_PAY+1
        jsr fn_put0
        lda FNJ
        jmp fn_nctl

; ---------- Verbindung zu WB_HOST:DPORT ----------
fn_open:
        stz FNHDR
        lda DPORT
        sta FNPORT
        lda NTLS                ; https: erst beim Senden der Anfrage oeffnen
        beq @tcp
        lda #1
        sta FNMODE
        clc
        rts
@tcp:   stz FNMODE
        lda FNFTP               ; ftp://: die FujiNet erledigt alles (N:FTP://)
        beq @nf
        lda #FN_PAY+4
        sta FNI
        lda #fn_s_ftp
        jsr fn_kpay
        jsr fn_host
        jsr net_ptr
        ldy #WB_PATH
        lda [PSRC],y
        and #$00FF
        sta FNK
        iny
@fp:    lda FNK
        beq @fe
        lda [PSRC],y
        ldx FNI
        jsr fn_put0
        inc FNI
        iny
        dec FNK
        bra @fp
@fe:    lda #4
        ldx FNFTP
        stz FNFTP
        cpx #2
        bne @fo
        lda #$80                ; Verzeichnis, lange Namen
        sta FNAUX2
        lda #6
@fo:    jsr fn_nopen
        stz FNAUX2
        bra @op
@nf:    lda FNSSH               ; SSH: vorher Benutzer ($FD) und Passwort ($FE)
        beq @t0
        lda #SSHU
        ldx #$FD
        jsr fn_cred
        lda #SSHP
        ldx #$FE
        jsr fn_cred
@t0:    lda #FN_PAY+4
        sta FNI
        lda #fn_s_tcp
        ldx FNSSH
        beq @t1
        lda #fn_s_ssh
@t1:    stz FNSSH
        jsr fn_kpay
        jsr fn_host
        lda #':'
        ldx FNI
        jsr fn_put0
        inc FNI
        lda DPORT
        jsr fn_dec
        lda #'/'
        ldx FNI
        jsr fn_put0
        inc FNI
        lda #12                 ; lesen und schreiben
        jsr fn_nopen
@op:    bcs @ref
        jsr fn_nstat            ; abgelehnt?
        bcs @ref
        lda FNST
        cmp #128
        bcc @ok
        cmp #FNS_EOF
        beq @ok
@ref:   lda #NE_CONN
        sta NERR
        jsr fn_close
        sec
        rts
@ok:    clc
        rts

; ---------- WCNT Bytes ab WPTR senden ----------
fn_write:
        lda FNMODE
        beq @tcp
        jmp fn_https
@tcp:   lda WPTR
        sta FNI
        lda WCNT
        sta FNJ
; FNJ Bytes ab Pufferbank+FNI senden (Stuecke zu 512)
fn_send:
@l:     lda FNJ
        bne @g
        clc
        rts
@g:     cmp #513
        bcc @n
        lda #512
@n:     sta FNN
        jsr net_ptr
        ldy FNI
        ldx #0
@c:     lda [PSRC],y
        phx
        phy
        jsr fn_put0
        ply
        plx
        iny
        inx
        cpx FNN
        bcc @c
        sty FNI
        lda #9                  ; WRITE
        jsr fn_rw
        bcs @e
        lda FNJ
        sec
        sbc FNN
        sta FNJ
        bra @l
@e:     lda #NE_CONN
        sta NERR
        sec
        rts

; https: Anfrage ab WPTR (WCNT Bytes) zerlegen und als N:HTTPS:// stellen
fn_https:
        stz FNMODE
        jsr net_ptr
        stz FNPOST
        ldy WPTR
        lda [PSRC],y
        and #$00FF
        cmp #'P'
        bne @m
        inc FNPOST
@m:     lda WPTR                ; Ende der Anfrage
        clc
        adc WCNT
        sta FNE
@sp:    cpy FNE                 ; Pfad: nach dem ersten Leerzeichen
        bcs @bad
        lda [PSRC],y
        iny
        and #$00FF
        cmp #' '
        bne @sp
        sty FNJ                 ; Anfang des Pfads
        lda #FN_PAY+4
        sta FNI
        lda #fn_s_https
        jsr fn_kpay
        jsr fn_host
        lda FNPORT
        cmp #443
        beq @p
        lda #':'
        ldx FNI
        jsr fn_put0
        inc FNI
        lda FNPORT
        jsr fn_dec
@p:     jsr net_ptr
        ldy FNJ
@pc:    cpy FNE
        bcs @pe
        lda [PSRC],y
        and #$00FF
        cmp #' '+1
        bcc @pe
        ldx FNI
        cpx #FN_PAY+500
        bcs @pe
        phy
        jsr fn_put0
        ply
        inc FNI
        iny
        bra @pc
@pe:    lda #12                 ; GET mit Kopfzeilen
        ldx FNPOST
        beq @o
        lda #13                 ; POST
@o:     jsr fn_nopen
        bcc @op
@bad:   lda #NE_CONN
        sta NERR
        sec
        rts
@op:    stz FNCT
        lda #1
        sta FNHDR
        lda FNPOST
        bne @post
        lda #'M'                ; Inhaltstyp sammeln
        ldx #1
        jsr fn_nctl2
        bcs @nh
        lda #FN_PAY
        sta FNI
        lda #fn_s_ctyp
        jsr fn_kpay
        lda #12
        sta FNN
        lda #9
        jsr fn_rw
        lda #'M'                ; gesammelte Kopfzeilen lesen
        ldx #2
        jsr fn_nctl2
        bcs @nh
        jsr fn_nstat            ; fuehrt die Anfrage aus
        lda #46
        sta FNN
        lda #8
        jsr fn_rw
        bcs @nh
        ldx #0                  ; bis $9B / CR / LF
@ct:    cpx FNCNT
        bcs @cte
        cpx #46
        bcs @cte
        phx
        jsr fn_get0
        plx
        cmp #$9B
        beq @cte
        cmp #' '
        bcc @cte
        sep #$20
        sta FNCT+1,x
        rep #$20
        inx
        bra @ct
@cte:   txa
        sep #$20
        sta FNCT
        rep #$20
@nh:    lda #'M'                ; zurueck zum Inhalt
        ldx #0
        jsr fn_nctl2
        clc
        rts
@post:  lda #'M'                ; Formulardaten senden
        ldx #4
        jsr fn_nctl2
        jsr net_ptr             ; Inhalt nach der Leerzeile
        ldy WPTR
@bl:    cpy FNE
        bcs @bn
        lda [PSRC],y
        cmp #$0A0D
        bne @bi
        iny
        iny
        lda [PSRC],y
        cmp #$0A0D
        bne @bl
        iny
        iny
        bra @bd
@bi:    iny
        bra @bl
@bd:    sty FNI
        lda FNE
        sec
        sbc FNI
        sta FNJ
        jsr fn_send
@bn:    lda #'M'
        ldx #0
        jsr fn_nctl2
        clc
        rts

; HTTP-Kopf vor die Antwort (https): Status aus dem Fehlercode
fn_head:
        stz FNHDR
        jsr fn_nstat
        lda #FN_PAY             ; Kopf erst in der Nutzlast bauen
        sta FNI
        lda FNST
        cmp #FNS_404
        bne @n4
        lda #fn_s_404
        jsr fn_kpay
        bra @end
@n4:    cmp #128
        bcc @ok
        cmp #FNS_EOF
        beq @ok
        lda FNAV
        bne @ok
        lda #fn_s_502
        jsr fn_kpay
        bra @end
@ok:    lda #fn_s_200
        jsr fn_kpay
        lda FNCT
        and #$00FF
        bne @ct
        lda #fn_s_html
        jsr fn_kpay
        bra @end
@ct:    sta FNK
        ldy #0
@c:     lda FNCT+1,y
        ldx FNI
        phy
        jsr fn_put0
        ply
        inc FNI
        iny
        cpy FNK
        bcc @c
@end:   lda #fn_s_crlf2
        jsr fn_kpay
        jsr net_ptr             ; nach WPTR kopieren
        ldx #0
@cp:    cpx FNI
        bcs @r
        ldy WPTR
        cpy NLIM
        bcs @r
        phx
        jsr fn_get0
        plx
        sep #$20
        sta [PSRC],y
        rep #$20
        inc WPTR
        inx
        bra @cp
@r:     rts

; FNAV Bytes (hoechstens 512, bis NLIM) nach WPTR holen
fn_take:
        lda NLIM
        sec
        sbc WPTR
        cmp FNAV
        bcc @m
        lda FNAV
@m:     cmp #513
        bcc @n
        lda #512
@n:     sta FNN
        beq @r
        lda #8                  ; READ
        jsr fn_rw
        bcs @e
        jsr net_ptr
        ldx #0
@c:     cpx FNCNT
        bcs @r
        phx
        jsr fn_get0
        plx
        ldy WPTR
        sep #$20
        sta [PSRC],y
        rep #$20
        inc WPTR
        inx
        bra @c
@r:     clc
        rts
@e:     sec
        rts

; ---------- alles bis zum Ende lesen (nach WPTR, hoechstens bis NLIM) ----------
fn_readall:
        stz NFULL
        lda FNHDR
        beq @s
        jsr fn_head
@s:     jsr tm_start
@l:     jsr net_esc
        bcc @n
        rts
@n:     jsr fn_nstat
        bcs @fail
        lda FNAV
        beq @empty
        lda NLIM
        cmp WPTR
        beq @full
        jsr fn_take
        bcs @fail
        lda NSTRM               ; Platz fuer ein weiteres Stueck?
        beq @pr
        lda NLIM
        sec
        sbc WPTR
        cmp #513
        bcs @pr
        lda #1
        sta NFULL
        clc
        rts
@pr:    lda NQUIET
        bne @q
        JSLF xf_web_prog
@q:     jsr tm_start
        bra @l
@full:  lda NSTRM
        sta NFULL
        clc
        rts
@empty: lda FNCON
        beq @done
        lda FNST
        cmp #FNS_EOF
        beq @done
        cmp #128
        bcs @done
        jsr t_elapsed
        cmp #1200               ; 20 s ohne Daten
        bcc @l
        lda #NE_TIME
        sta NERR
        sec
        rts
@fail:  lda #NE_CONN
        sta NERR
        sec
        rts
@done:  clc
        rts

; ---------- ohne Warten. C=1: Verbindung zu ----------
fn_poll:
        jsr fn_nstat
        bcs @cl
        lda FNAV
        beq @e
        jsr fn_take
        clc
        rts
@e:     lda FNCON
        beq @cl
        clc
        rts
@cl:    sec
        rts

; ---------- schliessen ----------
fn_close:
        stz FNMODE
        stz FNHDR
        lda FNUNIT
        beq @r
        lda #0
        ldx #FN_PAY
        jsr fn_put0
        lda #0
        ldx #FN_PAY+1
        jsr fn_put0
        lda #'C'
        jsr fn_nctl
@r:     clc
        rts

fn_s_tcp:   .byte 8,"N:TCP://"
fn_s_ssh:   .byte 8,"N:SSH://"
fn_s_ftp:   .byte 8,"N:FTP://"
fn_s_https: .byte 10,"N:HTTPS://"
fn_s_ctyp:  .byte 12,"Content-Type"
fn_s_200:   .byte 31,"HTTP/1.0 200 OK",13,10,"Content-Type: "
fn_s_html:  .byte 9,"text/html"
fn_s_404:   .byte 22,"HTTP/1.0 404 Not Found"
fn_s_502:   .byte 24,"HTTP/1.0 502 Bad Gateway"
fn_s_crlf2: .byte 4,13,10,13,10
