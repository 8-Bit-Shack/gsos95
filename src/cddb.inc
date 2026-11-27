; ================================================================
; GS/OS95: CD-Titelnamen aus dem Internet (CDDB ueber HTTP), Teil von cd.inc
; Disc-ID aus dem Inhaltsverzeichnis (freedb-Verfahren), dann
;   GET /~cddb/cddb.cgi?cmd=cddb+query+ID+N+Frames..+Sekunden&hello=..&proto=5
;   GET /~cddb/cddb.cgi?cmd=cddb+read+Kategorie+ID&hello=..&proto=5
; Server Standard freedb.dbpoweramp.com (Port 80), einstellbar mit
; about:cddb=rechner, E-Mail fuer "hello" mit about:cddbmail=name@rechner
; (gnudb.gnudb.org verlangt eine echte E-Mail-Adresse).
; Einstellungen: NPREF+1700 Server, NPREF+1764 E-Mail (je Laenge + 63)
; Anfrage ab NWB:$7000, Antwort ab NWB:$0000 (Rohbereich des Browsers)
; ================================================================
CDB_SRV = NPREF+1700
CDB_ML  = NPREF+1764
CDB_RQ  = $7000

; ---------- Daten (diese Bank) ----------
CDNF:   .res 2                  ; Titelnamen: 0 keine, 1 suche, 2 da, 3 nicht gefunden, 4 Netzwerkfehler
CDID:   .res 4                  ; Disc-ID: +0 Titelzahl, +1/+2 Laenge in s, +3 Pruefsumme
CDLID:  .res 4                  ; Disc-ID der geladenen Namen
CDF0:   .res 2                  ; 32-Bit-Zahl
CDF1:   .res 2
CDQ2:   .res 2
CDTS:   .res 2
CDRY:   .res 2
CDQ3:   .res 2
CDQD:   .res 2                  ; Ziel beim Anhaengen (Adresse, Laengenbyte)
CDQM:   .res 2                  ; hoechste Laenge
CDU8:   .res 2                  ; UTF-8: Rest fuer das naechste Byte
CDP:    .res 2                  ; Antwort: Position, Ende
CDPE:   .res 2
CDLS:   .res 2                  ; aktuelle Zeile
CDLE:   .res 2
CDSRV:  .res 64                 ; Server (Laenge + Text), aus GSOS95.NET
CDML:   .res 64                 ; E-Mail
CDCAT:  .res 24                 ; Kategorie (Laenge + Text)
CDDID:  .res 12                 ; Disc-ID der Antwort
CDART:  .res 44                 ; "Kuenstler / Album"
CDTT:   .res 100*32             ; Titelnamen, Index = Titelnummer (Laenge + bis 31)

cdb_dsrv:  .byte cdb_dsrv_e-cdb_dsrv-1,"freedb.dbpoweramp.com"
cdb_dsrv_e:
cdb_get:   .byte cdb_get_e-cdb_get-1,"GET /~cddb/cddb.cgi?cmd=cddb+query+"
cdb_get_e:
cdb_get2:  .byte cdb_get2_e-cdb_get2-1,"GET /~cddb/cddb.cgi?cmd=cddb+read+"
cdb_get2_e:
cdb_hel:   .byte cdb_hel_e-cdb_hel-1,"&hello="
cdb_hel_e:
cdb_hel2:  .byte cdb_hel2_e-cdb_hel2-1,"+GSOS95+2.18&proto=5 HTTP/1.0",13,10,"Host: "
cdb_hel2_e:
cdb_ua:    .byte cdb_ua_e-cdb_ua-1,13,10,"User-Agent: GSOS95/2.18 (Apple IIgs)",13,10,13,10
cdb_ua_e:
cdb_hdef:  .byte cdb_hdef_e-cdb_hdef-1,"gsos95+apple2gs"
cdb_hdef_e:
cdb_lat1:  .byte $20,$21,$63,$4C,$3F,$59,$7C,$53,$22,$63,$61,$22,$2D,$00,$52,$2D,$6F,$2B,$32,$33,$27,$75,$50,$87
           .byte $2C,$31,$6F,$22,$3F,$3F,$3F,$3F,$41,$41,$41,$41,$83,$41,$41,$43,$45,$45,$45,$45,$49,$49,$49,$49
           .byte $44,$4E,$4F,$4F,$4F,$4F,$84,$78,$4F,$55,$55,$55,$85,$59,$50,$86,$61,$61,$61,$61,$80,$61,$61,$63
           .byte $65,$65,$65,$65,$69,$69,$69,$69,$64,$6E,$6F,$6F,$6F,$6F,$81,$2F,$6F,$75,$75,$75,$82,$79,$70,$79

