; ================================================================
; GS/OS95: CD-Wiedergabe (Fensterart 23), Teil von net.s (XCODE3)
; Audio-CDs ueber den SCSI-CD-Treiber (SCSICD.DRIVER, DStatus/DControl
; mit SCSI-Befehlen $80xx):
;   SCSI-2-Laufwerke (AppleCD 300/600 u.a.): $8043 READ TOC, $8042 READ
;     SUB-CHANNEL, $8047 PLAY AUDIO MSF, $804B PAUSE/RESUME, $801B START/STOP
;   Apple CD SC / CD 150 ("Ruby", Apples eigener Befehlssatz): $80C1 Read TOC,
;     $80CC Audio Status, $80C2 Q-Subcode, $80C8 Track Search + Play,
;     $80CA Pause, $80CB Stop
; Erkannt wird automatisch: erst SCSI-2, sonst Apple. Der Ton kommt aus
; der Kopfhoererbuchse bzw. den Audio-Ausgaengen des Laufwerks.
; Aufbau: Logik (cdl_*) laeuft mit Datenbank = diese Bank (Variablen hier),
; Oberflaeche (cd_*) mit Datenbank = Hauptbank; Uebergang mit LG / cdg.
; ================================================================
CDKIND = 23
CDMAXD = 6                      ; hoechstens so viele CD-Laufwerke
CD_LX  = WX+8                   ; Anzeige
CD_LY  = WY+22
CD_LW  = 224
CD_LH  = 56
CD_BY  = WY+84                  ; Knoepfe
CD_BW  = 32
CD_BH  = 18
CD_GY  = WY+110                 ; Titelraster: 8 Spalten, 4 Zeilen
CD_GX  = WX+12
CD_GW  = 26
CD_GH  = 12
CD_GN  = 32
GREEN  = 10
CD_H   = 176                    ; Fensterhoehe

; Logik-Unterprogramm aufrufen (Datenbank = diese Bank), Carry bleibt
.macro LG r
        phb
        phk
        plb
        .byte $20               ; jsr r
        .addr r
        plb
.endmacro

; ---------- Einspruenge ----------
xf_cd_open:     jsr cd_open
        rtl
xf_cd_auto:     jsr cd_auto
        rtl
xf_cd_tick:     jsr cd_tick
        rtl
xf_vol_bad:     jsr vol_bad
        rtl
; Datentraeger im CD-Laufwerk A? (TEST UNIT READY) -> A = $0010 ja, 0 nein
xf_cd_tur:      phb
        phk
        plb
        jsr cdl_tur
        plb
        rtl

; ================================================================
; Daten (in dieser Codebank)
; ================================================================
cdv:
CDN:    .res 2                  ; Zahl der CD-Laufwerke
CDI:    .res 2                  ; gewaehltes Laufwerk (Index)
CDDV:   .res 2*CDMAXD           ; Geraetenummern
CDMD:   .res 2                  ; 0 unbekannt, 1 Apple (Ruby), 2 SCSI-2
CDOK:   .res 2                  ; 1 = Inhaltsverzeichnis gelesen
CDST:   .res 2                  ; 0 gestoppt, 1 spielt, 2 Pause
CDSTP:  .res 2                  ; 1 = Laufwerk mit STOP UNIT angehalten
CDT0:   .res 2                  ; erster Titel
CDT1:   .res 2                  ; letzter Titel
CDTR:   .res 2                  ; aktueller Titel
CDRM:   .res 2                  ; Zeit im Titel (Minuten, Sekunden)
CDRS:   .res 2
CDTRY:  .res 2
CDGC:   .res 2
CDSC:   .res 2
CDXD:   .res 2                  ; Geraet fuer cdl_tur (sonst 0)
CDK:    .res 2                  ; Zaehler: Inhaltsverzeichnis neu versuchen
CDE:    .res 2                  ; letzter Fehler (GS/OS)
CDF:    .res 2
CDQ0:   .res 2
CDQ1:   .res 2
CDQ5:   .res 2
CDX:    .res 2
CDY:    .res 2
CDA:    .res 2
CDO1:   .res 2                  ; alter Zustand (Vergleich nach Abfrage)
CDO2:   .res 2
CDO3:   .res 2
CDO4:   .res 2
CDO5:   .res 2
TM:     .res 101                ; Beginn je Titel (Index = Titel), Ende = Index CDT1+1
TS:     .res 101
TF:     .res 101
TC:     .res 101                ; Steuerbits (Bit 2: Datentitel)
CDNM:   .res 40                 ; Name des Laufwerks (Laengenbyte + Text)
CDL1:   .res 44                 ; Anzeige: Titel
CDL1R:  .res 24                 ;          Zeit
CDL2R:  .res 24                 ;          Gesamt
CDL2P:  .res 2                  ;          Zustandstext (Adresse)
CDL0P:  .res 2                  ;          Zeile 1: Kuenstler/Album oder Hinweis (Adresse)
CDPB:   .res 18                 ; DStatus/DControl: pcount, dev, code, list, request, transfer
cdlst:  .res 2                  ; Liste: Version 0
CDCB:   .res 12                 ;        Befehl
        .res 4                  ;        Pufferadresse
CDDI:   .res 22                 ; DInfo (pcount 8)
CDNB:   .res 44                 ; Ergebnispuffer Geraetename
CDB:    .res 816                ; Datenpuffer
cdv_end:

; ---------- Texte (Laengenbyte + Text, DE/EN) ----------
cds_nodrv:     .byte cds_nodrv_e-cds_nodrv-1,"Kein CD-ROM-Laufwerk gefunden"
cds_nodrv_e:
cds_nodrv_en:  .byte cds_nodrv_en_e-cds_nodrv_en-1,"No CD-ROM drive found"
cds_nodrv_en_e:
cds_nocd:      .byte cds_nocd_e-cds_nocd-1,"Keine Audio-CD eingelegt"
cds_nocd_e:
cds_nocd_en:   .byte cds_nocd_en_e-cds_nocd_en-1,"No audio CD inserted"
cds_nocd_en_e:
cds_play:      .byte cds_play_e-cds_play-1,"Wiedergabe"
cds_play_e:
cds_play_en:   .byte cds_play_en_e-cds_play_en-1,"Playing"
cds_play_en_e:
cds_pause:     .byte cds_pause_e-cds_pause-1,"Pause"
cds_pause_e:
cds_pause_en:  .byte cds_pause_en_e-cds_pause_en-1,"Paused"
cds_pause_en_e:
cds_stop:      .byte cds_stop_e-cds_stop-1,"Gestoppt"
cds_stop_e:
cds_stop_en:   .byte cds_stop_en_e-cds_stop_en-1,"Stopped"
cds_stop_en_e:
cds_trk:       .byte cds_trk_e-cds_trk-1,"Titel "
cds_trk_e:
cds_trk_en:    .byte cds_trk_en_e-cds_trk_en-1,"Track "
cds_trk_en_e:
cds_tot:       .byte cds_tot_e-cds_tot-1,"Gesamt "
cds_tot_e:
cds_tot_en:    .byte cds_tot_en_e-cds_tot_en-1,"Total "
cds_tot_en_e:
cds_cdrom:     .byte cds_cdrom_e-cds_cdrom-1,"CD-ROM"
cds_cdrom_e:
cds_srch:      .byte cds_srch_e-cds_srch-1,"Titelnamen werden gesucht"
cds_srch_e:
cds_srch_en:   .byte cds_srch_en_e-cds_srch_en-1,"Looking up track names.."
cds_srch_en_e:
cds_nf:        .byte cds_nf_e-cds_nf-1,"Keine Titelnamen gefunden"
cds_nf_e:
cds_nf_en:     .byte cds_nf_en_e-cds_nf_en-1,"No track names found"
cds_nf_en_e:
cds_ne:        .byte cds_ne_e-cds_ne-1,"Titel-Server nicht erreicht"
cds_ne_e:
cds_ne_en:     .byte cds_ne_en_e-cds_ne_en-1,"Could not reach the server"
cds_ne_en_e:
cds_hint:      .byte cds_hint_e-cds_hint-1,"Taste I: Titelnamen (Internet)"
cds_hint_e:
cds_hint_en:   .byte cds_hint_en_e-cds_hint_en-1,"Key I: track names (Internet)"
cds_hint_en_e:

; ================================================================
; Logik (Datenbank = diese Bank)
; ================================================================
; Bank dieser Codebank -> A
cd_bk:  phk
        sep #$20
        pla
        rep #$20
        and #$00FF
        rts

; GS/OS-Aufruf A mit Parameterblock X (diese Bank)
cd_gs:  sta CDGC
        jsr cd_bk
        pha
        phx
        lda CDGC
        pha
        jsl $E100B0
        rts

; Befehlsbereich loeschen
cd_cb0: ldx #10
@l:     stz CDCB,x
        dex
        dex
        bpl @l
        rts

; SCSI-Befehl: Code A ($80xx), X = $202D (DStatus) / $202E (DControl),
; Y = Anzahl Bytes; Befehl in CDCB, Daten in CDB. C=1 Fehler (A, CDE)
cd_scsi:
        sta CDPB+4
        stx CDGC
        sty CDPB+10
        stz CDPB+12
        stz CDPB+14
        stz CDPB+16
        lda #5
        sta CDPB
        lda CDXD                ; fester Geraet (TEST UNIT READY) oder gewaehltes
        bne @dv
        lda CDI
        asl
        tax
        lda CDDV,x
@dv:    sta CDPB+2
        lda #cdlst
        sta CDPB+6
        jsr cd_bk
        sta CDPB+8
        sta CDCB+14
        stz cdlst
        lda #CDB
        sta CDCB+12
        sep #$20
        lda CDPB+4              ; Befehlsbyte = unteres Byte des Codes
        sta CDCB
        rep #$20
        lda CDGC
        sta CDSC
        lda #3
        sta CDTRY
@go:    lda CDSC
        ldx #CDPB
        jsr cd_gs
        bcc @r
        sta CDE
        cmp #$002E              ; Datentraeger gewechselt: noch einmal
        bne @e
        dec CDTRY
        bne @go
@e:     lda CDE
        sec
@r:     rts

; BCD-Byte A -> binaer
cd_bcd: and #$00FF
        pha
        and #$000F
        sta CDX
        pla
        lsr
        lsr
        lsr
        lsr
        sta CDY
        asl
        asl
        adc CDY                 ; *5
        asl                     ; *10
        adc CDX
        rts
; binaer A (0..99) -> BCD
cd_tobcd:
        ldx #0
@l:     cmp #10
        bcc @e
        sbc #10
        inx
        bra @l
@e:     sta CDX
        txa
        asl
        asl
        asl
        asl
        ora CDX
        rts

; ---------- Laufwerke suchen (DInfo, Geraetekennung $0007) ----------
cdl_scan:
        stz CDN
        lda #1
        sta CDA
@l:     jsr cd_dinfo
        bcs @e
        lda CDDI+20
        cmp #$0007
        bne @n
        lda CDN
        cmp #CDMAXD
        bcs @n
        asl
        tax
        lda CDA
        sta CDDV,x
        inc CDN
@n:     inc CDA
        lda CDA
        cmp #64
        bcc @l
@e:     lda CDI
        cmp CDN
        bcc @k
        stz CDI
@k:     jmp cdl_dev

; DInfo fuer Geraet CDA -> CDDI (C=1 kein Geraet)
cd_dinfo:
        lda #8
        sta CDDI
        lda CDA
        sta CDDI+2
        lda #CDNB
        sta CDDI+4
        jsr cd_bk
        sta CDDI+6
        lda #40
        sta CDNB
        stz CDDI+20
        lda #$202C
        ldx #CDDI
        jmp cd_gs

; TEST UNIT READY an Geraet A (Diskettenabfrage): der Treiber meldet ein
; Auswerfen nicht immer, das Laufwerk selbst aber schon -> A = $0010 / 0
cdl_tur:
        sta CDXD
        jsr cd_cb0
        lda #$8000
        ldx #$202D
        ldy #0
        jsr cd_scsi
        stz CDXD
        lda #$0010
        bcc *+5
        lda #0
        rts

; Geraet A ein CD-Laufwerk? C=1 ja
cdl_isdev:
        sta CDA
        jsr cd_dinfo
        bcs @no
        lda CDDI+20
        cmp #$0007
        bne @no
        sec
        rts
@no:    clc
        rts

; Laufwerk mit Geraetenummer A waehlen (falls bekannt)
cdl_seldev:
        sta CDA
        ldx #0
@l:     cpx CDN
        bcs @r
        txa
        asl
        tay
        lda CDDV,y
        cmp CDA
        beq @f
        inx
        bra @l
@f:     stx CDI
        jmp cdl_dev
@r:     rts

; naechstes Laufwerk
cdl_nextdev:
        lda CDN
        cmp #2
        bcc @r
        lda CDI
        inc a
        cmp CDN
        bcc @s
        lda #0
@s:     sta CDI
        jmp cdl_dev
@r:     rts

; gewaehltes Laufwerk: Name (INQUIRY), Inhaltsverzeichnis, Zustand
cdl_dev:
        stz CDOK
        stz CDST
        stz CDMD
        stz CDSTP
        stz CDTR
        stz CDRM
        stz CDRS
        lda #6                  ; Name: "CD-ROM", falls INQUIRY fehlt
        sta CDNM
        ldx #4
