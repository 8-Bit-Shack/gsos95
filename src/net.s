; ================================================================
; GS/OS95: Netzwerk ueber die Uthernet II (WIZnet W5100)
; Indirekter Bus: $C0n4 Modus, $C0n5/$C0n6 Adresse, $C0n7 Daten (n = 8+Slot)
; Der W5100 rechnet TCP/IP selbst; hier nur Ansteuerung, DHCP und DNS.
; Socket 0 = TCP (je 4 KB Puffer), Socket 1 = UDP fuer DHCP/DNS (2 KB)
; Liegt in der vierten Codebank (XCODE3). Puffer: eine ganze Bank (NWB).
; ================================================================
        .segment "XCODE3"
FARMODE .set 3

xf_net_boot:    jsr net_boot
        rtl
xf_net_start:   jsr net_start
        rtl
xf_net_resolve: jsr net_resolve
        rtl
xf_tcp_open:    jsr tcp_open
        rtl
xf_tcp_write:   jsr tcp_write
        rtl
xf_tcp_readall: jsr tcp_readall
        rtl
xf_tcp_close:   jsr tcp_close
        rtl
xf_tcp_poll:    jsr tcp_poll
        rtl
xf_net_mem:     jsr net_mem
        rtl
xf_wc_ndlg:     lda WIN             ; Fensterart 22 Netzwerk, 23 CD-Wiedergabe, 24 Rechner
        cmp #23
        beq @cd
        bcs @ca
        jsr ndlg_draw
        rtl
@cd:    jsr cd_draw
        rtl
@ca:    cmp #24
        bne @so
        jsr ca_draw
        rtl
@so:    cmp #26
        beq @pt
        bcs @x4
        jsr so_draw
        rtl
@pt:    JSLF xf_pt_draw
        rtl
@x4:    JSLF xf_x4_draw         ; 27 Datum/Uhrzeit, 28 Editor
        rtl
xf_ndlg_click:  lda WIN
        cmp #23
        beq @cd
        bcs @ca
        jsr ndlg_click
        rtl
@cd:    jsr cd_click
        rtl
@ca:    cmp #24
        bne @so
        jsr ca_click
        rtl
@so:    cmp #26
        beq @pt
        bcs @x4
        jsr so_click
        rtl
@pt:    JSLF xf_pt_click
        rtl
@x4:    JSLF xf_x4_click         ; 27 Datum/Uhrzeit, 28 Editor
        rtl
xf_ndlg_key:    lda WIN
        cmp #23
        beq @cd
        bcs @ca
        jsr ndlg_key
        rtl
@cd:    jsr cd_key
        rtl
@ca:    cmp #24
        bne @so
        jsr ca_key
        rtl
@so:    cmp #26
        beq @pt
        bcs @x4
        jsr so_key
        rtl
@pt:    JSLF xf_pt_key
        rtl
@x4:    JSLF xf_x4_key         ; 27 Datum/Uhrzeit, 28 Editor
        rtl
xf_ndlg_init:   jsr ndlg_init
        rtl
xf_ui_str:      jsr ui_str
        rtl
xf_ui_s2:       jsr ui_s2           ; Text an [PDST] (Laenge + Zeichen)
        rtl
xf_ui_w2:       jsr ui_wid2         ; Breite von [PDST] -> A
        rtl
; A Zeichen ab [PDST] in festen Zellen (Breite WCELL) ab SX/SY zeichnen
xf_cells:
        sta WDN
        stz WDI
@l:     lda WDI
        cmp WDN
        bcs @r
        tay
        lda [PDST],y
        and #$00FF
        cmp #' '
        beq @n
        jsr gl_ptr
        sta SPR
        CALL blit1
@n:     lda SX
        clc
        adc WCELL
        sta SX
        inc WDI
        bra @l
@r:     rtl
xf_wd_chars:    jsr wd_chars
        rtl
xf_net_esc:     jsr net_esc
        rtl
xf_net_tick:    jsr t_now
        rtl
xf_net_save:    jsr net_save
        rtl
xf_putip:       jsr put_ip
        rtl
xf_glp:         jsr gl_ptr
        rtl
xf_glw:         jsr gl_ptr      ; Breite des Zeichens A -> A
        sta LP2
        lda ABANK
        sta LP2+2
        lda [LP2]
        and #$00FF
        rtl

UIO     = $E0C084               ; + NSLOTX: Modus, Adresse hi/lo, Daten
S0      = $0400                 ; Register Socket 0 / 1
S1      = $0500
SN_MR   = $00
SN_CR   = $01
SN_IR   = $02
SN_SR   = $03
SN_PORT = $04
SN_DIPR = $0C
SN_DPORT= $10
SN_FSR  = $20
SN_TXWR = $24
SN_RSR  = $26
SN_RXRD = $28
TX0 = $4000                     ; RMSR/TMSR = $06: Socket 0 4 KB, Socket 1 2 KB
RX0 = $6000
TX1 = $5000
RX1 = $7000

; ---------- Aufteilung der Pufferbank ----------
WB_RAW  = $0000                 ; Antwort des Servers (bis 32 KB)
WB_TXT  = $8000                 ; aufbereiteter Text
WB_TXTE = $BFE0                 ; dahinter Formulare/Felder ($C000..$C7FF)
WB_LIN  = $C800                 ; Zeilentabelle, 6 Bytes je Zeile
WB_LINE = $EFF0
WB_UDPH = $EFF0                 ; UDP-Kopf (8 Bytes)
WB_UDP  = $F000                 ; UDP-Paket (DHCP/DNS)
WB_REQ  = $F600                 ; HTTP-Anfrage
WB_URL  = $F800                 ; Adresszeile (Laenge + Text)
WB_HOST = $F900                 ; Rechnername
WB_PATH = $FA00                 ; Pfad mit Abfrage
WB_TURL = $FB00                 ; Ziel-Adresse beim Laden
WB_TITLE= $FC00                 ; Seitentitel (Laenge + 62)
WB_HREF = $FC40                 ; Link-Ziel / Location (Laenge + 190)
WB_ALT  = $FD00                 ; Alt-Text eines Bildes (Laenge + 62)
WB_CTYP = $FD40                 ; Inhaltstyp (Laenge + 30)
WB_BASE = $FD80                 ; Adresse der angezeigten Seite (bis $FE7F)
WB_HIS  = $FE80                 ; Verlauf: 3 x 128 Bytes (bis $FFFF)

NE_CARD = 1                     ; keine Karte
NE_DNS  = 3                     ; Name unbekannt / DNS antwortet nicht
NE_CONN = 4                     ; Verbindung abgelehnt
NE_TIME = 5                     ; Zeitueberschreitung
NE_ABRT = 6                     ; Esc
NE_MEM  = 7                     ; kein Speicher
NE_LINK = 8                     ; CoGS ohne Netz
NE_TLS  = 9                     ; TLS-Handshake fehlgeschlagen
NE_CERT = 10                    ; Zertifikat ungueltig
NE_MARI = 11                    ; Marinetti nicht geladen / keine Verbindung
NE_MDM  = 12                    ; WLAN-Modem antwortet nicht