; ---------- Disc-ID (nach dem Lesen des Inhaltsverzeichnisses) ----------
; Neue CD: Namen loeschen; gleiche CD wie zuletzt: Namen bleiben
cdl_discid:
        stz CDQ2                ; Summe der Ziffern der Startsekunden
        lda CDT0
        sta CDQ3
@l:     ldx CDQ3
        jsr cd_tsec
        sta CDF0
        stz CDF1
@d:     jsr cd_d10
        clc
        adc CDQ2
        sta CDQ2
        lda CDF0
        bne @d
        inc CDQ3
        lda CDQ3
        cmp CDT1
        bcc @l
        beq @l
@m:     lda CDQ2                ; mod 255
        cmp #255
        bcc @m2
        sbc #255
        sta CDQ2
        bra @m
@m2:    sep #$20
        sta CDID+3
        rep #$20
        ldx CDT1                ; Laenge = Auslauf - erster Titel (ganze Sekunden)
        inx
        jsr cd_tsec
        sta CDQ3
        ldx CDT0
        jsr cd_tsec
        eor #$FFFF
        sec
        adc CDQ3
        sta CDID+1              ; (+1/+2, +3 bleibt)
        sep #$20
        lda CDQ2
        sta CDID+3
        lda CDT1
        sec
        sbc CDT0
        inc a
        sta CDID
        rep #$20
        lda CDID                ; dieselbe CD wie die geladenen Namen?
        cmp CDLID
        bne @new
        lda CDID+2
        cmp CDLID+2
        bne @new
        lda CDNF
        cmp #2
        beq @r
        lda #2
        sta CDNF
@r:     rts
@new:   stz CDNF
        stz CDLID               ; Namen loeschen
        stz CDLID+2
        sep #$20
        stz CDART
        rep #$20
        ldx #0
@c:     sep #$20
        stz CDTT,x
        rep #$20
        txa
        clc
        adc #32
        tax
        cpx #100*32
        bcc @c
        rts

; Startsekunden von Titel X (Minuten*60 + Sekunden) -> A
cd_tsec:
        lda TS,x
        and #$00FF
        sta CDTS
        lda TM,x
        and #$00FF
        jsr cd_x60
        clc
        adc CDTS
        rts

; CDF1:CDF0 durch 10, Rest -> A
cd_d10: lda #0
        ldx #32
@l:     asl CDF0
        rol CDF1
        rol a
        cmp #10
        bcc @n
        sbc #10
        inc CDF0
@n:     dex
        bne @l
        rts

; ---------- Anfrage bauen (PSRC = Pufferbank, Offset 0) ----------
; A = 0 query, 1 read. Server nach WB_HOST, Anfrage ab CDB_RQ -> A = Laenge
cdl_req:
        sta CDQ3
        lda CDSRV               ; Server leer: Standard
        and #$00FF
        bne @h
        ldx #21
@ds:    lda cdb_dsrv,x
        sep #$20
        sta CDSRV,x
        rep #$20
        dex
        bpl @ds
