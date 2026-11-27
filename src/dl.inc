; ================================================================
; GS/OS95 Browser: Herunterladen (Teil von web.s)
; Antworten, die keine Textseite sind, landen in *:Downloads:NAME.
; Der Puffer (32 KB) wird blockweise geschrieben, waehrend die Karte weiter
; empfaengt (NSTRM/NFULL in net.s), daher sind auch grosse Dateien moeglich.
; ================================================================
        .pushseg
        .segment "BSS"
DLON:   .res 2                  ; 1 = Herunterladen laeuft (Fortschritt in KB)
DLTOT:  .res 4                  ; Bytes insgesamt
DLREF:  .res 2
DLII:    .res 2
DLNN:    .res 2
DLERR:  .res 2
DLPATH: .res 2+12+16            ; GS/OS-Pfad "*:Downloads:NAME"
        .popseg
        .pushseg
        .segment "RODATA"
dl_dir:  .word 11
         .byte "*:Downloads"
dl_ext:  ; Endung (4 Bytes, klein), Typ, Hilfstyp
         .byte ".txt"
         .word $04, $0000
         .byte ".shk"
         .word $E0, $8002
         .byte ".sdk"
         .word $E0, $8002
         .byte ".bxy"
         .word $E0, $8000
         .byte ".bny"
         .word $E0, $8000
         .byte ".gif"
         .word $C0, $8006
         .byte ".2mg"
         .word $E0, $0130
         .byte ".zip"
         .word $E0, $8008
         .byte ".s16"
         .word $B3, $0000
         .byte ".sys"
         .word $FF, $0000
         .byte 0
        .popseg
        .pushseg
        .segment "BSS"
dl_cdir: .res 2+4+2+2+4+2       ; Create: pCount 5, Pfad, Zugriff, Typ, Hilfstyp, Art
dl_cre:  .res 2+4+2+2+4         ; Create: pCount 4
dl_des:  .res 2+4               ; Destroy
dl_opn:  .res 2+2+4+2           ; Open: pCount 3, Ref, Pfad, Zugriff
dl_wr:   .res 2+2+4+4+4         ; Write
dl_cls:  .res 2+2
        .popseg

; Dateiname aus WB_PATH (letzter Teil, ohne ?...) nach DLPATH
dl_name:
        ldy #WB_PATH            ; Anfang des letzten Teils suchen
        BRD
        sta DLNN
        lda #WB_PATH+1
        sta DLII
        ldy #WB_PATH+1
        ldx DLNN
@f:     cpx #0
        beq @fe
        BRD
        cmp #'?'
        beq @fe
        cmp #'#'
        beq @fe
        iny
        cmp #'/'
        bne @f1
        sty DLII
@f1:    dex
        bra @f
@fe:    sty DLNN                 ; Ende
        lda DLII                ; Name (%XX, Endung, 15 Zeichen) in XCODE3
        sta LP2
        lda NWB
        sta LP2+2
        lda DLNN
        sec
        sbc DLII
        sta FNLEN
        JSLF xf_fname
        jmp net_ptr0

; Typ nach Endung -> dl_cre
dl_type:
        lda #$06                ; Vorgabe: BIN
        sta dl_cre+8
        stz dl_cre+10
        stz dl_cre+12
        lda DLNN
        cmp #5
        bcc @r
        clc                     ; letzte 4 Zeichen, klein
        adc #DLPATH+14-4
        tax
        lda a:0,x
        ora #$2020
        sta DLII
        lda a:2,x
        ora #$2020
        and #$FFFF
        sta DLERR
        ldx #0
@l:     lda dl_ext,x
        and #$00FF
        beq @r
        lda dl_ext,x
        ora #$2000
        cmp DLII
        bne @nx
        lda dl_ext+2,x
        cmp DLERR
        bne @nx
        lda dl_ext+4,x
        sta dl_cre+8
        lda dl_ext+6,x
        sta dl_cre+10
@r:     rts
@nx:    txa
        clc
        adc #8
        tax
        bra @l

; Datei anlegen und oeffnen. C=1 bei Fehler
dl_open:
        lda MYBANK
        sta dl_cdir+4
        sta dl_cre+4
        sta dl_des+4
        sta dl_opn+6
        lda #dl_dir             ; Ordner (Fehler, wenn es ihn gibt: egal)
        sta dl_cdir+2
        lda #5
        sta dl_cdir
        lda #$C3
        sta dl_cdir+6
        lda #$0F
        sta dl_cdir+8
        stz dl_cdir+10
        stz dl_cdir+12
        lda #$0D
        sta dl_cdir+14
        GSOS $2001,dl_cdir
        lda #DLPATH
        sta dl_des+2
        sta dl_cre+2
        sta dl_opn+4
        lda #1
        sta dl_des
        GSOS $2002,dl_des       ; alte Datei gleichen Namens ersetzen
        lda #4
        sta dl_cre
        lda #$C3
        sta dl_cre+6
        jsr dl_type
        GSOS $2001,dl_cre
        bcs @e
        lda #3
        sta dl_opn
        lda #2                  ; schreiben
        sta dl_opn+8
        GSOS $2010,dl_opn
        bcs @e
        lda dl_opn+2
        sta DLREF
        clc
        rts
