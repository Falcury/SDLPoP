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

#ifdef USE_SCRIPT

#include <libtcc.h>

// The compiler state
TCCState *s = NULL;

// Script functions called from the main program:
void (*on_init)(void) = NULL;
void (*on_load_room)(int) = NULL;
void (*on_start_game)(void) = NULL;
void (*on_load_level)(int) = NULL;
void (*on_end_level)(int) = NULL;
void (*on_drink_potion)(int) = NULL;
void (*custom_potion_anim)(int) = NULL;
void (*custom_timers)(void) = NULL;

// Used by custom_potion_anim
word* ptr_potion_color = NULL;
word* ptr_potion_pot_size = NULL;

// Used by on_level_end
word* ptr_next_level = NULL;

// Variable used in on_load_level, acts as a temporary 'reservation'
// for overriding the kid's level entry sequence (running, turning, falling, etc.)
word override_start_sequence = 0;


#define SAVELIST_MAX_VARS 256
#define SAVELIST_MAX_VAR_SIZE 65536
#define SAVELIST_MAX_VAR_NAME_LEN 64
#define SAVELIST_HEADER_BYTE 'S'

typedef struct savelist_var_type {
    byte name_len;
    char name[SAVELIST_MAX_VAR_NAME_LEN];
    word num_bytes;
    void* data;
} savelist_var_type;

savelist_var_type savelist[SAVELIST_MAX_VARS];
int savelist_num_vars = 0;
int savelist_size = 0;

// Writing and reading registered script variables to and from savestates:

void script__write_savelist(FILE* stream) {
    if (stream != NULL) {
        fputc(SAVELIST_HEADER_BYTE, stream);
        fwrite(&savelist_num_vars, sizeof(savelist_num_vars), 1, stream);
        int i;
        for (i = 0; i < savelist_num_vars; ++i) {
            byte name_len = savelist[i].name_len;
            word num_bytes = savelist[i].num_bytes;
            fwrite(&name_len,        sizeof(name_len),  1,         stream);
            fwrite(savelist[i].name, sizeof(char),      name_len,  stream);
            fwrite(&num_bytes,       sizeof(num_bytes), 1,         stream);
            fwrite(savelist[i].data, 1,                 num_bytes, stream);
        }
    }
}

void script__read_savelist(FILE* stream) {
    int savelist_num_vars_read = 0;
    if (stream != NULL) {
        // Confirm that script variables are actually included in the savestate
        byte header_byte = (byte) fgetc(stream);
        if (feof(stream) || ferror(stream)) {
            if (savelist_num_vars > 0) {
                fprintf(stderr, "Warning: Script variables cannot be restored: not found in savestate (expected %d).\n",
                        savelist_num_vars);
            }
            return;
        }
        if (header_byte != SAVELIST_HEADER_BYTE) {
            fseek(stream, -1, SEEK_CUR); // not a savelist
            return;
        }
        // Reserve enough memory as a buffer for the largest possible savelist variable
        byte* var_buffer = malloc(SAVELIST_MAX_VAR_SIZE);

        fread(&savelist_num_vars_read, sizeof(savelist_num_vars_read), 1, stream);
        if (savelist_num_vars != savelist_num_vars_read) {
            fprintf(stderr, "Warning: Found %d script variables in savestate; does not match "
                    "number expected by the active script (%d).\n", savelist_num_vars_read, savelist_num_vars);
        }

        // Read savestate's variables
        int i;
        for (i = 0; i < savelist_num_vars_read; ++i) {
            byte name_len_read = 0;
            char name_read[SAVELIST_MAX_VAR_NAME_LEN] = {0};
            word num_bytes_read = 0;

            fread(&name_len_read, sizeof(name_len_read), 1, stream);
            name_len_read = (byte) MIN(name_len_read, SAVELIST_MAX_VAR_NAME_LEN);
            fread(name_read, sizeof(char), name_len_read, stream);

            fread(&num_bytes_read, sizeof(num_bytes_read), 1, stream);
            num_bytes_read = (word) MIN(num_bytes_read, SAVELIST_MAX_VAR_SIZE);
            fread(var_buffer, 1, num_bytes_read, stream);

            if (feof(stream)) {
                fprintf(stderr, "Warning: Encountered unexpected end of file while restoring script variables "
                                "from a savestate.\n");
                return;
            }
            if (ferror(stream)) {
                fprintf(stderr, "Warning: A reading error occurred while restoring script variables "
                                "from a savestate.\n");
                return;
            }

            // Match with the script's registered variables
            int curr_var;
            for (curr_var = 0; curr_var < savelist_num_vars; ++curr_var) {
                if (strncmp(name_read, savelist[curr_var].name, SAVELIST_MAX_VAR_NAME_LEN) == 0) {
                    goto found;
                }
            }
            fprintf(stderr, "Warning: Savestate contains unregistered variable \"%s\".\n",
                    name_read);
            continue; // Matching script var not found, discard and read the next var in the savestate

            found:
            {
                // Matching script var found, try to replace that var's data with the data from the savestate
                word savelist_var_num_bytes = savelist[curr_var].num_bytes;
                if (savelist_var_num_bytes != num_bytes_read) {
                    fprintf(stderr, "Warning: Restored savestate variable \"%s\" has an unexpected size "
                                    "(%d bytes, expected %d bytes).\n",
                            savelist[curr_var].name, num_bytes_read, savelist_var_num_bytes);
                }
                memset(savelist[curr_var].data, 0, savelist_var_num_bytes);
                memcpy(savelist[curr_var].data, var_buffer, MIN(num_bytes_read, savelist_var_num_bytes));
            }
        }
        free(var_buffer);
    }
}


