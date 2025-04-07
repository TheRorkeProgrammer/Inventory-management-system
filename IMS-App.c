#include "inventoryMain.c"
#include "UI.c"
#include "Backup-Restore.c"
#include <stdio.h>
#include <stdlib.h>

/*Main entry code*/
int main() {
    int choice;
    print_header("===== Inventory Management System =====");

    while (1) {
        display_menu();
        choice = get_int_input("Enter Your choice");
        handle_menu_choice(choice);
    }

    return 0;
}
