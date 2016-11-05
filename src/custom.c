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
            switch(difficulty) {
                case 0: // normal mode
                    extra_minutes_to_be_added = 15;
                    break;
                case 1: // hard mode
                    extra_minutes_to_be_added = 12;
                    break;
                case 2: // impossible mode
                    extra_minutes_to_be_added = 4;
                    break;
            }
            hitp_max--;
            if (hitp_curr > hitp_max) --hitp_curr;
            draw_kid_hp(hitp_curr, hitp_max+1); // erase one hp box
            is_show_time = 1;
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
        case 10:
        {
            int link = curr_room + 100;
            sbyte link_timer = get_doorlink_timer(link);
            // is the event jammed?
            if (link_timer != 0x1F) {
                set_doorlink_timer(link, 5);
                if (link_timer < 2) {
                    add_trob(curr_room, curr_tilepos, 1);
                    redraw_11h();
                    is_guard_notice = 1;
                    //play_sound(sound_3_button_pressed); // button pressed
                }
                do_trigger_list(link, tiles_14_debris); // stay pernamently open
            }
        }
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
    }
}

void custom_init_game() {
    switch(difficulty) {
        default:
        case 0: // normal
            rem_min = -1;
            break;
        case 1: // hard
            rem_min = 60;
            break;
        case 2: // impossible
            rem_min = 20;
            break;
    }
    if (is_practice_mode) {
        rem_min = -1;
        hitp_beg_lev = practice_mode_hitp[start_level];
    }
}

void custom_init_level() {

}

void custom_init_room(byte room) {
    // Special event: if the kid hasn't obtained the sword yet, open a specific gate
    if (current_level == 3 && !have_sword && room == 13) {
        //printf("Opening the gate to the sword room.\n");
        get_room_address(13);
        curr_room_modif[19] = 0xFF;
    }
}

int custom_ending(byte* skip_to_hof) {
    if ((difficulty == 0 && rem_min >= 150) || (difficulty == 1 && rem_min > 60)) {
        load_intro(0, &alternate_end_sequence_anim, 1);
        *skip_to_hof = 1;
        return 1;
    }
    return 0;
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
    Char.y = 166;
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
    if (difficulty < 2) difficulty++;
    fade_out_1();
}

void show_practice_mode_dialog() {
    word key;
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
    if (level_number >= 1 && level_number <= 14) {
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

const rect_type mod_title_rect = {0, 0, 100, 320};
const rect_type tips_rect = {100, 0, 200, 320};

const char* mod_tips[] = (const char *[]) {
        "Tip:\nTo quicksave, press F6.\nTo quickload, press F9.",
        "Tip:\nTo enter practice mode,\npress Ctrl+P on the title screen.",
        "Tip:\nTo capture a replay, press Ctrl+Tab in-game.",
        "Tip:\nPurple potions restore full health.",
        "Tip:\nThe 'secrets' may be hard to reach...\nBut you can safely skip most of them!",
        "Tip:\nFor help/discussion, you can reach out at:\n\nforum.princed.org\npopot.org",
};
const word num_tips = COUNT(mod_tips);

void show_splash() {
    screen_updates_suspended = 0;
    current_target_surface = onscreen_surface_;
    draw_rect(&screen_rect, 0);
    show_text_with_color(&mod_title_rect, 0, 0,
                         "Secrets of the Citadel\n"
                         "\n"
                         "- by Falcury -\n",
                         color_15_brightwhite);
    int displayed_tip = prandom(num_tips-1);
    show_text_with_color(&tips_rect, 0, -1, mod_tips[displayed_tip], color_7_lightgray);

    int key = 0;
    do {
        idle();
        key = key_test_quit(); // Press any key to continue...

        if (key == SDL_SCANCODE_RIGHT) {
            draw_rect(&tips_rect, 0);
            displayed_tip = (displayed_tip + 1) % num_tips;
            show_text_with_color(&tips_rect, 0, -1, mod_tips[displayed_tip], color_7_lightgray);
            key = 0;
        }
        else if (key == SDL_SCANCODE_LEFT) {
            draw_rect(&tips_rect, 0);
            displayed_tip--;
            if (displayed_tip == -1) displayed_tip = num_tips-1;
            show_text_with_color(&tips_rect, 0, -1, mod_tips[displayed_tip], color_7_lightgray);
            key = 0;
        }
    } while(key == 0 && !(key_states[SDL_SCANCODE_LSHIFT] || key_states[SDL_SCANCODE_RSHIFT]));

    extern int last_key_scancode; // defined in seg009.c
    last_key_scancode = key;
    key_states[SDL_SCANCODE_LSHIFT] = 0;
    key_states[SDL_SCANCODE_RSHIFT] = 0;
}
