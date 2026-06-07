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

WINDOW* create_shadowed_window(int max_y, int max_x, int height, int width, const char *title) {
    int start_y = (max_y - height) / 2;
    int start_x = (max_x - width) / 2;

    attron(COLOR_PAIR(3));
    for (int i = 0; i < height; i++) {
        mvprintw(start_y + i + 1, start_x + width, "  ");
    }
    for (int i = 0; i < width + 1; i++) {
        mvaddch(start_y + height, start_x + i + 1, ' ');
    }
    attroff(COLOR_PAIR(3));

    WINDOW *win = newwin(height, width, start_y, start_x);
    wbkgd(win, COLOR_PAIR(2));
    box(win, 0, 0);

    if (title != NULL) {
        mvwprintw(win, 0, (width - (int)strlen(title) - 4) / 2, "[ %s ]", title);
    }

    return win;
}

void view_installed_packages(int max_y, int max_x) {
    erase();
    draw_background(max_y, max_x, "BrewMenu - Installed Packages", "Fetching packages...");

    WINDOW *load_win = create_shadowed_window(max_y, max_x, 5, 40, "Status");
    mvwprintw(load_win, 2, (40 - 29) / 2, "Loading packages from brew...");
    
    refresh();
    wrefresh(load_win);

    int capacity = 32;
    int total_packages = 0;
    char **packages = malloc(capacity * sizeof(char *));

    if (packages == NULL) {
        mvwprintw(load_win, 2, (40 - 25) / 2, "Error: Failed to allocate memory");
        getch();
        return;
    }

    FILE *fp = popen("brew list -1", "r");
    if (fp == NULL) {
        mvwprintw(load_win, 2, (40 - 33) / 2, "Error: Failed to run brew command");
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
                mvwprintw(load_win, 2, (40 - 29) / 2, "Error: Failed to reallocate memory");
                getch();
                return;
            }
            packages = temp;
        }

        packages[total_packages] = strdup(line);
        total_packages++;
    }
    pclose(fp);

    delwin(load_win);

    if (total_packages == 0) {
        erase();
        draw_background(max_y, max_x, "BrewMenu", "Press any key to return...");
        WINDOW *err_win = create_shadowed_window(max_y, max_x, 5, 40, "Error");
        mvwprintw(err_win, 2, (40 - 23) / 2, "No brew packages found");
        refresh();
        wrefresh(err_win);
        getch();
        delwin(err_win);
        free(packages);
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

        int list_h = max_y - 6;
        int list_w = 60;
        if (list_w > max_x - 4) {
            list_w = max_x - 4;
        }

        WINDOW *list_win = create_shadowed_window(max_y, max_x, list_h, list_w, "Installed Packages");

        int visible_rows = list_h - 2;

        for (int i = 0; i < visible_rows; i++) {
            int pkg_index = scroll_offset + i;

            if (pkg_index >= total_packages) {
                break;
            }

            if (pkg_index == selected_index) {
                attron(A_REVERSE);
            }

            mvwprintw(list_win, i + 1, 4, "%3d - [ %s ]", pkg_index + 1, packages[pkg_index]);

            if (pkg_index == selected_index) {
                wattroff(list_win, A_REVERSE);
            }
        }

        refresh();
        wrefresh(list_win);

        ch = getch();
        delwin(list_win);

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

        WINDOW *menu_win = create_shadowed_window(max_y, max_x, MENU_HEIGHT, MENU_WIDTH, "Main Menu");

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

                WINDOW *not_impl_win = create_shadowed_window(max_y, max_x, 5, 40, "Notice");

                mvwprintw(not_impl_win, 2, (40 - 28) / 2, "Featurne not yet implemented!");
                
                refresh();
                wrefresh(not_impl_win);
                getch();
                delwin(not_impl_win);
            }
        }
    }

    endwin();
    return 0;
}