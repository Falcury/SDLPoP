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
#include <ctype.h>

void use_default_options() {
    options.use_fixes_and_enhancements = 0;
    options.enable_copyprot = 0;
    options.enable_mixer = 1;
    options.enable_fade = 1;
    options.enable_flash = 1;
    options.enable_text = 1;
    options.enable_quicksave = 1;
    options.enable_quicksave_penalty = 1;
    options.enable_replay = 1;
    options.enable_crouch_after_climbing = 1;
    options.enable_freeze_time_during_end_music = 1;
    options.fix_gate_sounds = 1;
    options.fix_two_coll_bug = 1;
    options.fix_infinite_down_bug = 1;
    options.fix_gate_drawing_bug = 0;
    options.fix_bigpillar_climb = 0;
    options.fix_jump_distance_at_edge = 1;
    options.fix_edge_distance_check_when_climbing = 1;
    options.fix_painless_fall_on_guard = 1;
    options.fix_wall_bump_triggers_tile_below = 1;
    options.fix_stand_on_thin_air = 0;
    options.fix_press_through_closed_gates = 1;
    options.fix_grab_falling_speed = 1;
    options.fix_skeleton_chomper_blood = 1;
    options.fix_move_after_drink = 1;
    options.fix_loose_left_of_potion = 1;
    options.fix_guard_following_through_closed_gates = 1;
    options.fix_safe_landing_on_spikes = 1;
    // mod features
    options.difficulty = 0xFF; // select
}

void disable_fixes_and_enhancements() {
    options.enable_crouch_after_climbing = 0;
    options.enable_freeze_time_during_end_music = 0;
    options.fix_gate_sounds = 0;
    options.fix_two_coll_bug = 0;
    options.fix_infinite_down_bug = 0;
    options.fix_gate_drawing_bug = 0;
    options.fix_bigpillar_climb = 0;
    options.fix_jump_distance_at_edge = 0;
    options.fix_edge_distance_check_when_climbing = 0;
    options.fix_painless_fall_on_guard = 0;
    options.fix_wall_bump_triggers_tile_below = 0;
    options.fix_stand_on_thin_air = 0;
    options.fix_press_through_closed_gates = 0;
    options.fix_grab_falling_speed = 0;
    options.fix_skeleton_chomper_blood = 0;
    options.fix_move_after_drink = 0;
    options.fix_loose_left_of_potion = 0;
    options.fix_guard_following_through_closed_gates = 0;
    options.fix_safe_landing_on_spikes = 0;
}

// .ini file parser adapted from https://gist.github.com/OrangeTide/947070
/* Load an .ini format file
 * filename - path to a file
 * report - callback can return non-zero to stop, the callback error code is
 *     returned from this function.
 * return - return 0 on success
 */
int ini_load(const char *filename,
             int (*report)(const char *section, const char *name, const char *value))
{
    char name[64];
    char value[256];
    char section[128] = "";
    char *s;
    FILE *f;
    int cnt;

    f = fopen(filename, "r");
    if (!f) {
        perror(filename);
        return -1;
    }

    while (!feof(f)) {
        if (fscanf(f, "[%127[^];\n]]\n", section) == 1) {
        } else if ((cnt = fscanf(f, " %63[^=;\n] = %255[^;\n]", name, value))) {
            if (cnt == 1)
                *value = 0;
            for (s = name + strlen(name) - 1; s > name && isspace(*s); s--)
                *s = 0;
            for (s = value + strlen(value) - 1; s > value && isspace(*s); s--)
                *s = 0;
            report(section, name, value);
        }
        fscanf(f, " ;%*[^\n]");
        fscanf(f, " \n");
    }

    fclose(f);
    return 0;
}