@d:     lda cds_cdrom+1,x
        sta CDNM+1,x
        dex
        dex
        bpl @d
        lda CDN
        bne *+3
        rts
        jsr cd_cb0
        lda #36
        sta CDCB+4
        lda #$8012              ; INQUIRY
        ldx #$202D
        ldy #36
        jsr cd_scsi
        bcs @toc
        ; Hersteller (8) + Produkt (16): Leerzeichen am Ende weg, doppelte zusammen
        ldy #0
        ldx #8
@c:     lda CDB,x
        and #$007F
        cmp #' '
        bcs @c2
        lda #' '
@c2:    cmp #' '
        bne @put
        cpy #0
        beq @nx                 ; nicht am Anfang
        lda CDNM,y
        and #$00FF
        cmp #' '
        beq @nx                 ; nicht doppelt
        lda #' '
@put:   sep #$20
        sta CDNM+1,y
        rep #$20
        iny
@nx:    inx
        cpx #32
        bcc @c
@tr:    cpy #0                  ; Leerzeichen am Ende
        beq @tl
        lda CDNM,y
        and #$00FF
        cmp #' '
        bne @tl
        dey
        bra @tr
@tl:    tya
        beq @toc
        sep #$20
        sta CDNM
        rep #$20
@toc:   jsr cdl_toc
        bcs @r
        jmp cdl_stat
@r:     rts

; ---------- Inhaltsverzeichnis: erst SCSI-2, dann Apple ----------
cdl_toc:
        stz CDOK
        lda CDN
        beq @no
        lda CDMD
        cmp #1
        beq @ruby
        jsr cd_toc2
        bcc @ok2
        lda CDMD
        cmp #2
        beq @no                 ; SCSI-2-Laufwerk ohne CD
@ruby:  jsr cd_toc1
        bcs @no
        lda #1
        bra @ok
@ok2:   lda #2
@ok:    sta CDMD
        lda #1
        sta CDOK
        jsr cdl_discid
        lda CDTR                ; aktueller Titel im Bereich?
        cmp CDT0
        bcc @t0
        cmp CDT1
        bcc @r
        beq @r
@t0:    lda CDT0
        sta CDTR
@r:     clc
        rts
@no:    stz CDST
        sec
        rts

; SCSI-2: READ TOC (MSF)
cd_toc2:
        jsr cd_cb0
        lda #$0243              ; $43, MSF
        sta CDCB
        lda #$2403              ; Laenge 804
        sta CDCB+7
        lda #$8043
        ldx #$202D
        ldy #804
        jsr cd_scsi
        bcc *+3
        rts
        lda CDB+2
        and #$00FF
        sta CDT0
        lda CDB+3
        and #$00FF
        sta CDT1
        jsr cd_tchk
        bcc *+3
        rts
        lda CDB                 ; Datenlaenge (MSB zuerst) + 2
        xba
        clc
        adc #2
        cmp #804
        bcc *+5
        lda #804
        sta CDY
        ldy #4
@l:     tya
        clc
        adc #8
        cmp CDY
        beq @g
        bcs @e
@g:     lda CDB+2,y             ; Titel
        and #$00FF
        cmp #$00AA
        bne @t
        lda CDT1                ; Auslauf -> Index letzter + 1
        inc a
@t:     cmp #101
        bcs @nx
        tax
        sep #$20
        lda CDB+1,y
        and #$0F
        sta TC,x
        lda CDB+5,y
        sta TM,x
        lda CDB+6,y
        sta TS,x
        lda CDB+7,y
        sta TF,x
        rep #$20
@nx:    tya
        clc
        adc #8
        tay
        bra @l
@e:     clc
        rts

; Titelbereich pruefen: 1 <= CDT0 <= CDT1 <= 99, C=1 ungueltig
cd_tchk:
        lda CDT0
        beq @bad
        cmp CDT1
        beq @1
        bcs @bad
@1:     lda CDT1
        cmp #100
        bcs @bad
        clc
        rts
@bad:   sec
        rts

; Apple (Ruby): $80C1 Typ 0 (erster/letzter), Typ 1 (Auslauf), Typ 2 (Titel)
cd_toc1:
        jsr cd_cb0              ; Typ 0
        lda #$80C1
        ldx #$202D
        ldy #4
        jsr cd_scsi
        bcc *+3
        rts
        lda CDB
        jsr cd_bcd
        sta CDT0
        lda CDB+1
        jsr cd_bcd
        sta CDT1
        jsr cd_tchk
        bcc *+3
        rts
        jsr cd_cb0              ; Typ 1: Auslauf
        lda #$4000
        sta CDCB+4              ; Byte 5 = %01000000
        lda #$80C1
        ldx #$202D
        ldy #4
        jsr cd_scsi
        bcc *+3
        rts
        ldx CDT1
        inx
        lda CDB
        jsr cd_bcd
        sep #$20
        sta TM,x
        rep #$20
        lda CDB+1
        jsr cd_bcd
        sep #$20
        sta TS,x
        rep #$20
        lda CDB+2
        jsr cd_bcd
        sep #$20
        sta TF,x
        stz TC,x
        rep #$20
        jsr cd_cb0              ; Typ 2: alle Titel ab dem ersten
        lda CDT0
        jsr cd_tobcd
        sep #$20
        sta CDCB+2
        lda #$80
        sta CDCB+5
        rep #$20
        lda CDT1
        sec
        sbc CDT0
        inc a
        asl
        asl
        tay
        lda #$80C1
        ldx #$202D
        jsr cd_scsi
        bcc *+3
        rts
        lda CDT0
        sta CDA
        ldy #0
@l:     ldx CDA
        lda CDB,y
        and #$000F
        sep #$20
        sta TC,x
        rep #$20
        lda CDB+1,y
        jsr cd_bcd
        sep #$20
        sta TM,x
        rep #$20
        lda CDB+2,y
        jsr cd_bcd
        sep #$20
        sta TS,x
        rep #$20
        lda CDB+3,y
        jsr cd_bcd
        sep #$20
        sta TF,x
        rep #$20
        iny
        iny
        iny
        iny
        inc CDA
        lda CDA
        cmp CDT1
        bcc @l
        beq @l
        clc
        rts

; ---------- Zustand abfragen ----------
cdl_stat:
        lda CDOK
        bne *+4
        sec
        rts
        lda CDST
        sta CDQ0
        lda CDMD
        cmp #2
        beq @s2
        ; Apple: $80CC Audio-Status, dann $80C2 Q-Subcode
        jsr cd_cb0
        lda #$80CC
        ldx #$202D
        ldy #6
        jsr cd_scsi
        bcc *+5
        jmp @err
        lda CDB
        and #$00FF
        ldx #1                  ; 0 spielt, 2 stumm: spielt
        cmp #0
        beq @a1
        cmp #2
        beq @a1
        ldx #2                  ; 1 Pause
        cmp #1
        beq @a1
        ldx #0                  ; sonst fertig/Fehler
