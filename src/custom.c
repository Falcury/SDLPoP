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