// Functions callable by the script start below:

// Registered variables will be saved in and loaded from savestates (replays, quicksave)
void script__register_savestate_variable(void* source, int var_num_bytes, char* variable_name) {
    if (var_num_bytes <= 0) {
        fprintf(stderr, "Script: Error in register_savestate_variable \"%s\": invalid number of bytes given (%d)\n",
                variable_name, var_num_bytes);
        return;
    }
    if (var_num_bytes > SAVELIST_MAX_VAR_SIZE) {
        fprintf(stderr, "Script: Error in register_savestate_variable \"%s\": variable size is too large (%d)\n",
                variable_name, var_num_bytes);
        return;
    }
    if (savelist_num_vars >= SAVELIST_MAX_VARS) {
        fprintf(stderr, "Script: Error in register_savestate_variable \"%s\": limit of %d savestate variables reached\n",
                variable_name, SAVELIST_MAX_VARS);
        return;
    }
    ++savelist_num_vars;
    savelist_size += var_num_bytes;
    savelist[savelist_num_vars-1] = (savelist_var_type) {(byte) strnlen(variable_name, SAVELIST_MAX_VAR_NAME_LEN), {0},
                                                         (word) var_num_bytes, source};
    strncpy(savelist[savelist_num_vars-1].name, variable_name, SAVELIST_MAX_VAR_NAME_LEN);

    //printf("Registering savestate variable %s. Value = %d\n", variable_name, *((int*) source));
}

word script__get_minutes_remaining(void) { return rem_min; }
word script__get_ticks_remaining(void) { return rem_tick; }

void script__set_time_remaining(word minutes, word ticks) {
    rem_min = minutes;
    rem_tick = ticks;
}

void script__select_tile(int room, int column, int row) {
    get_tile(room, column, row);
}

void script__select_tile_at_tilepos(word room, word tilepos) {
    get_tile(room, tilepos % 10, tilepos / 10);
}

byte script__get_curr_tile(void) {
    return curr_tile2;
}

byte script__get_curr_modifier(void) {
    if (curr_room > 0) {
        return curr_room_modif[curr_tilepos];
    }
    else return 0;
}

void script__set_curr_tile(byte new_tile) {
    if (curr_room > 0) {
        curr_room_tiles[curr_tilepos] = new_tile;
    }
}

void script__set_curr_modifier(byte new_modifier) {
    if (curr_room > 0) {
        curr_room_modif[curr_tilepos] = new_modifier;
    }
}

void script__set_curr_tile_and_modifier(byte new_tile, byte new_modifier) {
    if (curr_room > 0) {
        curr_room_tiles[curr_tilepos] = new_tile;
        curr_room_modif[curr_tilepos] = new_modifier;
    }
}

void script__set_tile(word room, word tilepos, byte new_tile) {
    if (room > 0 && room <= level.used_rooms && tilepos < 30) {
        get_room_address(room);
        curr_room_tiles[tilepos] = new_tile;
    }
}

void script__set_modifier(word room, word tilepos, byte new_modifier) {
    if (room > 0 && room <= level.used_rooms && tilepos < 30) {
        get_room_address(room);
        curr_room_modif[tilepos] = new_modifier;
    }
}

void script__set_tile_and_modifier(word room, word tilepos, byte new_tile, byte new_modifier) {
    if (room > 0 && room <= level.used_rooms && tilepos < 30) {
        get_room_address(room);
        curr_room_tiles[tilepos] = new_tile;
        curr_room_modif[tilepos] = new_modifier;
    }
}