@a1:    stx CDST
        cpx #0
        bne *+5
        jmp @end
        jsr cd_cb0
        lda #$80C2
        ldx #$202D
        ldy #9
        jsr cd_scsi
        bcc *+5
        jmp @err
        lda CDB+1
        and #$00FF
        cmp #$00AA              ; Auslauf: fertig
        bne *+5
        jmp @end
        jsr cd_bcd
        jsr cd_settr
        lda CDB+3
        jsr cd_bcd
        sta CDRM
        lda CDB+4
        jsr cd_bcd
        sta CDRS
        jmp @fin
@s2:    jsr cd_cb0              ; SCSI-2: READ SUB-CHANNEL, aktuelle Position, MSF
        lda #$0242
        sta CDCB
        lda #$0140              ; Byte 2 SubQ, Byte 3 Format 1
        sta CDCB+2
        lda #$1000              ; Laenge 16
        sta CDCB+7
        lda #$8042
        ldx #$202D
        ldy #16
        jsr cd_scsi
        bcs @err
        lda CDB+1
        and #$00FF
        ldx #1
        cmp #$11                ; spielt
        beq @b1
        ldx #2
        cmp #$12                ; Pause
        beq @b1
        ldx #0
@b1:    stx CDST
        cpx #0
        beq @end
        lda CDB+6
        and #$00FF
        jsr cd_settr
        lda CDB+13
        and #$00FF
        sta CDRM
        lda CDB+14
        and #$00FF
        sta CDRS
@fin:   clc
        rts
@end:   stz CDST                ; nicht (mehr) spielend: Zeit 0,
        stz CDRM                ; nach dem Ende wieder Titel 1
        stz CDRS
        lda CDQ0
        cmp #1
        bne @e2
        lda CDT0
        sta CDTR
@e2:    clc
        rts
@err:   stz CDOK                ; CD entnommen o.ae.
        stz CDST
        sec
        rts

; Titel A uebernehmen, wenn gueltig
cd_settr:
        cmp CDT0
        bcc @r
        cmp CDT1
        beq @s
        bcs @r
@s:     sta CDTR
@r:     rts

; Datentitel? Titel A -> C=1 ja
cd_isdata:
        tax
        lda TC,x
        and #$0004
        cmp #$0004
        rts

; ---------- Wiedergabe ab Titel A ----------
cdl_play:
        sta CDA
        lda CDOK
        bne @k
        jsr cdl_toc             ; vielleicht inzwischen eingelegt
        bcc @k
        rts
@k:     lda CDA                 ; Datentitel ueberspringen
@sk:    cmp CDT1
        beq @ck
        bcs @none
@ck:    pha
        jsr cd_isdata
        pla
        bcc @go
        inc a
        bra @sk
@none:  sec
        rts
@go:    sta CDA
        sta CDTR
        lda CDSTP               ; vorher angehalten: Laufwerk starten
        beq @ns
        jsr cd_cb0
        lda #$0001
        sta CDCB+4
        lda #$801B
        ldx #$202E
        ldy #0
        jsr cd_scsi
        stz CDSTP
@ns:    lda CDMD
        cmp #2
        beq @p2
        jsr cd_cb0              ; Apple: Track Search + Play, Stereo
        lda #$0910              ; Byte 2 Play, Byte 3 Modus 9
        sta CDCB+2
        lda CDA
        jsr cd_tobcd
        sep #$20
        sta CDCB+7
        lda #$80                ; Adresse = Titelnummer
        sta CDCB+8
        rep #$20
        lda #$80C8
        bra @do
@p2:    jsr cd_cb0              ; SCSI-2: PLAY AUDIO MSF bis zum naechsten Datentitel/Auslauf
        ldx CDA
        sep #$20
        lda TM,x
        sta CDCB+3
        lda TS,x
        sta CDCB+4
        lda TF,x
        sta CDCB+5
        rep #$20
        lda CDA
@en:    inc a
        cmp CDT1
        beq @ec
        bcs @ee
@ec:    pha
        jsr cd_isdata
        pla
        bcc @en
@ee:    tax
        sep #$20
        lda TM,x
        sta CDCB+6
        lda TS,x
        sta CDCB+7
        lda TF,x
        sta CDCB+8
        rep #$20
        lda #$8047
@do:    ldx #$202E
        ldy #0
        jsr cd_scsi
        bcs @r
        lda #1
        sta CDST
        stz CDRM
        stz CDRS
        clc
@r:     rts

; Pause <-> weiter; gestoppt: abspielen
cdl_pause:
        lda CDST
        bne @p
        lda CDTR
        bne *+5
        lda CDT0
        jmp cdl_play
@p:     cmp #1
        beq @pa
        ldx #1                  ; weiter
        bra @do
@pa:    ldx #0
@do:    stx CDA
        jsr cd_cb0
        lda CDMD
        cmp #2
        beq @s2
        lda CDA                 ; Apple: Byte 2 = $10 anhalten, 0 weiter
        eor #1
        asl
        asl
        asl
        asl
        sta CDCB+2
        lda #$80CA
        bra @go
@s2:    lda CDA                 ; SCSI-2: Byte 8 Resume-Bit
        sta CDCB+8
        lda #$804B
@go:    ldx #$202E
        ldy #0
        jsr cd_scsi
        bcs @r
        lda CDA
        bne @pl
        lda #2
        sta CDST
        clc
        rts
@pl:    lda #1
        sta CDST
        clc
@r:     rts

; Wiedergabe beenden (Titel bleibt)
cdl_stop:
        lda CDOK
        beq @r
        jsr cd_cb0
        lda CDMD
        cmp #2
        beq @s2
        lda #$80CB              ; Apple: Audio Stop (Adresse 0 = sofort)
        bra @go
@s2:    lda #$801B              ; SCSI-2: STOP UNIT
        inc CDSTP
@go:    ldx #$202E
        ldy #0
        jsr cd_scsi
        stz CDST
        stz CDRM
        stz CDRS
@r:     clc
        rts

; Titel weiter (A=1) / zurueck (A=$FFFF)
cdl_skip:
        sta CDA
        lda CDOK
        beq @r
        lda CDA
        bmi @back
        lda CDTR
        cmp CDT1
        bcs @r
        inc a
        bra @set
@back:  lda CDST                ; laeuft schon > 2 s: Titelanfang
        beq @b2
        lda CDRM
        bne @same
        lda CDRS
        cmp #3
        bcs @same
@b2:    lda CDTR
        cmp CDT0
        beq @same
        bcc @same
        dec a
        bra @set
