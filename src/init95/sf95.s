; ================================================================
; sf95.s - Oeffnen/Sichern-Dialoge im Win95-Stil (Teil von GSOS95.INIT)
; Ersetzt SFGetFile2 ($0E17) und SFPutFile2 ($0F17) der Standard-File-
; Toolbox. Mit gedrueckter Option-Taste erscheint Apples Originaldialog.
; Eigene Direct Page (sfdp), damit die Rahmenprozedur des INIT, die
; waehrend des Dialogs aufgerufen wird, nichts ueberschreibt.
; ================================================================

; ---- Direct Page des Dialogs ----
SP0   = $00     ; 4 allgemeiner Zeiger
SP1   = $04     ; 4
LP    = $08     ; 4 Eintragsliste (Zeiger)
LH    = $0C     ; 4 Eintragsliste (Handle)
REPLY = $10     ; 4 Antwortsatz
TLIST = $14     ; 4 Typliste
FILT  = $18     ; 4 Filterprozedur
PRREF = $1C     ; 4 Prompt
PRDSC = $20
ONDSC = $22     ; Vorgabename (Sichern)
ONREF = $24     ; 4
MODE  = $28     ; 0 Oeffnen, 1 Sichern
LANGF = $2A     ; 0 deutsch, 1 englisch
XSC   = $2C     ; 1 (320) / 2 (640)
NENT  = $2E     ; Zahl der Eintraege
TOPI  = $30     ; erster sichtbarer Eintrag
SEL   = $32     ; gewaehlter Eintrag (sortierte Nummer), $FFFF = keiner
WIN   = $34     ; 4 Dialogfenster
OLDP  = $38     ; 4 vorheriger Port
CLG   = $3C     ; Inhalt links (global)
CTG   = $3E     ; Inhalt oben (global)
QX    = $40     ; Rechteck in 320er-Einheiten
QY    = $42
QW    = $44
QH    = $46
CWH   = $48     ; Farben
CBK   = $4A
CGR   = $4C
CDK   = $4E
CSE   = $50
CYE   = $52
U0    = $54
U1    = $56
U2    = $58
U3    = $5A
LASTW = $5C     ; 4 Zeit des letzten Klicks
LASTI = $60     ; Eintrag des letzten Klicks
DBLT  = $62     ; 4 Doppelklickzeit
FOC   = $66     ; 1: Tastatur im Namensfeld (Sichern)
REPL  = $68     ; 1: naechste Taste ersetzt den Namen
ERRC  = $6A
MX    = $6C     ; Maus lokal
MY    = $6E
DONE  = $70     ; 0 weiter, 1 OK, 2 Abbrechen
I0    = $72
I1    = $74
TYP   = $76
AUXL  = $78     ; 4
FLG   = $7C
RES1  = $7E
SVDP  = $80     ; DP des Aufrufers (fuer Filterprozeduren)
SVDB  = $82     ; Datenbank des Aufrufers
BTN   = $84
HIT   = $86
MONO  = $88     ; 1: Grau = Weiss (zu wenige Farben) -> schwarze Rahmen
LBW   = $8C     ; linker Rand der Felder (nach den Beschriftungen)
OLDF  = $8A     ; 1: alte Aufrufe SFGetFile/SFPutFile (Pascal-Strings)

MAXE  = 160     ; Eintraege
ESZ   = 72      ; Bytes je Eintrag: Flags, Typ, Zusatztyp(4), Name(1+63)
NROWS = 8       ; sichtbare Zeilen
ROWH  = 10
; Layout (320er Einheiten, Inhalt lokal)
DW    = 300
DH    = 140
LX0   = 6       ; Liste
LY0   = 20
LX1   = 294
LY1   = LY0+NROWS*ROWH+2
SBX   = LX1-12  ; Rollbalken

; Eintragsflags
E_DIR = $0001
E_NOS = $0002   ; nicht waehlbar
E_VOL = $0004

; ---------- Einstieg aus dem Dispatcher ----------
sfget2: lda #0
        bra sfgo
sfput2: lda #1
        bra sfgo
; alte Aufrufe: mit eigener Filterprozedur bleibt es bei Apples Dialog
sfget1: lda 12,s                ; Filter
        ora 14,s
        beq @own
        jmp jmlorig
@own:   lda #2
        bra sfgo
sfput1: lda #3
sfgo:   php
        rep #$30
        pha
        sep #$20
        .a8
        lda f:$E0C025           ; Option gedrueckt: Apples Dialog
        rep #$20
        .a16
        and #$0040
        beq @own
        pla
        plp
        jmp jmlorig
@own:   pla
        phb
        phd
        phk
        plb
        pha
        jsr dps
        tcd
        pla
        pha
        and #1
        sta MODE
        pla
        lsr
        sta OLDF
        lda 1,s
        sta SVDP
        lda 3,s
        and #$00FF
        sta SVDB
        ; Stapel: 1-2 D, 3 B, 4 P, 5-7 RTL, ab 8 Parameter
        lda OLDF
        beq @new
        lda MODE
        bne @op
        lda 8,s                 ; SFGetFile: Antwort, Typliste, Filter, Prompt
        sta REPLY
        lda 10,s
        sta REPLY+2
        lda 12,s
        sta TLIST
        lda 14,s
        sta TLIST+2
        stz FILT
        stz FILT+2
        lda 20,s
        sta PRREF
        lda 22,s
        sta PRREF+2
        stz PRDSC
        lda #20
        sta RES1
        jmp @run
@op:    lda 8,s                 ; SFPutFile: Antwort, maxLen, Name, Prompt
        sta REPLY
        lda 10,s
        sta REPLY+2
        lda 14,s
        sta ONREF
        lda 16,s
        sta ONREF+2
        lda #$80                ; Pascal-String
        sta ONDSC
        lda 18,s
        sta PRREF
        lda 20,s
        sta PRREF+2
        stz PRDSC
        stz TLIST
        stz TLIST+2
        stz FILT
        stz FILT+2
        lda #18
        sta RES1
        jmp @run
@new:   lda MODE
        bne @put
        lda 8,s                 ; SFGetFile2: Antwort, Typliste, Filter,
        sta REPLY               ; Prompt, Prompt-Art, y, x
        lda 10,s
        sta REPLY+2
        lda 12,s
        sta TLIST
        lda 14,s
        sta TLIST+2
        lda 16,s
        sta FILT
        lda 18,s
        sta FILT+2
        lda 20,s
        sta PRREF
        lda 22,s
        sta PRREF+2
        lda 24,s
        sta PRDSC
        lda #22
        sta RES1
        jmp @run
@put:   lda 8,s                 ; SFPutFile2: Antwort, Name, Name-Art,
        sta REPLY               ; Prompt, Prompt-Art, y, x
        lda 10,s
        sta REPLY+2
        lda 12,s
        sta ONREF
        lda 14,s
        sta ONREF+2
        lda 16,s
        sta ONDSC
        lda 18,s
        sta PRREF
        lda 20,s
        sta PRREF+2
        lda 22,s
        sta PRDSC
        stz TLIST
        stz TLIST+2
        stz FILT
        stz FILT+2
        lda #20
        sta RES1
@run:   jsr sfmain
        ldx ERRC
        ldy RES1
        pld
        plb
        plp
        rep #$30
        ; Ruecksprung ueber die Parameter (Y Bytes) schieben
        cpy #22
        beq @p22
        cpy #18
        bne @p20
        lda 2,s
        sta 20,s
        lda 1,s
        sta 19,s
        tsc
        clc
        adc #18
        tcs
        bra @ex
@p20:
        lda 2,s
        sta 22,s
        lda 1,s
        sta 21,s
        tsc
        clc
        adc #20
        tcs
        bra @ex
@p22:   lda 2,s
        sta 24,s
        lda 1,s
        sta 23,s
        tsc
        clc
        adc #22
        tcs
@ex:    txa
        cmp #1                  ; C=1 bei Fehler
        rtl

; ---------- Hauptablauf ----------
sfmain: stz ERRC
        stz DONE
        stz NENT
        stz LH
        stz LH+2
        stz WIN
        stz WIN+2
        lda #$FFFF
        sta SEL
        stz TOPI
        ; Antwort-Art pruefen
        lda REPLY
        ora REPLY+2
        bne @r1
        lda #$1704
        sta ERRC
        rts
@r1:    lda OLDF
        bne @r3
        ldy #8                  ; nameRefDesc
        lda [REPLY],y
        cmp #2
        bne @r2
        lda #$1704
        sta ERRC
        rts
@r2:    ldy #14                 ; pathRefDesc
        lda [REPLY],y
        cmp #2
        bne @r3
        lda #$1705
        sta ERRC
        rts