; Karten (NCARD, NCSEL)
NC_W5100 = 1
NC_COGS  = 2
NC_FUJI  = 3
NC_MARI  = 4
NC_MODEM = 5
NC_LAST  = 5
; Faehigkeiten (NCAPS)
CAP_NAME = 1                    ; Karte nimmt Rechnernamen (kein DNS noetig)
CAP_TLS  = 2                    ; https auf der Karte
CAP_UDP  = 4                    ; eigene UDP-Pakete (NTP)
CAP_DNS  = 8                    ; Namensaufloesung durch den Treiber (net_resolve)

.macro BRD8
        lda [PSRC],y
        and #$00FF
.endmacro
.macro BWR8
        sep #$20
        sta [PSRC],y
        rep #$20
.endmacro
        .pushseg
        .segment "BSS"
NCSLOT:   .res 2                 ; Slot der Karte 1..7
NDHCP:   .res 2                 ; 1 = Adresse per DHCP
NCFG:    .res 16                ; eingestellt: IP, Maske, Gateway, DNS
NCUR:    .res 16                ; aktiv
NMAC:    .res 6
NSTATE:  .res 2                 ; 1 = Karte bereit
NDHOK:   .res 2                 ; 1 = DHCP hat geklappt
NSLOTX:  .res 2
NWB:     .res 2                 ; Bank des Puffers (0 = noch keiner)
NERR:    .res 2
NT0:     .res 2
NPLD:    .res 2                 ; Einstellungen gelesen
NABORT:  .res 2
WT:      .res 2
WT2:     .res 2
WT3:     .res 2
WV:      .res 2
WSB:     .res 2
WMODE:   .res 2
WCNT:    .res 2
WPTR:    .res 2
WOFF:    .res 2
WRBASE:     .res 2
WRM:     .res 2
WTXB:    .res 2
WRXB:    .res 2
NSOCK:   .res 2                 ; 1 = tcp_* auf Socket 1 (FTP-Daten)
WTXW:    .res 2
WRXR:    .res 2
SPORT:   .res 2
DIP:     .res 4
DPORT:   .res 2
NXID:    .res 4
NSRV:    .res 4
NOFFER:  .res 4
NDT:     .res 16                ; Werte aus DHCP-Antwort
NDTYP:   .res 2
NQID:    .res 2
NRNG:    .res 2
NI:      .res 2
NJ:      .res 2
NTOP:    .res 2
NLIM:    .res 2
NREST:   .res 2
NTRY:    .res 2
DNCN:    .res 128               ; DNS-Zwischenspeicher: 4 x (Laenge + 31 Zeichen)
DNCI:    .res 16                ; 4 x Adresse
DCNX:    .res 2
DCI:     .res 2
DCK:     .res 2
DCL:     .res 2
NDNSTCP: .res 2
NDISC:   .res 2                 ; 1 = vor dem Senden alten Empfang verwerfen
NSTRM:   .res 2                 ; 1 = bei vollem Puffer anhalten (Rest folgt)
NFULL:   .res 2                 ; 1 = es kommt noch mehr                 ; UDP-DNS geht nicht: gleich TCP
NCARD:   .res 2                 ; 0 keine, sonst NC_* (1 Uthernet II, 2 CoGS, 3 FujiNet, 4 Marinetti, 5 WLAN-Modem)
STRBK:   .res 2                 ; Bank der Texte (webstr.inc, XCODE2)
NTLS:    .res 2                 ; 1 = naechste Verbindung verschluesselt
NQUIET:  .res 2
NPREF:   .res 2048              ; Inhalt von GSOS95.NET (+26 Startseite, +128 Favoriten, +1024 Telefonbuch, +1536 Netzwerk 2)
NCAPS:   .res 2                 ; Faehigkeiten der Karte (CAP_*)
NCSEL:   .res 2                 ; gewaehlte Karte: 0 automatisch, sonst NC_*
MDPORT:  .res 2                 ; WLAN-Modem: 1 Druckeranschluss, 2 Modemanschluss
MDBAUD:  .res 2                 ; WLAN-Modem: 0 2400, 1 9600, 2 19200, 3 38400, 4 57600
        .popseg

        .pushseg
        .segment "RODATA"
npath:   .word 19
         .byte "*:System:GSOS95.NET"
nopen_pb:  .word 3, 0
           .word npath, 0
           .word 1
ncreate_pb: .word 3
           .word npath, 0
           .word $C3, $06
nrw_pb:    .word 4, 0
           .word NPREF, 0
           .word 128, 0
           .word 0, 0
nclose_pb: .word 1, 0
ndefcfg:  .byte 192,168,1,50, 255,255,255,0, 192,168,1,1, 1,1,1,1
        .popseg

; ---------- beim Programmstart ----------
net_boot:
        stz NQUIET
        stz NCARD
        stz NCAPS
        stz FNBLK
        stz MRSTART
        stz NTLS
        stz WBINIT
        stz HISN
        stz NLINES
        stz STMSG
        stz NSTATE
        stz NWB
        stz NPLD
        stz NABORT
        stz NDHOK
        rts

; ---------- Puffer: eine ganze Bank (64 KB) ----------
; C=1 wenn kein Speicher
net_mem:
        lda NWB
        beq @get
        clc
        rts
@get:   pha
        pha
        pea $0001               ; 64 KB
        pea $0000
        lda USERID
        pha
        pea $C018               ; fest, gesperrt, keine Bankgrenze, kein Spezialspeicher
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        pla
        sta HND
        pla
        sta HND+2
        bcs @fail
        ldy #2
        lda [HND],y
        and #$00FF
        sta NWB
        sta PSRC+2
        stz PSRC
        sep #$20                ; leere Adresszeile, kein Titel
        lda #0
        ldy #WB_URL
        sta [PSRC],y
        ldy #WB_TITLE
        sta [PSRC],y
        rep #$20
        clc
        rts
@fail:  lda #NE_MEM
        sta NERR
        sec
        rts

; Zeiger PSRC auf die Pufferbank (Offset 0)
net_ptr:
        lda NWB
        sta PSRC+2
        stz PSRC
        rts

; ---------- Einstellungen (*:System:GSOS95.NET) ----------
; Aufbau: "N1", Slot, DHCP, IP/Maske/Gateway/DNS (16), MAC (6), Startseite (1+n)
net_load:
        lda NPLD
        beq @go
        rts
@go:    inc NPLD
        stz NCSEL               ; Vorgaben: Karte automatisch, Modem am Modemanschluss, 9600
        lda #2
        sta MDPORT
        lda #1
        sta MDBAUD
        lda #3
        sta NCSLOT
        lda #1
        sta NDHCP
        ldx #14
@d:     lda ndefcfg,x
        sta NCFG,x
        dex
        dex
        bpl @d
        ldx #2046               ; keine eigene Startseite, keine Favoriten, kein Telefonbuch
@z:     stz NPREF,x
        dex
        dex
        bpl @z
        lda MYBANK
        sta nopen_pb+6
        sta nrw_pb+6
        sta ncreate_pb+4
        lda #1
        sta nopen_pb+8
        GSOS $2010,nopen_pb
        bcs @mac
        lda nopen_pb+2
        sta nrw_pb+2
        sta nclose_pb+2
        lda #2048
        sta nrw_pb+8
        GSOS $2012,nrw_pb
        bcc @rd
        cmp #$4C                ; aeltere, kuerzere Datei: Dateiende ist in Ordnung
        beq @rd
        GSOS $2014,nclose_pb
        bra @mac