@e:     sec
        rts

; Puffer A..WPTR schreiben, DLTOT erhoehen. C=1 bei Fehler
dl_write:
        sta dl_wr+4
        lda WPTR
        sec
        sbc dl_wr+4
        sta dl_wr+8
        beq @r
        clc
        adc DLTOT
        sta DLTOT
        bcc *+5
        inc DLTOT+2
        lda #4
        sta dl_wr
        lda DLREF
        sta dl_wr+2
        lda NWB
        sta dl_wr+6
        stz dl_wr+10
        GSOS $2013,dl_wr
        rts
@r:     clc
        rts

dl_close:
        lda #1
        sta dl_cls
        lda DLREF
        sta dl_cls+2
        GSOS $2014,dl_cls
        rts

; Herunterladen: Antwortkopf ist gelesen, Verbindung offen, wenn NFULL
dl_run: jsr net_ptr0
        jsr dl_name
        jsr dl_open
        bcc @o
        JSLF xf_tcp_close
        WS ws_dlerr
        jmp err_page
@o:     stz DLTOT
        stz DLTOT+2
        lda #1
        sta DLON
        lda HEND
        jsr dl_write
        bcs @we
@more:  lda NFULL
        beq @fin
        stz WPTR
        lda #$8000
        sta NLIM
        inc NSTRM
        JSLF xf_tcp_readall
        stz NSTRM
        bcs @ne
        lda #0
        jsr dl_write
        bcc @more
@we:    JSLF xf_tcp_close
        jsr dl_close
        stz DLON
        WS ws_dlerr
        jmp err_page
@ne:    JSLF xf_tcp_close
        jsr dl_close
        stz DLON
        jmp net_err
@fin:   JSLF xf_tcp_close
        jsr dl_close
        stz DLON
        ; Ergebnisseite
        jsr net_ptr0
        stz HO
        lda #w_fv1
        jsr raw_m
        WS ws_dlok
        jsr raw_x
        lda #w_fv2
        jsr raw_m
        WS ws_dlok
        jsr raw_x
        lda #w_dl1              ; </h1><p>
        jsr raw_m
        lda #DLPATH+2           ; Pfad (Wortlaenge davor)
        ldx DLPATH
        jsr raw_n
        lda #w_dl2              ; <br>
        jsr raw_m
        lda DLTOT               ; Groesse in KB (aufgerundet)
        clc
        adc #1023
        sta DLII
        lda DLTOT+2
        adc #0
        xba
        and #$FF00
        sta DLERR
        lda DLII
        xba
        and #$00FF
        ora DLERR
        lsr
        lsr
        jsr raw_dec
        WS ws_dlkb
        jsr raw_x
        lda #w_dl3
        jsr raw_m
        WS ws_dlhint
        jsr raw_x
        lda HO
        sta HE
        stz HR
        jsr h2t
        jmp page_done

; X Zeichen ab A (Hauptbank) in den Rohpuffer
raw_n:  sta DLII
        stx DLNN
@l:     lda DLNN
        beq @r
        ldx DLII
        lda a:0,x
        and #$00FF
        jsr raw_c
        inc DLII
        dec DLNN
        bra @l
@r:     rts
; Zahl A (0..65535) dezimal ohne fuehrende Nullen
raw_dec:
        sta DLII
        stz DLERR               ; schon eine Ziffer geschrieben?
        ldx #0
@d:     lda dl_pow,x
        sta DLNN
        ldy #0
        lda DLII
@s:     cmp DLNN
        bcc @o
        sbc DLNN
        iny
        bra @s
@o:     sta DLII
        tya
        bne @w
        lda DLERR
        bne @w0
        cpx #8
        bne @nx
@w0:    tya
@w:     clc
        adc #'0'
        phx
        jsr raw_c
        plx
        inc DLERR
@nx:    inx
        inx
        cpx #10
        bcc @d
        rts
        .pushseg
        .segment "RODATA"
dl_pow: .word 10000, 1000, 100, 10, 1
w_dl1:  .byte w_dl1e-w_dl1-1,"</h1><p>"
w_dl1e:
w_dl2:  .byte w_dl2e-w_dl2-1,"<br>"
w_dl2e:
w_dl3:  .byte w_dl3e-w_dl3-1,"</p><p>"
w_dl3e:
        .popseg
