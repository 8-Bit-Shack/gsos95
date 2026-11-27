; ================================================================
; GS/OS95 Browser: Cookies fuer die Sitzung (Teil von web.s, XCODE2)
; Set-Cookie im Antwortkopf wird gemerkt (Name=Wert, Domain), jede Anfrage
; an denselben Rechner (oder eine Unterdomain der Domain) schickt sie mit
; "Cookie: a=1; b=2" zurueck. Nur im Speicher (bis zum Beenden), Path,
; Expires und Secure werden nicht ausgewertet; ein leerer Wert loescht.
; Speicher: eigener Block (2,25 KB): 16 Eintraege zu 128 Bytes
;   +0 Rechner/Domain (Laenge + bis 39; Bit 7 der Laenge: ohne Domain=, nur genau
;      dieser Rechner), +40 "Name=Wert" (Laenge + bis 87)
;   Eintrag 16 = Arbeitsplatz beim Einlesen.
; ================================================================
        .pushseg
        .segment "BSS"
CKP:    .res 4                  ; Adresse des Blocks (Bank 0 = noch keiner)
CKK:    .res 2
CKI:    .res 2
CKJ:    .res 2
CKN:    .res 2
CKRR:   .res 2                  ; naechster zu ersetzender Eintrag
        .popseg
CK_N = 16
CK_L = 128
CK_V = 40
CK_T = CK_N*CK_L
CK_SZ = CK_T+CK_L

; PDST = Cookie-Block (beim ersten Mal anlegen). C=1 kein Speicher
ck_ptr: lda CKP+2
        bne @set
        pha
        pha
        pea $0000
        pea CK_SZ
        lda USERID
        pha
        pea $C018
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        pla
        sta HND
        pla
        sta HND+2
        bcc *+3
        rts
        lda [HND]
        sta CKP
        ldy #2
        lda [HND],y
        and #$00FF
        sta CKP+2
        lda CKP
        sta PDST
        lda CKP+2
        sta PDST+2
        ldy #0
        lda #0
@z:     sta [PDST],y
        iny
        iny
        cpy #CK_SZ
        bcc @z
        stz CKRR
@set:   lda CKP
        sta PDST
        lda CKP+2
        sta PDST+2
        clc
        rts

; Byte A nach Block+X
ck_wb:  phy
        txy
        sep #$20
        sta [PDST],y
        rep #$20
        ply
        rts
; Byte Block+X -> A
ck_rb:  phy
        txy
        lda [PDST],y
        and #$00FF
        ply
        rts

; ---------- Set-Cookie-Zeile ab Y (Zeilenanfang) merken ----------
ck_set: lda UK                  ; UK braucht hdr_parse noch
        pha
        lda PDST
        pha
        lda PDST+2
        pha
        phy
        jsr ck_ptr
        ply
        bcc @s
        jmp @out
@s:     cpy RAWEND              ; ':' und Leerzeichen ueberspringen
        bcc *+5
        jmp @out
        BRD
        iny
        cmp #':'
        bne @s
@sp:    cpy RAWEND
        bcc *+5
        jmp @out
        BRD
        cmp #' '
        bne @v
        iny
        bra @sp
@v:     stz CKK                 ; Name=Wert bis ';' / Zeilenende
@cv:    cpy RAWEND
        bcs @cve
        BRD
        cmp #';'
        beq @cve
        cmp #$0D
        beq @cve
        cmp #$0A
        beq @cve
        ldx CKK
        cpx #87
        bcs @sk
        pha
        txa
        clc
        adc #CK_T+CK_V+1
        tax
        pla
        jsr ck_wb
        inc CKK
@sk:    iny
        bra @cv
@cve:   lda CKK
        ldx #CK_T+CK_V
        jsr ck_wb
        phy                     ; Vorgabe: Rechner der Anfrage
        stz CKK
        ldy #WB_HOST
        BRD
        sta CKN
