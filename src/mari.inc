; ================================================================
; GS/OS95: Netzwerk ueber Marinetti (TCP/IP-Toolset $36, Teil von net.s)
; Marinetti bringt die Link Layer fuer Uthernet I (CS8900A), LANceGS,
; Uthernet II, SLIP/PPP ueber Modem, MacIP und die Emulatoren mit.
; GS/OS95 benutzt nur die TCP-Aufrufe: Login/OpenTCP, WriteTCP, ReadTCP,
; StatusTCP, CloseTCP/Logout und die Namensaufloesung (DNR).
; Eine Verbindung (MRIPID), wie bei den anderen Karten Socket 0.
; ================================================================
        .pushseg
        .segment "BSS"
MRIPID: .res 2                  ; ipid der offenen Verbindung (0 = keine)
MRSTART:.res 2                  ; 1 = TCPIPStartUp von uns
MRWANT: .res 2                  ; 1 = notfalls verbinden (TCPIPConnect)
MRSR:   .res 22                 ; srBuff von TCPIPStatusTCP
MRRR:   .res 14                 ; rrBuff von TCPIPReadTCP
MRDNR:  .res 6                  ; dnrBuffer: Status, IP-Adresse
MRDNS:  .res 8                  ; DNS-Server (Haupt, Zusatz)
MRN:    .res 2
        .popseg

MR_ESTAB = 4                    ; tcpsESTABLISHED
MR_CLOSEWAIT = 7

; ---------- Marinetti da (und verbunden)? A=0: nur wenn schon verbunden,
; A=1: notfalls TCPIPConnect. C=0 ja (NCUR = eigene Adresse, DNS)
mr_probe:
        sta MRWANT
        pha                     ; TCPIPStatus: aktiv?
        TOOL $0636
        bcc @ld
        pla                     ; Toolset nicht geladen
        sec
        rts
@ld:    pla
        bne @act
        lda MRWANT              ; geladen, aber nicht gestartet
        bne @st
        sec
        rts
@st:    TOOL $0236              ; TCPIPStartUp
        bcc @st1
        sec
        rts
@st1:   lda #1
        sta MRSTART
@act:   pha                     ; TCPIPGetConnectStatus
        TOOL $0936
        pla
        bne @con
        lda MRWANT
        bne @cn
        sec
        rts
@cn:    pea $0000               ; TCPIPConnect (displayPtr = nil: eigene Meldungen von Marinetti)
        pea $0000
        TOOL $1236
        pha
        TOOL $0936
        pla
        bne @con
        sec
        rts
@con:   pha                     ; eigene Adresse
        pha
        TOOL $0F36              ; TCPIPGetMyIPAddress
        pla
        sta NCUR
        pla
        sta NCUR+2
        lda MYBANK              ; DNS-Server
        pha
        pea MRDNS
        TOOL $1C36              ; TCPIPGetDNS
        lda MRDNS
        sta NCUR+12
        lda MRDNS+2
        sta NCUR+14
        stz NCUR+4              ; Maske, Gateway: unbekannt
        stz NCUR+6
        stz NCUR+8
        stz NCUR+10
        stz MRIPID
        clc
        rts

; Marinetti arbeiten lassen
mr_poll1:
        TOOL $2236              ; TCPIPPoll
        rts

; ---------- Name WB_HOST -> DIP (DNR von Marinetti) ----------
mr_resolve:
        stz MRDNR
        lda NWB                 ; Zeiger auf den Namen (Laengenbyte + Text)
        pha
        pea WB_HOST
        lda MYBANK
        pha
        pea MRDNR
        TOOL $2136              ; TCPIPDNRNameToIP
        bcs @bad
        jsr tm_start
@w:     jsr mr_poll1
        lda MRDNR
        bne @done
        jsr net_esc
        bcs @cancel
        jsr t_elapsed
        cmp #900                ; 15 s
        bcc @w
@cancel:
        lda MYBANK
        pha
        pea MRDNR
        TOOL $2036              ; TCPIPCancelDNR
        lda NABORT
        bne @ab
        bra @bad
@done:  cmp #1                  ; DNR_OK
        bne @bad
        lda MRDNR+2
        sta DIP
        lda MRDNR+4
        sta DIP+2
        clc
        rts
@bad:   lda #NE_DNS
        sta NERR
@ab:    sec
        rts

; Zustand der Verbindung -> MRSR (C=1 bei Fehler)
mr_stat:
        pha
        lda MRIPID
        pha
        lda MYBANK
        pha
        pea MRSR
        TOOL $3136              ; TCPIPStatusTCP
        pla
        bcs @e
        cmp #0
        bne @e
        clc
        rts
@e:     sec
        rts

; ---------- Verbindung zu DIP:DPORT ----------
mr_open:
        lda MRIPID              ; alte Verbindung noch offen?
        beq @l
        jsr mr_close