@same:  lda CDTR
@set:   sta CDTR
        lda CDST
        beq @s0
        lda CDTR
        jmp cdl_play
@s0:    stz CDRM
        stz CDRS
@r:     clc
        rts

; Auswerfen
cdl_eject:
        lda CDN
        bne *+4
        clc
        rts
        jsr cd_cb0
        lda CDMD
        cmp #1
        beq @std
        lda #$0002              ; SCSI-2: START/STOP mit LoEj
        sta CDCB+4
        lda #$801B
        ldx #$202E
        ldy #0
        jsr cd_scsi
        bcc @ok
@std:   lda #5                  ; Apple bzw. sonst: Eject des Treibers
        sta CDPB
        lda CDI
        asl
        tax
        lda CDDV,x
        sta CDPB+2
        lda #2
        sta CDPB+4
        lda #cdlst
        sta CDPB+6
        jsr cd_bk
        sta CDPB+8
        stz CDPB+10
        stz CDPB+12
        lda #$202E
        ldx #CDPB
        jsr cd_gs
@ok:    stz CDOK
        stz CDST
        stz CDSTP
        stz CDMD
        stz CDRM
        stz CDRS
        clc
        rts

; Abfrage im Sekundentakt: C=1 wenn neu zeichnen, A = 1 nur Anzeige, 2 alles
cdl_poll:
        lda CDN
        bne *+4
        clc
        rts
        lda CDOK
        bne @st
        inc CDK                 ; ohne CD: alle 2 s nachsehen
        lda CDK
        and #1
        bne @no
        jsr cdl_toc
        bcs @no
        jsr cdl_stat
        lda #2
        sec
        rts
@no:    clc
        rts
@st:    lda CDST
        sta CDO1
        lda CDTR
        sta CDO2
        lda CDRM
        sta CDO3
        lda CDRS
        sta CDO4
        jsr cdl_stat
        bcc @c
        lda #2                  ; CD weg
        sec
        rts
@c:     lda CDST
        cmp CDO1
        bne @all
        lda CDTR
        cmp CDO2
        bne @all
        lda CDRM
        cmp CDO3
        bne @lcd
        lda CDRS
        cmp CDO4
        bne @lcd
        clc
        rts
@lcd:   lda #1
        sec
        rts
@all:   lda #2
        sec
        rts

; ---------- Anzeige-Texte bauen ----------
cdl_fmt:
        ; Zustandstext
        lda CDN
        bne @d
        lda #cds_nodrv
        ldx #cds_nodrv_en
        bra @s
@d:     lda CDOK
        bne @ok
        lda #cds_nocd
        ldx #cds_nocd_en
        bra @s
@ok:    lda CDST
        cmp #1
        bne @p
        lda #cds_play
        ldx #cds_play_en
        bra @s
@p:     cmp #2
        bne @g
        lda #cds_pause
        ldx #cds_pause_en
        bra @s
@g:     lda #cds_stop
        ldx #cds_stop_en
@s:     ldy LANG
        beq *+3
        txa
        sta CDL2P
        stz CDL1
        stz CDL1R
        stz CDL2R
        stz CDL0P
        lda CDOK
        bne *+3
        rts
        ; Zeile 1: Kuenstler / Album bzw. Hinweis
        lda CDNF
        asl
        tax
        lda cd_nft,x
        ldy LANG
        beq *+5
        lda cd_nfte,x
        sta CDL0P
        ; "Titel 3/12" bzw. "3. Name"
        ldx #CDL1
        lda CDNF
        cmp #2
        bne @nt
        lda CDTR
        asl
        asl
        asl
        asl
        asl
        clc
        adc #CDTT
        sta CDQ5
        tay
        lda a:0,y
        and #$00FF
        beq @nt
        lda CDTR
        jsr cd_appn
        lda #'.'
        jsr cd_appc
        lda #' '
        jsr cd_appc
        lda CDQ5
        jsr cd_apps
        lda CDL1                ; hoechstens 26 Zeichen
        and #$00FF
        cmp #27
        bcc @tm
        sep #$20
        lda #26
        sta CDL1
        rep #$20
        bra @tm
@nt:    lda #cds_trk
        ldy LANG
        beq *+5
        lda #cds_trk_en
        jsr cd_apps
        lda CDTR
        jsr cd_appn
        lda #'/'
        jsr cd_appc
        lda CDT1
        jsr cd_appn
@tm:
        ; "01:23 / 04:56"
        ldx #CDL1R
        lda CDRM
        jsr cd_app2
        lda #':'
        jsr cd_appc
        lda CDRS
        jsr cd_app2
        lda #' '
        jsr cd_appc
        lda #'/'
        jsr cd_appc
        lda #' '
        jsr cd_appc
        lda CDTR
        jsr cd_tlen
        ldx #CDL1R
        jsr cd_appt
        ; "Gesamt 52:13"
        ldx #CDL2R
        lda #cds_tot
        ldy LANG
        beq *+5
        lda #cds_tot_en
        jsr cd_apps
        ldy CDT1                ; Auslauf - erster Titel
        iny
        lda CDT0
        jsr cd_dist
        ldx #CDL2R
        jmp cd_appt

; Laenge von Titel A in Sekunden -> A
cd_nft:  .addr cds_hint, cds_srch, CDART, cds_nf, cds_ne
cd_nfte: .addr cds_hint_en, cds_srch_en, CDART, cds_nf_en, cds_ne_en

cd_tlen:
        tay
        iny
        jmp cd_dist

; Sekunden von Titel-Beginn A bis Titel-Beginn Y -> A
cd_dist:
        tax
        lda TM,x
        and #$00FF
        sta CDO5
        lda TS,x
        and #$00FF
        pha
        lda TF,x
        and #$00FF
        sta CDF
        tyx
        lda TF,x                ; Frames: Uebertrag
        and #$00FF
        cmp CDF
        lda #0
        bcs *+5
        lda #1
        sta CDF
        lda CDO5
        jsr cd_x60
        sta CDO5
        pla
        clc
        adc CDO5
        sta CDO5                ; Sekunden Beginn
        lda TM,x
        and #$00FF
        jsr cd_x60
        sta CDA
        lda TS,x
        and #$00FF
        clc
        adc CDA
        sec
        sbc CDO5
        sec
        sbc CDF
        bpl *+5
        lda #0
        rts
; A * 60
cd_x60: asl
        asl
        sta CDA                 ; *4
        asl
        asl
        asl
        asl                     ; *64
        sec
        sbc CDA
        rts

; Sekunden A als m:ss an Text X anhaengen
cd_appt:
        ldy #0
@d:     cmp #60
        bcc @e
        sbc #60
        iny
        bra @d
@e:     pha
        tya
        jsr cd_app2
        lda #':'
        jsr cd_appc
        pla
        jmp cd_app2

