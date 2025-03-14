#include "inventory.h"
#include "InventoryUI.c"
#include <stdio.h>
#include <stdlib.h>

/*Main entry code*/
int main() {
    int choice;

    printf("=== Inventory Management System ===\n");

    while (1) {
        display_menu();
        choice = get_int_input("Enter Your choice");

        handle_menu_choice(choice);
    }

    return 0;
}
