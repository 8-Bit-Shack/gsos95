; ================================================================
; GS/OS95 Browser: HTTPS ueber einen Proxy (Teil von web.s)
; Fuer die Uthernet II (kein TLS auf der Karte): https://-Seiten werden als
; "GET https://rechner/pfad" an einen Proxy im Heimnetz geschickt, der die
; Verschluesselung uebernimmt (z.B. WebOne, Macproxy). Einstellung:
; about:proxy=192.168.1.10:8080 (leer: about:proxy=)
; ================================================================
        .pushseg
        .segment "BSS"
WPROXY: .res 2                  ; 1 = Anfrage geht an den Proxy
PXPORT: .res 2
PXI:    .res 2
PXN:    .res 2
        .popseg
PXSAVE  = WB_CTYP               ; Rechnername waehrend des Verbindens

; Proxy eintragen? Dann WB_HOST sichern und durch den Proxy ersetzen. C=1: keiner
px_setup:
        lda PROXP
        and #$00FF
        bne *+5
        jmp @no
        ldy #WB_HOST
        BRD
        cmp #63
        bcc *+5
        jmp @no
        ldx #WB_HOST
        ldy #PXSAVE
        jsr bs_copy
        ldy #WB_HOST            ; Rechner bis ':'
        lda #0
        BWR
        lda #8080
        sta PXPORT
        lda PROXP
        and #$00FF
        sta PXN
        ldx #0
@h:     cpx PXN
        bcs @ok
        lda PROXP+1,x
        and #$00FF
        cmp #':'
        beq @port
        phx
        ldy #WB_HOST
        pha
        BRD
        inc a
        BWR
        clc
        adc #WB_HOST
        tay
        pla
        BWR
        plx
        inx
        bra @h
@port:  stz PXPORT
@p:     inx
        cpx PXN
        bcs @ok
        lda PROXP+1,x
        and #$00FF
        sec
        sbc #'0'
        cmp #10
        bcs @ok
        sta PXI
        lda PXPORT
        asl
        asl
        clc
        adc PXPORT
        asl
        clc
        adc PXI
        sta PXPORT
        bra @p
@ok:    lda #1
        sta WPROXY
        clc
        rts
@no:    sec
        rts

px_restore:
        jsr net_ptr0
        ldx #PXSAVE
        ldy #WB_HOST
        jmp bs_copy

; about:proxy=... -> PROXP
px_set: ldy #WB_TURL
        BRD
        sec
        sbc #12                 ; "about:proxy=" hat 12 Zeichen
        bcs *+5
        lda #0
        cmp #22
        bcc @n
        lda #21
@n:     sta PXN
        sep #$20
        sta PROXP
        rep #$20
        ldx #0
@c:     cpx PXN
        bcs @r
        txa
        clc
        adc #WB_TURL+13
        tay
        BRD
        sep #$20
        sta PROXP+1,x
        rep #$20
        inx
        bra @c
@r:     rts
