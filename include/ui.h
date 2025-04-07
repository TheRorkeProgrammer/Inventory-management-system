#ifndef UI
#define UI

#include "inventory.h"
#include "backup-restore.h"
#include <stdio.h>
#include <stdlib.h>

/*Function prototypes*/
void display_menu();
void handle_menu_choice(int choice);
int get_int_input(const char* prompt);
void print_header(const char* text);

#endif // !UI