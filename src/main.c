#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

#define MENU_WIDTH 45
#define MENU_HEIGHT 10

void draw_background(int max_y, int max_x, const char *title, const char *footer) {
    attron(COLOR_PAIR(3) | A_BOLD);
    for (int x = 0; x < max_x; x++) {
        mvaddch(0, x, ' ');
    }

    mvprintw(0, (max_x - (int)strlen(title)) / 2, "%s", title);
    attroff(COLOR_PAIR(3) | A_BOLD);

    attron(COLOR_PAIR(3));
    for (int x = 60; x < max_x; x++) {
        mvaddch(max_y - 1, x, ' ');
    }
    mvprintw(max_y - 1, 0, " %s", footer);
    attroff(COLOR_PAIR(3));
}

void view_installed_packages(int max_y, int max_x) {
    erase();
    draw_background(max_y, max_x, "BrewMenu - Installed Packages", "Fetching packages...");
    mvprintw(max_y / 2, (max_x - 22) / 2, "Loading packages from brew...");
    refresh();

    int capacity = 32;
    int total_packages = 0;
    char **packages = malloc(capacity * sizeof(char *));

    if (packages == NULL) {
        mvprintw(max_y / 2 + 1, (max_x - 30) / 2, "\e[1;31mError:\e[0m Failed to allocate memory");
        getch();
        return;
    }

    FILE *fp = popen("brew list -1", "r");
    if (fp == NULL) {
        mvprintw(max_y / 2 + 1, (max_x - 32) / 2, "\e[1;31mError:\e[0m Failed to run brew command");
        free(packages);
        getch();
        return;
    }

    char line[64];
    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\n")] = '\0';

        if (total_packages >= capacity) {
            capacity *= 2;
            char **temp = realloc(packages, capacity * sizeof(char *));
            if (temp == NULL) {
                pclose(fp);
                for (int i = 0; i < total_packages; i++) {
                    free(packages[i]);
                }
                free(packages);
                mvprintw(max_y / 2 + 1, (max_x - 33) / 2, "\e[1;31mError:\e[0m Failed to reallocate memory");
                getch();
                return;
            }
            packages = temp;
        }

        packages[total_packages] = strdup(line);
        total_packages++;
    }
    pclose(fp);

    if (total_packages == 0) {
        mvprintw(max_y / 2 + 1, (max_x - 25) / 2, "\e[1;31mError:\e[0m No brew packages found");
        free(packages);
        getch();
        return;
    }

    int selected_index = 0;
    int scroll_offset = 0;
    int ch;

    while (1) {
        erase();
        getmaxyx(stdscr, max_y, max_x);

        char title_buf[64];
        snprintf(title_buf, sizeof(title_buf), "Installed Packages (Total: %d)", total_packages);
        draw_background(max_y, max_x, title_buf, "Use Up/Down to scroll, 'q' to return to menu");

        int visible_rows = max_y - 3;

        for (int i = 0; i < visible_rows; i++) {
            int pkg_index = scroll_offset + i;

            if (pkg_index >= total_packages) {
                break;
            }

            if (pkg_index == selected_index) {
                attron(A_REVERSE);
            }

            mvprintw(i + 2, 4, "%3d - [ %s ]", pkg_index + 1, packages[pkg_index]);

            if (pkg_index == selected_index) {
                attroff(A_REVERSE);
            }
        }

        refresh();

        ch = getch();

        if (ch == 'q') {
            break;
        } else if (ch == KEY_UP && selected_index > 0) {
            selected_index--;
            if (selected_index < scroll_offset) {
                scroll_offset = selected_index;
            }
        } else if (ch == KEY_DOWN && selected_index < total_packages - 1) {
            selected_index++;
            if (selected_index >= scroll_offset + visible_rows) {
                scroll_offset = selected_index - visible_rows + 1;
            }
        }
    }

    for (int i = 0; i < total_packages; i++) {
        free(packages[i]);
    }
    free(packages);
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

        draw_background(max_y, max_x, "BrewMenu", "Use Arrow Keys to navigate, [ENTER] to select, 'q' to quit");
        
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
        } else if (ch == '\n' ||ch == KEY_ENTER || ch == '\r') {
            if (selected_index == 2) {
                view_installed_packages(max_y, max_x);
            } else {
                erase();
                draw_background(max_y, max_x, "BrewMenu", "Press any key to return...");
                mvprintw(max_y / 2, (max_x - 24) / 2, "Featurne not yet implemented!");
                refresh();
                getch();
            }
        }
    }

    endwin();
    return 0;
}