@rd:    GSOS $2014,nclose_pb
        lda NPREF
        cmp #$314E              ; "N1"
        bne @mac
        jsr np2_get             ; Kartenwahl, Modem (NPREF+1536)
        lda NPREF+2
        and #$0007
        beq @ns
        sta NCSLOT
@ns:    lda NPREF+3
        and #$0001
        sta NDHCP
        ldx #14
@c:     lda NPREF+4,x
        sta NCFG,x
        dex
        dex
        bpl @c
        ldx #4
@m:     lda NPREF+20,x
        sta NMAC,x
        dex
        dex
        bpl @m
        lda NMAC
        cmp #$0800              ; 00:08:DC...
        bne @mac
        rts
@mac:   ; neue MAC-Adresse (WIZnet-Kennung + Zufall) und gleich speichern
        lda #$0800
        sta NMAC
        jsr t_now
        eor FRAMES
        and #$FF00
        ora #$00DC
        sta NMAC+2
        jsr t_now
        asl
        eor MX
        eor MY
        sta NMAC+4
        jmp net_save

; Netzwerk-Einstellungen 2 (NPREF+1536): Karte, Modem-Anschluss, Baudrate
np2_get:
        lda NPREF+1536
        and #$00FF
        cmp #NC_LAST+1
        bcc @c
        lda #0
@c:     sta NCSEL
        lda NPREF+1537
        and #$00FF
        cmp #1
        beq @p
        lda #2                  ; Vorgabe: Modemanschluss
@p:     sta MDPORT
        lda NPREF+1538
        and #$00FF
        cmp #5
        bcc @b
        lda #1                  ; Vorgabe 9600
@b:     sta MDBAUD
        rts
np2_put:
        sep #$20
        lda NCSEL
        sta NPREF+1536
        lda MDPORT
        sta NPREF+1537
        lda MDBAUD
        sta NPREF+1538
        rep #$20
        rts

net_save:
        jsr np2_put
        lda #$314E
        sta NPREF
        sep #$20
        lda NPREF+2             ; Bit 7: Terminal scharf/grau (vt.inc)
        and #$80
        ora NCSLOT
        sta NPREF+2
        lda NPREF+3             ; Bit 1..7: Zeitzone (ntp.inc)
        and #$FE
        ora NDHCP
        sta NPREF+3
        rep #$20
        ldx #14
@c:     lda NCFG,x
        sta NPREF+4,x
        dex
        dex
        bpl @c
        ldx #4
@m:     lda NMAC,x
        sta NPREF+20,x
        dex
        dex
        bpl @m
        lda MYBANK
        sta nopen_pb+6
        sta nrw_pb+6
        sta ncreate_pb+4
        GSOS $2001,ncreate_pb
        lda #2
        sta nopen_pb+8
        GSOS $2010,nopen_pb
        bcs @r
        lda nopen_pb+2
        sta nrw_pb+2
        sta nclose_pb+2
        lda #2048
        sta nrw_pb+8
        GSOS $2013,nrw_pb
        GSOS $2014,nclose_pb
@r:     rts

; ---------- Zeit ----------
t_now:  pha
        pha
        TOOL $2503              ; GetTick
        pla
        plx
        rts
tm_start:
        jsr t_now
        sta NT0
        rts
; Ticks seit tm_start -> A
t_elapsed:
        jsr t_now
        sec
        sbc NT0
        rts

; Esc gedrueckt? C=1 (NABORT gesetzt)
net_esc:
        lda NABORT
        bne @y
        pea $0000
        pea $0028               ; keyDown, autoKey
        pea $0000
        lda EVPTR
        pha
        TOOL $0A06              ; GetNextEvent
        pla
        beq @n
        lda EVREC+2
        and #$007F
        cmp #$1B
        bne @n
        lda #1
        sta NABORT
@y:     lda #NE_ABRT
        sta NERR
        sec
        rts
@n:     clc
        rts

; ================= W5100: einfache Zugriffe =================
; Konvention: X = NSLOTX in allen Routinen dieses Abschnitts
w_set:  ldx NSLOTX              ; A = Adresse im W5100
        sep #$20
        xba
        sta f:UIO+1,x
        xba
        sta f:UIO+2,x
        rep #$20
        rts
w_put:  sep #$20
        sta f:UIO+3,x
        rep #$20
        rts
w_get:  sep #$20
        lda f:UIO+3,x
        rep #$20
        and #$00FF
        rts
; Wort (hoeherwertiges Byte zuerst) an Adresse A -> A
w_rd16: jsr w_set
        jsr w_get
        xba
        sta WT
        jsr w_get
        ora WT
        rts
; Socket-Register A (relativ, Basis WSB)
s_rd8:  clc
        adc WSB
        jsr w_set
        jmp w_get
s_rd16: clc
        adc WSB
        jmp w_rd16
s_wr8:  clc
        adc WSB
        jsr w_set
        lda WV
        jmp w_put
s_wr16: clc
        adc WSB
        jsr w_set
        lda WV
        xba
        jsr w_put
        lda WV
        jmp w_put
; Befehl A geben und warten, bis der Chip ihn angenommen hat
s_cmd:  sta WV
        lda #SN_CR
        jsr s_wr8
        ldy #0
@w:     lda #SN_CR
        jsr s_rd8
        beq @r
        dey
        bne @w
@r:     rts
; empfangene Bytes (zweimal gleich gelesen) -> A
s_rsr:  lda #SN_RSR
        jsr s_rd16
        sta WT2
        lda #SN_RSR
        jsr s_rd16
        cmp WT2
        bne s_rsr
        rts
; 4 Bytes ab Y (Hauptbank) an die eingestellte Adresse
w_put4: lda #4
        sta WT3
@l:     lda a:0,y
        jsr w_put
        iny
        dec WT3
        bne @l
        rts

; ---------- Karte im eingestellten Slot pruefen und einrichten ----------
; C=1 wenn keine Karte antwortet
w_init: lda NCSLOT
        and #$0007
        bne @s
        jmp @no
@s:     asl
        asl
        asl
        asl
        sta NSLOTX
        tax
        sep #$20
        lda #$80                ; Reset
        sta f:UIO,x
        rep #$20
        ldy #200
@dl:    dey
        bne @dl
        ldx NSLOTX
        sep #$20
        lda #$03                ; indirekter Bus, Adresse zaehlt mit
        sta f:UIO,x
        lda f:UIO,x
        rep #$20
        and #$00FF
        cmp #$0003
        bne @no
        lda #$0017              ; Wiederholzeit: nach dem Reset $07D0
        jsr w_rd16
        cmp #$07D0
        bne @no
        lda #$0009              ; MAC-Adresse
        jsr w_set
        ldy #0
@m:     lda NMAC,y
        jsr w_put
        iny
        cpy #6
        bcc @m
        lda #$001A              ; Puffergroessen RX/TX
        jsr w_set
        lda #$06
        jsr w_put
        lda #$06
        jsr w_put
        clc
        rts
@no:    stz NSLOTX
        sec
        rts

; Gateway, Maske und eigene Adresse aus NCUR in den Chip
w_ipset:
        lda #$0001
        jsr w_set
        ldy #NCUR+8
        jsr w_put4              ; Gateway
        ldy #NCUR+4
        jsr w_put4              ; Maske
        lda #$000F
        jsr w_set
        ldy #NCUR
        jmp w_put4

