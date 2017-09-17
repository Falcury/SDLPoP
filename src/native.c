/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2017  Dávid Nagy

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

#ifdef _WIN32
#include <windows.h>
#else
// TODO: window menu for X11 and other platforms
#endif



// Need access to SDL_SysWMinfo to get the window handle
#if !defined(_MSC_VER)
#include "SDL2/SDL_syswm.h"
#else
#include "SDL_syswm.h"
#endif




#ifdef _WIN32


char* get_win32_error_message(DWORD error) {
    void* message;
    FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR) &message,
            0, NULL );

    return (char*) message;
}

typedef struct menubar_type {
    SDL_Window* window;
    HWND hwnd;
    HMENU handle;
    HMENU menu_game;
    HMENU menu_capture;
    HMENU menu_capture_replay_from;
    HMENU menu_options;
    HMENU menu_cheats;
    HMENU menu_cheats_rooms;
} menubar_type;

void show_menubar() {
    if (main_window_menubar == NULL) return;
    SetMenu(main_window_menubar->hwnd, main_window_menubar->handle);
    is_main_menubar_shown = 1;
}

void hide_menubar() {
    if (main_window_menubar == NULL) return;
    SetMenu(main_window_menubar->hwnd, NULL);
    is_main_menubar_shown = 0;
}

#define CHECKED_IF(expr) ( (UINT) ((expr) ? MF_CHECKED : MF_UNCHECKED) )
#define GRAYED_IF(expr) ( (UINT) ((expr) ? MF_GRAYED : 0) )