; Zeichen A an Text X anhaengen (X = Adresse, Laengenbyte vorne)
cd_appc:
        sta CDY
        stx CDX
        lda a:0,x
        and #$00FF
        inc a
        sep #$20
        sta a:0,x
        rep #$20
        clc
        adc CDX
        tax
        sep #$20
        lda CDY
        sta a:0,x
        rep #$20
        ldx CDX
        rts
; Zahl A zweistellig
cd_app2:
        ldy #'0'
@l:     cmp #10
        bcc @e
        sbc #10
        iny
        bra @l
@e:     pha
        tya
        jsr cd_appc
        pla
        clc
        adc #'0'
        jmp cd_appc
; Zahl A (0..99) ohne fuehrende Null
cd_appn:
        cmp #10
        bcs cd_app2
        clc
        adc #'0'
        jmp cd_appc
; Text A (Laengenbyte + Text) anhaengen
cd_apps:
        sta CDQ5
        tay
        lda a:0,y
        and #$00FF
        sta CDQ1
        ldy #0
@l:     cpy CDQ1
        bcs @r
        phy
        tya
        clc
        adc CDQ5
        tay
        lda a:1,y
        and #$00FF
        jsr cd_appc
        ply
        iny
        bra @l
@r:     rts

; Index eines Laufwerks/Titel-Bereichs lesen (Oberflaeche): Variable X -> A
cdg:    phb
        phk
        plb
        lda a:0,x
        plb
        ora #0
        rts
cdgb:   jsr cdg
        and #$00FF
        rts
; Wort A in Variable X (diese Bank) schreiben (Oberflaeche)
cdsw:   phb
        phk
        plb
        sta a:0,x
        plb
        rts
; Byte A
cdsb:   phb
        phk
        plb
        sep #$20
        sta a:0,x
        rep #$20
        plb
        rts

        .include "cddb.inc"

; ================================================================
; Oberflaeche (Datenbank = Hauptbank)
; ================================================================
; Fenster der CD-Wiedergabe suchen: C=1, A = Nummer
        .pushseg
        .segment "BSS"
CDW:     .res 2                 ; Oberflaeche (Hauptbank)
CDW2:    .res 2
CDW3:    .res 2
CDW4:    .res 2
CDAD:    .res 2
        .popseg

cd_find:
        stz CDW
@l:     lda CDW
        cmp NWIN
        bcs @no
        CALL recaddr
        sta PB
        lda (PB)
        cmp #CDKIND
        beq @y
        inc CDW
        bra @l
@y:     lda CDW
        sec
        rts
@no:    clc
        rts

cd_open:
        jsr cd_find
        bcc @new
        pha
        CALL win_unmin
        pla
        CALL win_front
        CALL redraw_all
        rts
@new:   LG cdl_scan
        lda #CDKIND
        CALL win_open
        bcc @o
        lda #8                  ; zu viele Fenster
        CALL show_msg_nr
        CALL redraw_all
        rts
@o:     jsr cd_geo
        CALL redraw_all
        jmp cd_auton

; neue Audio-CD in Geraet A (Diskettenabfrage): Fenster zeigen und abspielen
cd_auto:
        sta CDAD
        jsr cd_find
        bcs @h
        lda #CDKIND
        CALL win_open
        bcs @r
        jsr cd_geo
@h:     LG cdl_scan
        lda CDAD
        LG cdl_seldev
        ldx #CDOK
        jsr cdg
        beq @d
        ldx #CDT0
        jsr cdg
        LG cdl_play
@d:     jsr cd_find
        bcc @r
        CALL win_front
@r:     CALL redraw_all
        rts

; Doppelklick auf einen unlesbaren Datentraeger (Eintrag SEL2)
vol_bad:
        lda SEL2
        asl
        tax
        lda voldev,x
        sta CDAD
        LG cdl_isdev
        bcs @cd
        CALL act_erase
        rts
@cd:    jsr cd_find
        bcs @h
        lda #CDKIND
        CALL win_open
        bcs @r
        jsr cd_geo
@h:     LG cdl_scan
        lda CDAD
        LG cdl_seldev
        jsr cd_find
        bcc @r
        CALL win_front
@r:     CALL redraw_all
        rts

; neues Fenster: hoeher als ein Dialog
cd_geo: lda NWIN
        dec a
        CALL recaddr
        sta PB
        ldy #R_H
        lda #CD_H
        sta (PB),y
        ldy #R_Y
        lda #WY-8
        sta (PB),y
        lda NWIN
        dec a
        CALL win_load
        rts

; ---------- Titelnamen holen (Taste I, Klick auf Zeile 1, automatisch) ----------
cd_lookup:
        ldx #CDOK
        jsr cdg
        bne *+3
        rts
        lda #1                  ; "wird gesucht" anzeigen
        ldx #CDNF
        jsr cdsw
        jsr cd_rdlcd
        jsr cd_cfg
        jsr net_start
        bcs @ne
        lda #0                  ; query
        jsr cd_http
        bcs @ne
        LG cdl_pq
        bcs @nf
        lda #1                  ; read
        jsr cd_http
        bcs @ne
        LG cdl_pr
        bcc @d
@nf:    lda #3
        bra @s
@ne:    lda #4
@s:     ldx #CDNF
        jsr cdsw
@d:     stz NERR
        jsr cd_find
        bcc @r
        CALL recaddr
        sta PB
        jmp cd_rdall
@r:     rts

; Einstellungen (GSOS95.NET) in diese Bank: Server, E-Mail
cd_cfg: jsr net_load
        ldy #0
@l:     lda CDB_SRV,y
        ldx #CDSRV
        jsr cd_sby
        lda CDB_ML,y
        ldx #CDML
        jsr cd_sby
        iny
        cpy #64
        bcc @l
        lda CDB_SRV             ; Laengen pruefen
        and #$00FF
        cmp #64
        bcc @m
        lda #0
        ldx #CDSRV
        jsr cdsb
@m:     lda CDB_ML
        and #$00FF
        cmp #64
        bcc @r
        lda #0
        ldx #CDML
        jsr cdsb
@r:     rts
; Byte A nach X+Y (diese Bank)
cd_sby: pha
        tya
        stx CDW3
        clc
        adc CDW3
        tax
        pla
        jmp cdsb

; HTTP-Anfrage A (0 query, 1 read) an den Server, Antwort nach NWB:0 -> A = Laenge
cd_http:
        sta CDW4
        jsr net_ptr
        lda CDW4
        LG cdl_req
        sta WCNT
        lda #CDB_RQ
        sta WPTR
        stz NTLS
        lda #80
        sta DPORT
        lda NCAPS               ; Name an die Karte oder selbst aufloesen
        and #CAP_NAME
        bne @o
        jsr net_resolve
        bcs @e
