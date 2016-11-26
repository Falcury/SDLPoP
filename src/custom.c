/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2015  D�vid Nagy

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

The authors of this program may be contacted at http://forum.princed.org
*/

#include "common.h"

void custom_potion_effect(word potion_type) {
    switch (potion_type) {
        default: return;
        case 7: // shadow potion
            stop_sounds();
            united_with_shadow = 10; // blink
            is_shadow_effect = 220;
            flash_color = 7; // grey
            flash_time = 4;
            hitp_max--;
            hitp_delta = 1 - hitp_curr; // remove all but 1 hp
            draw_kid_hp(hitp_curr - 1, hitp_max + 1); // erase one hp box
            play_sound(sound_39_low_weight); // low weight
            break;
        case 8: // extra time potion
            stop_sounds();
            play_sound(sound_37_victory);
            flash_color = 7; // grey
            flash_time = 4;
            if (is_time_attack_mode) {
                extra_minutes_to_be_added = 5;
                is_show_time = 1;
            }
            hitp_max--;
            if (hitp_curr > hitp_max) --hitp_curr;
            draw_kid_hp(hitp_curr, hitp_max+1); // erase one hp box
            tbl_have_bonus_potion[current_level] = 1;
            break;
        case 9:
            if (hitp_curr != hitp_max) {
                stop_sounds();
                play_sound(sound_33_small_potion); // small potion
                set_health_life();
                flash_color = color_4_red;
                flash_time = 4;
            }
            break;
        case 10: // custom open potion
            do_trigger_list(curr_room + 100, tiles_14_debris); // stay pernamently open
            break;
        case 11: // strong poison
            stop_sounds();
            play_sound(sound_13_kid_hurt);
            hitp_delta = -hitp_curr;
            break;
    }
}

#define SMALL_POT 12
#define LARGE_POT 13

byte custom_potion_pot_id(word potion_type) {
    switch(potion_type) {
        default: return SMALL_POT;
        case 7: return LARGE_POT; // shadow potion
    }
}

void custom_potion_anim(word potion_type, word* color, word* pot_size) {
    switch(potion_type) {
        default: return;
        case 7: // shadow potion
            *color = color_7_lightgray;
            *pot_size = 1;
            break;
        case 8: // extra time potion
            *color = color_7_lightgray;
            break;
        case 9: // full health potion
            *color = color_13_brightmagenta;
            break;
        case 10: // custom open potion
            *color = color_10_brightgreen;
            break;
        case 11: // strong poison potion
            *color = color_1_blue;
    }
}

void custom_init_game() {
    extra_minutes_to_be_added = 0;

    rem_min = (is_time_attack_mode) ? 20 : -1;
    if (is_practice_mode) {
        rem_min = -1;
        hitp_beg_lev = tbl_practice_mode_hitp[start_level];
    }
    // reset the bonus potions
    memcpy(tbl_have_bonus_potion, tbl_bonus_potions, sizeof(tbl_bonus_potions));

    // reset the roomscript variables
    reset_room_script();
}

int check_have_all_bonus() {
    if (debug_cheats_enabled && check_param("bonus")) return true;
    int i;
    for (i = 0; i < 16; ++i) {
        if (tbl_have_bonus_potion[i] < 0) return false;
    }
    return true;
}

int custom_check_Jaffar_not_yet_defeated() {
    return (current_level < 13 ||
            (current_level == 13 && leveldoor_open < 2) ||
            (current_level == 13 && guardtype == 2 /*skeleton*/) ||
            (current_level == 14 && check_have_all_bonus() == 1 && leveldoor_open < 5));
}

void custom_init_level() {

}