@r3:    lda #0                  ; good = FALSE
        sta [REPLY]
        jsr sfpbfix
        jsr rdlang
        ; Speicher fuer die Liste
        pha
        pha
        pea $0000
        pea MAXE*ESZ
        lda myid
        pha
        pea $C010               ; gesperrt, fest, keine Bankgrenze
        pea $0000
        pea $0000
        ldx #$0902              ; NewHandle
        jsl $E10000
        pla
        sta LH
        pla
        sta LH+2
        bcc @m1
        lda #$0201
        sta ERRC
        rts
@m1:    lda [LH]
        sta LP
        ldy #2
        lda [LH],y
        sta LP+2
        ; Startordner: Prefix 8, sonst 0
        lda #8
        jsr getpfx
        lda pathb
        bne @hp
        lda #0
        jsr getpfx
@hp:    jsr pathstrip
        ; Vorgabename (Sichern)
        stz namef
        lda MODE
        beq @nn
        jsr orignm
        lda #1
        sta FOC
        sta REPL
@nn:    jsr sfcolors
        jsr mkwin
        bcc @w1
        lda #$0201
        sta ERRC
        bra @free
@w1:    jsr readdir
        jsr drawall
        jsr evloop
        jsr closewin
        lda DONE
        cmp #1
        bne @free
        jsr fillreply
@free:  lda LH
        ora LH+2
        beq @x
        lda LH+2
        pha
        lda LH
        pha
        ldx #$1002              ; DisposeHandle
        jsl $E10000
@x:     rts

; Zeiger in den Parameterbloecken auf unsere Bank setzen
sfpbfix:
        lda mybank
        sta gpfx_pb+6
        sta op_pb+6
        sta de_pb+12
        sta di_pb+6
        sta vo_pb+4
        sta vo_pb+8
        sta cr_pb+4
        sta sp_pb+6
        sta pf_pb+6
        sta pr_pb+6
        rts

; Sprache aus *:System:GSOS95.PREFS (Byte 0, Bit 0: 1 = englisch)
rdlang: stz LANGF
        lda #.loword(prefsnm)
        sta pf_pb+4
        lda #$2010              ; Open
        ldx #pf_pb
        jsr gsos
        bcs @x
        lda pf_pb+2
        sta pr_pb+2
        sta cl_pb+2
        lda #$2012              ; Read
        ldx #pr_pb
        jsr gsos
        php
        lda #$2014              ; Close
        ldx #cl_pb
        jsr gsos
        plp
        bcs @x
        lda prbuf
        and #1
        sta LANGF
@x:     rts

; GS/OS-Aufruf (Stapel): A = Nummer, X = Parameterblock in unserer Bank
gsos:   sta gsnum
        lda mybank
        pha
        phx
        lda gsnum
        pha
        jsl $E100B0
        rts

; Prefix A nach pathb (ohne letzten Trenner); leer bei Fehler
getpfx: sta gpfx_pb+2
        lda #.loword(pathr)
        sta gpfx_pb+4
        lda #516
        sta pathr
        stz pathb
        lda #$200A              ; GetPrefix
        ldx #gpfx_pb
        jsr gsos
        bcs @x
        rts
@x:     stz pathb
        rts
; pathb (Ergebnis in pathr+2) als Klasse-1-String; Endtrenner weg
pathstrip:
        ; pathr: Puffergroesse, dann Laenge + Zeichen = pathb
        lda pathb
        beq @x
        tax
        sep #$20
        .a8
        lda pathb+1,x
        cmp #':'
        beq @c
        cmp #'/'
        bne @n
@c:     rep #$20
        .a16
        dec pathb
        rts
@n:     rep #$20
        .a16
@x:     rts

; Vorgabename (GS/OS-Klasse-1-String) nach namef (Pascal, max. 63)
orignm: stz namef
        lda ONDSC
        cmp #$80
        bne @g
        lda ONREF               ; alter Aufruf: Pascal-String
        sta SP0
        lda ONREF+2
        sta SP0+2
        ora SP0
        beq @x
        lda [SP0]
        and #$00FF
        cmp #16
        bcc *+5
        lda #15
        sta U0
        sep #$20
        .a8
        sta namef
        ldy #1
@pl:    cpy U0
        beq @pe
        bcs @pe
        lda [SP0],y
        sta namef,y
        iny
        bra @pl
@pe:    lda [SP0],y             ; letztes Zeichen
        sta namef,y
        rep #$20
        .a16
        rts
@g:     lda ONDSC
        cmp #2
        bcs @x
        lda ONREF
        sta SP0
        lda ONREF+2
        sta SP0+2
        ora SP0
        beq @x
        lda ONDSC
        beq @p
        ldy #2                  ; Handle -> Zeiger
        lda [SP0],y
        tax
        lda [SP0]
        sta SP0
        stx SP0+2
@p:     lda [SP0]
        cmp #64
        bcc *+5
        lda #63
        sta U0
        sep #$20
        .a8
        sta namef
        ldx #0
        ldy #2
@l:     cpx U0
        beq @e
        lda [SP0],y
        sta namef+1,x
        inx
        iny
        bra @l
@e:     rep #$20
        .a16
@x:     rts

; ---------- Farben ----------
sfcolors:
        lda #$0FFF
        jsr sfnear
        sta CWH
        lda #$0000
        jsr sfnear
        sta CBK
        lda #$0CCC
        jsr sfnear
        sta CGR
        lda #$0777
        jsr sfnear
        sta CDK
        lda #$0008
        jsr sfnear
        sta CSE
        lda #$0FD4
        jsr sfnear
        sta CYE
        stz MONO
        lda CGR
        cmp CWH
        bne @c
        inc MONO
@c:     lda CSE
        cmp CWH
        bne @x
        lda CBK                 ; Auswahl nie weiss
        sta CSE
@x:     rts
; naechste Farbe ueber die Routinen der Titelleiste (deren DP)
sfnear: pha
        phd
        lda ourdp
        tcd
        lda #100
        sta WFT
        jsr scrmode
        lda 3,s
        jsr near
        sta 3,s
        pld
        pla
        rts

; ---------- Fenster ----------
mkwin:  pha
        pha
        ldx #$1C04              ; GetPort
        jsl $E10000
        pla
        sta OLDP
        pla
        sta OLDP+2
        pha
        ldx #$1704              ; GetMasterSCB
        jsl $E10000
        pla
        and #$0080
        beq @m3
        lda #2
        bra @m6
@m3:    lda #1
@m6:    sta XSC
        ; Inhalt: (320-DW)/2 links, Hoehe DH ab Zeile 36
        lda #(320-DW)/2
        jsr sx
        sta CLG
        sta wparm+64
        lda #DW
        jsr sx
        clc
        adc CLG
        sta wparm+68
        lda #36
        sta CTG
        sta wparm+62
        clc
        adc #DH
        sta wparm+66
        lda MODE
        jsr strp                ; Titel: Oeffnen / Speichern unter
        sta wparm+4
        lda mybank
        sta wparm+6
        pha
        pha
        lda mybank
        pha
        pea wparm
        ldx #$090E              ; NewWindow
        jsl $E10000
        pla
        sta WIN
        pla
        sta WIN+2
        bcs @f
        ora WIN
        beq @f
        lda WIN+2
        pha
        lda WIN
        pha
        ldx #$1B04              ; SetPort
        jsl $E10000
        lda XSC                 ; senkrechte Linien im 640er Modus 2 Pixel
        pha
        pea $0001
        ldx #$2C04              ; SetPenSize
        jsl $E10000
        pea $0000               ; Textmodus kopieren
        ldx #$9C04
        jsl $E10000
        ldx #$CA04              ; InitCursor (Pfeil)
        jsl $E10000
        pha
        pha
        ldx #$1106              ; GetDblTime
        jsl $E10000
        pla
        sta DBLT
        pla
        sta DBLT+2
        lda #$FFFF
        sta LASTI
        clc
        rts
@f:     sec
        rts

closewin:
        lda WIN
        ora WIN+2
        beq @p
        lda WIN+2
        pha
        lda WIN
        pha
        ldx #$0B0E              ; CloseWindow
        jsl $E10000
@p:     lda OLDP+2
        pha
        lda OLDP
        pha
        ldx #$1B04              ; SetPort
        jsl $E10000
        rts

; A * XSC
sx:     ldx XSC
        cpx #2
        bne @x
        asl
@x:     rts

; Textnummer A -> Zeiger auf Pascal-String (Sprache beachtet)
strp:   asl
        asl
        clc
        adc LANGF
        adc LANGF
        tax
        lda strtab,x
        rts

; ---------- Verzeichnis lesen ----------
readdir:
        stz NENT
        stz TOPI
        lda #$FFFF
        sta SEL
        lda pathb
        bne @dir
        jmp rdvols