static int ini_callback(const char *section, const char *name, const char *value)
{
    //fprintf(stdout, "[%s] '%s'='%s'\n", section, name, value);
    #define process(option)                                             \
    if (strcasecmp(name, #option) == 0) {                               \
        if (strcasecmp(value, "true") == 0) options.option = 1;         \
        else if (strcasecmp(value, "false") == 0) options.option = 0;}
    #define process_next(option) else process(option)

    // this option has an extra allowed value, "prompt"
    if(strcasecmp(name, "use_fixes_and_enhancements") == 0) {
        if (strcasecmp(value, "true") == 0) options.use_fixes_and_enhancements = 1;         \
        else if (strcasecmp(value, "false") == 0) options.use_fixes_and_enhancements = 0;
        else if (strcasecmp(value, "prompt") == 0) options.use_fixes_and_enhancements = 2;
    }

    process_next(enable_copyprot)
    process_next(enable_mixer)
    process_next(enable_fade)
    process_next(enable_flash)
    process_next(enable_text)
    process_next(enable_quicksave)
    process_next(enable_quicksave_penalty)
    process_next(enable_replay)
    process_next(enable_crouch_after_climbing)
    process_next(enable_freeze_time_during_end_music)
    process_next(fix_gate_sounds)
    process_next(fix_two_coll_bug)
    process_next(fix_infinite_down_bug)
    process_next(fix_gate_drawing_bug)
    process_next(fix_bigpillar_climb)
    process_next(fix_jump_distance_at_edge)
    process_next(fix_edge_distance_check_when_climbing)
    process_next(fix_painless_fall_on_guard)
    process_next(fix_wall_bump_triggers_tile_below)
    process_next(fix_stand_on_thin_air)
    process_next(fix_press_through_closed_gates)
    process_next(fix_grab_falling_speed)
    process_next(fix_skeleton_chomper_blood)
    process_next(fix_move_after_drink)
    process_next(fix_loose_left_of_potion)
    process_next(fix_guard_following_through_closed_gates)
    process_next(fix_safe_landing_on_spikes)

    #undef process_next
    #undef process

    // the following options have syntax other than "true" or "false"
    else if (strcasecmp(name, "difficulty") == 0) {
        if (strcasecmp(value, "normal") == 0) options.difficulty = 0;
        else if (strcasecmp(value, "hard") == 0) options.difficulty = 1;
        else if (strcasecmp(value, "impossible") == 0) options.difficulty = 2;
        else if (strcasecmp(value, "prompt") == 0) options.difficulty = 0xFF;
    }

    return 0;
}

void load_options() {
    use_default_options();
    ini_load("SDLPoP.ini", ini_callback);
    if (!options.use_fixes_and_enhancements) disable_fixes_and_enhancements();
}

#ifdef USE_DIFFICULTY
void load_difficulty() {
    if (check_param("normal")) options.difficulty = 0;
    if (check_param("hard")) options.difficulty = 1;
    if (check_param("impossible")) options.difficulty = 2;
    difficulty = options.difficulty; // apply option
    if (difficulty <= 2) return; // difficulty already chosen in SDLPoP.ini or overridden with a commandline param

    draw_rect(&screen_rect, 0);
    show_text(&screen_rect, 0, 0, "Select difficulty. . . .\n\n0: normal     \n1: hard        \n2: impossible\n\n");
    SDL_Event event;
    while (difficulty > 2) { // difficulty "select" option = 0xFF
        SDL_WaitEvent(&event);
        switch(event.type) {
            case SDL_QUIT:
                quit(0);
                break;
            case SDL_KEYDOWN:
                switch(event.key.keysym.scancode) {
                    case SDL_SCANCODE_0:
                    case SDL_SCANCODE_KP_0:
                        difficulty = 0;
                        break;
                    case SDL_SCANCODE_1:
                    case SDL_SCANCODE_KP_1:
                        difficulty = 1;
                        break;
                    case SDL_SCANCODE_2:
                    case SDL_SCANCODE_KP_2:
                        difficulty = 2;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        SDL_Delay(10);
    }
}
#endif // USE_DIFFICULTY


void show_disable_fixes_prompt() {
    if (options.use_fixes_and_enhancements != 2) return;
    draw_rect(&screen_rect, 0);
    show_text(&screen_rect, 0, 0,
		"\n"
		"Enable bug fixes and\n"
		"gameplay enhancements?\n"
		"\n"
		"NOTE:\n"
		"This option disables some game quirks.\n"
		"Certain tricks will no longer work by default.\n"
		"\n"
		"\n"
		"Y:  enhanced behavior \n"
		"N:  original behavior    \n"
		"\n"
		"Y / N ?\n"
		"\n"
		"\n"
		"\n"
		"You can fine-tune your preferences\n"
		"and/or bypass this screen by editing the file\n"
		"'SDLPoP.ini'"
	);
    while (options.use_fixes_and_enhancements == 2 ) {
		idle();
        switch (key_test_quit()) {
            case SDL_SCANCODE_Y:
                options.use_fixes_and_enhancements = 1;
                printf("Enabling game fixes and enhancements.\n");
                break;
            case SDL_SCANCODE_N:
                options.use_fixes_and_enhancements = 0;
                printf("Disabling game fixes and enhancements.\n");
                break;
		}
    }
    if (!options.use_fixes_and_enhancements) disable_fixes_and_enhancements();
}