void check_reload_guard_resources() {
    // Special event: fight Jaffar in level 14
    if (current_level == 14) {
        if (drawn_room == 10 && guardtype != 3 /*Jaffar*/) {
            // free the guard images
            if (chtab_addrs[id_chtab_5_guard]) {
                free_chtab(chtab_addrs[id_chtab_5_guard]);
                chtab_addrs[id_chtab_5_guard] = NULL;
            }
            load_chtab_from_file(id_chtab_5_guard, 750, "VIZIER.DAT", 1<<8);
            curr_guard_color = 0;
            guardtype = 3; // Jaffar
        }
        else if (guardtype == 3 /*Jaffar*/) {
            // free the guard images
            if (chtab_addrs[id_chtab_5_guard]) {
                free_chtab(chtab_addrs[id_chtab_5_guard]);
                chtab_addrs[id_chtab_5_guard] = NULL;
            }
            dat_type* dathandle = open_dat("GUARD2.DAT", 0);
            load_chtab_from_file(id_chtab_5_guard, 750, "GUARD.DAT", 1<<8);
            close_dat(dathandle);
            guardtype = 0; // guard
        }
    }
}

void custom_init_room(byte room) {
    // Special event: if the kid hasn't obtained the sword yet, open a specific gate
    if (current_level == 3 && !have_sword && room == 13) {
        //printf("Opening the gate to the sword room.\n");
        get_room_address(13);
        curr_room_modif[19] = 0xFF;
    }
}

extern byte seqtbl_offsets[];
extern short disable_keys;

void alternate_end_sequence_anim() {
    disable_keys = 1;
    if (!is_sound_on) {
        turn_sound_on_off(0x0F);
    }
    copy_screen_rect(&screen_rect);
    Char.charid = charid_5_princess;
    Char.x = 120;
    Char.y = 166;
    Char.direction = dir_FF_left;
    seqtbl_offset_char(seq_95_Jaffar_stand_PV1); // Jaffar stand [PV1]
    play_seq();
    saveshad();
    init_ending_kid();
    Char.y = 164;
    savekid();
    play_sound(sound_4_gate_closing); // gate closing
    if (proc_cutscene_frame(5)) return;
    seqtbl_offset_kid_char(seq_13_stop_run); // stop run
    if (proc_cutscene_frame(5)) return;
    play_sound(sound_32_shadow_music);
    if (proc_cutscene_frame(47)) return;
    seqtbl_offset_shad_char(100); // Vexit
    Guard.curr_seq += 10;
    if (proc_cutscene_frame(15)) return;
    seqtbl_offset_shad_char(102); // Vraise
    if (proc_cutscene_frame(5)) return;
    play_sound(sound_1_falling);
    if (proc_cutscene_frame(15)) return;
    flash_time = 10;
    flash_color = 15; // white
    united_with_shadow = 10;
    seqtbl_offset_kid_char(seq_71_dying);
    if (proc_cutscene_frame(6)) return;
    united_with_shadow = 0;
    Kid.x = 240;
    if (proc_cutscene_frame(10)) return;
    seqtbl_offset_shad_char(100); // Vexit
    fade_out_1();
}

void show_practice_mode_dialog() {
    rect_type rect;
    short bgcolor = 0;
    short color = 15;
    current_target_surface = onscreen_surface_;
    method_1_blit_rect(offscreen_surface, onscreen_surface_, &copyprot_dialog->peel_rect, &copyprot_dialog->peel_rect, 0);
    draw_dialog_frame(copyprot_dialog);
    shrink2_rect(&rect, &copyprot_dialog->text_rect, 2, 1);
    show_text_with_color(&rect, 0, 0, "Practice mode\nenter the level number...\n", color_15_brightwhite);
    screen_updates_suspended = 0;
    request_screen_update();
    clear_kbd_buf();

    rect_type text_rect = {105,   152,  119,  238};
    rect_type input_rect;
    offset4_rect_add(&input_rect, &text_rect, -4, -1, -10, -1);
    //peel_type* peel = read_peel_from_screen(&input_rect);
    draw_rect(&input_rect, bgcolor);
    current_target_surface = onscreen_surface_;

    char level_number_buffer[3] = "";
    while(input_str(&input_rect, level_number_buffer, 2, "", 0, 4, color, bgcolor) <= 0);
    //restore_peel(peel);

    int level_number = atoi(level_number_buffer);
    if (level_number >= 1 && level_number <= 13) {
        start_level = level_number;
        is_practice_mode = 1;
    } else {
        start_level = 0;
    }
    start_game();

    //restore_dialog_peel_2(copyprot_dialog->peel);
    //current_target_surface = old_target;
    redraw_screen(0); // lazy: instead of neatly restoring only the relevant part, just redraw the whole screen
}