@o:     jsr tcp_open
        bcs @e
        jsr tcp_write
        bcs @c
        stz WPTR
        lda #CDB_RQ-$100
        sta NLIM
        stz NSTRM
        inc NQUIET
        jsr tcp_readall
        stz NQUIET
        php
        jsr tcp_close
        plp
        bcs @e
        jsr net_ptr
        lda WPTR
        clc
        rts
@c:     jsr tcp_close
@e:     sec
        rts

; Anzeige der CD-Wiedergabe neu zeichnen (falls offen)
cd_rdlcd:
        jsr cd_find
        bcc @r
        CALL recaddr
        sta PB
        jmp cd_rdl
@r:     rts

; Sekundentakt (aus der Diskettenabfrage)
cd_tick:
        lda MENU
        ora DROP
        bne @r
        jsr cd_find
        bcc @r
        CALL recaddr
        sta PB
        LG cdl_poll
        bcc @r
        cmp #2
        beq @all
        jmp cd_rdl
@all:   jsr cd_rdall
        jmp cd_auton
@r:     rts

; automatisch Titelnamen holen: CD da, noch keine Namen, Netzwerk schon gestartet
cd_auton:
        lda NSTATE
        beq @r
        ldx #CDOK
        jsr cdg
        beq @r
        ldx #CDNF
        jsr cdg
        bne @r
        jmp cd_lookup
@r:     rts

; Anzeige (Fenster in PB) neu zeichnen
cd_rdl: ldy #R_X
        lda (PB),y
        clc
        adc #CD_LX-WX+2
        sta KL
        clc
        adc #CD_LW-4
        sta KR
        ldy #R_Y
        lda (PB),y
        clc
        adc #CD_LY-WY+2
        sta KT
        clc
        adc #CD_LH-4
        sta KB
        CALL redraw_rect
        rts
; ganzen Inhalt (Fenster in PB) neu zeichnen
cd_rdall:
        ldy #R_X
        lda (PB),y
        clc
        adc #4
        sta KL
        clc
        adc #WW-8
        sta KR
        ldy #R_Y
        lda (PB),y
        clc
        adc #CD_LY-WY
        sta KT
        clc
        adc #CD_H-(CD_LY-WY)-4
        sta KB
        CALL redraw_rect
        rts

; Text an Adresse A (diese Bank) an SX/SY
cd_str: sta PDST
        jsr cd_bkm
        sta PDST+2
        jmp ui_s2
; Breite von Text A (diese Bank) -> A
cd_width:
        sta PDST
        jsr cd_bkm
        sta PDST+2
        jmp ui_wid2
cd_bkm: phk
        sep #$20
        pla
        rep #$20
        and #$00FF
        rts

; Text A rechtsbuendig, rechter Rand X, Zeile Y
cd_rstr:
        sta CDW
        stx CDW2
        sty SY
        jsr cd_width
        sta SX
        lda CDW2
        sec
        sbc SX
        sta SX
        lda CDW
        jmp cd_str

; Rechteck RX..RH vertieft, Fuellfarbe A
cd_sunk:
        CALL setcolor
        CALL fillrect
        lda #DKGRAY
        sta C1
        lda #WHITE
        sta C2
        CALL bevel
        inc RX
        inc RY
        dec RW
        dec RW
        dec RH
        dec RH
        lda #BLACK
        sta C1
        lda #GRAY
        sta C2
        CALL bevel
        rts

; Knopf RX..RH, A=1 gedrueckt
cd_btn: pha
        lda #GRAY
        CALL setcolor
        CALL fillrect
        pla
        bne @dn
        lda #WHITE
        sta C1
        lda #BLACK
        sta C2
        CALL bevel
        inc RX
        inc RY
        dec RW
        dec RW
        dec RH
        dec RH
        lda #GRAY
        sta C1
        lda #DKGRAY
        sta C2
        bra @b
@dn:    lda #BLACK
        sta C1
        lda #WHITE
        sta C2
        CALL bevel
        inc RX
        inc RY
        dec RW
        dec RW
        dec RH
        dec RH
        lda #DKGRAY
        sta C1
        lda #GRAY
        sta C2
@b:     CALL bevel
        rts

; Knopf Nummer A (0..5): Lage nach RX/RY/RW/RH
cd_brect:
        asl
        asl
        sta RX                  ; *4
        asl
        asl
        asl                     ; *32
        clc
        adc RX                  ; *36
        clc
        adc #CD_GX
        sta RX
        lda #CD_BY
        sta RY
        lda #CD_BW
        sta RW
        lda #CD_BH
        sta RH
        rts

; schwarzes Rechteck (Symbol) relativ zur linken oberen Ecke des Knopfs A
.macro GL bn,xx,yy,ww,hh
        FILL CD_GX+bn*36+10+xx,CD_BY+5+yy,ww,hh,BLACK
.endmacro

cd_draw:
        LG cdl_fmt
        ; Anzeige
        lda #CD_LX
        sta RX
        lda #CD_LY
        sta RY
        lda #CD_LW
        sta RW
        lda #CD_LH
        sta RH
        lda #BLACK
        jsr cd_sunk
        ldx #CDL0P              ; Zeile 1: Kuenstler / Album (gelb), Hinweis (grau)
        jsr cdg
        beq @l1
        pha
        ldx #CDNF
        jsr cdg
        ldx #YELLOW
        cmp #2
        beq *+5
        ldx #GRAY
        txa
        CALL setcolor
        lda #CD_LX+6
        sta SX
        lda #CD_LY+5
        sta SY
        pla
        jsr cd_str
@l1:    lda #GREEN
        CALL setcolor
        lda #CD_LX+6
        sta SX
        lda #CD_LY+17
        sta SY
        lda #CDL1
        jsr cd_str
        lda #CDL1R
        ldx #CD_LX+CD_LW-6
        ldy #CD_LY+17
        jsr cd_rstr
        lda #CD_LX+6
        sta SX
        lda #CD_LY+29
        sta SY
        ldx #CDL2P
        jsr cdg
        jsr cd_str
        lda #CDL2R
        ldx #CD_LX+CD_LW-6
        ldy #CD_LY+29
        jsr cd_rstr
        lda #GRAY               ; Laufwerk
        CALL setcolor
        lda #CD_LX+6
        sta SX
        lda #CD_LY+41
        sta SY
        ldx #CDN
        jsr cdg
        beq @nb
        lda #CDNM
        jsr cd_str
        ldx #CDN                ; mehrere: (1/2)
        jsr cdg
        cmp #2
        bcc @nb
        lda #' '
        CALL putch
        lda #'('
        CALL putch
        ldx #CDI
        jsr cdg
        inc a
        CALL drawnum
        lda #'/'
        CALL putch
        ldx #CDN
        jsr cdg
        CALL drawnum
        lda #')'
        CALL putch
