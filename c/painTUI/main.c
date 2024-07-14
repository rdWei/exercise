#include <ncurses.h>
#include <stdio.h>

void printPalette(WINDOW* win);

int main() {
    initscr();
    cbreak();
    noecho();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_WHITE);
    init_pair(10, COLOR_WHITE, COLOR_BLUE);
    init_pair(11, COLOR_WHITE, COLOR_WHITE);
    init_pair(17, COLOR_WHITE, COLOR_YELLOW);
    init_pair(12, COLOR_WHITE, COLOR_BLACK);
    init_pair(13, COLOR_WHITE, COLOR_RED);
    init_pair(14, COLOR_WHITE, COLOR_CYAN);
    init_pair(15, COLOR_WHITE, COLOR_GREEN);
    init_pair(16, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_WHITE, COLOR_CYAN);
    init_pair(3, COLOR_BLACK, COLOR_YELLOW);

    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    printf("\033[?1003h\n");

    bkgd(COLOR_PAIR(10));
    clear();

    MEVENT event;
    int left_button_pressed = 0;

    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);

    WINDOW* tela = newwin(max_y - 2, max_x - 7, 1, 1);
    WINDOW* palette = newwin(10, 3, max_y / 2 - 5, max_x - 5);

    int c = '/';

    int current_tela_bg = 12;
    int current_color = 11;

    for (;;) {
        box(tela, 0, 0);
        wrefresh(tela);
        printPalette(palette);

        if (c == 'q') {
            printf("\033[?1003l\n");
            endwin();
            return 0;
        } else if (c == 't') {
            if (current_tela_bg >= 17) {
                current_tela_bg = 11;
            }
            wbkgd(tela, COLOR_PAIR(current_tela_bg));
            wclear(tela);
            current_tela_bg++;
        }

        switch (c) {
            case 'c': current_color = 14; break;
            case 'r': current_color = 13; break;
            case 'g': current_color = 15; break;
            case 'b': current_color = 16; break;
            case 'y': current_color = 17; break;
        }

        if (c == ERR) {
            // No action if there are no events
        } else if (c == KEY_MOUSE) {
            if (getmouse(&event) == OK) {
                if (event.bstate & BUTTON1_PRESSED) {
                    left_button_pressed = 1;
                } else if (event.bstate & BUTTON1_RELEASED) {
                    left_button_pressed = 0;
                }

                if (left_button_pressed) {
                    if (event.y >= 1 && event.y < max_y - 1 && event.x >= 1 && event.x < max_x - 7) {
                        wattron(tela, COLOR_PAIR(current_color));
                        mvwprintw(tela, event.y - 1, event.x - 1, " ");
                        wattroff(tela, COLOR_PAIR(current_color));
                        wrefresh(tela);
                    }
                }
            }
        }

        box(palette, 0, 0);
        wrefresh(palette);
        c = wgetch(stdscr);
    }

    printf("\033[?1003l\n");
    endwin();
    return 0;
}

void printPalette(WINDOW* win) {
    wattron(win, COLOR_PAIR(2));
    mvwprintw(win, 1, 1, "C");
    wattroff(win, COLOR_PAIR(2));
    
    wattron(win, COLOR_PAIR(17));
    mvwprintw(win, 2, 1, "Y");
    wattroff(win, COLOR_PAIR(17));

    wattron(win, COLOR_PAIR(13));
    mvwprintw(win, 3, 1, "R");
    wattroff(win, COLOR_PAIR(13));

    wattron(win, COLOR_PAIR(15));
    mvwprintw(win, 4, 1, "G");
    wattroff(win, COLOR_PAIR(15));

    wattron(win, COLOR_PAIR(16));
    mvwprintw(win, 5, 1, "B");
    wattroff(win, COLOR_PAIR(16));

    box(win, 0, 0);
    wrefresh(win);
}

