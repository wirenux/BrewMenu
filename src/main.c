#include <stdio.h>
#include <ncurses.h>

int main(void) {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    // dummy data
    char *packages[] = {"fastfetch", "nvim", "wget", "btop", "quickstart"};
    int total_packages = 5;
    int selected_index = 0;

    int ch;

    while (1) {
        clear();

        mvprintw(0, 2, "=== Brew Menu ===");

        for (int i = 0; i < total_packages; i++) {
            if (i == selected_index) {
                attron(A_REVERSE); // highlight turn on
            }

            mvprintw(i + 2, 2, "[ %s ]", packages[i]);

            if (i == selected_index) {
                attroff(A_REVERSE); // highlight turn off
            }
        }

        refresh();

        ch = getch();

        if (ch == 'q') {
            break;
        } else if (ch == KEY_UP && selected_index > 0) {
            selected_index--;
        } else if (ch == KEY_DOWN && selected_index < total_packages - 1) {
            selected_index++;
        }
    }

    endwin();
    return 0;
}