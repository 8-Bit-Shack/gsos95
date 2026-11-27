; ================================================================
; GS/OS95: Rechner (Fensterart 24), Teil von net.s (XCODE3)
; Zahlen dezimal mit 16 Stellen (BCD, Dezimalmodus des 65816), Exponent
; bis +-9999; angezeigt werden 12 Stellen (gerundet), sonst 1,23e+45.
; Grundrechenarten, %, Wurzel, x hoch 2, 1/x, n!, Pi, Vorzeichen,
; Speicher (MC MR M+ M-), Zurueck, CE, C.
; Tasten: Ziffern , . + - * / : = Return % Delete; W Wurzel, Q x hoch 2,
; R 1/x, ! n!, P Pi, N Vorzeichen; Esc = C (zweimal Esc: Fenster zu).
; Logik (cl_*) mit Datenbank = diese Bank (LG), Oberflaeche (ca_*) mit
; Datenbank = Hauptbank.
; Zahl (12 Bytes): +0 Vorzeichen ($8000 negativ), +2 Exponent (Wert =
;   0,m1m2..m16 * 10^Exp), +4..+11 Mantisse: 4 BCD-Worte, +10 = m1..m4
; ================================================================
CAKIND = 24
CA_DX  = WX+8                   ; Anzeige
CA_DY  = WY+20
CA_DW  = 224
CA_DH  = 22
CA_BX  = WX+10                  ; Tasten 6 x 5
CA_BY  = WY+52
CA_BW  = 34
CA_BH  = 18
CA_SX  = 37
CA_SY  = 22
CA_H   = 176
C_BLUE = 4
C_RED  = 7
NM_S = 0
NM_E = 2
NM_M = 4
NSZ  = 12

; ---------- Einspruenge (ueber xf_wc_ndlg / xf_ndlg_click / xf_ndlg_key) ----------
xf_calc_open:   jsr ca_open
        rtl

; ================================================================
; Daten (diese Bank)
; ================================================================
CLX:    .res NSZ                ; angezeigte Zahl
CLACC:  .res NSZ                ; erster Operand
CLMEM:  .res NSZ                ; Speicher
CLA:    .res NSZ                ; Rechenregister
CLB:    .res NSZ
CLR:    .res NSZ
CLT:    .res NSZ
CLU:    .res NSZ
CLK:    .res NSZ
CLOP:   .res 2                  ; anstehende Rechenart: 0, '+', '-', '*', '/'
CLHX:   .res 2                  ; neue Zahl seit der Rechenart
CLERR:  .res 2
CLEN:   .res 2                  ; Eingabe laeuft
CLEB:   .res 24                 ; Eingabe (Zeichen)
CLEL:   .res 2                  ; Laenge
CLED:   .res 2                  ; Ziffern
CLES:   .res 2                  ; Komma schon da
CLEG:   .res 2                  ; Eingabe negativ
CLDSP:    .res 40                 ; Anzeige (Laengenbyte + Text)
CLHM:   .res 2                  ; Speicher belegt
CLP:    .res 20                 ; Produkt / Rest (10 Worte)
CLQ:    .res 10                 ; Quotient (5 Worte)
CLVV:    .res 10                 ; Teiler (5 Worte)
CLII:    .res 2
CLJ:    .res 2
CLN:    .res 2
CLC0:   .res 2
CLFN:   .res 2
CLCL:   .res 2                  ; zuletzt C gedrueckt (Esc noch einmal: Fenster zu)
CLKY:   .res 2                  ; Taste/Befehl
CLDG:   .res 16                 ; Ziffern fuer die Anzeige
CLND:   .res 2

; Konstanten
cl_one:  .word 0,1,0,0,0,$1000
cl_half: .word 0,0,0,0,0,$5000
cl_hund: .word 0,3,0,0,0,$1000
cl_pi:   .word 0,1,$9793,$5358,$5926,$3141

; ================================================================
; Zahlenrechnung (Datenbank = diese Bank)
; ================================================================
; Zahl X -> Zahl Y kopieren
n_cp:   lda #NSZ-2
        sta CLC0
@l:     phx
        phy
        txa
        clc
        adc CLC0
        tax
        tya
        clc
        adc CLC0
        tay
        lda a:0,x
        sta a:0,y
        ply
        plx
        dec CLC0
        dec CLC0
        bpl @l
        rts
; Zahl X = 0
n_zero: stz a:0,x
        stz a:2,x
        stz a:4,x
        stz a:6,x
        stz a:8,x
        stz a:10,x
        rts
; Z=1 wenn Mantisse X null
n_isz:  lda a:4,x
        ora a:6,x
        ora a:8,x
        ora a:10,x
        rts
; Mantisse X eine Stelle nach rechts / links
m_sr:   ldy #4
@l:     lsr a:10,x
        ror a:8,x
        ror a:6,x
        ror a:4,x
        dey
        bne @l
        rts
m_sl:   ldy #4
@l:     asl a:4,x
        rol a:6,x
        rol a:8,x
        rol a:10,x
        dey
        bne @l
        rts
; Zahl X normalisieren (erste Stelle nicht 0), 0 ohne Vorzeichen
n_norm: jsr n_isz
        bne @l
        stz a:0,x
        stz a:2,x
        rts