@nb:    ; Knoepfe: zurueck, Wiedergabe, Pause, Stopp, weiter, Auswerfen
        stz CDW
@k:     lda CDW
        jsr cd_brect
        ldx #CDST
        jsr cdg
        sta CDW2
        lda #0
        ldx CDW
        cpx #1
        bne @k2
        ldx CDW2
        cpx #1
        bne @k3
        lda #1                  ; spielt: Wiedergabe gedrueckt
        bra @k3
@k2:    cpx #2
        bne @k3
        ldx CDW2
        cpx #2
        bne @k3
        lda #1                  ; Pause gedrueckt
@k3:    jsr cd_btn
        inc CDW
        lda CDW
        cmp #6
        bcc @k
        ; Symbole (11x9)
        GL 0,0,0,2,9            ; |<<
        GL 0,3,4,1,1
        GL 0,4,3,1,3
        GL 0,5,2,1,5
        GL 0,6,1,1,7
        GL 0,7,0,1,9
        GL 0,8,4,1,1
        GL 0,9,3,1,3
        GL 0,10,2,1,5
        GL 0,11,1,1,7
        GL 0,12,0,1,9
        GL 1,3,0,1,9            ; >
        GL 1,4,1,1,7
        GL 1,5,2,1,5
        GL 1,6,3,1,3
        GL 1,7,4,1,1
        GL 2,2,0,3,9            ; ||
        GL 2,7,0,3,9
        GL 3,2,1,8,8            ; Stopp
        GL 4,0,0,1,9            ; >>|
        GL 4,1,1,1,7
        GL 4,2,2,1,5
        GL 4,3,3,1,3
        GL 4,4,4,1,1
        GL 4,5,0,1,9
        GL 4,6,1,1,7
        GL 4,7,2,1,5
        GL 4,8,3,1,3
        GL 4,9,4,1,1
        GL 4,10,0,2,9
        GL 5,5,0,1,1            ; Auswerfen
        GL 5,4,1,3,1
        GL 5,3,2,5,1
        GL 5,2,3,7,1
        GL 5,1,4,9,1
        GL 5,1,6,9,2
        ; Titelraster
        ldx #CDOK
        jsr cdg
        bne *+3
        rts
        ldx #CDT0
        jsr cdg
        sta CDW
@t:     lda CDW
        jsr cd_trect
        bcc *+3
        rts
        ldx #CDTR
        jsr cdg
        cmp CDW
        bne @tn
        lda #NAVY               ; aktueller Titel
        jsr cd_sunk
        lda #WHITE
        bra @tt
@tn:    lda #0
        jsr cd_btn
        lda #BLACK              ; Datentitel grau
        sta CDW2
        lda CDW
        clc
        adc #TC
        tax
        jsr cdgb
        and #$0004
        beq @tb
        lda #DKGRAY
        sta CDW2
@tb:    lda CDW2
@tt:    CALL setcolor
        lda CDW
        jsr cd_trect
        lda RY
        clc
        adc #2
        sta SY
        lda #10
        ldx CDW
        cpx #10
        bcc *+5
        lda #7
        clc
        adc RX
        sta SX
        lda CDW
        CALL drawnum
        inc CDW
        ldx #CDT1
        jsr cdg
        cmp CDW
        bcs @t
        rts

; Titel A -> Rechteck im Raster (C=1 nicht sichtbar)
cd_trect:
        sta CDW3
        ldx #CDT0
        jsr cdg
        sta CDW4
        lda CDW3
        sec
        sbc CDW4
        cmp #CD_GN
        bcc *+3
        rts
        pha
        and #7
        ldx #27
        CALL gmul
        clc
        adc #CD_GX
        sta RX
        pla
        lsr
        lsr
        lsr
        ldx #14
        CALL gmul
        clc
        adc #CD_GY
        sta RY
        lda #CD_GW
        sta RW
        lda #CD_GH
        sta RH
        clc
        rts

; ---------- Klicks ----------
cd_click:
        stz CDW
@k:     lda CDW
        jsr cd_brect
        CALL inrect
        bcc *+5
        jmp @hit
        inc CDW
        lda CDW
        cmp #6
        bcc @k
        HIT CD_LX,CD_LY+38,CD_LW,14     ; Laufwerk wechseln
        bcc @g0
        LG cdl_nextdev
        jmp cd_rd
@g0:    HIT CD_LX,CD_LY+2,CD_LW,13      ; Zeile 1: Titelnamen holen
        bcc @g
        jmp cd_lookup
@g:     ldx #CDOK               ; Titelraster
        jsr cdg
        beq @r
        ldx #CDT0
        jsr cdg
        sta CDW
@t:     lda CDW
        jsr cd_trect
        bcs @r
        CALL inrect
        bcs @th
        inc CDW
        ldx #CDT1
        jsr cdg
        cmp CDW
        bcs @t
@r:     rts
@th:    lda CDW
        LG cdl_play
        jmp cd_rd
@hit:   lda CDW
        jmp cd_do

; Knopf A ausfuehren
cd_do:  asl
        tax
        jmp (cd_dot,x)
cd_dot: .addr cd_prev, cd_play, cd_pause, cd_stop, cd_next, cd_eject
cd_prev:
        lda #$FFFF
        LG cdl_skip
        jmp cd_rd
cd_next:
        lda #1
        LG cdl_skip
        jmp cd_rd
cd_play:
        ldx #CDST
        jsr cdg
        cmp #2
        beq cd_pause            ; Pause: weiter
        cmp #1
        beq cd_rd
        ldx #CDTR
        jsr cdg
        bne @p
        ldx #CDT0
        jsr cdg
@p:     LG cdl_play
        jmp cd_rd
cd_pause:
        LG cdl_pause
        jmp cd_rd
cd_stop:
        LG cdl_stop
        jmp cd_rd
cd_eject:
        LG cdl_eject
cd_rd:  CALL redraw_win
        rts

; ---------- Tasten ----------
cd_key: lda KEYC
        cmp #$1B
        bne @k1
        CALL close_win
        rts
@k1:    cmp #' '                ; Leertaste: Wiedergabe/Pause
        beq cd_pause
        cmp #$0D
        beq cd_play
        cmp #$08                ; links: zurueck
        beq cd_prev
        cmp #$15                ; rechts: weiter
        beq cd_next
        and #$00DF
        cmp #'S'
        beq cd_stop
        cmp #'E'
        beq cd_eject
        cmp #'I'                ; Titelnamen aus dem Internet
        bne @kl
        jmp cd_lookup
@kl:    cmp #'L'                ; Laufwerk wechseln
        bne @r
        LG cdl_nextdev
        jmp cd_rd
@r:     rts