@h:     ldy #WB_HOST            ; Rechnername (Laenge + Text)
        lda #CDSRV
        jsr cq_sl
        ldy #CDB_RQ
        lda CDQ3
        bne @rd
        lda #cdb_get
        jsr cq_s
        lda CDID+3              ; Disc-ID hexadezimal
        jsr cq_hx
        lda CDID+2
        jsr cq_hx
        lda CDID+1
        jsr cq_hx
        lda CDID
        jsr cq_hx
        lda #'+'
        jsr pk_b
        lda CDID
        and #$00FF
        jsr cq_dec
        lda CDT0                ; Beginn jedes Titels in Frames
        sta CDQ2
@t:     lda #'+'
        jsr pk_b
        ldx CDQ2
        jsr cd_tsec
        jsr cd_m75
        ldx CDQ2
        lda TF,x
        and #$00FF
        clc
        adc CDF0
        sta CDF0
        lda CDF1
        adc #0
        sta CDF1
        jsr cq_d32
        inc CDQ2
        lda CDQ2
        cmp CDT1
        bcc @t
        beq @t
        lda #'+'                ; Gesamtlaenge in Sekunden (Auslauf)
        jsr pk_b
        ldx CDT1
        inx
        jsr cd_tsec
        jsr cq_dec
        bra @hel
@rd:    lda #cdb_get2           ; read Kategorie ID
        jsr cq_s
        lda #CDCAT
        jsr cq_s
        lda #'+'
        jsr pk_b
        lda #CDDID
        jsr cq_s
@hel:   lda #cdb_hel            ; &hello=name+rechner+GSOS95+2.15
        jsr cq_s
        lda CDML
        and #$00FF
        bne @ml
        lda #cdb_hdef
        jsr cq_s
        bra @h2
@ml:    sta CDQM                ; name@rechner -> name+rechner
        ldx #1
        stz CDQ2
@mc:    lda CDML,x
        and #$00FF
        cmp #'@'
        bne @mp
        inc CDQ2
        lda #'+'
@mp:    cmp #' '
        beq @mn
        jsr pk_b
@mn:    cpx CDQM
        bcs @md
        inx
        bra @mc
@md:
        lda CDQ2
        bne @h2
        lda #'+'                ; ohne @: Rechner "apple2gs"
        jsr pk_b
        lda #'a'
        jsr pk_b
@h2:    lda #cdb_hel2
        jsr cq_s
        lda #CDSRV
        jsr cq_s
        lda #cdb_ua
        jsr cq_s
        tya
        sec
        sbc #CDB_RQ
        rts

; Text A (Laenge + Text, diese Bank) nach [PSRC],Y
cq_s:   tax
        lda a:0,x
        and #$00FF
        sta CDQM
@l:     lda CDQM
        beq @r
        inx
        lda a:0,x
        jsr pk_b
        dec CDQM
        bra @l
@r:     rts
; dasselbe mit Laengenbyte
cq_sl:  tax
        lda a:0,x
        jsr pk_b
        txa
        jmp cq_s
; Byte A als zwei Hexziffern
cq_hx:  and #$00FF
        pha
        lsr
        lsr
        lsr
        lsr
        jsr @d
        pla
        and #$000F
@d:     cmp #10
        bcc @n
        adc #'a'-'0'-11
@n:     adc #'0'
        jmp pk_b
; Zahl A dezimal
cq_dec: sta CDF0
        stz CDF1
; CDF1:CDF0 dezimal
cq_d32: ldx #0
@l:     phx
        jsr cd_d10
        plx
        pha
        inx
        lda CDF0
        ora CDF1
        bne @l
@o:     pla
        clc
        adc #'0'
        jsr pk_b
        dex
        bne @o
        rts
; A * 75 -> CDF1:CDF0
cd_m75: sta CDTS
        stz CDF0
        stz CDF1
        ldx #75
@l:     lda CDF0
        clc
        adc CDTS
        sta CDF0
        lda CDF1
        adc #0
        sta CDF1
        dex
        bne @l
        rts

; ---------- Antwort lesen ----------
; Byte an [PSRC],Y -> A
rp_b:   lda [PSRC],y
        and #$00FF
        rts

