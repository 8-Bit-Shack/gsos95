; ================================================================
; GS/OS95: Ansicht "Details" (Liste mit Name, Groesse, Typ) und Sortierung
; ================================================================
LVY0  = WY+40                   ; erste Listenzeile
LVRH  = 8                       ; Zeilenhoehe
LCN   = WX+8                    ; Spalte Name
LCS   = WX+108                  ; Spalte Groesse
LCT   = WX+150                  ; Spalte Typ

; Mit Ordnerbaum: Liste um die Baumbreite TW nach rechts verschieben
lv_shift:
        lda TW
        beq @r
        clc
        adc DXO
        sta DXO
        lda CW
        sec
        sbc TW
        sta CW
@r:     rts
lv_unshift:
        php
        pha
        lda TW
        beq @r
        lda DXO
        sec
        sbc TW
        sta DXO
        lda CW
        clc
        adc TW
        sta CW
@r:     pla
        plp
        rts
wc_list:
        lda TW
        beq @n
        JSLF xf_tree_draw       ; Ordnerbaum links
@n:     jsr lv_shift
        jsr wc_list0
        jmp lv_unshift
draw_row:
        jsr lv_shift
        jsr draw_row0
        jmp lv_unshift
list_slot:
        jsr lv_shift
        jsr list_slot0
        jmp lv_unshift

; Kopfzeile + alle Zeilen der Seite
wc_list0:
        lda #WX+6
        sta RX
        lda #WY+30
        sta RY
        lda CW
        sec
        sbc #12+SBW+2
        sta RW
        lda #9
        sta RH
        lda #GRAY
        jsr setcolor
        jsr fillrect
        lda #DKGRAY
        sta C1
        TEXT LCN,WY+30,t_hname,BLACK
        TEXT LCS,WY+30,t_hsize,BLACK
        TEXT LCT,WY+30,t_htype,BLACK
        FILL LCS-4,WY+30,1,9,DKGRAY
        FILL LCT-4,WY+30,1,9,DKGRAY
        stz SLOT
@sl:    jsr pg_first
        clc
        adc SLOT
        sta IDX
        cmp COUNT
        bcs @r
        jsr draw_row0
        inc SLOT
        lda SLOT
        cmp PPG
        bcc @sl
@r:     rts

; Zeile SLOT mit Eintrag IDX
draw_row0:
        lda SLOT
        asl
        asl
        asl
        clc
        adc #LVY0
        sta IY
        lda IDX
        JSLF xf_is_sel
        bcc @nsel
        lda #NAVY
        bra @bg
@nsel:  lda CLRCELL
        beq @nbg
        lda #WHITE
@bg:    jsr setcolor
        lda #WX+6
        sta RX
        lda IY
        sta RY
        lda CW
        sec
        sbc #12+SBW+2
        sta RW
        lda #LVRH
        sta RH
        jsr fillrect
@nbg:   lda IDX
        jsr entep
        lda IDX
        JSLF xf_is_sel
        bcc @blk
        lda #WHITE
        bra @col
@blk:   lda #BLACK
@col:   sta LVCOL
        jsr setcolor
        ; Name (abgeschnitten vor der Groessen-Spalte)
        lda #LCS-6
        clc
        adc DXO
        sta CLIPR
        jsr ep_nm
        ldy #2
        lda [EP],y
        and #$00FF
        sta STRN
        lda #LCN
        sta SX
        lda IY
        sta SY
        jsr drawstr
        lda #320
        sta CLIPR
        ; Groesse in KB (nicht bei Ordnern/Laufwerken)
        lda [EP]
        cmp #$000F
        beq @nosz
        cmp #$0100
        bcs @nosz
        ldy #E_EOF
        lda [EP],y
        clc
        adc #1023
        sta NUMLO
        iny
        iny
        lda [EP],y
        adc #0
        sta NUMHI
        ldx #10
@sh:    lsr NUMHI
        ror NUMLO
        dex
        bne @sh
        lda #LCS
        sta SX
        lda IY
        sta SY
        lda LVCOL
        jsr setcolor
        jsr putdec32
        LSPR t_kb
        jsr blit1
@nosz:  ; Typ (Klartext)
        lda [EP]
        jsr type_icon
        lda TYPN
        sta SPR
        lda #LCT
        sta SX
        lda IY
        sta SY
        lda #WX-SBW-10
        clc
        adc CW
        clc
        adc DXO
        sta CLIPR
        lda LVCOL
        jsr setcolor
        jsr blit1
        lda #320
        sta CLIPR
        rts

; Mausposition -> Zeile in SLOT. C=1 innerhalb der Liste
list_slot0:
        lda #WX+6
        sta RX
        lda #LVY0
        sta RY
        lda CW
        sec
        sbc #12+SBW+2
        sta RW
        lda NRV
        asl
        asl
        asl
        sta RH
        jsr inrect
        bcs @in
        rts
@in:    jsr lmy
        sec
        sbc #LVY0
        lsr
        lsr
        lsr
        sta SLOT
        sec
        rts

