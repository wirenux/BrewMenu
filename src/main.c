#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
    int capacity = 32;
    int total_packages = 0;

    char **packages = malloc(capacity * sizeof(char *));
    
    if (packages == NULL) {
        puts("\e[1;31mError:\e[0m Failed to allocate inital memory");
        return 1;
    }

    FILE *fp = popen("brew list -1", "r");
    if (fp == NULL) {
        puts("\e[1;31mError:\e[0m Failed to run the brew command");
        free(packages);
        return 1;
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
                perror("\e[1;31mError:\e[0m Failed to reallocate memory");
                return 1;
            }
            packages = temp;
        }

        packages[total_packages] = strdup(line);
        total_packages++;
    }
    pclose(fp);

    if (total_packages == 0) {
        puts("\e[1;31mError:\e[0m No brew packages found");
        free(packages);
        return 1;
    }

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

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

    for (int i = 0; i < total_packages; i++) {
        free(packages[i]);
    }
    free(packages);

    return 0;
}