menubar_type* create_native_menubar() {
    menubar_type* menubar = (menubar_type*) calloc(1, sizeof(menubar_type));

    menubar->handle = CreateMenu();
    menubar->menu_game = CreateMenu();
    menubar->menu_capture = CreateMenu();
    menubar->menu_capture_replay_from = CreateMenu();
    menubar->menu_options = CreateMenu();
    menubar->menu_cheats = CreateMenu();
    menubar->menu_cheats_rooms = CreateMenu();

    HMENU current_menu = menubar->menu_game;
    AppendMenuA(menubar->handle, MF_POPUP, (UINT_PTR)current_menu, "&Game");
    {
        AppendMenuA(current_menu, MF_STRING, MENU_RESTART_GAME, "&Restart game\tCtrl+R");
        AppendMenuA(current_menu, MF_STRING, MENU_RESTART_LEVEL, "&Restart level\tCtrl+A");
        AppendMenuA(current_menu, MF_SEPARATOR, 0, NULL);

        HMENU previous_menu = current_menu;
        current_menu = CreateMenu();
        AppendMenuA(previous_menu, MF_POPUP, (UINT_PTR)current_menu, "Levelset");
        {
            AppendMenuA(current_menu, MF_STRING | MF_CHECKED, MENU_LEVELSET_ORIGINAL, "&Prince of Persia (original)");
            AppendMenuA(current_menu, MF_SEPARATOR, 0, NULL);
            AppendMenuA(current_menu, MF_STRING, MENU_LEVELSET_SELECT, "&Select...");
        }
        current_menu = previous_menu;

        AppendMenuA(current_menu, MF_SEPARATOR, 0, NULL);
        AppendMenuA(current_menu, MF_STRING, MENU_SAVE_GAME, "&Save game...");
        AppendMenuA(current_menu, MF_STRING, MENU_LOAD_GAME, "&Load game...");
        AppendMenuA(current_menu, MF_SEPARATOR, 0, NULL);
        AppendMenuA(current_menu, MF_STRING, MENU_QUICKSAVE, "&Quicksave\tF6");
        AppendMenuA(current_menu, MF_STRING, MENU_QUICKLOAD, "&Quickload\tF9");
        AppendMenuA(current_menu, MF_SEPARATOR, 0, NULL);
        AppendMenuA(current_menu, MF_STRING, MENU_LOAD_REPLAY, "&Load replay...");
        AppendMenuA(current_menu, MF_SEPARATOR, 0, NULL);
        AppendMenuA(current_menu, MF_STRING, MENU_QUIT, "&Quit\tCtrl+Q");
    }

    current_menu = menubar->menu_options;
    AppendMenuA(menubar->handle, MF_POPUP, (UINT_PTR)current_menu, "&Options");
    {

        HMENU previous_menu = current_menu;
        current_menu = CreateMenu();
        AppendMenuA(previous_menu, MF_POPUP, (UINT_PTR)current_menu, "Video");
        {
            AppendMenuA(current_menu, MF_STRING | CHECKED_IF(start_fullscreen), MENU_ENABLE_FULLSCREEN, "&Enable fullscreen\tAlt+Enter");
            AppendMenuA(current_menu, MF_STRING | CHECKED_IF(use_correct_aspect_ratio), MENU_USE_CORRECT_ASPECT_RATIO, "&Use 4:3 aspect ratio");
            AppendMenuA(current_menu, MF_SEPARATOR, 0, NULL);
            AppendMenuA(current_menu, MF_STRING | CHECKED_IF(enable_lighting), MENU_ENABLE_LIGHTING, "&Enable lighting");

        }
        current_menu = previous_menu;

        previous_menu = current_menu;
        current_menu = CreateMenu();
        AppendMenuA(previous_menu, MF_POPUP, (UINT_PTR)current_menu, "Sound");
        {
            AppendMenuA(current_menu, MF_STRING | CHECKED_IF(is_sound_on), MENU_ENABLE_SOUND, "&Enable sound\tCtrl+S");
            AppendMenuA(current_menu, MF_STRING | CHECKED_IF(enable_mixer), MENU_ENABLE_MUSIC, "&Enable music");
        }
        current_menu = previous_menu;

        previous_menu = current_menu;
        current_menu = CreateMenu();
        AppendMenuA(previous_menu, MF_POPUP, (UINT_PTR)current_menu, "Controller");
        {
            HMENU previous_menu2 = current_menu;
            current_menu = CreateMenu();
            AppendMenuA(previous_menu2, MF_POPUP, (UINT_PTR)current_menu, "Joystick movement");
            {
                AppendMenuA(current_menu, MF_STRING | CHECKED_IF(joystick_only_horizontal), MENU_JOY_HORIZONTAL_ONLY, "&Horizontal only");
                AppendMenuA(current_menu, MF_STRING | CHECKED_IF(!joystick_only_horizontal), MENU_JOY_ALL_DIRECTIONAL, "&All-directional");
            }
            current_menu = previous_menu2;
            AppendMenuA(current_menu, MF_STRING | CHECKED_IF(enable_controller_rumble), MENU_ENABLE_CONTROLLER_RUMBLE, "&Enable controller rumble");

        }
        current_menu = previous_menu;

        previous_menu = current_menu;
        current_menu = CreateMenu();
        AppendMenuA(previous_menu, MF_POPUP, (UINT_PTR)current_menu, "Bug fixes / enhancements");
        {
            AppendMenuA(current_menu, MF_STRING | CHECKED_IF(!use_fixes_and_enhancements), MENU_DISABLE_ALL_FIXES_ENHANCEMENTS, "&Disable all (original behavior)");
            AppendMenuA(current_menu, MF_STRING | CHECKED_IF(use_fixes_and_enhancements), MENU_ENABLE_ALL_FIXES_ENHANCEMENTS, "&Enable all");
            AppendMenuA(current_menu, MF_STRING | MF_UNCHECKED, MENU_SELECT_FIXES_ENHANCEMENTS, "&Enable selected...");
        }
        current_menu = previous_menu;
        // TODO: submenu gameplay fixes?

        AppendMenuA(current_menu, MF_SEPARATOR, 0, NULL);
        AppendMenuA(current_menu, MF_STRING, MENU_EDIT_CONFIGURATION_FILE, "&Edit configuration file...");
        AppendMenuA(current_menu, MF_STRING, MENU_RELOAD_CONFIGURATION_FILE, "&Reload configuration file");
    }

    current_menu = CreateMenu();
    AppendMenuA(menubar->handle, MF_POPUP, (UINT_PTR)current_menu, "&Capture");
    {
        AppendMenuA(current_menu, MF_STRING, MENU_SCREENSHOT, "&Screenshot\tShift+F12");
        AppendMenuA(current_menu, MF_STRING, MENU_LEVEL_SCREENSHOT, "&Level screenshot\tCtrl+Shift+F12");
        AppendMenuA(current_menu, MF_SEPARATOR, 0, NULL);
        AppendMenuA(current_menu, MF_STRING, MENU_RECORD_REPLAY, "&Replay\tCtrl+Tab");

        HMENU previous_menu = current_menu;
        current_menu = menubar->menu_capture_replay_from;
        {
            AppendMenuA(previous_menu, MF_POPUP, (UINT_PTR)current_menu, "Record from");
            AppendMenuA(current_menu, MF_STRING | MF_CHECKED, MENU_RECORD_FROM_CURRENT_POSITION, "&Current position");
            AppendMenuA(current_menu, MF_STRING, MENU_RECORD_FROM_CURRENT_LEVEL_START, "&Start of level");
            AppendMenuA(current_menu, MF_STRING, MENU_RECORD_FROM_QUICKSAVE, "&Quicksave");
        }
        current_menu = previous_menu;

    }

    // Only show the cheat menu if cheats are enabled.
    if (cheats_enabled) {
        current_menu = CreateMenu();
        AppendMenuA(menubar->handle, MF_POPUP, (UINT_PTR)current_menu, "Cheat");
        {
            AppendMenuA(current_menu, MF_STRING | CHECKED_IF(cheats_enabled), MENU_ENABLE_CHEATS, "&Enable cheats");
            AppendMenuA(current_menu, MF_SEPARATOR, 0, NULL);

            UINT cheat_grayed = GRAYED_IF(!cheats_enabled);
            AppendMenuA(current_menu, MF_STRING | cheat_grayed, MENU_CHEAT_NEXT_LEVEL, "&Next level\tShift+L");
            AppendMenuA(current_menu, MF_STRING | cheat_grayed, MENU_CHEAT_HEALTH, "&Health\tShift+S");
            AppendMenuA(current_menu, MF_STRING | cheat_grayed, MENU_CHEAT_LIFE, "&Life\tShift+T");
            AppendMenuA(current_menu, MF_STRING | cheat_grayed, MENU_CHEAT_FEATHER_FALL, "&Feather fall\tShift+W");
            AppendMenuA(current_menu, MF_STRING | cheat_grayed, MENU_CHEAT_RESURRECT, "&Resurrect\tR");
            AppendMenuA(current_menu, MF_STRING | cheat_grayed, MENU_CHEAT_KILL_GUARD, "&Kill guard\tK");
            AppendMenuA(current_menu, MF_STRING | cheat_grayed, MENU_CHEAT_INCREASE_TIME, "&Increase time\t+");
            AppendMenuA(current_menu, MF_STRING | cheat_grayed, MENU_CHEAT_DECREASE_TIME, "&Decrease time\t-");
            AppendMenuA(current_menu, MF_SEPARATOR, 0, NULL);


            HMENU previous_menu = current_menu;
            current_menu = menubar->menu_cheats_rooms;
            {
                AppendMenuA(previous_menu, MF_POPUP | cheat_grayed, (UINT_PTR)current_menu, "Look at room");
                AppendMenuA(current_menu, MF_STRING | cheat_grayed, MENU_CHEAT_LOOK_LEFT, "&Left\tH");
                AppendMenuA(current_menu, MF_STRING | cheat_grayed, MENU_CHEAT_LOOK_RIGHT, "&Right\tJ");
                AppendMenuA(current_menu, MF_STRING | cheat_grayed, MENU_CHEAT_LOOK_ABOVE, "&Above\tU");
                AppendMenuA(current_menu, MF_STRING | cheat_grayed, MENU_CHEAT_LOOK_BELOW, "&Below\tN");

            }
            current_menu = previous_menu;

            AppendMenuA(current_menu, MF_STRING | cheat_grayed, MENU_CHEAT_SHOW_ADJACENT_ROOMS, "&Show adjacent rooms\tC");
            AppendMenuA(current_menu, MF_STRING | cheat_grayed, MENU_CHEAT_SHOW_DIAGONAL_ROOMS, "&Show diagonal rooms\tShift+C");

            AppendMenuA(current_menu, MF_SEPARATOR, 0, NULL);
            AppendMenuA(current_menu, MF_STRING | cheat_grayed, MENU_CHEAT_FLIP_SCREEN_VERTICALLY, "&Flip screen vertically\tShift+I");
            AppendMenuA(current_menu, MF_STRING | cheat_grayed, MENU_CHEAT_BLIND_MODE, "&Blind mode\tShift+B");

            if (debug_cheats_enabled) {
                AppendMenuA(current_menu, MF_SEPARATOR, 0, NULL);
                AppendMenuA(current_menu, MF_STRING, MENU_CHEAT_TOGGLE_TIMER, "&Toggle timer\tT");
                AppendMenuA(current_menu, MF_STRING, MENU_CHEAT_NUDGE_LEFT, "&Nudge left\t[");
                AppendMenuA(current_menu, MF_STRING, MENU_CHEAT_NUDGE_RIGHT, "&Nudge right\t]");
            }

        }
    }

    return menubar;

}