; ---------- Netzwerk bereit machen ----------
; C=1 bei Fehler (NERR)
net_start:
        jsr net_mem
        bcc @m
        rts
@m:     jsr net_load
        lda NSTATE
        beq @go
        clc
        rts
@go:    stz NCARD
        stz NCAPS
        lda NCSEL               ; feste Wahl?
        beq @auto
        asl
        tax
        jmp (@tb-2,x)
@tb:    .addr ns_w5, ns_cg, ns_fn, ns_mr, ns_md
@auto:  jsr cg_probe            ; 1. CoGS (nur lesend)
        bcs @a2
        jmp ns_cg1
@a2:    jsr fn_probe            ; 2. FujiNet am SmartPort
        bcs @a3
        jmp ns_fn1
@a3:    lda #0                  ; 3. Marinetti, wenn schon verbunden
        jsr mr_probe
        bcs @a4
        jmp ns_mr1
@a4:    jsr w_init              ; 4. Uthernet II direkt
        bcs @a5
        jmp ns_w51
@a5:    lda #1                  ; 5. Marinetti verbinden lassen (Uthernet I, LANceGS, PPP ...)
        jsr mr_probe
        bcs @no
        jmp ns_mr1
@no:    lda #NE_CARD
        sta NERR
        sec
        rts

ns_cg:  jsr cg_probe
        bcc ns_cg1
        jmp ns_nocard
ns_cg1: lda #NC_COGS
        sta NCARD
        lda #CAP_NAME+CAP_TLS+CAP_DNS
        sta NCAPS
        jsr cg_start
        bcs @r
        lda #1
        sta NSTATE
        clc
@r:     rts

ns_fn:  jsr fn_probe
        bcc ns_fn1
        jmp ns_nocard
ns_fn1: lda #NC_FUJI
        sta NCARD
        lda #CAP_NAME+CAP_TLS
        sta NCAPS
        jsr fn_start
        bcs @r
        lda #1
        sta NSTATE
        clc
@r:     rts

ns_mr:  lda #1
        jsr mr_probe
        bcc ns_mr1
        lda #NE_MARI
        sta NERR
        sec
        rts
ns_mr1: lda #NC_MARI
        sta NCARD
        lda #CAP_DNS
        sta NCAPS
        lda #1
        sta NSTATE
        sta NDHOK
        clc
        rts

ns_md:  lda #NC_MODEM
        sta NCARD
        lda #CAP_NAME
        sta NCAPS
        jsr md_start
        bcs @r
        lda #1
        sta NSTATE
        clc
@r:     rts

ns_w5:  jsr w_init
        bcc ns_w51
ns_nocard:
        lda #NE_CARD
        sta NERR
        sec
        rts
ns_w51: lda #NC_W5100
        sta NCARD
        lda #CAP_UDP
        sta NCAPS
        ldx #14
@c:     lda NCFG,x
        sta NCUR,x
        dex
        dex
        bpl @c
        stz NDHOK
        lda NDHCP
        beq @man
        jsr dhcp
        bcs @man
        lda #1
        sta NDHOK
@man:   jsr w_ipset
        lda #1
        sta NSTATE
        clc
        rts

; ---------- Socket oeffnen: A = Modus (1 TCP, 2 UDP), WSB, SPORT ----------
s_open: sta WMODE
        lda #SN_SR              ; nur schliessen, wenn noch offen: GSSquared 1.x meldet
        jsr s_rd8               ; CLOSED spaeter als Ereignis; kaeme das nach dem OPEN an,
        cmp #0                  ; waere der neue Socket tot (Empfang gesperrt)
        beq @nc
        lda #$10                ; CLOSE
        jsr s_cmd
        jsr tm_start
@cw:    lda #SN_SR              ; jeder Zugriff holt Ereignisse ab
        jsr s_rd8
        jsr t_elapsed
        cmp #6
        bcc @cw
@nc:    lda WMODE
        sta WV
        lda #SN_MR
        jsr s_wr8
        lda SPORT
        sta WV
        lda #SN_PORT
        jsr s_wr16
        lda #$01                ; OPEN
        jsr s_cmd
        jsr tm_start
@w:     lda #SN_SR
        jsr s_rd8
        cmp #$13                ; INIT (TCP)
        beq @ok
        cmp #$22                ; UDP
        beq @ok
        jsr t_elapsed
        cmp #60
        bcc @w
        sec
        rts
@ok:    clc
        rts

; Ziel (DIP, DPORT) in den Socket WSB schreiben
s_dest: lda #SN_DIPR
        clc
        adc WSB
        jsr w_set
        ldy #DIP
        jsr w_put4
        lda DPORT
        sta WV
        lda #SN_DPORT
        jmp s_wr16

; Zufaelliger lokaler Port ab $C000
s_rport:
        jsr t_now
        eor NRNG
        clc
        adc #$3711
        sta NRNG
        and #$3FFF
        ora #$C000
        sta SPORT
        rts

; ---------- Daten senden: WCNT Bytes ab Puffer-Offset WPTR ----------
; WRBASE/WRM = TX-Bereich des Sockets
s_send: jsr net_ptr
        lda #SN_TXWR
        jsr s_rd16
        sta WTXW
        and WRM
        sta WOFF
        clc
        adc WRBASE
        jsr w_set
        ldy WPTR
        lda WCNT
        beq @done
        sta WT3
@l:     lda [PSRC],y
        jsr w_put
        iny
        inc WOFF
        lda WOFF
        cmp WRM
        bcc @nw
        beq @nw
        stz WOFF                ; am Ende des Bereichs wieder vorn
        lda WRBASE
        jsr w_set
@nw:    dec WT3
        bne @l
@done:  lda WTXW
        clc
        adc WCNT
        sta WV
        lda #SN_TXWR
        jsr s_wr16
        lda #$20                ; SEND
        jmp s_cmd

; ---------- Daten holen: WCNT Bytes nach Puffer-Offset WPTR (danach WPTR weiter) ----------
s_recv: jsr net_ptr
        lda #SN_RXRD
        jsr s_rd16
        sta WRXR
        and WRM
        sta WOFF
        clc
        adc WRBASE
        jsr w_set
        ldy WPTR
        lda WCNT
        beq @done
        sta WT3
@l:     sep #$20
        lda f:UIO+3,x
        sta [PSRC],y
        rep #$20
        iny
        inc WOFF
        lda WOFF
        cmp WRM
        bcc @nw
        beq @nw
        stz WOFF
        lda WRBASE
        jsr w_set
@nw:    dec WT3
        bne @l
@done:  sty WPTR
        lda WRXR
        clc
        adc WCNT
        sta WV
        lda #SN_RXRD
        jsr s_wr16
        lda #$40                ; RECV
        jmp s_cmd

; ================= UDP (Socket 1) =================
udp_sel:
        lda #S1
        sta WSB
        lda #TX1
        sta WRBASE
        lda #$07FF
        sta WRM
        rts
udp_rxsel:
        lda #RX1
        sta WRBASE
        lda #$07FF
        sta WRM
        rts

; Paket (WCNT Bytes ab WB_UDP) an DIP:DPORT senden
udp_send:
        jsr udp_sel
        jsr s_dest
        lda #WB_UDP
        sta WPTR
        jmp s_send