; ---------- Sortieren: Ordner zuerst, dann nach SORTM (0 Name, 1 Groesse, 2 Typ) ----------
; Shellsort (Eintraege liegen in der Eintragsbank, Tausch ueber srtmp)
sort_entries:
        lda MODE
        beq @no
        lda SORTM
        cmp #3                  ; 3 = Reihenfolge wie im Verzeichnis
        bne @go
@no:    rts
@go:    stz SRG
@gl:    ldx SRG
        lda gaps,x
        bne @gok
        rts
@gok:   sta SRGAP
        inc SRG
        inc SRG
        cmp COUNT
        bcs @gl
        sta SRI
@i:     lda SRI
        cmp COUNT
        bcs @gl
        sta SRJ
@j:     lda SRJ
        cmp SRGAP
        bcc @ni
        sec
        sbc SRGAP
        jsr entep               ; EP = j-gap
        lda SRJ
        jsr entaddr
        sta EP2
        jsr epent2
        jsr ent_cmp             ; C=1: j-gap gehoert hinter j
        bcc @ni
        ; tauschen
        lda EP
        tax
        ldy #srtmp
        jsr ec_em
        ldx EP2
        ldy EP
        jsr ec_ee
        ldx #srtmp
        ldy EP2
        jsr ec_me
        lda SRJ
        sec
        sbc SRGAP
        sta SRJ
        bra @j
@ni:    inc SRI
        bra @i
gaps:   .word 132,57,23,10,4,1,0

; ---------- Kopieren zwischen Eintragsbank und Programmbank ----------
; X = Quelle, Y = Ziel (je 16 Bit), ENTSZ Bytes
ec_ee:  lda ENTBK
        xba
        ora ENTBK
        bra ec_go
ec_em:  lda ENTBK               ; Eintrag -> Programmbank
        xba
        ora MYBANK
        bra ec_go
ec_me:  lda MYBANK              ; Programmbank -> Eintrag
        xba
        ora ENTBK
ec_go:  sta ecmv+1              ; +1 Zielbank, +2 Quellbank
        lda #ENTSZ-1
ec_n:   phb
ecmv:   .byte $54,0,0           ; MVN
        plb
        rts
; Variante mit Laenge: A = Anzahl-1, Baenke wie ec_me / ec_em
xc_me:  pha
        lda MYBANK
        xba
        ora ENTBK
        sta ecmv+1
        pla
        bra ec_n
xc_em:  pha
        lda ENTBK
        xba
        ora MYBANK
        sta ecmv+1
        pla
        bra ec_n

; Long-Zeiger-Helfer (A bleibt erhalten)
entep:  jsr entaddr
        sta EP
epent:  pha
        lda ENTBK
        sta EP+2
        pla
        rts
epent2: pha
        lda ENTBK
        sta EP2+2
        pla
        rts
epmy:   pha
        lda MYBANK
        sta EP+2
        pla
        rts
epmy2:  pha
        lda MYBANK
        sta EP2+2
        pla
        rts
; Name des Eintrags [EP] in die Programmbank (epnm) -> STRP
ep_nm:  ldy #2
        ldx #0
@c:     lda [EP],y
        sta epnm,x
        iny
        iny
        inx
        inx
        cpx #34
        bcc @c
        lda #epnm+1
        sta STRP
        rts

; Vergleich EP mit EP2: C=1 wenn EP nach EP2 kommt
ent_cmp:
        ; Ordner zuerst
        lda [EP]
        cmp #$000F
        bne @a
        lda [EP2]
        cmp #$000F
        beq @same
        clc                     ; Ordner vor Datei
        rts
@a:     lda [EP2]
        cmp #$000F
        bne @same
        sec
        rts
@same:  lda SORTM
        cmp #1
        beq @size
        cmp #2
        beq @type
        jmp name_cmp
@size:  ldy #E_EOF+2
        lda [EP],y
        cmp [EP2],y
        bne @d
        ldy #E_EOF
        lda [EP],y
        cmp [EP2],y
        bne @d
        jmp name_cmp
@type:  lda [EP]
        cmp [EP2]
        bne @d
        jmp name_cmp
@d:     beq @eq                 ; C aus cmp: C=1 wenn EP >= EP2
        rts
@eq:    clc
        rts

; Namen vergleichen (ohne Gross/Klein): C=1 wenn EP > EP2
name_cmp:
        ldy #2
        lda [EP],y
        and #$00FF
        sta SRL1
        lda [EP2],y
        and #$00FF
        sta SRL2
        ldy #3
@c:     tya
        sec
        sbc #3
        cmp SRL1
        bcs @e1
        cmp SRL2
        bcs @gt
        lda [EP2],y
        jsr upc8
        sta TMP
        lda [EP],y
        jsr upc8
        cmp TMP
        bne @ne
        iny
        bra @c
@ne:    rts                     ; C=1 wenn groesser (gleich ausgeschlossen)
@e1:    clc                     ; EP kuerzer oder gleich
        rts
@gt:    sec
        rts