@l:     lda a:10,x
        and #$F000
        bne @r
        jsr m_sl
        dec a:2,x
        bra @l
@r:     rts

; Ueberlauf pruefen: Exponent ausserhalb +-9999 -> Fehler (klein: 0)
n_chk:  ldx #CLR
        jsr n_isz
        beq @r
        lda CLR+NM_E
        bmi @n
        cmp #10000
        bcc @r
        lda #1
        sta CLERR
        rts
@n:     cmp #$D8F1              ; -9999
        bcs @r
        jsr n_zero
@r:     rts

; CLR = CLA + CLB
n_add:  ldx #CLB
        jsr n_isz
        bne @b
        ldx #CLA
        ldy #CLR
        jmp n_cp
@b:     ldx #CLA
        jsr n_isz
        bne @g
        ldx #CLB
        ldy #CLR
        jmp n_cp
@g:     lda CLA+NM_E            ; groesseren Exponenten nach CLR, kleineren nach CLT
        sec
        sbc CLB+NM_E
        bvc *+5
        eor #$8000
        bmi @sw
        ldx #CLA
        ldy #CLR
        jsr n_cp
        ldx #CLB
        ldy #CLT
        jsr n_cp
        bra @al
@sw:    ldx #CLB
        ldy #CLR
        jsr n_cp
        ldx #CLA
        ldy #CLT
        jsr n_cp
@al:    lda CLR+NM_E            ; CLT angleichen
        sec
        sbc CLT+NM_E
        cmp #17
        bcc @s
        rts                     ; zu klein: CLR bleibt
@s:     sta CLN
@s1:    lda CLN
        beq @do
        ldx #CLT
        jsr m_sr
        dec CLN
        bra @s1
@do:    lda CLR+NM_S
        cmp CLT+NM_S
        bne @sub
        sed                     ; gleiches Vorzeichen: addieren
        clc
        lda CLR+NM_M
        adc CLT+NM_M
        sta CLR+NM_M
        lda CLR+NM_M+2
        adc CLT+NM_M+2
        sta CLR+NM_M+2
        lda CLR+NM_M+4
        adc CLT+NM_M+4
        sta CLR+NM_M+4
        lda CLR+NM_M+6
        adc CLT+NM_M+6
        sta CLR+NM_M+6
        cld
        bcc @r
        ldx #CLR                ; Uebertrag: eine Stelle nach rechts, 1 vorn
        jsr m_sr
        lda CLR+NM_M+6
        ora #$1000
        sta CLR+NM_M+6
        inc CLR+NM_E
@r:     rts
@sub:   ldx #6                  ; Betraege vergleichen
@c:     lda CLR+NM_M,x
        cmp CLT+NM_M,x
        bne @cd
        dex
        dex
        bpl @c
        ldx #CLR                ; gleich: 0
        jmp n_zero
@cd:    bcs @rt                 ; |CLR| >= |CLT|: CLR - CLT
        ldx #CLT                ; sonst CLT - CLR mit Vorzeichen von CLT
        ldy #CLU
        jsr n_cp
        ldx #CLR
        ldy #CLT
        jsr n_cp
        ldx #CLU
        ldy #CLR
        jsr n_cp
@rt:    sed
        sec
        lda CLR+NM_M
        sbc CLT+NM_M
        sta CLR+NM_M
        lda CLR+NM_M+2
        sbc CLT+NM_M+2
        sta CLR+NM_M+2
        lda CLR+NM_M+4
        sbc CLT+NM_M+4
        sta CLR+NM_M+4
        lda CLR+NM_M+6
        sbc CLT+NM_M+6
        sta CLR+NM_M+6
        cld
        ldx #CLR
        jmp n_norm
; CLR = CLA - CLB
n_sub:  ldx #CLB
        jsr n_isz
        beq @z
        lda CLB+NM_S
        eor #$8000
        sta CLB+NM_S
@z:     jmp n_add

; CLR = CLA * CLB
n_mul:  ldx #CLA
        jsr n_isz
        beq @z
        ldx #CLB
        jsr n_isz
        bne @g
@z:     ldx #CLR
        jmp n_zero
@g:     ldx #18
@c:     stz CLP,x
        dex
        dex
        bpl @c
        ldx #CLB                ; Ziffern von CLB (hinten zuerst) in CLT
        ldy #CLT
        jsr n_cp
        lda #16
        sta CLII
@d:     lda CLT+NM_M
        and #$000F
        sta CLN
        ldx #CLT
        jsr m_sr
@ad:    lda CLN                 ; CLP[4..8] += CLA * Ziffer
        beq @sh
        sed
        clc
        lda CLP+8
        adc CLA+NM_M
        sta CLP+8
        lda CLP+10
        adc CLA+NM_M+2
        sta CLP+10
        lda CLP+12
        adc CLA+NM_M+4
        sta CLP+12
        lda CLP+14
        adc CLA+NM_M+6
        sta CLP+14
        lda CLP+16
        adc #0
        sta CLP+16
        cld
        dec CLN
        bra @ad
