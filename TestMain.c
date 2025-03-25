#include "InventoryUI.c"
#include "inventory.h"
#include <stdio.h>
#include <stdlib.h>

/*Main entry code*/
// int main() {
//     int choice;

//     printf("=== Inventory Management System ===\n");

//     while (1) {
//         display_menu();
//         choice = get_int_input("Enter Your choice");

//         handle_menu_choice(choice);
//     }

//     return 0;
// }

void test_transaction_log() {
    // Clear old log
    remove(LOG_FILE);

    Product p = { .id = 100, .name = "Test Item", .price = 10, .quantity = 5 };

    add_product(p);
    restock_product(100, 3);
    sell_product(100, 2);
    delete_product(100);

    display_transaction_log();
}

int main() {
    test_transaction_log();
    system("PAUSE");
    return 0;
}