; auf ein Paket warten (A = Ticks). C=1 bei Zeitablauf/Esc.
; Kopf nach WB_UDPH, Daten nach WB_UDP, Laenge -> WCNT
udp_wait:
        sta NLIM
        jsr tm_start
@w:     jsr net_esc
        bcc @n
        rts
@n:     jsr udp_sel
        jsr s_rsr
        cmp #8
        bcs @got
        jsr t_elapsed
        cmp NLIM
        bcc @w
        lda #NE_TIME
        sta NERR
        sec
        rts
@got:   jsr udp_rxsel
        lda #WB_UDPH
        sta WPTR
        lda #8
        sta WCNT
        jsr s_recv
        jsr net_ptr
        ldy #WB_UDPH+6          ; Laenge (hoeherwertiges Byte zuerst)
        lda [PSRC],y
        xba
        sta WCNT
        stz NREST
        cmp #$0600
        bcc @ok
        sec
        sbc #$0600
        sta NREST
        lda #$0600
        sta WCNT                ; zu lang: nur den Anfang
@ok:    jsr udp_sel
        jsr udp_rxsel
        lda #WB_UDP
        sta WPTR
        lda WCNT
        pha
        jsr s_recv
        pla
        sta WCNT
        lda NREST               ; Rest verwerfen
        beq @ret
        lda #SN_RXRD
        jsr s_rd16
        clc
        adc NREST
        sta WV
        lda #SN_RXRD
        jsr s_wr16
        lda #$40
        jsr s_cmd
@ret:   clc
        rts

; Byte A an [PSRC],y schreiben, Y weiter
pk_b:   sep #$20
        sta [PSRC],y
        rep #$20
        iny
        rts
; A Nullbytes ab Y
pk_z:   sta WT3
@l:     lda #0
        jsr pk_b
        dec WT3
        bne @l
        rts
; 4 Bytes aus der Hauptbank (X) nach [PSRC],y
pk_4:   lda #4
        sta WT3
@l:     lda a:0,x
        jsr pk_b
        inx
        dec WT3
        bne @l
        rts

; ================= DHCP =================
dhcp:   ldx #14                 ; ohne Adresse anfangen
@z:     stz NCUR,x
        dex
        dex
        bpl @z
        jsr w_ipset
        jsr udp_sel
        lda #68
        sta SPORT
        lda #2
        jsr s_open
        bcc @o
        rts
@o:     lda #$FFFF
        sta DIP
        sta DIP+2
        lda #67
        sta DPORT
        jsr t_now
        sta NXID
        eor NMAC+4
        sta NXID+2
        lda #3
        sta NTRY                ; Versuche
@disc:  lda #1
        jsr dh_build
        jsr udp_send
@wo:    lda #120
        jsr udp_wait
        bcs @again
        jsr dh_parse
        bcs @wo
        lda NDTYP
        cmp #2                  ; OFFER
        bne @wo
        ldx #2                  ; angebotene Adresse und Server merken
@co:    lda NDT,x
        sta NOFFER,x
        dex
        dex
        bpl @co
        lda #3
        jsr dh_build
        jsr udp_send
@wa:    lda #120
        jsr udp_wait
        bcs @again
        jsr dh_parse
        bcs @wa
        lda NDTYP
        cmp #5                  ; ACK
        beq @ack
        cmp #6                  ; NAK
        beq @again
        bra @wa
@again: lda NABORT
        bne @fail
        dec NTRY
        bne @disc
@fail:  lda #$10
        jsr s_cmd
        ldx #14                 ; keine Antwort: feste Adressen zurueck
@rc:    lda NCFG,x
        sta NCUR,x
        dex
        dex
        bpl @rc
        sec
        rts
@ack:   ldx #14
@cp:    lda NDT,x
        sta NCUR,x
        dex
        dex
        bpl @cp
        lda NCUR+12             ; kein DNS-Server genannt: eingestellten nehmen
        ora NCUR+14
        bne @dn
        lda NCFG+12
        sta NCUR+12
        lda NCFG+14
        sta NCUR+14
@dn:    lda #$10
        jsr s_cmd
        clc
        rts

; DHCP-Paket der Art A (1 DISCOVER, 3 REQUEST) nach WB_UDP, Laenge -> WCNT
dh_build:
        sta NDTYP
        jsr net_ptr
        ldy #WB_UDP
        lda #1
        jsr pk_b                ; op = Anfrage
        lda #1
        jsr pk_b                ; Ethernet
        lda #6
        jsr pk_b
        lda #0
        jsr pk_b
        ldx #NXID
        jsr pk_4
        lda #2
        jsr pk_z                ; secs
        lda #$80
        jsr pk_b                ; Antwort per Broadcast
        lda #0
        jsr pk_b
        lda #16
        jsr pk_z                ; ciaddr, yiaddr, siaddr, giaddr
        ldx #0
@m:     lda NMAC,x
        jsr pk_b
        inx
        cpx #6
        bcc @m
        lda #10+64+128
        jsr pk_z
        lda #$63
        jsr pk_b
        lda #$82
        jsr pk_b
        lda #$53
        jsr pk_b
        lda #$63
        jsr pk_b
        lda #53                 ; Nachrichtentyp
        jsr pk_b
        lda #1
        jsr pk_b
        lda NDTYP
        jsr pk_b
        lda NDTYP
        cmp #3
        bne @nr
        lda #50                 ; gewuenschte Adresse
        jsr pk_b
        lda #4
        jsr pk_b
        ldx #NOFFER
        jsr pk_4
        lda #54                 ; Server
        jsr pk_b
        lda #4
        jsr pk_b
        ldx #NSRV
        jsr pk_4
@nr:    lda #55                 ; gewuenschte Angaben: Maske, Router, DNS
        jsr pk_b
        lda #3
        jsr pk_b
        lda #1
        jsr pk_b
        lda #3
        jsr pk_b
        lda #6
        jsr pk_b
        lda #255
        jsr pk_b
        tya                     ; auf 300 Bytes auffuellen
        sec
        sbc #WB_UDP
        cmp #300
        bcs @l
        eor #$FFFF
        sec
        adc #300
        jsr pk_z
@l:     tya
        sec
        sbc #WB_UDP
        sta WCNT
        rts

; DHCP-Antwort in WB_UDP pruefen. C=1 wenn nicht fuer uns.
; NDTYP = Art, NDT = Adresse, Maske, Router, DNS; NSRV = Server
dh_parse:
        jsr net_ptr
        ldx #14
@z:     stz NDT,x
        dex
        dex
        bpl @z
        stz NDTYP
        ldy #WB_UDP
        lda [PSRC],y
        and #$00FF
        cmp #2
        beq *+5
        jmp @no
        ldy #WB_UDP+4           ; xid
        lda [PSRC],y
        cmp NXID
        beq *+5
        jmp @no
        iny
        iny
        lda [PSRC],y
        cmp NXID+2
        beq *+5
        jmp @no
        ldy #WB_UDP+16          ; yiaddr
        lda [PSRC],y
        sta NDT
        iny
        iny
        lda [PSRC],y
        sta NDT+2
        ldy #WB_UDP+240         ; Optionen
@op:    tya
        sec
        sbc #WB_UDP
        cmp WCNT
        bcs @end
        lda [PSRC],y
        and #$00FF
        beq @pad
        cmp #255
        beq @end
        sta NI
        iny
        lda [PSRC],y
        and #$00FF
        sta WT
        iny
        lda NI
        cmp #53
        bne @o1
        lda [PSRC],y
        and #$00FF
        sta NDTYP
        bra @nx