@sh:    jsr p_sr                ; eine Stelle nach rechts
        dec CLII
        bne @d
        lda CLA+NM_E
        clc
        adc CLB+NM_E
        sta CLR+NM_E
        lda CLA+NM_S
        eor CLB+NM_S
        sta CLR+NM_S
@n:     lda CLP+14              ; vorne 0: nach links (Stellen aus CLP+6 nachziehen)
        and #$F000
        bne @t
        jsr p_sl
        dec CLR+NM_E
        bra @n
@t:     ldx #6
@m:     lda CLP+8,x
        sta CLR+NM_M,x
        dex
        dex
        bpl @m
        rts
; CLP (9 Worte) eine Stelle nach rechts / links
p_sr:   ldy #4
@l:     lsr CLP+16
        ror CLP+14
        ror CLP+12
        ror CLP+10
        ror CLP+8
        ror CLP+6
        ror CLP+4
        ror CLP+2
        ror CLP
        dey
        bne @l
        rts
p_sl:   ldy #4
@l:     asl CLP
        rol CLP+2
        rol CLP+4
        rol CLP+6
        rol CLP+8
        rol CLP+10
        rol CLP+12
        rol CLP+14
        rol CLP+16
        dey
        bne @l
        rts

; CLR = CLA / CLB (CLB = 0: Fehler)
n_div:  ldx #CLB
        jsr n_isz
        bne @g
        lda #1
        sta CLERR
        rts
@g:     ldx #CLA
        jsr n_isz
        bne @g2
        ldx #CLR
        jmp n_zero
@g2:    ldx #6                  ; Rest = CLA, Teiler = CLB, Quotient = 0 (je 5 Worte)
@c:     lda CLA+NM_M,x
        sta CLP,x
        lda CLB+NM_M,x
        sta CLVV,x
        stz CLQ,x
        dex
        dex
        bpl @c
        stz CLP+8
        stz CLVV+8
        stz CLQ+8
        lda #17
        sta CLII
@d:     stz CLN
@t:     ldx #8                  ; Rest >= Teiler?
@tc:    lda CLP,x
        cmp CLVV,x
        bne @td
        dex
        dex
        bpl @tc
        bra @ge
@td:    bcc @nx
@ge:    sed
        sec
        lda CLP
        sbc CLVV
        sta CLP
        lda CLP+2
        sbc CLVV+2
        sta CLP+2
        lda CLP+4
        sbc CLVV+4
        sta CLP+4
        lda CLP+6
        sbc CLVV+6
        sta CLP+6
        lda CLP+8
        sbc CLVV+8
        sta CLP+8
        cld
        inc CLN
        bra @t
@nx:    ldy #4                  ; Quotient: Stelle anhaengen
@q:     asl CLQ
        rol CLQ+2
        rol CLQ+4
        rol CLQ+6
        rol CLQ+8
        dey
        bne @q
        lda CLQ
        ora CLN
        sta CLQ
        ldy #4                  ; Rest * 10
@r:     asl CLP
        rol CLP+2
        rol CLP+4
        rol CLP+6
        rol CLP+8
        dey
        bne @r
        dec CLII
        beq *+5
        jmp @d
        lda CLA+NM_E
        sec
        sbc CLB+NM_E
        sta CLR+NM_E
        lda CLA+NM_S
        eor CLB+NM_S
        sta CLR+NM_S
        lda CLQ+8               ; 17. Stelle (vor dem Komma) belegt?
        and #$000F
        beq @m
        ldy #4
@rs:    lsr CLQ+8
        ror CLQ+6
        ror CLQ+4
        ror CLQ+2
        ror CLQ
        dey
        bne @rs
        inc CLR+NM_E
@m:     ldx #6
@mm:    lda CLQ,x
        sta CLR+NM_M,x
        dex
        dex
        bpl @mm
        ldx #CLR
        jmp n_norm

; CLR = Wurzel(CLA) (Newton)
n_sqrt: lda CLA+NM_S
        beq @p
        ldx #CLA
        jsr n_isz
        beq @p
        lda #1
        sta CLERR
        rts
@p:     ldx #CLA
        jsr n_isz
        bne @g
        ldx #CLR
        jmp n_zero
@g:     ldx #CLA                ; Startwert: Mantisse, halber Exponent
        ldy #CLK
        jsr n_cp
        ldx #CLA
        ldy #CLU
        jsr n_cp
        lda CLK+NM_E
        inc a
        cmp #$8000
        ror a
        sta CLK+NM_E
        lda #24
        sta CLJ
@it:    ldx #CLU                ; CLR = a / x
        ldy #CLA
        jsr n_cp
        ldx #CLK
        ldy #CLB
        jsr n_cp
        jsr n_div
        ldx #CLR                ; x = (x + a/x) * 0,5
        ldy #CLA
        jsr n_cp
        ldx #CLK
        ldy #CLB
        jsr n_cp
        jsr n_add
        ldx #CLR
        ldy #CLA
        jsr n_cp
        ldx #cl_half
        ldy #CLB
        jsr n_cp
        jsr n_mul
        ldx #CLR
        ldy #CLK
        jsr n_cp
        dec CLJ
        bne @it
        rts

; CLR = CLA! (ganze Zahl 0..999)
n_fact: lda CLA+NM_S
        beq *+5
        jmp @bad
        ldx #CLA
        jsr n_isz
        bne @g
        ldx #cl_one             ; 0! = 1
        ldy #CLR
        jmp n_cp
@g:     lda CLA+NM_E
        bne *+5
        jmp @bad
        bpl *+5
        jmp @bad
        cmp #4
        bcc *+5
        jmp @bad
        sta CLII
        ldx #CLA
        ldy #CLT
        jsr n_cp
        stz CLFN                 ; Ganzzahl aus den ersten Stellen
@d:     lda CLFN
        asl
        sta CLJ
        asl
        asl
        clc
        adc CLJ
        sta CLFN
        lda CLT+NM_M+6
        xba
        lsr
        lsr
        lsr
        lsr
        and #$000F
        clc
        adc CLFN
        sta CLFN
        ldx #CLT
        jsr m_sl
        dec CLII
        bne @d
        ldx #CLT                ; Rest muss 0 sein
        jsr n_isz
        bne @bad
        ldx #cl_one
        ldy #CLU
        jsr n_cp
        ldx #cl_one
        ldy #CLK
        jsr n_cp
@l:     lda CLFN
        cmp #2
        bcc @e
        ldx #CLK                ; k = k + 1
        ldy #CLA
        jsr n_cp
        ldx #cl_one
        ldy #CLB
        jsr n_cp
        jsr n_add
        ldx #CLR
        ldy #CLK
        jsr n_cp
        ldx #CLU                ; u = u * k
        ldy #CLA
        jsr n_cp
        ldx #CLK
        ldy #CLB
        jsr n_cp
        jsr n_mul
        ldx #CLR
        ldy #CLU
        jsr n_cp
        dec CLFN
        bra @l
@e:     ldx #CLU
        ldy #CLR
        jmp n_cp
@bad:   lda #1
        sta CLERR
        rts

; ================================================================
; Rechner-Logik
; ================================================================
; Eingabe -> CLX (wenn eine laeuft)
cl_take:
        lda CLEN
        bne @g
        rts
@g:     stz CLEN
        ldx #CLX
        jsr n_zero
        stz CLN                 ; Exponent
        stz CLII                 ; Ziffern in der Mantisse
        stz CLJ                 ; Komma gesehen
        stz CLC0                ; fuehrende Nullen vorbei
        ldy #0
@l:     cpy CLEL
        bcs @e
        lda CLEB,y
        and #$00FF
        iny
        cmp #'0'
        bcc @sep
        cmp #'9'+1
        bcs @sep
        and #$000F
        bne @nz
        ldx CLC0
        bne @nz
        ldx CLJ                 ; fuehrende 0
        beq @l
        dec CLN
        bra @l
@nz:    inc CLC0
        ldx CLII
        cpx #16
        bcs @ni
        phy
        pha
        ldx #CLX
        jsr m_sl
        pla
        ora CLX+NM_M
        sta CLX+NM_M
        ply
        inc CLII
@ni:    ldx CLJ
        bne @l
        inc CLN
        bra @l
@sep:   inc CLJ
        bra @l
@e:     lda CLII
        beq @z
@sl:    lda CLII                 ; nach vorn schieben
        cmp #16
        bcs @x
        phy
        ldx #CLX
        jsr m_sl
        ply
        inc CLII
        bra @sl
@x:     lda CLN
        sta CLX+NM_E
        lda CLEG
        beq @z
        lda #$8000
        sta CLX+NM_S
@z:     lda #1
        sta CLHX
        rts

; anstehende Rechenart mit CLACC und CLX -> CLX
cl_doop:
        lda CLOP
        bne @g
        rts
@g:     ldx #CLACC
        ldy #CLA
        jsr n_cp
        ldx #CLX
        ldy #CLB
        jsr n_cp
        lda CLOP
        cmp #'+'
        bne @1
        jsr n_add
        bra @e
@1:     cmp #'-'
        bne @2
        jsr n_sub
        bra @e
@2:     cmp #'*'
        bne @3
        jsr n_mul
        bra @e
@3:     jsr n_div
@e:     jsr n_chk
        ldx #CLR
        ldy #CLX
        jmp n_cp

; alles loeschen
cl_clear:
        ldx #CLX
        jsr n_zero
        ldx #CLACC
        jsr n_zero
        stz CLOP
        stz CLHX
        stz CLERR
        stz CLEN
        rts

; Taste A (Zeichen oder Befehl) ausfuehren
; Befehle: 'c' C, 'e' CE, 8 Zurueck, '%', '!', 'p' Pi, 'w' Wurzel, 'q' x2,
; 'r' 1/x, 'n' Vorzeichen, 'M' MC, 'R' MR, 'P' M+, 'S' M-, '=', Ziffern, ','
cl_key: sta CLKY
        stz CLCL
        cmp #'c'
        bne @nc
        inc CLCL
        jmp cl_clear
@nc:    ldx CLERR               ; nach einem Fehler: nur C, Ziffern beginnen neu
        beq @ok
        jsr cl_clear
        lda CLKY
        cmp #'0'
        bcc @rr
        cmp #'9'+1
        bcs @rr
@ok:    lda CLKY
        cmp #'0'
        bcc @n0
        cmp #'9'+1
        bcs @n0
        jmp cl_digit
@n0:    cmp #','
        bne @n1
        jmp cl_comma
@n1:    cmp #8
        bne @n2
        jmp cl_back
@n2:    cmp #'e'                ; CE
        bne @n3
        stz CLEN
        ldx #CLX
        jsr n_zero
@rr:    rts
@n3:    cmp #'+'
        beq @op
        cmp #'-'
        beq @op
        cmp #'*'
        beq @op
        cmp #'/'
        bne @n4
@op:    jsr cl_take
        lda CLOP
        beq @o1
        lda CLHX
        beq @o1
        jsr cl_doop
@o1:    ldx #CLX
        ldy #CLACC
        jsr n_cp
        lda CLKY
        sta CLOP
        stz CLHX
        rts
@n4:    cmp #'='
        bne @n5
        jsr cl_take
        jsr cl_doop
        stz CLOP
        stz CLHX
        rts
@n5:    cmp #'n'                ; Vorzeichen (auch waehrend der Eingabe)
        bne @n6
        lda CLEN
        beq @ns
        lda CLEG
        eor #1
        sta CLEG
        rts
@ns:    ldx #CLX
        jsr n_isz
        beq @r
        lda CLX+NM_S
        eor #$8000
        sta CLX+NM_S
        lda #1
        sta CLHX
@r:     rts
@n6:    cmp #'M'                ; Speicher
        bne @n7
        ldx #CLMEM
        jsr n_zero
        stz CLHM
        rts
@n7:    cmp #'R'
        bne @n8
        stz CLEN
        ldx #CLMEM
        ldy #CLX
        jsr n_cp
        lda #1
        sta CLHX
        rts
@n8:    cmp #'P'
        beq @mp
        cmp #'S'
        bne @n9
@mp:    jsr cl_take
        ldx #CLMEM
        ldy #CLA
        jsr n_cp
        ldx #CLX
        ldy #CLB
        jsr n_cp
        lda CLKY
        cmp #'P'
        bne @mm
        jsr n_add
        bra @me
@mm:    jsr n_sub
@me:    jsr n_chk
        lda CLERR
        bne @r
        ldx #CLR
        ldy #CLMEM
        jsr n_cp
        stz CLHM
        ldx #CLMEM
        jsr n_isz
        beq @m0
        lda #1
        sta CLHM
@m0:    stz CLHX
        rts
@n9:    cmp #'p'                ; Pi
        bne @n10
        stz CLEN
        ldx #cl_pi
        ldy #CLX
        jsr n_cp
        lda #1
        sta CLHX
        rts
@n10:   cmp #'%'                ; Prozent: a + b% = a + a*b/100
        bne @n11
        jsr cl_take
        ldx #CLX
        ldy #CLA
        jsr n_cp
        ldx #cl_hund
        ldy #CLB
        jsr n_cp
        jsr n_div
        lda CLOP
        beq @pe
        ldx #CLR
        ldy #CLA
        jsr n_cp
        ldx #CLACC
        ldy #CLB
        jsr n_cp
        jsr n_mul
@pe:    bra cl_res
@n11:   pha                     ; einstellige Rechenarten auf CLX
        jsr cl_take
        ldx #CLX
        ldy #CLA
        jsr n_cp
        ldx #CLX
        ldy #CLB
        jsr n_cp
        pla
        cmp #'w'
        bne @u1
        jsr n_sqrt
        bra cl_res
@u1:    cmp #'q'
        bne @u2
        jsr n_mul
        bra cl_res
@u2:    cmp #'r'
        bne @u3
        ldx #cl_one
        ldy #CLA
        jsr n_cp
        jsr n_div
        bra cl_res
@u3:    cmp #'!'
        bne @u4
        jsr n_fact
        bra cl_res
@u4:    rts
cl_res: jsr n_chk
        lda CLERR
        bne @r
        ldx #CLR
        ldy #CLX
        jsr n_cp
        lda #1
        sta CLHX
@r:     rts

cl_digit:
        lda CLEN
        bne @g
        jsr cl_new
@g:     lda CLED
        cmp #16
        bcs @r
        lda CLEL                ; nur "0": ersetzen
        cmp #1
        bne @a
        lda CLEB
        and #$00FF
        cmp #'0'
        bne @a
        stz CLEL
        stz CLED
@a:     lda CLKY
        ldx CLEL
        sep #$20
        sta CLEB,x
        rep #$20
        inc CLEL
        inc CLED
@r:     rts
cl_comma:
        lda CLEN
        bne @g
        jsr cl_new
@g:     lda CLES
        bne @r
        inc CLES
        lda CLEL
        bne @a
        lda #'0'
        sep #$20
        sta CLEB
        rep #$20
        inc CLEL
        inc CLED
@a:     ldx CLEL
        lda #','
        sep #$20
        sta CLEB,x
        rep #$20
        inc CLEL
@r:     rts
cl_new: lda #1
        sta CLEN
        sta CLHX
        stz CLEL
        stz CLED
        stz CLES
        stz CLEG
        rts
cl_back:
        lda CLEN
        beq @r
        lda CLEL
        beq @r
        dec CLEL
        ldx CLEL
        lda CLEB,x
        and #$00FF
        cmp #','
        bne @d
        stz CLES
        rts
@d:     dec CLED
@r:     rts

; Anzeige nach CLDSP (Laengenbyte + Text)
cl_fmt: stz CLDSP
        lda CLERR
        beq @n1
        ldx #cs_err
        lda LANG
        beq @x
        ldx #cs_err_en
@x:     stx CLII
        lda a:0,x
        and #$00FF
        sta CLJ
        ldy #0
@el:    cpy CLJ
        bcs @er0
        tya
        clc
        adc CLII
        tax
        lda a:1,x
        and #$00FF
        jsr cl_out
        iny
        bra @el
@er0:   rts
@n1:    lda CLEN
        beq @n2
        lda CLEG
        beq @e0
        lda #'-'
        jsr cl_out
@e0:    ldy #0
@e1:    cpy CLEL
        bcs @er
        lda CLEB,y
        and #$00FF
        cmp #','
        bne @e2
        jsr cl_sep
@e2:    jsr cl_out
        iny
        bra @e1
@er:    rts
@n2:    ldx #CLX
        jsr n_isz
        bne @n3
        lda #'0'
        jmp cl_out
@n3:    ldx #CLX                ; runden auf 12 Stellen
        ldy #CLT
        jsr n_cp
        sed
        clc
        lda CLT+NM_M
        adc #$5000
        sta CLT+NM_M
        lda CLT+NM_M+2
        adc #0
        sta CLT+NM_M+2
        lda CLT+NM_M+4
        adc #0
        sta CLT+NM_M+4
        lda CLT+NM_M+6
        adc #0
        sta CLT+NM_M+6
        cld
        bcc @ru
        lda #$1000
        sta CLT+NM_M+6
        stz CLT+NM_M+4
        stz CLT+NM_M+2
        inc CLT+NM_E
@ru:    stz CLT+NM_M
        ldx #0                  ; 12 Ziffern
@dg:    lda CLT+NM_M+6
        xba
        lsr
        lsr
        lsr
        lsr
        and #$000F
        sep #$20
        sta CLDG,x
        rep #$20
        phx
        ldx #CLT
        jsr m_sl
        plx
        inx
        cpx #12
        bcc @dg
        ldx #12                 ; Nullen hinten weg
@tz:    lda CLDG-1,x
        and #$00FF
        bne @tze
        dex
        bne @tz
@tze:   stx CLND
        lda CLX+NM_S
        beq @ps
        lda #'-'
        jsr cl_out
@ps:    lda CLT+NM_E            ; 1..12: normal
        bmi @sm
        beq @sm
        cmp #13
        bcs @sci
        sta CLII
        ldx #0
@ip:    cpx CLII                 ; Stellen vor dem Komma
        bcs @ipe
        jsr cl_dig
        inx
        bra @ip
@ipe:   cpx CLND
        bcs @r
        jsr cl_sep
        jsr cl_out
@fp:    cpx CLND
        bcs @r
        jsr cl_dig
        inx
        bra @fp
@r:     rts
@sm:    cmp #0                  ; 0,000..
        beq @sz
        cmp #$FFFB
        bcc @sci
@sz:
        eor #$FFFF
        inc a
        sta CLII
        lda #'0'
        jsr cl_out
        jsr cl_sep
        jsr cl_out
@z0:    lda CLII
        beq @zd
        lda #'0'
        jsr cl_out
        dec CLII
        bra @z0
@zd:    ldx #0
        bra @fp
@sci:   ldx #0                  ; 1,2345e+67
        jsr cl_dig
        inx
        cpx CLND
        bcs @ex
        jsr cl_sep
        jsr cl_out
@sf:    cpx CLND
        bcs @ex
        jsr cl_dig
        inx
        bra @sf
@ex:    lda #'e'
        jsr cl_out
        lda CLT+NM_E
        dec a
        bpl @ep
        eor #$FFFF
        inc a
        pha
        lda #'-'
        bra @es
@ep:    pha
        lda #'+'
@es:    jsr cl_out
        pla
        sta CLII
        stz CLJ
        lda #1000
        jsr @dd
        lda #100
        jsr @dd
        lda #10
        jsr @dd
        lda CLII
        clc
        adc #'0'
        jmp cl_out
@dd:    sta CLN
        ldy #0
        lda CLII
@d1:    cmp CLN
        bcc @d2
        sbc CLN
        iny
        bra @d1
@d2:    sta CLII
        tya
        ora CLJ
        beq @d3
        sty CLJ
        inc CLJ
        tya
        clc
        adc #'0'
        jmp cl_out
@d3:    rts
cl_dig: lda CLDG,x
        and #$00FF
        clc
        adc #'0'
        jmp cl_out
; Komma bzw. Punkt (nach Sprache) -> A
cl_sep: lda LANG
        beq @c
        lda #'.'
        rts
@c:     lda #','
        rts
; Zeichen A an CLDSP
cl_out: phx
        pha
        lda CLDSP
        and #$00FF
        cmp #38
        bcs @f
        inc a
        tax
        pla
        sep #$20
        sta CLDSP,x
        txa
        sta CLDSP
        rep #$20
        plx
        rts
@f:     pla
        plx
        rts

cs_err:    .byte 6,"Fehler"
cs_err_en: .byte 5,"Error"

; ================================================================
; Oberflaeche (Datenbank = Hauptbank)
; ================================================================
        .pushseg
        .segment "BSS"
CAW:    .res 2
CAW2:   .res 2
CAW3:   .res 2
        .popseg

ca_find:
        stz CAW
@l:     lda CAW
        cmp NWIN
        bcs @no
        CALL recaddr
        sta PB
        lda (PB)
        cmp #CAKIND
        beq @y
        inc CAW
        bra @l
@y:     lda CAW
        sec
        rts
@no:    clc
        rts

ca_open:
        jsr ca_find
        bcc @new
        pha
        CALL win_unmin
        pla
        CALL win_front
        CALL redraw_all
        rts
@new:   lda #CAKIND
        CALL win_open
        bcc @o
        lda #8                  ; zu viele Fenster
        CALL show_msg_nr
        CALL redraw_all
        rts
@o:     LG cl_clear
        lda NWIN
        dec a
        CALL recaddr
        sta PB
        ldy #R_H
        lda #CA_H
        sta (PB),y
        ldy #R_Y
        lda #WY-8
        sta (PB),y
        lda NWIN
        dec a
        CALL win_load
        CALL redraw_all
        rts

; Tasten: Beschriftung (diese Bank), Befehl, Farbe
ca_lab: .addr cb_bs, cb_ce, cb_c, cb_pc, cb_fa, cb_pi
        .addr cb_mc, cb_7, cb_8, cb_9, cb_dv, cb_sq
        .addr cb_mr, cb_4, cb_5, cb_6, cb_ml, cb_x2
        .addr cb_mp, cb_1, cb_2, cb_3, cb_mi, cb_rc
        .addr cb_ms, cb_0, cb_pm, cb_cm, cb_pl, cb_eq
ca_cmd: .byte 8,'e','c','%','!','p'
        .byte 'M','7','8','9','/','w'
        .byte 'R','4','5','6','*','q'
        .byte 'P','1','2','3','-','r'
        .byte 'S','0','n',',','+','='
ca_col: .byte C_RED,C_RED,C_RED,NAVY,NAVY,NAVY
        .byte C_RED,C_BLUE,C_BLUE,C_BLUE,C_RED,NAVY
        .byte C_RED,C_BLUE,C_BLUE,C_BLUE,C_RED,NAVY
        .byte C_RED,C_BLUE,C_BLUE,C_BLUE,C_RED,NAVY
        .byte C_RED,C_BLUE,C_BLUE,C_BLUE,C_RED,C_RED
cb_bs:  .byte 2,"<-"
cb_ce:  .byte 2,"CE"
cb_c:   .byte 1,"C"
cb_pc:  .byte 1,"%"
cb_fa:  .byte 2,"n!"
cb_pi:  .byte 2,"pi"
cb_mc:  .byte 2,"MC"
cb_mr:  .byte 2,"MR"
cb_mp:  .byte 2,"M+"
cb_ms:  .byte 2,"M-"
cb_0:   .byte 1,"0"
cb_1:   .byte 1,"1"
cb_2:   .byte 1,"2"
cb_3:   .byte 1,"3"
cb_4:   .byte 1,"4"
cb_5:   .byte 1,"5"
cb_6:   .byte 1,"6"
cb_7:   .byte 1,"7"
cb_8:   .byte 1,"8"
cb_9:   .byte 1,"9"
cb_dv:  .byte 1,"/"
cb_ml:  .byte 1,"*"
cb_mi:  .byte 1,"-"
cb_pl:  .byte 1,"+"
cb_eq:  .byte 1,"="
cb_sq:  .byte 0                 ; Wurzelzeichen wird gezeichnet
cb_x2:  .byte 1,"x"
cb_rc:  .byte 3,"1/x"
cb_pm:  .byte 3,"+/-"
cb_cm:  .byte 1,","
cb_pt:  .byte 1,"."
cb_m:   .byte 1,"M"

; Taste A (0..29) -> RX/RY/RW/RH
ca_brect:
        ldx #6
        CALL gdiv               ; A = Zeile, GMR = Spalte
        pha
        lda GMR
        sta CAW3
        pla
        ldx #CA_SY
        CALL gmul
        clc
        adc #CA_BY
        sta RY
        lda CAW3
        ldx #CA_SX
        CALL gmul
        clc
        adc #CA_BX
        sta RX
        lda #CA_BW
        sta RW
        lda #CA_BH
        sta RH
        rts

; Byte X aus Tabelle der Bank (Datenbank = Hauptbank)
ca_tb:  phb
        phk
        plb
        lda a:0,x
        plb
        and #$00FF
        rts

ca_draw:
        LG cl_fmt
        lda #CA_DX              ; Anzeige: weiss, vertieft
        sta RX
        lda #CA_DY
        sta RY
        lda #CA_DW
        sta RW
        lda #CA_DH
        sta RH
        lda #WHITE
        jsr cd_sunk
        lda #BLACK
        CALL setcolor
        lda #CLDSP
        ldx #CA_DX+CA_DW-8
        ldy #CA_DY+7
        jsr cd_rstr
        ldx #CLHM               ; Speicher belegt: M
        jsr cdg
        beq @nm
        lda #CA_DX+5
        sta SX
        lda #CA_DY+7
        sta SY
        lda #cb_m
        jsr cd_str
@nm:    stz CAW
@k:     lda CAW
        jsr ca_brect
        lda #0
        jsr cd_btn
        lda CAW
        clc
        adc #ca_col
        tax
        jsr ca_tb
        CALL setcolor
        lda CAW
        asl
        clc
        adc #ca_lab
        tax
        phb
        phk
        plb
        lda a:0,x
        plb
        cmp #cb_cm              ; Dezimaltrenner nach Sprache
        bne @l
        ldx LANG
        beq @l
        lda #cb_pt
@l:     sta CAW2
        jsr cd_width
        sta SX
        lda CAW
        jsr ca_brect
        lda #CA_BW
        sec
        sbc SX
        lsr
        clc
        adc RX
        sta SX
        lda RY
        clc
        adc #5
        sta SY
        lda CAW2
        jsr cd_str
        inc CAW
        lda CAW
        cmp #30
        bcc @k
        ; Wurzelzeichen (Taste 11) und hochgestellte 2 (Taste 17)
        lda #NAVY
        CALL setcolor
        ldx #0
@w:     phx
        lda ca_sqv,x
        and #$00FF
        clc
        adc #CA_BX+5*CA_SX+10
        sta RX
        lda ca_sqv+1,x
        and #$00FF
        clc
        adc #CA_BY+CA_SY+4
        sta RY
        lda ca_sqv+2,x
        and #$00FF
        sta RW
        lda ca_sqv+3,x
        and #$00FF
        sta RH
        CALL fillrect
        plx
        inx
        inx
        inx
        inx
        cpx #ca_sqve-ca_sqv
        bcc @w
        lda #CA_BX+5*CA_SX+16   ; x unter dem Wurzelstrich
        sta SX
        lda #CA_BY+CA_SY+5
        sta SY
        lda #'x'
        CALL putch
        lda #CA_BX+5*CA_SX+20
        sta SX
        lda #CA_BY+2*CA_SY+2
        sta SY
        lda #'2'
        CALL putch
        rts
; Wurzelzeichen: x, y, Breite, Hoehe (Tabelle wird mit DB = Hauptbank gelesen: liegt in RODATA)
        .pushseg
        .segment "RODATA"
ca_sqv: .byte 0,5,1,1, 1,6,1,2, 2,8,1,2, 3,6,1,2, 3,4,1,2, 4,2,1,2, 4,0,1,2, 5,0,10,1
ca_sqve:
        .popseg

; Klick: Taste suchen
ca_click:
        stz CAW
@k:     lda CAW
        jsr ca_brect
        CALL inrect
        bcs @hit
        inc CAW
        lda CAW
        cmp #30
        bcc @k
        rts
@hit:   lda CAW                 ; kurz gedrueckt zeigen
        jsr ca_brect
        lda #1
        jsr cd_btn
        lda CAW
        clc
        adc #ca_cmd
        tax
        jsr ca_tb
        jmp ca_do

; Taste der Tastatur
ca_key: lda KEYC
        and #$00FF
        cmp #$1B                ; Esc: C, zweimal Esc: Fenster zu
        bne @k1
        ldx #CLCL
        jsr cdg
        beq @c
        CALL close_win
        rts
@c:     lda #'c'
        bra ca_do
@k1:    cmp #$0D
        bne @k2
        lda #'='
        bra ca_do
@k2:    cmp #$7F                ; Delete: Zurueck
        beq @bs
        cmp #$08
        bne @k3
@bs:    lda #8
        bra ca_do
@k3:    cmp #'.'
        bne @k4
        lda #','
        bra ca_do
@k4:    cmp #':'
        bne @k5
        lda #'/'
        bra ca_do
@k5:    cmp #'0'                ; Ziffern, Rechenzeichen
        bcc @s
        cmp #'9'+1
        bcc ca_do
@s:     sta CAW2
        ldx #0
@sl:    lda ca_kc,x
        and #$00FF
        cmp CAW2
        beq @sk
        inx
        cpx #ca_kce-ca_kc
        bcc @sl
        lda CAW2                ; Buchstaben
        ora #$20
        sta CAW2
        ldx #0
@ll:    lda ca_kl,x
        and #$00FF
        cmp CAW2
        beq @sk
        inx
        cpx #ca_kle-ca_kl
        bcc @ll
        rts
@sk:    lda CAW2
        bra ca_do

        .pushseg
        .segment "RODATA"
ca_kc:  .byte "+-*/=%!,"
ca_kce:
ca_kl:  .byte "wqrpnc"
ca_kle:
        .popseg

; Befehl A ausfuehren, Anzeige neu
ca_do:  LG cl_key
        CALL redraw_win
        rts