@hc:    lda CKK
        cmp CKN
        bcs @hce
        cmp #39
        bcs @hce
        clc
        adc #WB_HOST+1
        tay
        BRD
        jsr lower
        pha
        lda CKK
        clc
        adc #CK_T+1
        tax
        pla
        jsr ck_wb
        inc CKK
        bra @hc
@hce:   lda CKK
        ora #$80                ; Bit 7: nur genau dieser Rechner (ohne Domain=)
        ldx #CK_T
        jsr ck_wb
        ply
@at:    cpy RAWEND              ; Attribute: nur Domain=
        bcs @fin
        BRD
        cmp #$0D
        beq @fin
        cmp #$0A
        beq @fin
        iny
        cmp #';'
        bne @at
@as:    cpy RAWEND
        bcs @fin
        BRD
        cmp #' '
        bne @ad
        iny
        bra @as
@ad:    sty UK
        ldx #h_dom
        jsr pfx_ci
        bcs @nd
        jsr ck_dom
        bra @at
@nd:    ldy UK
        bra @at
@fin:   jsr ck_store
@out:   pla
        sta PDST+2
        pla
        sta PDST
        pla
        sta UK
        rts

; Domain ab Y (ohne fuehrenden Punkt, bis ';' / Leerzeichen / Zeilenende) in den Arbeitsplatz
ck_dom: cpy RAWEND
        bcs @r
        BRD
        cmp #'.'
        bne @g
        iny
@g:     stz CKK
@l:     cpy RAWEND
        bcs @e
        BRD
        cmp #';'
        beq @e
        cmp #' '+1
        bcc @e
        ldx CKK
        cpx #39
        bcs @sk
        jsr lower
        pha
        txa
        clc
        adc #CK_T+1
        tax
        pla
        jsr ck_wb
        inc CKK
@sk:    iny
        bra @l
@e:     lda CKK
        beq @r
        ldx #CK_T
        jmp ck_wb
@r:     rts

; Arbeitsplatz in einen Eintrag: gleicher Rechner und Name ersetzt, leerer Wert loescht
ck_store:
        ldx #CK_T+CK_V          ; kein Name: nichts tun
        jsr ck_rb
        bne *+3
        rts
        stz CKI
@f:     jsr ck_same
        bcc @hit
        lda CKI
        clc
        adc #CK_L
        sta CKI
        cmp #CK_T
        bcc @f
        stz CKI                 ; freier Eintrag?
@e:     lda CKI
        clc
        adc #CK_V
        tax
        jsr ck_rb
        beq @hit
        lda CKI
        clc
        adc #CK_L
        sta CKI
        cmp #CK_T
        bcc @e
        lda CKRR                ; sonst reihum ersetzen
        sta CKI
        clc
        adc #CK_L
        cmp #CK_T
        bcc @rr
        lda #0
@rr:    sta CKRR
@hit:   jsr ck_empty            ; leerer Wert: Eintrag loeschen
        bcc @cp
        lda #0
        ldx CKI
        jsr ck_wb
        lda CKI
        clc
        adc #CK_V
        tax
        lda #0
        jmp ck_wb
@cp:    ldx #0                  ; 128 Bytes kopieren
@c:     phx
        txa
        clc
        adc #CK_T
        tax
        jsr ck_rb
        plx
        phx
        pha
        txa
        clc
        adc CKI
        tax
        pla
        jsr ck_wb
        plx
        inx
        cpx #CK_L
        bcc @c
        rts

; Wert im Arbeitsplatz leer ("name=")? C=1 ja
ck_empty:
        ldx #CK_T+CK_V
        jsr ck_rb
        sta CKN
        ldx #0
@l:     cpx CKN
        bcs @y
        phx
        txa
        clc
        adc #CK_T+CK_V+1
        tax
        jsr ck_rb
        plx
        inx
        cmp #'='
        bne @l
        cpx CKN                 ; nach '=' noch Zeichen?
        bcs @y
        clc
        rts
@y:     sec
        rts