@o1:    ldx #NDT+4
        cmp #1
        beq @ip
        ldx #NDT+8
        cmp #3
        beq @ip
        ldx #NDT+12
        cmp #6
        beq @ip
        ldx #NSRV
        cmp #54
        bne @nx
@ip:    lda [PSRC],y            ; erste Adresse der Option
        sta a:0,x
        iny
        iny
        lda [PSRC],y
        sta a:2,x
        dey
        dey
@nx:    tya
        clc
        adc WT
        tay
        bra @op
@pad:   iny
        bra @op
@end:   lda NDTYP
        beq @no
        clc
        rts
@no:    sec
        rts

; ================= DNS =================
; Name WB_HOST (Laenge + Text) -> DIP. C=1 bei Fehler.
net_resolve:
        jsr net_ptr
        jsr dq_dotted           ; schon eine Zahlenadresse?
        bcs @dns
        rts
@dns:   lda NCARD
        cmp #NC_COGS
        bne @w0
        jmp cg_resolve
@w0:    cmp #NC_MARI
        bne @w1
        jmp mr_resolve
@w1:    cmp #NC_FUJI            ; FujiNet und Modem: Name geht an die Karte
        beq @nm
        cmp #NC_MODEM
        bne @w
@nm:    stz DIP
        stz DIP+2
        clc
        rts
@w:     jsr dc_find             ; schon bekannt?
        bcs @m
        rts
@m:     jsr dns_any
        bcs @r
        jsr net_ptr
        jsr dc_put
        clc
@r:     rts

; Zwischenspeicher fuer Namen (4 Eintraege)
dc_find:
        ldy #WB_HOST
        lda [PSRC],y
        and #$00FF
        sta DCL
        cmp #32
        bcs @no
        stz DCI
@e:     lda DCI
        asl
        asl
        asl
        asl
        asl
        tax
        lda DNCN,x
        and #$00FF
        cmp DCL
        bne @nx
        sta DCK
        ldy #WB_HOST+1
@c:     lda DCK
        beq @hit
        lda [PSRC],y
        eor DNCN+1,x
        and #$00FF
        bne @nx
        iny
        inx
        dec DCK
        bra @c
@hit:   lda DCI
        asl
        asl
        tax
        lda DNCI,x
        sta DIP
        lda DNCI+2,x
        sta DIP+2
        clc
        rts
@nx:    inc DCI
        lda DCI
        cmp #4
        bcc @e
@no:    sec
        rts
dc_put: lda DCL
        beq @r
        cmp #32
        bcs @r
        lda DCNX
        asl
        asl
        tax
        lda DIP
        sta DNCI,x
        lda DIP+2
        sta DNCI+2,x
        lda DCNX
        asl
        asl
        asl
        asl
        asl
        tax
        lda DCNX
        inc a
        and #3
        sta DCNX
        ldy #WB_HOST
        lda DCL
        sta DCK
        inc DCK
@c:     lda [PSRC],y
        sep #$20
        sta DNCN,x
        rep #$20
        iny
        inx
        dec DCK
        bne @c
@r:     rts
dc_clear:
        ldx #126
@c:     stz DNCN,x
        dex
        dex
        bpl @c
        stz NDNSTCP
        rts

; DNS ueber UDP (2 Versuche), sonst TCP; nach einem TCP-Erfolg gleich TCP
dns_any:
        lda NDNSTCP
        beq @u
        jmp dns_tcp
@u:     jsr udp_sel
        jsr s_rport
        lda #2
        jsr s_open
        bcc @o
        lda #NE_DNS
        sta NERR
        rts
@o:     lda #2
        sta NTRY
@try:   ldx #2
@cd:    lda NCUR+12,x
        sta DIP,x
        dex
        dex
        bpl @cd
        lda #53
        sta DPORT
        jsr dns_build
        jsr udp_send
@wt:    lda #120
        jsr udp_wait
        bcs @again
        jsr dns_parse
        bcs @wt
        lda #$10
        jsr s_cmd
        lda NERR
        cmp #NE_DNS
        bne @ok
        sec
        rts
@ok:    clc
        rts
@again: lda NABORT
        bne @fail
        dec NTRY
        bne @try
        lda #$10                ; UDP aufgeben, DNS ueber TCP versuchen
        jsr s_cmd
        jsr dns_tcp
        bcs @x
        lda #1                  ; ab jetzt gleich TCP
        sta NDNSTCP
        clc
@x:     rts
@fail:  lda #$10
        jsr s_cmd
        sec
        rts

; DNS-Anfrage ueber TCP (Port 53): 2 Bytes Laenge + Anfrage. C=1 bei Fehler
dns_tcp:
        ldx #2
@cd:    lda NCUR+12,x
        sta DIP,x
        dex
        dex
        bpl @cd
        lda #53
        sta DPORT
        jsr dns_build           ; Anfrage nach WB_UDP, Laenge WCNT
        jsr net_ptr
        lda WCNT                ; 2 Bytes nach hinten schieben
        clc
        adc #WB_UDP-1
        tay
@mv:    BRD8
        iny
        iny
        BWR8
        dey
        dey
        dey
        cpy #WB_UDP-1
        bne @mv
        ldy #WB_UDP
        lda WCNT
        xba
        sta [PSRC],y            ; Laenge, hoeherwertiges Byte zuerst
        inc WCNT
        inc WCNT
        jsr tcp_open
        bcs @bad
        lda #WB_UDP
        sta WPTR
        jsr tcp_write
        bcs @bad2
        lda #WB_UDP
        sta WPTR
        lda #WB_UDP+$600
        sta NLIM
        inc NQUIET
        jsr tcp_readall
        php
        stz NQUIET
        jsr tcp_close
        plp
        bcs @bad
        jsr net_ptr             ; Laenge entfernen
        ldy #WB_UDP+2
@dn:    cpy WPTR
        bcs @pd
        BRD8
        dey
        dey
        BWR8
        iny
        iny
        iny
        bra @dn
@pd:    jsr dns_parse
        bcs @bad
        lda NERR
        cmp #NE_DNS
        beq @bad
        clc
        rts
@bad2:  jsr tcp_close
@bad:   lda NABORT
        bne @ab
        lda #NE_DNS
        sta NERR
@ab:    sec
        rts

; Zahlenadresse a.b.c.d in WB_HOST -> DIP, C=0; sonst C=1
dq_dotted:
        ldy #WB_HOST
        lda [PSRC],y
        and #$00FF
        beq @no
        clc
        adc #WB_HOST+1
        sta NTOP
        iny
        stz NI                  ; Byte-Nr.
@n:     stz WT
        stz NJ                  ; Ziffern
@d:     cpy NTOP
        bcs @e
        lda [PSRC],y
        and #$00FF
        cmp #'.'
        beq @e
        sec
        sbc #'0'
        cmp #10
        bcs @no
        sta WT2
        lda WT
        asl
        asl
        clc
        adc WT
        asl
        clc
        adc WT2
        sta WT
        cmp #256
        bcs @no
        inc NJ
        iny
        bra @d
