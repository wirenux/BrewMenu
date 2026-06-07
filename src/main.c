#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

#define MENU_WIDTH 45
#define MENU_HEIGHT 10

void draw_background(int max_y, int max_x) {
    attron(COLOR_PAIR(3) | A_BOLD);
    for (int x = 0; x < max_x; x++) {
        mvaddch(0, x, ' ');
    }

    char *title = "BrewMenu";
    mvprintw(0, (max_x - (int)strlen(title)) / 2, "%s", title);
    attroff(COLOR_PAIR(3) | A_BOLD);

    attron(COLOR_PAIR(3));
    mvprintw(max_y - 1, 0, " Use Arrow Keys to navigate, [ENTER] to select, 'q' to quit");

    for (int x = 60; x < max_x; x++) {
        mvaddch(max_y - 1, x, ' ');
    }
    
    attroff(COLOR_PAIR(3));
}

int main(void) {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    if (!has_colors()) {
        endwin();
        puts("\e[1;31mError:\e[0m Your terminal doesn't support colors!\n");
        return 1;
    }
    start_color();

    init_pair(1, COLOR_WHITE, COLOR_BLUE); // bg
    init_pair(2, COLOR_BLACK, COLOR_WHITE); // body
    init_pair(3, COLOR_WHITE, COLOR_BLACK); // shadow

    wbkgd(stdscr, COLOR_PAIR(1));

    char *main_option[] = {
        "Install packages",
        "Search packages",
        "List installed packages",
        "Uninstall packages"
    };

    int total_option = 4;
    int selected_index = 0;
    int ch;

    while (1) {
        clear();

        int max_x, max_y;
        getmaxyx(stdscr, max_y, max_x);

        draw_background(max_y, max_x);
        
        int start_y = (max_y - MENU_HEIGHT) / 2;
        int start_x = (max_x - MENU_WIDTH) / 2;

        attron(COLOR_PAIR(3));
        for (int i = 0; i < MENU_HEIGHT; i++) {
            mvprintw(start_y + i + 1, start_x + MENU_WIDTH, "  "); // right shadow
        }
        for (int i = 0; i < MENU_WIDTH; i++) {
            mvaddch(start_y + MENU_HEIGHT, start_x + i + 1, ' '); // right shadow
        }
        attroff(COLOR_PAIR(3));

        WINDOW *menu_win = newwin(MENU_HEIGHT, MENU_WIDTH, start_y, start_x);
        wbkgd(menu_win, COLOR_PAIR(2));
        box(menu_win, 0, 0);

        mvwprintw(menu_win, 0, (MENU_WIDTH - 13) / 2, "[ Main Menu ]");

        for (int i = 0; i < total_option; i++) {
            if (i == selected_index) {
                wattron(menu_win, COLOR_PAIR(3));
            }

            mvwprintw(menu_win, i + 3, 4, "  %s  ", main_option[i]);

            if (i == selected_index) {
                wattroff(menu_win, COLOR_PAIR(3));
            }
        }

        refresh();
        wrefresh(menu_win);

        ch = getch();
        delwin(menu_win);

        if (ch == 'q') {
            break;
        } else if (ch == KEY_UP && selected_index > 0) {
            selected_index--;
        } else if (ch == KEY_DOWN && selected_index < total_option - 1) {
            selected_index++;
        }
    }

    endwin();
    return 0;
}