; Eintrag CKI = Arbeitsplatz (gleicher Rechner, gleicher Name)? C=0 ja
ck_same:
        ldx CKI                 ; Rechner gleich (mit Laenge)
        jsr ck_rb
        beq @no
        sta CKN
        ldx #CK_T
        jsr ck_rb
        cmp CKN
        bne @no
        and #$007F
        sta CKN
        stz CKJ
@h:     lda CKJ
        cmp CKN
        bcs @nm
        inc a
        pha
        clc
        adc CKI
        tax
        jsr ck_rb
        sta CKK
        pla
        clc
        adc #CK_T
        tax
        jsr ck_rb
        cmp CKK
        bne @no
        inc CKJ
        bra @h
@nm:    stz CKJ                 ; Name bis '=' gleich
@n:     lda CKJ
        inc a
        pha
        clc
        adc #CK_V
        adc CKI
        tax
        jsr ck_rb
        sta CKK
        pla
        clc
        adc #CK_T+CK_V
        tax
        jsr ck_rb
        cmp CKK
        bne @no
        cmp #'='
        beq @y
        inc CKJ
        lda CKJ
        cmp #87
        bcc @n
@y:     clc
        rts
@no:    sec
        rts

; ---------- passende Cookies als "Cookie: ..." an die Anfrage haengen ----------
ck_req: lda CKP+2               ; noch keine Cookies
        bne *+3
        rts
        lda PDST
        pha
        lda PDST+2
        pha
        jsr ck_ptr
        stz CKK                 ; schon einer geschrieben?
        stz CKI
@l:     lda CKI
        clc
        adc #CK_V
        tax
        jsr ck_rb
        beq @nx
        jsr ck_match
        bcs @nx
        lda CKK
        bne @sep
        ldx #r_cook
        jsr rq_m
        bra @w
@sep:   lda #';'
        jsr rq_c
        lda #' '
        jsr rq_c
@w:     inc CKK
        lda CKI                 ; Name=Wert
        clc
        adc #CK_V
        tax
        jsr ck_rb
        sta CKN
        stz CKJ
@c:     lda CKJ
        cmp CKN
        bcs @nx
        inc a
        clc
        adc CKI
        adc #CK_V
        tax
        jsr ck_rb
        jsr rq_c
        inc CKJ
        bra @c
@nx:    lda CKI
        clc
        adc #CK_L
        sta CKI
        cmp #CK_T
        bcc @l
        lda CKK
        beq @r
        ldx #r_crlf
        jsr rq_m
@r:     pla
        sta PDST+2
        pla
        sta PDST
        rts

; passt Eintrag CKI zum Rechner WB_HOST (gleich oder ".domain" am Ende)? C=0 ja
ck_match:
        ldx CKI
        jsr ck_rb
        beq @no
        tax
        and #$7F
        sta CKN                 ; Laenge der Domain
        ldy #WB_HOST
        BRD
        sta CKJ                 ; Laenge des Rechners
        cmp CKN
        bcc @no
        beq @cmp
        txa                     ; nur dieser Rechner: keine Unterdomain
        and #$0080
        bne @no
        lda CKJ
        sec                     ; laenger: davor muss ein Punkt stehen
        sbc CKN
        clc
        adc #WB_HOST
        tay
        BRD
        cmp #'.'
        bne @no
@cmp:   lda CKJ                 ; die letzten CKN Zeichen vergleichen
        sec
        sbc CKN
        clc
        adc #WB_HOST+1
        sta UT_P
        stz UT_N
@l:     lda UT_N
        cmp CKN
        bcs @y
        clc
        adc UT_P
        tay
        BRD
        jsr lower
        sta UT_C
        lda UT_N
        inc a
        clc
        adc CKI
        tax
        jsr ck_rb
        cmp UT_C
        bne @no
        inc UT_N
        bra @l
@y:     clc
        rts
@no:    sec
        rts

        .pushseg
        .segment "RODATA"
h_setc: .byte 11,"set-cookie:"
h_dom:  .byte 7,"domain="
r_cook: .byte 8,"Cookie: "
        .popseg