@dir:   lda #.loword(pathb)
        sta op_pb+4
        lda #$2010              ; Open
        ldx #op_pb
        jsr gsos
        bcc @o
        stz pathb               ; Ordner fehlt: Laufwerke zeigen
        jmp rdvols
@o:     lda op_pb+2
        sta de_pb+2
        sta cl_pb+2
        lda #1
        sta de_pb+6             ; Basis 1, Abstand 1: naechster Eintrag
        sta de_pb+8
@e:     lda #$201C              ; GetDirEntry
        ldx #de_pb
        jsr gsos
        bcs @end
        lda de_pb+42            ; unsichtbar?
        and #$0004
        bne @e
        lda nmr+2               ; Namenslaenge
        beq @e
        cmp #64
        bcs @e
        lda de_pb+16
        sta TYP
        lda de_pb+44
        sta AUXL
        lda de_pb+46
        sta AUXL+2
        stz FLG
        lda TYP
        cmp #$000F
        bne @f
        lda #E_DIR
        sta FLG
        bra @add
@f:     lda MODE
        bne @add
        jsr typeok
        bcs @e
        jsr callfilt
        bcs @e
@add:   jsr addent
        lda NENT
        cmp #MAXE
        bcc @e
@end:   lda #$2014              ; Close
        ldx #cl_pb
        jsr gsos
        jmp sortents

