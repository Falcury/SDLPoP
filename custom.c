/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2015  Dávid Nagy

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
                    extra_minutes_to_be_added = 7;
                    break;
            }
            if (hitp_curr >= hitp_max) hitp_curr--;
            hitp_max--;
            draw_kid_hp(0, hitp_max + 1); // erase one hp box
            draw_kid_hp(hitp_curr, hitp_max); // erase one hp box
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
            *color = color_7_grey;
            *pot_size = 1;
            break;
        case 8: // extra time potion
            *color = color_7_grey;
            break;
        case 9: // full health potion
            *color = 13; // purple
            break;
    }
}

void custom_init_game() {
    if (check_param("hard")) {
        difficulty = 1;
        rem_min = 60;
    }
    else if (check_param("impossible")) {
        difficulty = 2;
        rem_min = 30;
    }
    else { // normal mode
        difficulty = 0;
        rem_min = 150;
    }
}