@l:     pha                     ; TCPIPLogin
        lda USERID
        pha
        lda DIP+2
        pha
        lda DIP
        pha
        lda DPORT
        pha
        pea $0000               ; TOS
        pea $0040               ; TTL 64
        TOOL $2336
        pla
        bcc @lo
        lda #NE_CONN
        sta NERR
        sec
        rts
@lo:    sta MRIPID
        pha                     ; TCPIPOpenTCP
        lda MRIPID
        pha
        TOOL $2C36
        pla
        bcs @ref
        cmp #0
        bne @ref
        jsr tm_start
@w:     jsr mr_poll1
        jsr mr_stat
        bcs @ref
        lda MRSR
        cmp #MR_ESTAB
        beq @ok
        cmp #MR_CLOSEWAIT       ; schon Daten und Ende
        beq @ok
        cmp #0                  ; wieder geschlossen: abgelehnt
        beq @ref
        jsr net_esc
        bcs @fail
        jsr t_elapsed
        cmp #900                ; 15 s
        bcc @w
        lda #NE_TIME
        sta NERR
        bra @fail
@ref:   lda #NE_CONN
        sta NERR
@fail:  jsr mr_close
        sec
        rts
@ok:    clc
        rts

; ---------- WCNT Bytes ab WPTR (Pufferbank) senden ----------
mr_write:
        pha
        lda MRIPID
        pha
        lda NWB                 ; dataPtr
        pha
        lda WPTR
        pha
        pea $0000               ; dataLength (lang)
        lda WCNT
        pha
        pea $0001               ; pushFlag
        pea $0000               ; urgentFlag
        TOOL $2D36              ; TCPIPWriteTCP
        pla
        bcs @e
        cmp #0
        bne @e
        jsr mr_poll1
        clc
        rts
@e:     lda #NE_CONN
        sta NERR
        sec
        rts

; was da ist nach WPTR..NLIM lesen -> MRN Bytes (0 = nichts). C=1 bei Fehler
mr_read1:
        stz MRN
        lda NLIM
        sec
        sbc WPTR
        bne @g
        clc
        rts
@g:     sta MRN
        pha
        lda MRIPID
        pha
        pea $0000               ; buffType: Zeiger
        lda NWB
        pha
        lda WPTR
        pha
        pea $0000               ; buffLen (lang)
        lda MRN
        pha
        lda MYBANK
        pha
        pea MRRR
        TOOL $2E36              ; TCPIPReadTCP
        pla
        stz MRN
        bcs @e
        lda MRRR                ; rrBuffCount
        sta MRN
        clc
        adc WPTR
        sta WPTR
        clc
        rts
@e:     sec
        rts

; ---------- alles bis zum Ende lesen (nach WPTR, hoechstens bis NLIM) ----------
mr_readall:
        stz NFULL
        jsr tm_start
@l:     jsr net_esc
        bcc @n
        jmp @fail
@n:     jsr mr_poll1
        jsr mr_stat
        bcs @fail0
        lda MRSR+8              ; srRcvQueued
        ora MRSR+10
        beq @empty
        lda NLIM
        cmp WPTR
        beq @full
        jsr mr_read1
        bcs @fail0
        lda MRN
        beq @l
        lda NQUIET
        bne @q
        JSLF xf_web_prog
@q:     jsr tm_start
        bra @l
@full:  lda NSTRM               ; Puffer voll: Rest spaeter oder verwerfen
        sta NFULL
        clc
        rts
@empty: lda MRSR
        cmp #MR_ESTAB
        bne @done               ; Gegenseite hat beendet
        jsr t_elapsed
        cmp #1200               ; 20 s ohne Daten
        bcc @l
        lda #NE_TIME
        sta NERR
        bra @fail
@fail0: lda #NE_CONN
        sta NERR
@fail:  sec
        rts
@done:  clc
        rts

; ---------- ohne Warten: vorhandene Daten holen. C=1: Verbindung zu ----------
mr_poll:
        jsr mr_poll1
        jsr mr_stat
        bcs @cl
        lda MRSR+8
        ora MRSR+10
        beq @e
        jsr mr_read1
        bcs @cl
        clc
        rts
@e:     lda MRSR
        cmp #MR_ESTAB
        bne @cl
        clc
        rts
@cl:    sec
        rts

; ---------- Verbindung schliessen ----------
mr_close:
        lda MRIPID
        beq @r
        pha
        lda MRIPID
        pha
        TOOL $2F36              ; TCPIPCloseTCP
        pla
        jsr tm_start            ; kurz warten, bis Marinetti das Ende ausgehandelt hat
@w:     jsr mr_poll1
        jsr mr_stat
        bcs @lo
        lda MRSR
        beq @lo                 ; CLOSED
        cmp #10                 ; TIMEWAIT
        beq @lo
        jsr t_elapsed
        cmp #60                 ; 1 s
        bcc @w
        pha                     ; dann eben abbrechen
        lda MRIPID
        pha
        TOOL $3036              ; TCPIPAbortTCP
        pla
@lo:    lda MRIPID
        pha
        TOOL $2436              ; TCPIPLogout
        stz MRIPID
@r:     rts
