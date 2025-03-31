#include "InventoryMain.c"
#include "InventoryUI.c"
#include <stdio.h>
#include <stdlib.h>

/*Main entry code*/
int main() {
    print_header("Inventory Management System");

    while (1) {
        display_menu();
        int choice = get_int_input("Enter Your choice");

        handle_menu_choice(choice);
    }

    return 0;
}