; Laufwerke (5,25"-Laufwerke werden nicht abgefragt: ohne Diskette langsam)
rdvols: lda #1
        sta I0
@d:     lda I0
        sta di_pb+2
        lda #$202C              ; DInfo
        ldx #di_pb
        jsr gsos
        bcc @ok
        cmp #$0011              ; keine weiteren Geraete
        beq @x
        bra @n
@ok:    lda di_pb+8
        and #$0080              ; Blockgeraet?
        beq @n
        lda di_pb+8
        and #$0084
        cmp #$0084
        bne @v
        lda di_pb+20
        beq @n                  ; 5,25"
@v:     lda #$2008              ; Volume
        ldx #vo_pb
        jsr gsos
        bcs @n
        lda vnbuf+2             ; ":NAME" -> "NAME"
        beq @n
        dec a
        cmp #64
        bcs @n
        sta nmr+2
        sta U0
        ldx #0
        sep #$20
        .a8
@c:     lda vnbuf+5,x           ; ohne den fuehrenden Trenner
        sta nmr+4,x
        inx
        cpx U0
        bne @c
        rep #$20
        .a16
        lda #E_DIR|E_VOL
        sta FLG
        stz TYP
        stz AUXL
        stz AUXL+2
        jsr addent
        lda NENT
        cmp #MAXE
        bcs @x
@n:     inc I0
        lda I0
        cmp #40
        bcc @d
@x:     jmp sortents

; Typliste pruefen (Eintrag TYP/AUXL). C=1: nicht zeigen; FLG ggf. E_NOS
typeok: lda TLIST
        ora TLIST+2
        bne @t
        clc
        rts
@t:     lda OLDF
        beq @t2
        lda [TLIST]             ; alt: Anzahl (Byte), dann Typ-Bytes
        and #$00FF
        beq @no
        sta I1
        ldy #1
@ol:    lda [TLIST],y
        and #$00FF
        cmp TYP
        beq @s
        iny
        dec I1
        bne @ol
        bra @no
@t2:    lda [TLIST]
        beq @no
        sta I1
        ldy #2
@l:     lda [TLIST],y           ; Flags
        sta U0
        and #$4000
        bne @ft
        iny
        iny
        lda [TLIST],y
        dey
        dey
        cmp TYP
        bne @nx
@ft:    lda U0
        and #$8000
        bne @hit
        iny
        iny
        iny
        iny
        lda [TLIST],y
        cmp AUXL
        bne @ny
        iny
        iny
        lda [TLIST],y
        cmp AUXL+2
        beq @hit6
        dey
        dey
@ny:    dey
        dey
        dey
        dey
@nx:    tya
        clc
        adc #8
        tay
        dec I1
        bne @l
@no:    sec
        rts
@hit6:  ; Treffer
@hit:   lda U0
        and #$2000
        beq @s
        lda FLG
        ora #E_NOS
        sta FLG
@s:     clc
        rts

; Filterprozedur des Programms (Zeiger auf GetDirEntry-Block)
; Ergebnis 0: nicht zeigen, 1: grau, 2: waehlbar. C=1: nicht zeigen
callfilt:
        lda FILT
        ora FILT+2
        bne @f
        clc
        rts
@f:     lda FILT
        sta filtjml+1
        sep #$20
        .a8
        lda FILT+2
        sta filtjml+3
        rep #$20
        .a16
        phb
        phd
        pha                     ; Ergebnis
        lda mybank
        pha
        pea de_pb
        sep #$20                ; mit DP und Datenbank des Programms
        .a8
        lda SVDB
        pha
        rep #$20
        .a16
        lda SVDP
        tcd
        plb
        phk
        pea @ret-1
        jmp filtjml
@ret:   pla
        pld
        plb
        cmp #0
        beq @no
        cmp #1
        bne @ok
        lda FLG
        ora #E_NOS
        sta FLG
@ok:    clc
        rts
@no:    sec
        rts

; Eintrag anhaengen: Name aus nmr, FLG, TYP, AUXL
addent: lda NENT
        jsr entp
        lda FLG
        sta [SP0]
        ldy #2
        lda TYP
        sta [SP0],y
        ldy #4
        lda AUXL
        sta [SP0],y
        ldy #6
        lda AUXL+2
        sta [SP0],y
        lda nmr+2
        sta U0
        sep #$20
        .a8
        ldy #8
        sta [SP0],y
        ldx #0
@c:     cpx U0
        beq @e
        lda nmr+4,x
        iny
        sta [SP0],y
        inx
        bra @c
@e:     rep #$20
        .a16
        lda NENT                ; Reihenfolge zunaechst wie gelesen
        asl
        tax
        lda NENT
        sta sfidx,x
        inc NENT
        rts

; Eintrag A (Rohnummer) -> SP0
entp:   asl
        asl
        asl
        sta U3                  ; *8
        asl
        asl
        asl                     ; *64
        clc
        adc U3
        clc
        adc LP
        sta SP0
        lda LP+2
        adc #0
        sta SP0+2
        rts
; sortierte Nummer A -> Rohnummer -> SP0
sentp:  asl
        tax
        lda sfidx,x
        bra entp

; Sortieren: Ordner zuerst, dann Name ohne Gross/Klein (Einfuegen)
sortents:
        lda #1
        sta I0
@o:     lda I0
        cmp NENT
        bcs @x
        sta I1
@i:     lda I1
        beq @n
        asl
        tax
        lda sfidx-2,x           ; a = vorheriger, b = aktueller
        jsr entp
        lda SP0
        sta SP1
        lda SP0+2
        sta SP1+2
        lda I1
        asl
        tax
        lda sfidx,x
        jsr entp
        jsr ecmp                ; C=1: SP1 > SP0 -> tauschen
        bcc @n
        lda I1
        asl
        tax
        lda sfidx,x
        pha
        lda sfidx-2,x
        sta sfidx,x
        pla
        sta sfidx-2,x
        dec I1
        bra @i
@n:     inc I0
        bra @o
@x:     rts
; Vergleich SP1 (davor) mit SP0: C=1, wenn SP1 hinter SP0 gehoert
ecmp:   lda [SP1]
        and #E_DIR
        sta U0
        lda [SP0]
        and #E_DIR
        cmp U0
        beq @nm
        bcc @no                 ; SP0 Datei, SP1 Ordner: richtig
        sec                     ; SP0 Ordner, SP1 Datei: tauschen
        rts
@no:    clc
        rts
@nm:    ldy #8
        lda [SP1],y
        and #$00FF
        sta U0
        lda [SP0],y
        and #$00FF
        sta U1
@l:     iny
        tya
        sec
        sbc #9
        cmp U0
        bcs @e1                 ; SP1 zu Ende: nicht tauschen
        cmp U1
        bcs @gt                 ; SP0 zu Ende, SP1 laenger: tauschen
        lda [SP1],y
        jsr upc
        sta U2
        lda [SP0],y
        jsr upc
        cmp U2
        beq @l
        bcc @gt                 ; SP0 < SP1: tauschen
@e1:    clc
        rts
@gt:    sec
        rts
upc:    and #$00FF
        cmp #'a'
        bcc @x
        cmp #'z'+1
        bcs @x
        and #$00DF
@x:     rts

; ---------- Zeichnen (lokale Koordinaten, x in 320er Einheiten) ----------
; QX/QY/QW/QH -> qrect (Pixel)
mkq:    lda QY
        sta qrect
        clc
        adc QH
        sta qrect+4
        lda QX
        jsr sx
        sta qrect+2
        lda QX
        clc
        adc QW
        jsr sx
        sta qrect+6
        rts
; Rechteck in Farbe A fuellen
fill:   pha
        ldx #$3704              ; SetSolidPenPat
        jsl $E10000
        jsr mkq
        lda mybank
        pha
        pea qrect
        ldx #$5404              ; PaintRect
        jsl $E10000
        rts
; Kasten mit Linien: hell oben/links (U0), dunkel unten/rechts (U1)
box2:   lda QH
        pha
        lda QW
        pha
        lda #1
        sta QH
        lda U0
        jsr fill                ; oben
        pla
        sta QW
        pla
        sta QH
        lda QW
        pha
        lda #1
        sta QW
        lda U0
        jsr fill                ; links
        pla
        sta QW
        lda QX
        pha
        clc
        adc QW
        dec a
        sta QX
        lda #1
        pha
        lda QW
        pha
        lda #1
        sta QW
        lda U1
        jsr fill                ; rechts
        pla
        sta QW
        pla
        pla
        sta QX
        lda QY
        pha
        clc
        adc QH
        dec a
        sta QY
        lda QH
        pha
        lda #1
        sta QH
        lda U1
        jsr fill                ; unten
        pla
        sta QH
        pla
        sta QY
        rts
; erhabener Knopf (QX..), Flaeche grau
raised: lda MONO
        beq @c
        lda CWH
        jsr fill
        jmp frm
@c:     lda CGR
        jsr fill
        lda CWH
        sta U0
        lda CBK
        sta U1
        jsr box2
        jsr shrink
        lda CGR
        sta U0
        lda CDK
        sta U1
        jsr box2
        jmp grow
; vertiefter Kasten, Inneres weiss
sunken: lda CWH
        jsr fill
        lda MONO
        beq @c
        jmp frm
@c:
        lda CDK
        sta U0
        lda CWH
        sta U1
        jsr box2
        jsr shrink
        lda CBK
        sta U0
        lda CGR
        sta U1
        jsr box2
        jmp grow
; gedrueckter Knopf
pressed:
        lda MONO
        beq @c
        lda CWH
        jsr fill
        jsr frm
        jsr shrink
        jsr frm
        jmp grow
@c:     lda CGR
        jsr fill
        lda CBK
        sta U0
        sta U1
        jsr box2
        jsr shrink
        lda CDK
        sta U0
        lda CGR
        sta U1
        jsr box2
        jmp grow
shrink: inc QX
        inc QY
        dec QW
        dec QW
        dec QH
        dec QH
        rts
grow:   dec QX
        dec QY
        inc QW
        inc QW
        inc QH
        inc QH
        rts

; Farben fuer Text: A = Vordergrund, X = Hintergrund
tcol:   phx
        pha
        ldx #$A004              ; SetForeColor
        jsl $E10000
        ldx #$A204              ; SetBackColor
        jsl $E10000
        rts
; Pascal-String (unsere Bank) A an QX (320er), Grundlinie QY
txt:    pha
        lda QX
        jsr sx
        pha
        lda QY
        pha
        ldx #$3A04              ; MoveTo
        jsl $E10000
        pla
txtp:   tax
        lda mybank
        pha
        phx
        ldx #$A504              ; DrawString
        jsl $E10000
        rts
; Breite des Pascal-Strings A (Pixel)
twid:   tax
        pha
        lda mybank
        pha
        phx
        ldx #$A904              ; StringWidth
        jsl $E10000
        pla
        rts
; Beschriftung A zentriert im Knopf QX/QY/QW/QH
btext:  sta U2
        jsr twid
        sta U3
        lda QW
        jsr sx
        sec
        sbc U3
        lsr
        sta U3
        lda QX
        jsr sx
        clc
        adc U3
        pha
        lda QY
        clc
        adc #10
        pha
        ldx #$3A04
        jsl $E10000
        lda U2
        jmp txtp
; Clip auf QX.. setzen / ganzer Inhalt
clipr:  jsr mkq
        lda mybank
        pha
        pea qrect
        ldx #$2604              ; ClipRect
        jsl $E10000
        rts
clipall:
        stz QX
        stz QY
        lda #DW
        sta QW
        lda #DH
        sta QH
        bra clipr

; Knopf mit Text: X = Knopfnummer (Tabelle btab), A = 0 normal, 1 gedrueckt
sfbtn: stx BTN
        pha
        lda btab,x
        sta QX
        lda btab+2,x
        sta QY
        lda btab+4,x
        sta QW
        lda btab+6,x
        sta QH
        lda btab+8,x            ; Textnummer
        sta U3
        cpx #BT_OK
        bne @n1
        lda MODE                ; OK-Knopf: Oeffnen / Speichern
        clc
        adc #5
        sta U3
@n1:    pla
        beq @r
        jsr pressed
        bra @t
@r:     jsr raised
@t:     lda U3
        cmp #$FF
        beq @ic
        jsr strp
        pha
        lda CBK
        ldx CGR
        jsr tcol
        pla
        jmp btext
@ic:    ; Symbolknopf
        ldx BTN
        cpx #BT_UP
        bne @nf
        ; Pfeil nach oben: senkrechter Strich und zwei Schraegen
        lda CBK
        pha
        ldx #$3704
        jsl $E10000
        lda QX
        clc
        adc #9
        sta U0
        lda QY
        clc
        adc #3
        sta U1
        jsr mvto
        lda QY
        clc
        adc #10
        sta U1
        jsr lnto
        lda QY
        clc
        adc #3
        sta U1
        jsr mvto
        lda U0
        sec
        sbc #3
        sta U0
        lda QY
        clc
        adc #6
        sta U1
        jsr lnto
        lda QX
        clc
        adc #9
        sta U0
        lda QY
        clc
        adc #3
        sta U1
        jsr mvto
        lda U0
        clc
        adc #3
        sta U0
        lda QY
        clc
        adc #6
        sta U1
        jmp lnto
@nf:    ; neuer Ordner: kleines Ordnersymbol mit Stern
        lda QX
        clc
        adc #4
        sta QX
        lda QY
        clc
        adc #3
        sta QY
        jsr folder
        rts
mvto:   lda U0
        jsr sx
        pha
        lda U1
        pha
        ldx #$3A04
        jsl $E10000
        rts
lnto:   lda U0
        jsr sx
        pha
        lda U1
        pha
        ldx #$3C04              ; LineTo
        jsl $E10000
        rts

; Symbole an QX/QY (Zeile oben)
folder: lda QX
        pha
        lda QY
        pha
        lda #4
        sta QW
        lda #1
        sta QH
        inc QY
        lda CYE
        jsr fill
        lda #10
        sta QW
        lda #6
        sta QH
        inc QY
        lda CYE
        jsr fill
        jsr frm
        pla
        sta QY
        pla
        sta QX
        rts
fileic: lda QX
        pha
        lda QY
        pha
        inc QX
        inc QY
        lda #8
        sta QW
        lda #8
        sta QH
        lda CWH
        jsr fill
        jsr frm
        pla
        sta QY
        pla
        sta QX
        rts
volic:  lda QX
        pha
        lda QY
        pha
        inc QY
        inc QY
        inc QY
        lda #11
        sta QW
        lda #5
        sta QH
        lda CGR
        jsr fill
        jsr frm
        pla
        sta QY
        pla
        sta QX
        rts
; Rahmen um QX.. in Schwarz
frm:    lda CBK
        pha
        ldx #$3704
        jsl $E10000
        jsr mkq
        lda mybank
        pha
        pea qrect
        ldx #$5304              ; FrameRect
        jsl $E10000
        rts

; ---------- ganzer Dialog ----------
drawall:
        jsr layout
        jsr clipall
        stz QX
        stz QY
        lda #DW
        sta QW
        lda #DH
        sta QH
        lda CGR
        jsr fill
        lda CBK                 ; Beschriftungen
        ldx CGR
        jsr tcol
        lda #6
        sta QX
        lda #12
        sta QY
        lda MODE
        clc
        adc #2
        jsr strp
        jsr txt
        lda #6
        sta QX
        lda #116
        sta QY
        lda #4
        jsr strp
        jsr txt
        ldx #BT_UP
        lda #0
        jsr sfbtn
        lda MODE
        beq @nb
        ldx #BT_NEW
        lda #0
        jsr sfbtn
@nb:    ldx #BT_OK
        lda #0
        jsr sfbtn
        ldx #BT_CAN
        lda #0
        jsr sfbtn
        jsr drawprompt
        jsr drawpath
        jsr drawfield
        jmp drawlist

; Beschriftungsspalte nach der Textbreite (320er Modus: Schrift doppelt so breit)
layout: lda MODE
        clc
        adc #2
        jsr strp
        jsr twid
        sta U0
        lda #4
        jsr strp
        jsr twid
        cmp U0
        bcc @a
        sta U0
@a:     lda U0
        ldx XSC
        cpx #2
        bne @b
        lsr
@b:     clc
        adc #10
        cmp #56
        bcs @c
        lda #56
@c:     cmp #140
        bcc @d
        lda #140
@d:     sta LBW
        sta hot+40              ; Trefferflaechen anpassen
        sta hot+50
        lda #242
        sec
        sbc LBW
        sta hot+44
        lda #218
        sec
        sbc LBW
        sta hot+54
        rts

; Prompt des Programms unten links
drawprompt:
        lda PRREF
        ora PRREF+2
        beq @x
        lda PRDSC
        cmp #2
        bcs @x
        lda PRREF
        sta SP0
        lda PRREF+2
        sta SP0+2
        lda PRDSC
        beq @p
        ldy #2
        lda [SP0],y
        tax
        lda [SP0]
        sta SP0
        stx SP0+2
@p:     lda #4
        sta QX
        lda #122
        sta QY
        lda #214
        sta QW
        lda #16
        sta QH
        jsr clipr
        lda CBK
        ldx CGR
        jsr tcol
        lda #6
        jsr sx
        pha
        pea 132
        ldx #$3A04
        jsl $E10000
        lda SP0+2
        pha
        lda SP0
        pha
        ldx #$A504
        jsl $E10000
        jsr clipall
@x:     rts

; Feld "Suchen in": Name des aktuellen Ordners
drawpath:
        lda LBW
        sta QX
        lda #3
        sta QY
        lda #242
        sec
        sbc LBW
        sta QW
        lda #13
        sta QH
        jsr sunken
        lda LBW
        clc
        adc #4
        sta QX
        lda #5
        sta QY
        lda pathb
        bne @f
        jsr volic
        lda #8
        jsr strp
        bra @t
@f:     jsr folder
        jsr lastcomp
        lda #.loword(tbuf2)
@t:     pha
        lda LBW
        inc a
        inc a
        sta QX
        lda #4
        sta QY
        lda #238
        sec
        sbc LBW
        sta QW
        lda #11
        sta QH
        jsr clipr
        lda CBK
        ldx CWH
        jsr tcol
        lda LBW
        clc
        adc #18
        sta QX
        lda #13
        sta QY
        pla
        jsr txt
        jmp clipall
; letzter Teil von pathb -> tbuf2 (Pascal)
lastcomp:
        ldx pathb
        stx U0
@l:     cpx #0
        beq @f
        sep #$20
        .a8
        lda pathb+1,x
        rep #$20
        .a16
        and #$00FF
        cmp #':'
        beq @f
        dex
        bra @l
@f:     ; Zeichen x+1 .. U0
        stx U1
        lda U0
        sec
        sbc U1
        cmp #64
        bcc *+5
        lda #63
        sta U2
        sep #$20
        .a8
        sta tbuf2
        ldy #0
@c:     cpy U2
        beq @e
        lda pathb+2,x
        sta tbuf2+1,y
        inx
        iny
        bra @c
@e:     rep #$20
        .a16
        rts

; Dateinamenfeld
drawfield:
        lda LBW
        sta QX
        lda #106
        sta QY
        lda #218
        sec
        sbc LBW
        sta QW
        lda #13
        sta QH
        jsr sunken
        lda LBW
        inc a
        inc a
        sta QX
        lda #107
        sta QY
        lda #214
        sec
        sbc LBW
        sta QW
        lda #11
        sta QH
        jsr clipr
        ; Text: Sichern = namef, Oeffnen = gewaehlte Datei
        lda MODE
        bne @s
        stz tbuf2
        lda SEL
        cmp NENT
        bcs @d
        jsr sentp
        lda [SP0]
        and #E_DIR
        bne @d
        jsr selname
        bra @d
@s:     ldx #64
@c:     lda namef,x
        sta tbuf2,x
        dex
        dex
        bpl @c
@d:     lda CBK
        ldx CWH
        ldy REPL                ; ganz markiert?
        beq @n
        ldy MODE
        beq @n
        ldy namef
        beq @n
        lda CWH
        ldx CSE
@n:     jsr tcol
        lda LBW
        clc
        adc #3
        sta QX
        lda #116
        sta QY
        lda #.loword(tbuf2)
        jsr txt
        lda MODE                ; Schreibmarke
        beq @x
        lda FOC
        beq @x
        lda CBK
        pha
        ldx #$3704
        jsl $E10000
        lda mybank              ; aktuelle Stiftposition
        pha
        pea qrect
        ldx #$2904              ; GetPen
        jsl $E10000
        lda qrect               ; v
        sta QY
        lda qrect+2             ; h
        clc
        adc #1
        sta qrect+2
        lda qrect+2
        pha
        lda QY
        sec
        sbc #8
        pha
        ldx #$3A04
        jsl $E10000
        lda qrect+2
        pha
        lda QY
        inc a
        pha
        ldx #$3C04
        jsl $E10000
@x:     jmp clipall
; Name des gewaehlten Eintrags (SP0) -> tbuf2
selname:
        ldy #8
        lda [SP0],y
        and #$00FF
        sta U0
        sep #$20
        .a8
        sta tbuf2
        ldx #0
@c:     cpx U0
        beq @e
        iny
        lda [SP0],y
        sta tbuf2+1,x
        inx
        bra @c
@e:     rep #$20
        .a16
        rts

; Liste
drawlist:
        lda #LX0
        sta QX
        lda #LY0
        sta QY
        lda #LX1-LX0
        sta QW
        lda #LY1-LY0
        sta QH
        jsr sunken
        lda #LX0+2
        sta QX
        lda #LY0+1
        sta QY
        lda #SBX-LX0-2
        sta QW
        lda #NROWS*ROWH
        sta QH
        jsr clipr
        stz I0
@r:     lda I0
        cmp #NROWS
        bcc *+5
        jmp @sb
        clc
        adc TOPI
        cmp NENT
        bcc *+5
        jmp @sb
        sta I1
        jsr sentp
        ; Zeilenoberkante
        lda I0
        asl
        sta U0
        asl
        asl
        clc
        adc U0                  ; *10
        clc
        adc #LY0+1
        sta U2
        ; Auswahlbalken
        lda I1
        cmp SEL
        bne @ns
        lda #LX0+15
        sta QX
        lda U2
        sta QY
        lda #SBX-LX0-17
        sta QW
        lda #ROWH
        sta QH
        lda CSE
        jsr fill
@ns:    lda #LX0+3
        sta QX
        lda U2
        sta QY
        lda [SP0]
        and #E_VOL
        beq @nv
        jsr volic
        bra @nm
@nv:    lda [SP0]
        and #E_DIR
        beq @nf
        jsr folder
        bra @nm
@nf:    jsr fileic
@nm:    lda I1
        jsr sentp               ; SP0 neu (Symbole aendern es nicht, sicher ist sicher)
        jsr selname
        lda CBK
        ldx CWH
        ldy I1
        cpy SEL
        bne @c1
        lda CWH
        ldx CSE
        bra @c2
@c1:    lda [SP0]
        and #E_NOS
        beq @c2
        lda CDK
@c2:    jsr tcol
        lda #LX0+17
        sta QX
        lda U2
        clc
        adc #8
        sta QY
        lda #.loword(tbuf2)
        jsr txt
        inc I0
        jmp @r
@sb:    jsr clipall
        ; Rollbalken
        lda #SBX
        sta QX
        lda #LY0+1
        sta QY
        lda #LX1-SBX-1
        sta QW
        lda #NROWS*ROWH
        sta QH
        lda CGR
        jsr fill
        lda #LY0+1
        sta QY
        lda #10
        sta QH
        jsr raised
        lda #LY1-11
        sta QY
        jsr raised
        ; Pfeile als kleine Dreiecke (Striche)
        jsr arrows
        lda NENT
        cmp #NROWS+1
        bcc @x
        ; Schieber: Spur LY0+11 .. LY1-11, Hoehe 10
        lda NENT
        sec
        sbc #NROWS
        sta U1                  ; groesste Startzeile
        lda TOPI
        cmp U1
        bcc *+4
        lda U1
        ; y = TOPI * (Spur-10) / U1
        sta U0
        stz U2
        ldx #NROWS*ROWH-30
@m:     lda U2
        clc
        adc U0
        sta U2
        dex
        bne @m
        lda U2
        ldx #0
@d:     cmp U1
        bcc @dd
        sbc U1
        inx
        bra @d
@dd:    txa
        clc
        adc #LY0+11
        sta QY
        lda #10
        sta QH
        lda #SBX
        sta QX
        lda #LX1-SBX-1
        sta QW
        jsr raised
@x:     rts
arrows: lda CBK
        pha
        ldx #$3704
        jsl $E10000
        lda #SBX+3
        sta U0
        lda #LY0+8
        sta U1
        jsr mvto
        lda #SBX+6
        sta U0
        lda #LY0+4
        sta U1
        jsr lnto
        lda #SBX+9
        sta U0
        lda #LY0+8
        sta U1
        jsr lnto
        lda #SBX+3
        sta U0
        lda #LY1-8
        sta U1
        jsr mvto
        lda #SBX+6
        sta U0
        lda #LY1-4
        sta U1
        jsr lnto
        lda #SBX+9
        sta U0
        lda #LY1-8
        sta U1
        jmp lnto

; ---------- Ereignisse ----------
evloop: lda DONE
        beq @g
        rts
@g:     pha
        pea $002E               ; Maus runter/hoch, Tasten
        lda mybank
        pha
        pea evrec
        ldx #$0A06              ; GetNextEvent
        jsl $E10000
        pla
        beq evloop
        lda evrec
        cmp #1
        bne @k
        jsr domouse
        bra evloop
@k:     cmp #3
        beq @key
        cmp #5
        bne evloop
@key:   jsr dokey
        bra evloop

beep:   ldx #$2C03              ; SysBeep
        jsl $E10000
        rts

; Maus: Treffer in der Tabelle hot suchen
domouse:
        lda evrec+10
        sec
        sbc CTG
        sta MY
        lda evrec+12
        sec
        sbc CLG
        ldx XSC
        cpx #2
        bne @1
        cmp #$8000
        ror
@1:     sta MX
        ldx #0
@l:     lda hot,x
        cmp #$FFFF
        beq @miss
        lda MX
        sec
        sbc hot,x
        bmi @n
        cmp hot+4,x
        bcs @n
        lda MY
        sec
        sbc hot+2,x
        bmi @n
        cmp hot+6,x
        bcs @n
        lda hot+8,x
        asl
        tax
        jmp (acts,x)
@n:     txa
        clc
        adc #10
        tax
        bra @l
@miss:  jmp beep

; Knopf X druecken: gedrueckt zeichnen, Maustaste abwarten, normal zeichnen
press:  phx
        lda #1
        jsr sfbtn
        jsr waitup
        plx
        lda #0
        jmp sfbtn
waitup: pha
        pea $0000
        ldx #$0E06              ; StillDown
        jsl $E10000
        pla
        bne waitup
        rts

a_list: lda MY                  ; Zeile
        sec
        sbc #LY0+1
        ldx #0
@d:     cmp #ROWH
        bcc @r
        sbc #ROWH
        inx
        bra @d
@r:     txa
        clc
        adc TOPI
        cmp NENT
        bcc @in
        rts
@in:    sta HIT
        ; Doppelklick?
        cmp LASTI
        bne @single
        lda evrec+6
        sec
        sbc LASTW
        tax
        lda evrec+8
        sbc LASTW+2
        bne @single
        cpx DBLT
        bcs @single
        lda #$FFFF
        sta LASTI
        lda HIT
        jmp activate
@single:
        lda HIT
        sta LASTI
        lda evrec+6
        sta LASTW
        lda evrec+8
        sta LASTW+2
        lda HIT
        jmp select

a_sbar: lda MY
        cmp #LY0+11
        bcs @1
        lda #$FFFF              ; Pfeil hoch
        bra scrl
@1:     cmp #LY1-11
        bcc @2
        lda #1                  ; Pfeil runter
        bra scrl
@2:     cmp #LY0+40             ; Spur: seitenweise
        bcs @3
        lda #$10000-(NROWS-1)
        bra scrl
@3:     lda #NROWS-1
; Liste um A Zeilen verschieben
scrl:   clc
        adc TOPI
        bpl @a
        lda #0
@a:     sta TOPI
        lda NENT
        sec
        sbc #NROWS
        bpl @b
        lda #0
@b:     cmp TOPI
        bcs @c
        sta TOPI
@c:     jsr drawlist
        ; gedrueckt halten: weiter rollen
        pha
        pea $0000
        ldx #$0E06
        jsl $E10000
        pla
        beq @x
        ldx #6000
@w:     dex
        bne @w
        jmp a_sbar
@x:     rts

a_up:   ldx #BT_UP
        jsr press
        jmp goup
a_new:  lda MODE
        bne @1
        rts
@1:     ldx #BT_NEW
        jsr press
        jmp newdir
a_path: lda pathb
        bne @1
        rts
@1:     stz pathb               ; zu den Laufwerken
        jsr readdir
        jsr drawpath
        jsr drawfield
        jmp drawlist
a_field:
        lda MODE
        beq @x
        lda #1
        sta FOC
        jsr drawfield
@x:     rts
a_ok:   ldx #BT_OK
        jsr press
        jmp dook
a_can:  ldx #BT_CAN
        jsr press
        lda #2
        sta DONE
        rts

; Eintrag waehlen (sortierte Nummer A)
select: sta HIT
        jsr sentp
        lda [SP0]
        and #E_NOS
        beq @ok
        jmp beep
@ok:    lda HIT
        sta SEL
        jsr showsel
        lda MODE                ; Sichern: Dateiname uebernehmen
        beq @d
        lda [SP0]
        and #E_DIR
        bne @d
        jsr selname
        ldx #64
@c:     lda tbuf2,x
        sta namef,x
        dex
        dex
        bpl @c
        lda #1
        sta REPL
@d:     jsr drawlist
        jmp drawfield
; Auswahl sichtbar machen
showsel:
        lda SEL
        cmp TOPI
        bcs @1
        sta TOPI
        rts
@1:     sec
        sbc #NROWS-1
        bmi @x
        cmp TOPI
        bcc @x
        beq @x
        sta TOPI
@x:     rts

; Eintrag oeffnen: Ordner betreten, Datei waehlen
activate:
        sta HIT
        jsr sentp
        lda [SP0]
        and #E_NOS
        beq @1
        jmp beep
@1:     lda [SP0]
        and #E_DIR
        bne @dir
        lda HIT
        sta SEL
        lda MODE
        beq @op
        jsr select              ; Sichern: Name uebernehmen und sichern
        jmp dook
@op:    lda #1
        sta DONE
        rts
@dir:   lda [SP0]
        and #E_VOL
        beq @sub
        stz pathb               ; Laufwerk: ":NAME"
@sub:   jsr appname
        bcc @rd
        jmp beep
@rd:    jsr readdir
        jsr drawpath
        jsr drawfield
        jmp drawlist
; pathb + ":" + Name von SP0 (C=1: zu lang)
appname:
        ldy #8
        lda [SP0],y
        and #$00FF
        sta U0
        clc
        adc pathb
        inc a
        cmp #500
        bcc @ok
        sec
        rts
@ok:    ldx pathb
        sep #$20
        .a8
        lda #':'
        sta pathb+2,x
        inx
@c:     iny
        lda [SP0],y
        sta pathb+2,x
        inx
        dec U0
        bne @c
        rep #$20
        .a16
        stx pathb
        clc
        rts

; eine Ebene hoeher
goup:   lda pathb
        bne @1
        jmp beep
@1:     ldx pathb
@l:     dex
        beq @vol
        sep #$20
        .a8
        lda pathb+2,x
        rep #$20
        .a16
        and #$00FF
        cmp #':'
        bne @l
        stx pathb
        bra @rd
@vol:   stz pathb
@rd:    jsr readdir
        jsr drawpath
        jsr drawfield
        jmp drawlist

; Neuer Ordner im aktuellen Ordner
newdir: lda pathb
        bne @1
        jmp beep
@1:     lda #'1'
        sta U3
@try:   ; tpath = pathb + ":" + Name (+ Ziffer)
        lda pathb
        inc a
        inc a
        and #$FFFE
        tax
@c0:    lda pathb,x             ; pathb (Laenge + Zeichen) kopieren
        sta tpath,x
        dex
        dex
        bpl @c0
        lda #12
        jsr strp
        sta SP1
        lda mybank
        sta SP1+2
        lda [SP1]
        and #$00FF
        sta U0
        ldx tpath
        ldy #1
        sep #$20
        .a8
        lda #':'
        sta tpath+2,x
        inx
@c1:    lda [SP1],y
        sta tpath+2,x
        inx
        iny
        dec U0
        bne @c1
        lda U3
        cmp #'1'
        beq @nd
        sta tpath+2,x
        inx
@nd:    rep #$20
        .a16
        stx tpath
        lda #.loword(tpath)
        sta cr_pb+2
        lda #$2001              ; Create
        ldx #cr_pb
        jsr gsos
        bcc @ok
        cmp #$0047              ; schon vorhanden: naechste Ziffer
        bne @err
        inc U3
        lda U3
        cmp #'9'+1
        bcc @try
@err:   jmp beep
@ok:    jsr readdir
        ; neuen Ordner auswaehlen: Name = letzter Teil von tpath
        jsr drawpath
        jsr drawlist
        rts

; OK / Eingabe
dook:   lda MODE
        bne @save
        lda SEL                 ; Oeffnen
        cmp NENT
        bcc @a
        jmp beep
@a:     jmp activate
@save:  lda SEL                 ; gewaehlter Ordner: betreten
        cmp NENT
        bcs @nm
        jsr sentp
        lda [SP0]
        and #E_DIR
        beq @nm
        lda SEL
        jmp activate
@nm:    lda pathb               ; im Arbeitsplatz kann man nicht sichern
        bne @p
        jmp beep
@p:     lda namef
        and #$00FF
        bne @q
        jmp beep
@q:     ; Name schon vorhanden?
        stz I0
@l:     lda I0
        cmp NENT
        bcs @new
        jsr sentp
        jsr nmeq
        bcs @same
        inc I0
        bra @l
@same:  lda [SP0]
        and #E_DIR
        beq @f
        lda I0                  ; Ordner gleichen Namens: betreten
        jmp activate
@f:     jsr confirm
        bcs @new
        rts
@new:   lda #1
        sta DONE
        rts
; Name von SP0 gleich namef (ohne Gross/Klein)? C=1
nmeq:   ldy #8
        lda [SP0],y
        and #$00FF
        sta U0
        lda namef
        and #$00FF
        cmp U0
        bne @no
        ldx #0
@l:     cpx U0
        beq @y
        iny
        lda [SP0],y
        jsr upc
        sta U1
        lda namef+1,x
        jsr upc
        cmp U1
        bne @no
        inx
        bra @l
@y:     sec
        rts
@no:    clc
        rts

; "Datei ersetzen?" Ja/Nein. C=1: ja
confirm:
        lda #60
        sta QX
        lda #40
        sta QY
        lda #180
        sta QW
        lda #52
        sta QH
        jsr raised
        lda CBK
        ldx CGR
        jsr tcol
        lda #74
        sta QX
        lda #56
        sta QY
        lda #9
        jsr strp
        jsr txt
        ldx #BT_YES
        lda #0
        jsr sfbtn
        ldx #BT_NO
        lda #0
        jsr sfbtn
@ev:    pha
        pea $002A
        lda mybank
        pha
        pea evrec
        ldx #$0A06
        jsl $E10000
        pla
        beq @ev
        lda evrec
        cmp #1
        beq @m
        lda evrec+2
        and #$00DF              ; Grossbuchstabe
        cmp #'J'
        beq @yes
        cmp #'Y'
        beq @yes
        cmp #'N'
        beq @no
        lda evrec+2
        and #$00FF
        cmp #13
        beq @yes
        cmp #27
        beq @no
        bra @ev
@m:     jsr domouse0            ; MX/MY bestimmen
        lda MY
        cmp #68
        bcc @ev
        cmp #82
        bcs @ev
        lda MX
        cmp #80
        bcc @ev
        cmp #130
        bcc @yes
        cmp #170
        bcc @ev
        cmp #220
        bcs @ev
@no:    jsr drawall
        clc
        rts
@yes:   jsr drawall
        sec
        rts
domouse0:
        lda evrec+10
        sec
        sbc CTG
        sta MY
        lda evrec+12
        sec
        sbc CLG
        ldx XSC
        cpx #2
        bne @1
        lsr
@1:     sta MX
        rts

; Tasten
dokey:  lda evrec+2
        and #$00FF
        sta U3
        lda evrec+14
        and #$0100              ; Apfeltaste
        beq @pl
        lda U3
        cmp #'.'
        beq @can
        cmp #$0B                ; Apfel-Hoch: Ordner hoeher
        bne @a2
        jmp goup
@a2:    cmp #$0A                ; Apfel-Runter: oeffnen
        bne @ax
        lda SEL
        cmp NENT
        bcs @ax
        jmp activate
@ax:    rts
@can:   lda #2
        sta DONE
        rts
@pl:    lda U3
        cmp #13
        bne @k1
        jmp dook
@k1:    cmp #27
        beq @can
        cmp #$0B                ; Pfeil hoch
        bne @k2
        lda SEL
        cmp NENT
        bcs @k0
        dec a
        bpl @mv
@k0:    lda #0
        bra @mv
@k2:    cmp #$0A                ; Pfeil runter
        bne @k3
        lda SEL
        inc a
        cmp NENT
        bcc @mv
        lda NENT
        dec a
        bmi @ax
@mv:    ldx NENT
        beq @ax
        jsr select
        rts
@k3:    cmp #9                  ; Tab: Namensfeld / Liste
        bne @k4
        lda MODE
        beq @ax
        lda FOC
        eor #1
        sta FOC
        jmp drawfield
@k4:    lda MODE
        bne @sv
        ; Oeffnen: Buchstabe springt zum naechsten passenden Eintrag
        lda U3
        jsr upc
        sta U2
        lda SEL
        sta I1
        stz I0
@ts:    inc I1
        lda I1
        cmp NENT
        bcc @t1
        stz I1
@t1:    lda I1
        jsr sentp
        ldy #9
        lda [SP0],y
        jsr upc
        cmp U2
        beq @tf
        inc I0
        lda I0
        cmp NENT
        bcc @ts
        rts
@tf:    lda I1
        jmp select
@sv:    ; Sichern: Namensfeld bearbeiten
        lda #1
        sta FOC
        lda U3
        cmp #$7F                ; Loeschen
        beq @del
        cmp #8                  ; Pfeil links wie Loeschen
        beq @del
        cmp #' '
        bcc @ax2
        cmp #$7F
        bcs @ax2
        cmp #':'
        beq @bp
        cmp #'/'
        beq @bp
        lda REPL
        beq @ap
        stz namef
        stz REPL
@ap:    lda namef
        and #$00FF
        cmp #63
        bcs @bp
        tax
        sep #$20
        .a8
        lda U3
        sta namef+1,x
        inc namef
        rep #$20
        .a16
        bra @chg
@del:   lda REPL
        beq @d1
        stz namef
        stz REPL
        bra @chg
@d1:    lda namef
        and #$00FF
        beq @chg
        sep #$20
        .a8
        dec namef
        rep #$20
        .a16
@chg:   lda SEL                 ; Tippen hebt die Auswahl auf
        cmp #$FFFF
        beq @nf
        lda #$FFFF
        sta SEL
        jsr drawlist
@nf:    jmp drawfield
@bp:    jmp beep
@ax2:   rts

; ---------- Antwort ----------
fillreply:
        ; Name -> tbuf2 (Pascal), Typ/Zusatztyp
        stz TYP
        stz AUXL
        stz AUXL+2
        lda MODE
        bne @s
        lda SEL
        jsr sentp
        jsr selname
        ldy #2
        lda [SP0],y
        sta TYP
        ldy #4
        lda [SP0],y
        sta AUXL
        ldy #6
        lda [SP0],y
        sta AUXL+2
        bra @g
@s:     ldx #64
@c:     lda namef,x
        sta tbuf2,x
        dex
        dex
        bpl @c
@g:     lda OLDF
        beq @g2
        jmp oldreply
@g2:    lda #1
        sta [REPLY]             ; good
        ldy #2
        lda TYP
        sta [REPLY],y
        ldy #4
        lda AUXL
        sta [REPLY],y
        ldy #6
        lda AUXL+2
        sta [REPLY],y
        ; Prefix 8 = aktueller Ordner
        lda #8
        sta sp_pb+2
        lda #.loword(pathb)
        sta sp_pb+4
        lda #$2009              ; SetPrefix
        ldx #sp_pb
        jsr gsos
        ; tpath = Name als Klasse-1-String
        lda tbuf2
        and #$00FF
        sta tpath
        tax
@n:     dex
        bmi @n2
        lda tbuf2+1,x
        sta tpath+2,x
        bra @n
@n2:    ldy #8
        jsr outstr
        bcs @x
        ; tpath = pathb + ":" + Name
        lda pathb
        inc a
        inc a
        and #$FFFE
        tax
@p0:    lda pathb,x
        sta tpath,x
        dex
        dex
        bpl @p0
        lda tbuf2
        and #$00FF
        sta U0
        ldx tpath
        sep #$20
        .a8
        lda #':'
        sta tpath+2,x
        inx
        ldy #0
@p1:    cpy U0
        beq @p2
        lda tbuf2+1,y
        sta tpath+2,x
        inx
        iny
        bra @p1
@p2:    rep #$20
        .a16
        stx tpath
        ldy #14
        jsr outstr
@x:     rts

; alter Antwortsatz: good, Typ, Zusatztyp (Wort), Name[16], Pfad[129]
oldreply:
        lda #1
        sta [REPLY]
        ldy #2
        lda TYP
        sta [REPLY],y
        ldy #4
        lda AUXL
        sta [REPLY],y
        lda #8
        sta sp_pb+2
        lda #.loword(pathb)
        sta sp_pb+4
        lda #$2009              ; SetPrefix 8
        ldx #sp_pb
        jsr gsos
        ; Name (max. 15)
        lda tbuf2
        and #$00FF
        cmp #16
        bcc *+5
        lda #15
        sta U0
        sep #$20
        .a8
        ldy #6
        sta [REPLY],y
        ldx #0
@n:     cpx U0
        beq @ne
        lda tbuf2+1,x
        iny
        sta [REPLY],y
        inx
        bra @n
@ne:    ; Pfad (max. 128): pathb + ":" + Name
        rep #$20
        .a16
        lda pathb
        clc
        adc U0
        inc a
        cmp #129
        bcc @pk
        lda #0                  ; zu lang: nicht gewaehlt
        sta [REPLY]
        rts
@pk:    sep #$20
        .a8
        ldy #22
        sta [REPLY],y
        ldx #0
@p:     cpx pathb
        beq @pc
        lda pathb+2,x
        iny
        sta [REPLY],y
        inx
        bra @p
@pc:    lda #':'
        iny
        sta [REPLY],y
        ldx #0
@q:     cpx U0
        beq @qe
        lda tbuf2+1,x
        iny
        sta [REPLY],y
        inx
        bra @q
@qe:    rep #$20
        .a16
        rts

; tpath (Klasse 1) ausgeben nach Art [REPLY],y / Ziel [REPLY],y+2
outstr: sty I0
        lda [REPLY],y
        sta I1                  ; Art
        iny
        iny
        lda [REPLY],y
        sta SP1
        iny
        iny
        lda [REPLY],y
        sta SP1+2
        lda I1
        cmp #3
        beq @new
        lda SP1
        ora SP1+2
        bne @h
        sec
        rts
@h:     lda I1
        beq @put
        ldy #2                  ; Handle -> Zeiger
        lda [SP1],y
        tax
        lda [SP1]
        sta SP1
        stx SP1+2
        bra @put
@new:   pha
        pha
        pea $0000
        lda tpath
        clc
        adc #4
        pha
        lda myid
        pha
        pea $0000
        pea $0000
        pea $0000
        ldx #$0902              ; NewHandle
        jsl $E10000
        pla
        sta SP0
        pla
        sta SP0+2
        bcc @hv
        lda #$0201
        sta ERRC
        sec
        rts
@hv:    ldy I0                  ; Handle in den Antwortsatz
        iny
        iny
        lda SP0
        sta [REPLY],y
        iny
        iny
        lda SP0+2
        sta [REPLY],y
        ldy #2
        lda [SP0],y
        sta SP1+2
        lda [SP0]
        sta SP1
        lda tpath
        clc
        adc #4
        sta [SP1]               ; Puffergroesse
@put:   lda [SP1]               ; Platz: Groesse - 4
        sec
        sbc #4
        bpl *+5
        lda #0
        sta U0
        ldy #2
        lda tpath
        sta [SP1],y
        cmp U0
        bcc @k
        lda U0
@k:     sta U1
        ldx #0
        ldy #4
        sep #$20
        .a8
@cp:    cpx U1
        beq @e
        lda tpath+2,x
        sta [SP1],y
        inx
        iny
        bra @cp
@e:     rep #$20
        .a16
        clc
        rts

; ---------- Daten des Dialogs ----------
BT_UP  = 0
BT_NEW = 10
BT_OK  = 20
BT_CAN = 30
BT_YES = 40
BT_NO  = 50
btab:   .word 246, 3, 18, 13, $FF
        .word 268, 3, 18, 13, $FF
        .word 224, 106, 70, 14, 5
        .word 224, 123, 70, 14, 7
        .word 80, 68, 50, 14, 10
        .word 170, 68, 50, 14, 11
hot:    .word LX0+2, LY0+1, SBX-LX0-2, NROWS*ROWH, 0
        .word SBX, LY0+1, LX1-SBX, NROWS*ROWH, 1
        .word 246, 3, 18, 13, 2
        .word 268, 3, 18, 13, 3
        .word 62, 3, 180, 13, 4
        .word 66, 106, 150, 13, 5
        .word 224, 106, 70, 14, 6
        .word 224, 123, 70, 14, 7
        .word $FFFF
acts:   .word a_list, a_sbar, a_up, a_new, a_path, a_field, a_ok, a_can

strtab: .word s_ttlo, s_ttlo_e, s_ttls, s_ttls_e, s_look, s_look_e, s_savi, s_savi_e
        .word s_fnam, s_fnam_e, s_bopn, s_bopn_e, s_bsav, s_bsav_e, s_bcan, s_bcan_e
        .word s_myc, s_myc_e, s_repl, s_repl_e, s_yes, s_yes_e, s_no, s_no_e
        .word s_ndir, s_ndir_e
.macro PSTR str
        .byte .strlen(str), str
.endmacro
s_ttlo:   .byte 7, $85, "ffnen "
s_ttlo_e: PSTR "Open "
s_ttls:   PSTR "Speichern unter "
s_ttls_e: PSTR "Save As "
s_look:   PSTR "Suchen in:"
s_look_e: PSTR "Look in:"
s_savi:   PSTR "Speichern:"
s_savi_e: PSTR "Save in:"
s_fnam:   PSTR "Dateiname:"
s_fnam_e: PSTR "File name:"
s_bopn:   .byte 6, $85, "ffnen"
s_bopn_e: PSTR "Open"
s_bsav:   PSTR "Speichern"
s_bsav_e: PSTR "Save"
s_bcan:   PSTR "Abbrechen"
s_bcan_e: PSTR "Cancel"
s_myc:    PSTR "Arbeitsplatz"
s_myc_e:  PSTR "My Computer"
s_repl:   PSTR "Datei ersetzen?"
s_repl_e: PSTR "Replace file?"
s_yes:    PSTR "Ja"
s_yes_e:  PSTR "Yes"
s_no:     PSTR "Nein"
s_no_e:   PSTR "No"
s_ndir:   PSTR "NEUER.ORDNER"
s_ndir_e: PSTR "NEW.FOLDER"

prefsnm: .word 21
         .byte "*:System:GSOS95.PREFS"
filtjml: .byte $5C, 0, 0, 0
gsnum:  .word 0
evrec:  .res 16
qrect:  .res 8
wparm:  .word 78, $8020         ; Laenge, Titel + sichtbar
        .word 0, 0              ; Titel (wird gesetzt)
        .word 0, 0              ; RefCon
        .word 0, 0, 0, 0        ; Zoom
        .word 0, 0              ; Farben
        .word 0, 0, 0, 0, 0, 0  ; Ursprung, Daten, Max
        .word 0, 0, 0, 0        ; Rollen, Seiten
        .word 0, 0              ; Info-RefCon
        .word 0                 ; Info-Hoehe
        .word 0, 0, 0, 0, 0, 0  ; Rahmen-, Info-, Inhalts-defProc
        .word 0, 0, 0, 0        ; Position (oben, links, unten, rechts)
        .word $FFFF, $FFFF      ; ganz vorn
        .word 0, 0              ; Speicher
gpfx_pb: .word 2, 0, 0, 0
op_pb:  .word 2, 0, 0, 0
de_pb:  .word 13, 0, 0, 1, 1
        .word nmr, 0
        .res 48-14
di_pb:  .word 8, 0
        .word dnbuf, 0
        .res 14
vo_pb:  .word 2
        .word dnbuf+2, 0
        .word vnbuf, 0
cr_pb:  .word 5, 0, 0, $C3, $0F, 0, 0, $000D
sp_pb:  .word 2, 8, 0, 0
pf_pb:  .word 2, 0, prefsnm, 0
pr_pb:  .word 4, 0, prbuf, 0, 1, 0, 0, 0
cl_pb:  .word 1, 0
prbuf:  .word 0
dnbuf:  .word 36, 0
        .res 34
vnbuf:  .word 70, 0
        .res 68
nmr:    .word 260, 0
        .res 258
namef:  .res 66
tbuf2:  .res 66
sfidx:  .res MAXE*2
pathr:  .word 516
pathb:  .word 0
        .res 516
tpath:  .word 0
        .res 520