word script__get_hp(void) {return hitp_curr; }

void script__set_hp(word new_hp) {
    hitp_delta = new_hp - hitp_curr;
}
word script__get_max_hp(void) {return hitp_max; }

void script__set_max_hp(word new_max_hp) {
    word old_hitp_max = hitp_max;
    hitp_max = new_max_hp;
    if (hitp_curr > hitp_max) hitp_curr = hitp_max; // remove excess health if necessary
    draw_kid_hp(hitp_curr, MAX(old_hitp_max, new_max_hp));
}

void script__set_flash(word color, word duration) {
    flash_color = color;
    flash_time = duration;
}

// Call this function only from within custom_potion_anim()
void script__set_potion_color(word color) {
    if (ptr_potion_color != NULL) {
        *ptr_potion_color = color;
    }
}

// Call this function only from within custom_potion_anim()
void script__set_potion_pot_size(word pot_size) {
    if (ptr_potion_pot_size != NULL) {
        *ptr_potion_pot_size = pot_size;
    }
}

void script__show_time(void) {
    text_time_remaining = 0;
    text_time_total = 0;
    is_show_time = 1;
}

short script__have_sword(void) { return have_sword; }
void script__set_have_sword(short kid_has_sword) { have_sword = kid_has_sword; }

word script__get_curr_level(void) { return current_level; }
word script__is_leveldoor_open(void) { return leveldoor_open; }

// Call this only from on_level_load
void script__set_level_start_sequence(word sequence_index) {
    override_start_sequence = sequence_index;
}

// Call this only from within on_level_end
void script__set_next_level(word level_number) {
    if (ptr_next_level != NULL) {
        *ptr_next_level = level_number;
    }
}

// Not callable directly! This simply applies the 'reservation' made by override_level_start_sequence
// (this is automatically called shortly after)
void script__apply_set_level_start_sequence() {
    if (override_start_sequence != 0) {
        seqtbl_offset_char(override_start_sequence);
        override_start_sequence = 0;
    }
}

void script__disable_level1_music(void) {
    need_level1_music = 0;
}


// End of functions that can be called by scripts.


// Loads an ANSI text file into a newly allocated buffer and returns the buffer.
char* load_script(char* filename) {
    char* buffer = NULL;
    FILE* script_file = fopen(filename, "rb");
    fseek(script_file, 0, SEEK_END);
    off_t script_file_size = ftell(script_file);
    if (script_file_size > 0) {
        buffer = malloc(script_file_size + 1);
        if (buffer != NULL) {
            rewind(script_file);
            fread(buffer, 1, script_file_size, script_file);
            buffer[script_file_size] = '\0';
        }
    }
    fclose(script_file);
    return buffer;
}

