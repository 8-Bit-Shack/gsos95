; Sprungstellen fuer Code in der zweiten Bank (CALL im XCODE)
tp_gmul: jsr gmul
        rtl
tp_gdiv: jsr gdiv
        rtl
tp_setcolor: jsr setcolor
        rtl
tp_fillrect: jsr fillrect
        rtl
tp_bevel: jsr bevel
        rtl
tp_blit1: jsr blit1
        rtl
tp_blit1c: jsr blit1c
        rtl
tp_blit4: jsr blit4
        rtl
tp_inrect: jsr inrect
        rtl
tp_gsos: jsr gsos
        rtl
tp_setpixel_raw: jsr setpixel_raw
        rtl
tp_waitvbl: jsr waitvbl
        rtl
tp_cur_hide: jsr cur_hide
        rtl
tp_cur_show: jsr cur_show
        rtl
tp_org0: jsr org0
        rtl
tp_orgr: jsr orgr
        rtl
tp_clip_full: jsr clip_full
        rtl
tp_redraw_all: jsr redraw_all
        rtl
tp_redraw_win: jsr redraw_win
        rtl
tp_redraw_rect: jsr redraw_rect
        rtl
tp_drawstr: jsr drawstr
        rtl
tp_strwidth: jsr strwidth
        rtl
tp_drawnum: jsr drawnum
        rtl
tp_putch: jsr putch
        rtl
tp_lmx: jsr lmx
        rtl
tp_lmy: jsr lmy
        rtl
tp_win_open: jsr win_open
        rtl
tp_win_pop: jsr win_pop
        rtl
tp_show_msg: jsr show_msg
        rtl
tp_pcopy: jsr pcopy
        rtl
tp_pappend: jsr pappend
        rtl
tp_pstrip: jsr pstrip
        rtl
tp_entep: jsr entep
        rtl
tp_type_icon: jsr type_icon
        rtl
tp_sprhdr: jsr sprhdr
        rtl
tp_load_dir: jsr load_dir
        rtl
tp_fail: jsr fail
        rtl
tp_nda_name: jsr nda_name
        rtl
tp_open_entry: jsr open_entry
        rtl
tp_load_volumes: jsr load_volumes
        rtl
tp_redraw_top: jsr redraw_top
        rtl
tp_is_active: jsr is_active
        rtl
tp_nda_list: jsr nda_list
        rtl
tp_recaddr: jsr recaddr
        rtl
tp_save_prefs: jsr save_prefs
        rtl
tp_upc8: jsr upc8
        rtl

tp_gsosp: jsr gsosp
        rtl
tp_launch: jsr launch
        rtl
tp_path_append: jsr path_append
        rtl
tp_setpixel: jsr setpixel
        rtl
tp_show_msg_nr: jsr show_msg_nr
        rtl
tp_close_win: jsr close_win
        rtl
tp_dblclick: JSLF xf_dblclick
        rtl
tp_desk_init: jsr desk_init
        rtl
tp_explorer: jsr explorer
        rtl
tp_nda_open: jsr nda_open
        rtl
tp_open_access: jsr open_access
        rtl
tp_read_clock: jsr read_clock
        rtl
tp_toggle_lang: jsr toggle_lang
        rtl
tp_act_confirm: jsr act_confirm
        rtl
tp_act_erase: jsr act_erase
        rtl
tp_anc_b: jsr anc_b
        rtl
tp_anc_end: jsr anc_end
        rtl
tp_confirm_btns: jsr confirm_btns
        rtl
tp_copy_file: jsr copy_file
        rtl
tp_edit_redraw: jsr edit_redraw
        rtl
tp_entry_path: jsr entry_path
        rtl
tp_new_explorer: jsr new_explorer
        rtl
tp_putdec32: jsr putdec32
        rtl
tp_reload_view: jsr reload_view
        rtl
tp_samevol: jsr samevol
        rtl
tp_desktop_scb: jsr desktop_scb
        rtl
tp_draw_window: jsr draw_window
        rtl
tp_win_close: jsr win_close
        rtl
wc_view_m: JSLF xf_wc_view
        rts
tp_ensure_vis: jsr ensure_vis
        rtl
tp_pl_addr: JSLF xf_pl_addr
        rtl
tp_pl_append: JSLF xf_pl_append
        rtl
tp_pl_del: JSLF xf_pl_del
        rtl
tp_pl_save: JSLF xf_pl_save
        rtl
tp_pl_sel: JSLF xf_pl_sel
        rtl
tp_win_geom: jsr win_geom
        rtl
act_link_m: JSLF xf_act_link
        rts
tp_draw_menu: jsr draw_menu
        rtl
tp_open_explorer: jsr open_explorer
        rtl
tp_pl_find: JSLF xf_pl_find
        rtl
tp_redraw_menu: jsr redraw_menu
        rtl
act_prgadd_m: JSLF xf_act_prgadd
        rts
tp_exp_frame: jsr exp_frame
        rtl
tp_draw_sbar: jsr draw_sbar
        rtl
tp_sbar_click: jsr sbar_click
        rtl
tp_win_unmin: jsr win_unmin
        rtl
tp_win_front: jsr win_front
        rtl
tp_win_load: jsr win_load
        rtl
tp_redraw_cont: jsr redraw_cont
        rtl
tp_prog_begin: JSLF xf_prog_begin
        rtl
tp_prog_poll: JSLF xf_prog_poll
        rtl
tp_prog_file: JSLF xf_prog_file
        rtl
tp_prog_step: JSLF xf_prog_step
        rtl
tp_prog_end: JSLF xf_prog_end
        rtl
tp_destroy_tree: jsr destroy_tree
        rtl
tp_bin_clear: JSLF xf_bin_clear
        rtl

tp_cell_at: jsr cell_at
        rtl
tp_draw_drop: jsr draw_drop
        rtl
tp_drop_count: jsr drop_count
        rtl
tp_is_sel: JSLF xf_is_sel
        rtl
tp_msel_clear: JSLF xf_msel_clear
        rtl
tp_rect_set: jsr rect_set
        rtl
tp_show_busy: jsr show_busy
        rtl
tp_win_save: jsr win_save
        rtl
tp_xfer: jsr xfer
        rtl
act_dup_m: JSLF xf_act_dup
        rts
act_find_m: JSLF xf_act_find
        rts
act_findnext_m: JSLF xf_act_findnext
        rts
act_open_m: JSLF xf_act_open
        rts
act_print_m: JSLF xf_act_print
        rts
wc_prog_m: JSLF xf_wc_prog
        rts
tp_ec_n: jsr ec_n
        rtl
tp_pequal: jsr pequal
        rtl
tp_sel_list: JSLF xf_sel_list
        rtl
act_restore_m: JSLF xf_act_restore
        rts
tp_ec_em: jsr ec_em
        rtl
tp_entaddr: jsr entaddr
        rtl