void show_stub_message() {
    printf("TODO: This menu item is not yet implemented!\n");
}

void check_menu_item(dword menu_item, int checked) {
    CheckMenuItem(main_window_menubar->handle, menu_item, CHECKED_IF(checked));
}

LRESULT CALLBACK
win32_window_callback(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    LRESULT result = 0;
    switch (message) {
        case WM_COMMAND: {
            switch (wparam) {
                case MENU_RESTART_GAME:
                    last_menu_command_scancode = SDL_SCANCODE_R | WITH_CTRL;
                    break;
                case MENU_RESTART_LEVEL:
                    last_menu_command_scancode = SDL_SCANCODE_A | WITH_CTRL;
                    break;
                case MENU_SAVE_GAME:
                case MENU_LOAD_GAME:
                case MENU_DUMMY_ITEM:
                    show_stub_message();
                    break;
                case MENU_QUICKSAVE:
                    last_menu_command_scancode = SDL_SCANCODE_F6;
                    break;
                case MENU_QUICKLOAD:
                    last_menu_command_scancode = SDL_SCANCODE_F9;
                    break;
                case MENU_QUIT:
                    last_menu_command_scancode = SDL_SCANCODE_Q | WITH_CTRL;
                    break;

                case MENU_ENABLE_FULLSCREEN:
                    toggle_fullscreen();
                    break;
                case MENU_USE_CORRECT_ASPECT_RATIO:
                    use_correct_aspect_ratio = (byte) !use_correct_aspect_ratio;
                    apply_aspect_ratio();
                    request_screen_update();
                    check_menu_item(MENU_USE_CORRECT_ASPECT_RATIO, use_correct_aspect_ratio);
                    break;

                case MENU_ENABLE_LIGHTING:
                    enable_lighting = (byte) !enable_lighting;
                    extern image_type* lighting_mask; // TODO: cleanup
                    if (lighting_mask == NULL) {
                        init_lighting();
                    }
                    need_full_redraw = 1;
                    check_menu_item(MENU_ENABLE_LIGHTING, enable_lighting);
                    break;

                case MENU_ENABLE_SOUND:
                    turn_sound_on_off((!is_sound_on) * 15);
                    break;

                case MENU_ENABLE_MUSIC:
                    // TODO: this doesn't actually do anything..
                    enable_mixer = (byte) !enable_mixer;
                    check_menu_item(MENU_ENABLE_MUSIC, enable_mixer);
                    break;

                case MENU_JOY_HORIZONTAL_ONLY:
                    joystick_only_horizontal = 1;
                    check_menu_item(MENU_JOY_HORIZONTAL_ONLY, 1);
                    check_menu_item(MENU_JOY_ALL_DIRECTIONAL, 0);
                    break;

                case MENU_JOY_ALL_DIRECTIONAL:
                    joystick_only_horizontal = 0;
                    check_menu_item(MENU_JOY_HORIZONTAL_ONLY, 0);
                    check_menu_item(MENU_JOY_ALL_DIRECTIONAL, 1);
                    break;

                case MENU_ENABLE_CONTROLLER_RUMBLE:
                    enable_controller_rumble = (byte) !enable_controller_rumble;
                    check_menu_item(MENU_ENABLE_CONTROLLER_RUMBLE, enable_controller_rumble);
                    break;

                case MENU_DISABLE_ALL_FIXES_ENHANCEMENTS:
                    show_stub_message();
                    break;
                case MENU_ENABLE_ALL_FIXES_ENHANCEMENTS:
                    show_stub_message();
                    break;
                case MENU_SELECT_FIXES_ENHANCEMENTS:
                    show_stub_message();
                    break;
                case MENU_EDIT_CONFIGURATION_FILE:
                    show_stub_message();
                    break;
                case MENU_RELOAD_CONFIGURATION_FILE:
                    show_stub_message();
                    break;


                case MENU_SCREENSHOT:       last_menu_command_scancode = SDL_SCANCODE_F12 | WITH_SHIFT;             break;
                case MENU_LEVEL_SCREENSHOT: last_menu_command_scancode = SDL_SCANCODE_F12 | WITH_CTRL | WITH_SHIFT; break;
                case MENU_RECORD_REPLAY:    last_menu_command_scancode = SDL_SCANCODE_TAB | WITH_CTRL;              break;
                case MENU_RECORD_FROM_CURRENT_POSITION:
                    show_stub_message();
                    break;
                case MENU_RECORD_FROM_CURRENT_LEVEL_START:
                    show_stub_message();
                    break;
                case MENU_RECORD_FROM_QUICKSAVE:
                    show_stub_message();
                    break;


                case MENU_ENABLE_CHEATS:
                    cheats_enabled = (word) !cheats_enabled;
                    check_menu_item(MENU_ENABLE_CHEATS, cheats_enabled);
                    UINT grayed = GRAYED_IF(!cheats_enabled);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_NEXT_LEVEL, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_HEALTH, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_LIFE, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_FEATHER_FALL, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_RESURRECT, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_KILL_GUARD, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_INCREASE_TIME, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_DECREASE_TIME, grayed);
                    EnableMenuItem(main_window_menubar->handle, (UINT_PTR)main_window_menubar->menu_cheats_rooms, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_LOOK_LEFT, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_LOOK_RIGHT, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_LOOK_ABOVE, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_LOOK_BELOW, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_SHOW_ADJACENT_ROOMS, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_SHOW_DIAGONAL_ROOMS, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_FLIP_SCREEN_VERTICALLY, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_BLIND_MODE, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_TOGGLE_TIMER, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_NUDGE_LEFT, grayed);
                    EnableMenuItem(main_window_menubar->handle, MENU_CHEAT_NUDGE_RIGHT, grayed);
                    break;

                case MENU_CHEAT_NEXT_LEVEL:             last_menu_command_scancode = SDL_SCANCODE_L | WITH_SHIFT;   break;
                case MENU_CHEAT_HEALTH:                 last_menu_command_scancode = SDL_SCANCODE_S | WITH_SHIFT;   break;
                case MENU_CHEAT_LIFE:                   last_menu_command_scancode = SDL_SCANCODE_T | WITH_SHIFT;   break;
                case MENU_CHEAT_FEATHER_FALL:           last_menu_command_scancode = SDL_SCANCODE_W | WITH_SHIFT;   break;
                case MENU_CHEAT_RESURRECT:              last_menu_command_scancode = SDL_SCANCODE_R;                break;
                case MENU_CHEAT_KILL_GUARD:             last_menu_command_scancode = SDL_SCANCODE_K;                break;
                case MENU_CHEAT_INCREASE_TIME:          last_menu_command_scancode = SDL_SCANCODE_KP_PLUS;          break;
                case MENU_CHEAT_DECREASE_TIME:          last_menu_command_scancode = SDL_SCANCODE_KP_MINUS;         break;
                case MENU_CHEAT_LOOK_LEFT:              last_menu_command_scancode = SDL_SCANCODE_H;                break;
                case MENU_CHEAT_LOOK_RIGHT:             last_menu_command_scancode = SDL_SCANCODE_J;                break;
                case MENU_CHEAT_LOOK_ABOVE:             last_menu_command_scancode = SDL_SCANCODE_U;                break;
                case MENU_CHEAT_LOOK_BELOW:             last_menu_command_scancode = SDL_SCANCODE_N;                break;
                case MENU_CHEAT_SHOW_ADJACENT_ROOMS:    last_menu_command_scancode = SDL_SCANCODE_C;                break;
                case MENU_CHEAT_SHOW_DIAGONAL_ROOMS:    last_menu_command_scancode = SDL_SCANCODE_C | WITH_SHIFT;   break;
                case MENU_CHEAT_FLIP_SCREEN_VERTICALLY: last_menu_command_scancode = SDL_SCANCODE_I | WITH_SHIFT;   break;
                case MENU_CHEAT_BLIND_MODE:             last_menu_command_scancode = SDL_SCANCODE_B | WITH_SHIFT;   break;
                case MENU_CHEAT_TOGGLE_TIMER:           last_menu_command_scancode = SDL_SCANCODE_T;                break;
                case MENU_CHEAT_NUDGE_LEFT:             last_menu_command_scancode = SDL_SCANCODE_LEFTBRACKET;      break;
                case MENU_CHEAT_NUDGE_RIGHT:            last_menu_command_scancode = SDL_SCANCODE_RIGHTBRACKET;     break;


                default: break;
            }
        } break;

        default:
            result = DefWindowProcA(window, message, wparam, lparam);
    }

    return result;
}


SDL_Window* create_native_window(const char* window_title, int width, int height, menubar_type* menubar) {

    HINSTANCE instance = GetModuleHandleA(NULL);

    WNDCLASSA window_class = {0};
    window_class.style = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = win32_window_callback;
    window_class.hInstance = instance;
    window_class.lpszClassName = "SDLPoPMainWindow";

    if (!RegisterClassA(&window_class)) {
        DWORD error = GetLastError();
        char* error_message = get_win32_error_message(error);
        printf("Error: could not register the main window class (error %lu): %s\n", error, error_message);
        LocalFree(error_message);
    };

    bool window_has_menu;
    HMENU menu_handle;
    if (menubar != NULL) {
        window_has_menu = true;
        menu_handle = menubar->handle;
    } else {
        window_has_menu = false;
        menu_handle = NULL;
    }

    // The window dimensions need to be slightly bigger, we need to take the title and menu bar into account.
    RECT desired_window_rect = {0};
    desired_window_rect.right = width;
    desired_window_rect.bottom = height;
    DWORD window_style = WS_OVERLAPPEDWINDOW /*| WS_VISIBLE */; // NOTE: window starts hidden!
    AdjustWindowRect(&desired_window_rect, window_style, window_has_menu);
    int initial_window_width = desired_window_rect.right - desired_window_rect.left;
    int initial_window_height = desired_window_rect.bottom - desired_window_rect.top;


    // TODO: Place the window in the center of the screen, similar to what SDL2 does by default.

    HWND hwnd = CreateWindowExA(0, window_class.lpszClassName, window_title, window_style,
                                CW_USEDEFAULT, CW_USEDEFAULT, initial_window_width, initial_window_height,
                                NULL, menu_handle, instance, NULL);

    if (!hwnd) {
        DWORD error = GetLastError();
        char* error_message = get_win32_error_message(error);
        printf("Error: could not create the main window (error %lu): %s\n", error, error_message);
        LocalFree(error_message);
    }

    SDL_Window* sdl_window = SDL_CreateWindowFrom(hwnd);
    if (!sdl_window) {
        sdlperror("SDL_CreateWindowFrom (hwnd)");
        quit(1);
    }

    // Attach the menu bar to the window
    if (window_has_menu) {
        menubar->hwnd = hwnd;
        menubar->window = sdl_window;
        if (!start_fullscreen) {
            show_menubar();
        }
    }

    return sdl_window;

}

#endif // _WIN32