@e:     lda NJ
        beq @no
        ldx NI
        cpx #4
        bcs @no
        sep #$20
        lda WT
        sta DIP,x
        rep #$20
        inc NI
        cpy NTOP
        bcs @end
        iny                     ; Punkt
        bra @n
@end:   lda NI
        cmp #4
        bne @no
        clc
        rts
@no:    sec
        rts

; DNS-Anfrage (Typ A) fuer WB_HOST nach WB_UDP
dns_build:
        jsr net_ptr
        jsr t_now
        eor NRNG
        sta NQID
        ldy #WB_UDP
        lda NQID
        jsr pk_b
        lda NQID
        xba
        jsr pk_b
        lda #$01                ; Rekursion erwuenscht
        jsr pk_b
        lda #0
        jsr pk_b
        lda #0
        jsr pk_b
        lda #1                  ; 1 Frage
        jsr pk_b
        lda #6
        jsr pk_z
        ; Name: Teile mit Laengenbyte
        ldx #WB_HOST
        txa
        sta NI                  ; Leseposition (Laengenbyte)
        phy
        ldy NI
        lda [PSRC],y
        ply
        and #$00FF
        clc
        adc #WB_HOST+1
        sta NTOP
        inc NI
@lab:   sty NJ                  ; Stelle des Laengenbytes
        lda #0
        jsr pk_b
        stz WT3
@c:     lda NI
        cmp NTOP
        bcs @le
        phy
        ldy NI
        lda [PSRC],y
        ply
        inc NI
        and #$00FF
        cmp #'.'
        beq @le
        jsr pk_b
        inc WT3
        bra @c
@le:    phy
        ldy NJ
        lda WT3
        sep #$20
        sta [PSRC],y
        rep #$20
        ply
        lda NI
        cmp NTOP
        bcc @lab
        lda #0
        jsr pk_b
        lda #0
        jsr pk_b
        lda #1                  ; Typ A
        jsr pk_b
        lda #0
        jsr pk_b
        lda #1                  ; Klasse IN
        jsr pk_b
        tya
        sec
        sbc #WB_UDP
        sta WCNT
        rts

; Name ab Y ueberspringen
dn_skip:
@l:     lda [PSRC],y
        and #$00FF
        beq @z
        cmp #$C0
        bcs @p
        sec                     ; Laenge + 1
        adc #0
        sta WT
        tya
        clc
        adc WT
        tay
        cpy #WB_UDP+$600
        bcc @l
        rts
@z:     iny
        rts
@p:     iny
        iny
        rts

; DNS-Antwort in WB_UDP auswerten. C=1: nicht unsere Antwort (weiter warten).
; Sonst C=0 und DIP gesetzt oder NERR = NE_DNS.
dns_parse:
        jsr net_ptr
        ldy #WB_UDP
        lda [PSRC],y
        cmp NQID
        beq @id
        sec
        rts
@id:    stz NERR
        ldy #WB_UDP+3
        lda [PSRC],y
        and #$000F              ; Fehlercode
        bne @bad
        ldy #WB_UDP+6
        lda [PSRC],y
        xba
        sta NI                  ; Antworten
        beq @bad
        ldy #WB_UDP+12
        jsr dn_skip
        iny
        iny
        iny
        iny
@ans:   lda NI
        beq @bad
        dec NI
        jsr dn_skip
        lda [PSRC],y            ; Typ
        xba
        sta WT2
        tya
        clc
        adc #8
        tay
        lda [PSRC],y            ; Laenge der Daten
        xba
        sta WT3
        iny
        iny
        lda WT2
        cmp #1
        bne @nx
        lda WT3
        cmp #4
        bne @nx
        lda [PSRC],y
        sta DIP
        iny
        iny
        lda [PSRC],y
        sta DIP+2
        clc
        rts
@nx:    tya
        clc
        adc WT3
        tay
        cpy #WB_UDP+$600
        bcc @ans
@bad:   lda #NE_DNS
        sta NERR
        clc
        rts

; ================= TCP (Socket 0) =================
tcp_sel:
        lda NSOCK               ; 1: FTP-Daten auf Socket 1 (2 KB)
        bne @s1
        lda #S0
        sta WSB
        lda #TX0
        sta WRBASE
        sta WTXB
        lda #RX0
        sta WRXB
        lda #$0FFF
        sta WRM
        rts
@s1:    lda #S1
        sta WSB
        lda #TX1
        sta WRBASE
        sta WTXB
        lda #RX1
        sta WRXB
        lda #$07FF
        sta WRM
        rts

; Verbindung zu DIP:DPORT. C=1 bei Fehler (NERR)
tcp_open:
        lda NCARD
        cmp #NC_COGS
        bcc @cw
        asl
        tax
        jmp (@tb-4,x)
@tb:    .addr cg_open, fn_open, mr_open, md_open
@cw:    jsr tcp_sel
        jsr s_rport
        lda #1
        jsr s_open
        bcc @o
        lda #NE_CONN
        sta NERR
        rts
@o:     jsr s_dest
        lda #$04                ; CONNECT
        jsr s_cmd
        jsr tm_start
@w:     jsr net_esc
        bcs @fail
        jsr tcp_sel
        lda #SN_SR
        jsr s_rd8
        cmp #$17                ; ESTABLISHED
        beq @ok
        cmp #$1C                ; CLOSE_WAIT: schon Daten und Ende
        beq @ok
        cmp #$00
        beq @ref
        jsr t_elapsed
        cmp #900                ; 15 s
        bcc @w
        lda #NE_TIME
        sta NERR
        bra @fail
@ref:   lda #NE_CONN
        sta NERR
@fail:  jsr tcp_close
        sec
        rts
@ok:    lda #1                  ; vor der ersten Anfrage Reste verwerfen
        sta NDISC
        clc
        rts

; WCNT Bytes ab WPTR senden (hoechstens 4 KB)
tcp_write:
        lda NCARD
        cmp #NC_COGS
        bcc @cw
        asl
        tax
        jmp (@tb-4,x)
@tb:    .addr cg_write, fn_write, mr_write, md_write
@cw:    jsr tcp_discard
        jsr tcp_sel
        jsr tm_start
@w:     lda #SN_FSR
        jsr s_rd16
        cmp WCNT
        bcs @go
        jsr net_esc
        bcs @r
        jsr t_elapsed
        cmp #600
        bcc @w
        lda #NE_TIME
        sta NERR
        sec
@r:     rts
@go:    lda WTXB                ; Sendebereich (tcp_poll/readall stellen RX0 ein)
        sta WRBASE
        jsr s_send
        clc
        rts

; alles bis zum Verbindungsende lesen: nach WPTR, hoechstens bis NLIM.
; Nach jedem Stueck JSLF xf_web_prog. C=1 bei Fehler (NERR); WPTR = Ende.
tcp_readall:
        lda NCARD
        cmp #NC_COGS
        bcc @cw
        asl
        tax
        jmp (@tb-4,x)
@tb:    .addr cg_readall, fn_readall, mr_readall, md_readall
@cw:    stz NFULL
        jsr tm_start
@l:     jsr net_esc
        bcc @n
        jmp @fail
@n:     jsr tcp_sel
        jsr s_rsr
        cmp #0
        beq @empty
        sta WCNT
        lda NLIM
        sec
        sbc WPTR
        beq @full
        cmp WCNT
        bcs @fits
        sta WCNT