; Antwortlaenge A: Kopf ueberspringen (Leerzeile), CDP = Anfang des Inhalts
cdl_body:
        sta CDPE
        ldy #0
@l:     cpy CDPE
        bcs @no
        jsr rp_b
        iny
        cmp #10
        bne @l
        jsr rp_b                ; LF LF oder LF CR LF
        cmp #13
        bne @lf
        iny
        jsr rp_b
@lf:    cmp #10
        bne @l
        iny
        sty CDP
        clc
        rts
@no:    sec
        rts

; naechste Zeile: CDLS..CDLE (ohne CR/LF), C=1 am Ende
cd_line:
        ldy CDP
        cpy CDPE
        bcs @r
        sty CDLS
@l:     cpy CDPE
        bcs @e
        jsr rp_b
        cmp #10
        beq @e
        iny
        bra @l
@e:     sty CDLE
        iny
        sty CDP
        ldy CDLE                ; CR am Ende weg
        cpy CDLS
        beq @ok
        dey
        jsr rp_b
        cmp #13
        bne @ok
        sty CDLE
@ok:    clc
@r:     rts

; Zeile beginnt mit Text A (Laenge + Text, diese Bank)? C=1 ja, Y = dahinter
cd_starts:
        tax
        lda a:0,x
        and #$00FF
        sta CDQM
        ldy CDLS
@l:     lda CDQM
        beq @y
        cpy CDLE
        bcs @no
        inx
        lda a:0,x
        and #$00FF
        sta CDTS
        jsr rp_b
        cmp CDTS
        bne @no
        iny
        dec CDQM
        bra @l
@y:     sec
        rts
@no:    clc
        rts

; Wort ab Y (bis Leerzeichen) -> Text an X (Laenge + Text, hoechstens A), Y dahinter
cd_word:
        stx CDQD
        sta CDQM
        sep #$20
        stz a:0,x
        rep #$20
@sp:    cpy CDLE                ; Leerzeichen davor
        bcs @r
        jsr rp_b
        cmp #' '
        bne @l
        iny
        bra @sp
@l:     cpy CDLE
        bcs @r
        jsr rp_b
        cmp #' '
        beq @r
        ldx CDQD
        pha
        lda a:0,x
        and #$00FF
        cmp CDQM
        pla
        bcs @nx
        jsr cd_app1
@nx:    iny
        bra @l
@r:     rts

; Zeichen A an Text CDQD (diese Bank) anhaengen
cd_app1:
        pha
        ldx CDQD
        lda a:0,x
        and #$00FF
        inc a
        sep #$20
        sta a:0,x
        rep #$20
        clc
        adc CDQD
        tax
        pla
        sep #$20
        sta a:0,x
        rep #$20
        rts

; Rest der Zeile ab Y an Text CDQD anhaengen (hoechstens CDQM Zeichen),
; Latin-1 bzw. UTF-8 -> Zeichensatz von GS/OS95
cd_appv:
        stz CDU8
@l:     cpy CDLE
        bcs @r
        jsr rp_b
        iny
        jsr cd_cnv
        bcs @l
        pha
        ldx CDQD
        lda a:0,x
        and #$00FF
        cmp CDQM
        pla
        bcs @r
        jsr cd_app1
        bra @l
@r:     rts

; Zeichen A umsetzen: C=1 nichts ausgeben. Latin-1, dazu UTF-8 (erkannt am
; Folgebyte $80..$BF): C2/C3 xx -> Latin-1, laengere Folgen -> "?"
cd_cnv: ldx CDU8
        bne @cont
        cmp #$80
        bcs @hi
        cmp #' '
        bcc @no
        clc
        rts
@hi:    cmp #$C0
        bcc @lat
        sta CDTS
        cpy CDLE
        bcs @lat0
        jsr rp_b                ; Folgebyte?
        and #$00C0
        cmp #$0080
        bne @lat0
        lda CDTS
        cmp #$C4
        bcs @oth
        and #$0001              ; C2 -> 1, C3 -> $41
        beq @c2
        lda #$41
        bra @s8