int init_script() {
    if (!(enable_scripts && use_custom_levelset)) return 1; // only load scripts as part of mods

    char filename[256];
    snprintf(filename, sizeof(filename), "mods/%s/%s", levelset_name, "mod.p1s");
    char* script_program = load_script(filename); // script must be an ANSI-encoded text file
    if (script_program == NULL) return 1;

    s = tcc_new();
    if (s == NULL) {
        fprintf(stderr, "Could not create tcc state\n");
        return 1;
    }

    tcc_add_include_path(s, "data/script/"); // location of pop_script.h
    tcc_set_lib_path(s, "data/script/"); // location of lib/libtcc1.a
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

    if (tcc_compile_string(s, script_program) == -1) {
        return 1;
    }
    free(script_program);

    // Data symbols accessible in the script.
    // They are linked as pointers. However, the script header file (pop_script.h) provides macros
    // that dereference those pointers, so you can still access the symbols as if they were "plain data" fields.
    tcc_add_symbol(s, "ptr_level", &level);
    tcc_add_symbol(s, "ptr_kid", &Kid);
    tcc_add_symbol(s, "ptr_guard", &Guard);

    // Function symbols accessible in the script:

    // Functions for proper communication between the main program and the script
    tcc_add_symbol(s, "register_savestate_variable_explicitly", script__register_savestate_variable);

    // PoP functions that can be called directly
    tcc_add_symbol(s, "play_sound", play_sound);
    tcc_add_symbol(s, "stop_sounds", stop_sounds);
    tcc_add_symbol(s, "draw_kid_hp", draw_kid_hp);
    tcc_add_symbol(s, "take_hp", take_hp);
    tcc_add_symbol(s, "set_hp_full", set_health_life);
    tcc_add_symbol(s, "set_char_sequence", seqtbl_offset_char);

    // Script functions
    tcc_add_symbol(s, "get_minutes_remaining", script__get_minutes_remaining);
    tcc_add_symbol(s, "get_ticks_remaining", script__get_ticks_remaining);
    tcc_add_symbol(s, "set_time_remaining", script__set_time_remaining);
    tcc_add_symbol(s, "select_tile_at_col_row", script__select_tile);
    tcc_add_symbol(s, "select_tile_at_tilepos", script__select_tile_at_tilepos);
    tcc_add_symbol(s, "get_curr_tile", script__get_curr_tile);
    tcc_add_symbol(s, "get_curr_modifier", script__get_curr_modifier);
    tcc_add_symbol(s, "set_curr_tile", script__set_curr_tile);
    tcc_add_symbol(s, "set_curr_modifier", script__set_curr_modifier);
    tcc_add_symbol(s, "set_curr_tile_and_modifier", script__set_curr_tile_and_modifier);
    tcc_add_symbol(s, "set_tile", script__set_tile);
    tcc_add_symbol(s, "set_modifier", script__set_modifier);
    tcc_add_symbol(s, "set_tile_and_modifier", script__set_tile_and_modifier);
    tcc_add_symbol(s, "get_hp", script__get_hp);
    tcc_add_symbol(s, "set_hp", script__set_hp);
    tcc_add_symbol(s, "get_max_hp", script__get_max_hp);
    tcc_add_symbol(s, "set_max_hp", script__set_max_hp);
    tcc_add_symbol(s, "set_flash", script__set_flash);
    tcc_add_symbol(s, "set_potion_color", script__set_potion_color);
    tcc_add_symbol(s, "set_potion_pot_size", script__set_potion_pot_size);
    tcc_add_symbol(s, "show_time", script__show_time);
    tcc_add_symbol(s, "have_sword", script__have_sword);
    tcc_add_symbol(s, "set_have_sword", script__set_have_sword);
    tcc_add_symbol(s, "get_curr_level", script__get_curr_level);
    tcc_add_symbol(s, "is_leveldoor_open", script__is_leveldoor_open);
    tcc_add_symbol(s, "set_next_level", script__set_next_level);
    tcc_add_symbol(s, "set_level_start_sequence", script__set_level_start_sequence);
    tcc_add_symbol(s, "disable_level1_music", script__disable_level1_music);

    /* relocate the code */
    if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0){
        return 1;
    }

    // Look for script entry points
    on_init = tcc_get_symbol(s, "on_init");
    on_load_room = tcc_get_symbol(s, "on_load_room");
    on_start_game = tcc_get_symbol(s, "on_start_game");
    on_load_level = tcc_get_symbol(s, "on_load_level");
    on_end_level = tcc_get_symbol(s, "on_end_level");
    on_drink_potion = tcc_get_symbol(s, "on_drink_potion");
    custom_potion_anim = tcc_get_symbol(s, "custom_potion_anim");
    custom_timers = tcc_get_symbol(s, "custom_timers");

    if (on_init != NULL) on_init(); // on_init called in the script itself

    return 0;
}


// Functions that invoke the script:

void script__on_load_room(int room) {
    if (on_load_room != NULL) on_load_room(room);
    get_room_address(drawn_room); // careful, scripted on_load_room() might change curr_room_tiles[]/modif[]!
}

void script__on_start_game(void) {
    #ifdef USE_REPLAY
    if (replaying) return;
    #endif
    if (on_start_game != NULL) on_start_game();
}

void script__on_load_level(int level_number) {
    override_start_sequence = 0;
    if (on_load_level != NULL) on_load_level(level_number);
}

void script__on_end_level(int level_number, word* next_level_number) {
    // ptr_next_level is used by set_next_level
    ptr_next_level = next_level_number; // do not expose raw pointers in the script
    if (on_end_level != NULL) on_end_level(level_number);
    ptr_next_level = NULL; // safety
}

void script__on_drink_potion(int potion_id) {
    if (on_drink_potion != NULL) on_drink_potion(potion_id);
}

void script__custom_potion_anim(int potion_id, word *color, word *pot_size) {
    // do not expose raw pointers in the script!
    // instead, we can call set_potion_color() and set_potion_pot_size() while we are in custom_potion_anim()
    ptr_potion_color = color;
    ptr_potion_pot_size = pot_size;
    if (custom_potion_anim != NULL) custom_potion_anim(potion_id);

    // safety: set_potion_color() and set_potion_pot_size() will not do anything when the pointers are NULL
    ptr_potion_color = NULL;
    ptr_potion_pot_size = NULL;
}

void script__custom_timers() {
    if (custom_timers != NULL) custom_timers();
}

#endif