@fits:  lda WRXB
        sta WRBASE
        jsr s_recv
        JSLF xf_web_prog
        jsr tm_start
        bra @l
@full:  lda NSTRM               ; Puffer voll: Rest verwerfen oder spaeter weiterlesen
        sta NFULL
        clc
        rts
@empty: lda #SN_SR
        jsr s_rd8
        cmp #$17
        bne @done               ; Server hat beendet
        jsr t_elapsed
        cmp #1200               ; 20 s ohne Daten
        bcc @l
        lda #NE_TIME
        sta NERR
@fail:  sec
        rts
@done:  jsr s_rsr               ; noch Reste?
        cmp #0
        bne @l
        clc
        rts

; Ohne Warten: was da ist nach WPTR..NLIM holen. C=1 wenn die Verbindung zu ist
tcp_poll:
        lda NCARD
        cmp #NC_COGS
        bcc @w
        asl
        tax
        jmp (@tb-4,x)
@tb:    .addr cg_poll, fn_poll, mr_poll, md_poll
@w:     jsr tcp_sel
        jsr s_rsr
        cmp #0
        beq @e
        sta WCNT
        lda NLIM
        sec
        sbc WPTR
        beq @ok
        cmp WCNT
        bcs @f
        sta WCNT
@f:     lda WRXB
        sta WRBASE
        jsr s_recv
@ok:    clc
        rts
@e:     lda #SN_SR
        jsr s_rd8
        cmp #$17
        bne @cl
        clc
        rts
@cl:    sec
        rts

tcp_close:
        lda NCARD
        cmp #NC_COGS
        bcc @cw
        asl
        tax
        jmp (@tb-4,x)
@tb:    .addr cg_close, fn_close, mr_close, md_close
@cw:    jsr tcp_sel
        lda #$08                ; DISCON
        jsr s_cmd
        lda #$10                ; CLOSE
        jsr s_cmd
        lda NSOCK               ; FTP: danach auch die Steuerverbindung
        beq @c0
        stz NSOCK
        bra @cw
@c0:    jsr tm_start            ; kurz warten: GSSquared liefert sonst noch Daten und
@st:    jsr tcp_sel             ; Zustaende der alten Verbindung in die naechste
        lda #SN_SR
        jsr s_rd8
        jsr t_elapsed
        cmp #6                  ; 0,1 s
        bcc @st
        rts

; Vor der ersten Anfrage: alles verwerfen, was schon im Empfangspuffer liegt
; (kann nur von einer alten Verbindung stammen). NDISC setzt tcp_open.
tcp_discard:
        lda NDISC
        beq @r
        stz NDISC
        jsr tcp_sel
        jsr s_rsr
        cmp #0
        beq @r
        sta WCNT
        lda #SN_RXRD
        jsr s_rd16
        clc
        adc WCNT
        sta WV
        lda #SN_RXRD
        jsr s_wr16
        lda #$40                ; RECV
        jsr s_cmd
@r:     rts

; ================= Text zeichnen (mit Umlauten) =================
; Zeichensatz: $20..$7E wie ASCII, $80..$87 = ae oe ue Ae Oe Ue ss Punkt
; Zeichen A -> Zeiger auf das Glyphen-Sprite
gl_ptr: and #$00FF
        cmp #$80
        bcs @x
        cmp #$20
        bcc @q
        cmp #$7F
        bcs @q
        sec
        sbc #$20
        asl
        tax
        lda font_tab,x
        rts
@x:     cmp #$88
        bcs @q
        sec
        sbc #$80
        asl
        tax
        lda font_ext,x
        rts
@q:     lda #'?'-$20
        asl
        tax
        lda font_tab,x
        rts

; WDN Zeichen ab [PDST] an SX/SY (Farbe gesetzt); WDBOLD = fett
        .pushseg
        .segment "BSS"
WDN:     .res 2
WDI:     .res 2
WDBOLD:  .res 2
        .popseg
wd_chars:
        stz WDI
@l:     lda WDI
        cmp WDN
        bcs @r
        tay
        lda [PDST],y
        jsr gl_ptr
        sta SPR
        CALL blit1
        lda WDBOLD
        beq @nb
        inc SX
        CALL blit1
@nb:    lda SX
        sec
        adc W
        sta SX
        inc WDI
        bra @l
@r:     rts

; Zeichenkette (Laengenbyte + Text) an Adresse A zeichnen:
; ui_str = in dieser Codebank, ui_strm = in der Hauptbank
ui_str: sta PDST
        jsr pbank
        sta PDST+2
        bra ui_s2
ui_strm:
        sta PDST
        lda MYBANK
        sta PDST+2
ui_s2:  lda [PDST]
        and #$00FF
        sta WDN
        inc PDST
        stz WDBOLD
        jmp wd_chars

; Bank der Zeichenketten (webstr.inc, Codebank des Browsers) -> A
pbank:  lda STRBK
        rts

; Breite der Zeichenkette an A (diese Codebank) -> A
ui_width:
        sta PDST
        jsr pbank
        sta PDST+2
ui_wid2:
        lda [PDST]
        and #$00FF
        sta WT3
        stz WT2
        ldy #1
@l:     lda WT3
        beq @r
        lda [PDST],y
        phy
        jsr gl_ptr
        sta LP2
        lda ABANK
        sta LP2+2
        lda [LP2]
        and #$00FF
        sec
        adc WT2
        sta WT2
        ply
        iny
        dec WT3
        bra @l
@r:     lda WT2
        rts

; Adresse ab X (4 Bytes, Hauptbank) als a.b.c.d an SX/SY
        .pushseg
        .segment "BSS"
IPB:     .res 20
        .popseg
put_ip: jsr ip_text
        lda #IPB
        jmp ui_strm
; 4 Bytes ab X -> Text in IPB (Laengenbyte + "a.b.c.d")
ip_text:
        stx WT
        ldy #1
        stz NI
@b:     ldx WT
        lda a:0,x
        and #$00FF
        sta WT2
        cmp #100
        bcc @t
        ldx #100
        jsr dig
        ldx #10                 ; nach Hundertern immer die Zehnerstelle
        jsr dig
        bra @o2
@t:     cmp #10
        bcc @o2
        ldx #10
        jsr dig
@o2:    lda WT2
        clc
        adc #'0'
        sep #$20
        sta IPB,y
        rep #$20
        iny
        inc WT
        inc NI
        lda NI
        cmp #4
        bcs @e
        lda #'.'
        sep #$20
        sta IPB,y
        rep #$20
        iny
        bra @b
@e:     tya
        dec a
        sep #$20
        sta IPB
        rep #$20
        rts
; Stelle WT2 / X als Ziffer anhaengen, Rest in WT2
dig:    stx WT3
        lda WT2
        ldx #0
@l:     cmp WT3
        bcc @d
        sbc WT3
        inx
        bra @l
@d:     sta WT2
        txa
        clc
        adc #'0'
        sep #$20
        sta IPB,y
        rep #$20
        iny
        rts

        .include "cogs.inc"
        .include "fujinet.inc"
        .include "mari.inc"
        .include "modem.inc"
        .include "ntp.inc"
        .include "netdlg.inc"
        .include "x3ext.inc"
        .include "cd.inc"
        .include "pfilt.inc"
        .include "xfer.inc"
        .include "calc.inc"
        .include "sol.inc"

FARMODE .set 0
        .segment "CODE"