@c2:    lda #1
@s8:    sta CDU8
        bra @no
@oth:   lda #$FF                ; Rest der Folge ueberspringen
        sta CDU8
        lda #'?'
        clc
        rts
@cont:  cpx #$FF
        bne @cv
        pha
        and #$00C0
        cmp #$0080
        pla
        beq @no
        stz CDU8
        bra cd_cnv
@cv:    clc
        adc CDU8
        dec a
        stz CDU8
        bra @lat
@lat0:  lda CDTS
@lat:   cmp #$A0
        bcs @t
        lda #'?'
        clc
        rts
@t:     sec
        sbc #$A0
        tax
        lda cdb_lat1,x
        and #$00FF
        beq @no
        clc
        rts
@no:    sec
        rts

; ---------- query auswerten: Kategorie und ID -> CDCAT/CDDID, C=1 nicht gefunden ----------
cdl_pq: jsr cdl_body
        bcs @no
        jsr cd_line
        bcs @no
        lda #cdb_200
        jsr cd_starts
        bcs @w
        lda #cdb_210
        jsr cd_starts
        bcs @nx
        lda #cdb_211
        jsr cd_starts
        bcc @no
@nx:    jsr cd_line             ; mehrere: den ersten nehmen
        bcs @no
        ldy CDLS
@w:     ldx #CDCAT
        lda #22
        jsr cd_word
        ldx #CDDID
        lda #10
        jsr cd_word
        lda CDCAT
        and #$00FF
        beq @no
        lda CDDID
        and #$00FF
        beq @no
        clc
        rts
@no:    sec
        rts

; ---------- read auswerten: DTITLE, TTITLEn ----------
cdl_pr: jsr cdl_body
        bcc *+5
        jmp @no
        jsr cd_line
        bcc *+5
        jmp @no
        lda #cdb_210
        jsr cd_starts
        bcs *+5
        jmp @no
        sep #$20
        stz CDART
        rep #$20
@l:     jsr cd_line
        bcc *+5
        jmp @e
        ldy CDLS                ; "." = Ende
        cpy CDLE
        beq @l
        jsr rp_b
        cmp #'.'
        bne @d
        iny
        cpy CDLE
        beq @e
@d:     lda #cdb_dt
        jsr cd_starts
        bcc @t
        lda #CDART
        sta CDQD
        lda #40
        sta CDQM
        jsr cd_appv
        bra @l
@t:     lda #cdb_tt
        jsr cd_starts
        bcc @l
        stz CDQ2                ; Nummer
@n:     cpy CDLE
        bcs @l
        jsr rp_b
        iny
        cmp #'='
        beq @v
        sec
        sbc #'0'
        cmp #10
        bcs @l
        sta CDTS
        lda CDQ2
        asl
        asl
        adc CDQ2
        asl
        adc CDTS
        sta CDQ2
        bra @n
@v:     lda CDQ2
        clc
        adc CDT0
        cmp CDT1
        beq @v2
        bcs @l
@v2:    asl                     ; * 32
        asl
        asl
        asl
        asl
        clc
        adc #CDTT
        sta CDQD
        lda #31
        sta CDQM
        jsr cd_appv
        jmp @l
@e:     lda #2
        sta CDNF
        lda CDID
        sta CDLID
        lda CDID+2
        sta CDLID+2
        clc
        rts
@no:    sec
        rts

cdb_200: .byte cdb_200_e-cdb_200-1,"200 "
cdb_200_e:
cdb_210: .byte cdb_210_e-cdb_210-1,"210"
cdb_210_e:
cdb_211: .byte cdb_211_e-cdb_211-1,"211"
cdb_211_e:
cdb_dt:  .byte cdb_dt_e-cdb_dt-1,"DTITLE="
cdb_dt_e:
cdb_tt:  .byte cdb_tt_e-cdb_tt-1,"TTITLE"
cdb_tt_e:
