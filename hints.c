#include "common.h"


#ifdef USE_HINTS

rect_type hints_rect = {135, 36, 179, 284};
dialog_type* hints_dialog;
word room_status[24] = {0};

void init_hints_dialog() {
    hints_dialog = make_dialog_info(&dialog_settings, &hints_rect, &hints_rect, NULL);
    hints_dialog->peel = read_peel_from_screen(&hints_dialog->peel_rect);
}

void show_hint_dialog(char *text) {
    word key;
    rect_type rect;
    screen_updates_suspended = 1;
    method_1_blit_rect(offscreen_surface, onscreen_surface_, &hints_dialog->peel_rect, &hints_dialog->peel_rect, 0);
    draw_dialog_frame(hints_dialog);
    shrink2_rect(&rect, &hints_dialog->text_rect, 2, 1);
    show_text_with_color(&rect, 0, 0, text, color_15_white);
    screen_updates_suspended = 0;
    request_screen_update();
    clear_kbd_buf();
    do {
        idle();
        key = key_test_quit(); // Press any key to continue...
    } while(key == 0);
    savekid();
    redraw_screen(0);
    loadkid();
    return;
}

#define ROOM_NOT_VISITED 0
#define ROOM_VISITED 10

void visit_room(word room) {
    if (room_status[room] == 0) { // first visit
        room_status[room] = ROOM_VISITED;
    }
}

void show_hint() {
    char hint_text[128] = "Hm..."; // default hint text
    #define set_text(text) strcpy(hint_text, text)

    switch (current_level) {
        case 1:
            switch (curr_room) {
                case 2: // starting room
                    if (room_status[1] == ROOM_NOT_VISITED)
                        set_text("Classic...\nThe door slammed shut the moment I stepped through!");
                    else set_text("It looks like there might be another path down there...");
                    break;
                case 1: // chasm next to starting room
                    if (Kid.curr_col <= 3) {
                        set_text("Returning here may be difficult\nafter making that jump...");
                    } else if (room_status[5] == ROOM_VISITED) {
                        set_text("The drop looks painful, but survivable...");
                    }
                    break;
                case 5: // life potion room
                    if (room_status[17] == ROOM_NOT_VISITED) {
                        if (hitp_max == 3) set_text("Suspicious...");
                        else set_text("Should this have been so easy?");
                    }
                    else {
                        if (hitp_max == 3) set_text("Yay, a life potion!");
                        else if (leveldoor_open) set_text("Now to get to the exit...");
                    }
                    break;
                case 4: // room below chasm
                    set_text("OK, so where do I go now?");
                    break;
                case 11:
                    set_text("Is this what we call a leap of faith? It looks a little dangerous.");
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    show_hint_dialog(hint_text);
}

void reset_hints() {
    memset(&room_status, 0, COUNT(room_status));
}





#endif
