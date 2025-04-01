#include "include/inventory.h"
#include "include/InventoryUI.h"
#include <stdio.h>
#include <stdlib.h>


/*Function to display menu*/
void display_menu() {
    printf("\n===== Main Menu =====\n");
    printf("1. Add New Product\n");
    printf("2. List All Products\n");
    printf("3. Update Product\n");
    printf("4. Delete Product\n");
    printf("5. Search Product\n");
    printf("6. Restock Product\n");
    printf("7. Sell Product\n");
    printf("8. Generate Report\n");
    printf("9. View Transaction Log\n");
    printf("10. Exit\n");
}

void handle_menu_choice(int choice) {
    switch (choice) {
        // Adding a product 
    case 1: {
        Product p;
        input_product_data(&p);
        if (add_product(p)) {
            printf("\nProduct added successfully! ID: %d\n", p.id);
        }
        else {
            printf("\nFailed to add product\n");
        }
        break;
    }

          // Displaying product in inventory
    case 2:
        display_all_products();
        break;

        // updating a specific product
    case 3: {
        int id = get_int_input("Enter product ID to update");
        Product* existing = get_product(id);
        if (existing) {
            Product updated = *existing;
            printf("\nCurrent product details:\n");
            display_product(*existing);

            // Getting new details
            printf("\nEnter new details:\n");
            input_product_data(&updated);
            updated.id = id; // preserving original ID

            if (update_product(id, updated)) {
                printf("\nProduct updated successfully!\n");
            }
            else {
                printf("\nUpdate failed!\n");
            }
            free(existing);
        }
        else {
            printf("\nProduct not found\n");
        }
        break;
    }

          // Deleting a product
    case 4: {
        int id = get_int_input("Enter product ID to delete");
        if (delete_product(id)) {
            printf("\nProduct deleted successfully!\n");
        }
        else {
            printf("\n Delete failed or product not found!\n");
        }
        break;
    }

          // Searching for a product
    case 5: {
        int id = get_int_input("Enter product ID to search");
        Product* p = get_product(id);
        if (p) {
            display_product(*p);
            free(p);
        }
        else {
            printf("\nProduct not found\n");
        }
        break;
    }

          // Restocking product 
    case 6: {
        int id = get_int_input("Enter product ID to restock");
        int qty = get_int_input("Enter quantity to add");
        restock_product(id, qty);
        break;
    }

          // Selling product
    case 7: {
        int id = get_int_input("Enter product ID to sell");
        int qty = get_int_input("Enter quantity to sell");
        sell_product(id, qty);
        break;
    }

          // Generating report
    case 8:
        generate_report();
        break;

        // Viewing transaction log
    case 9:
        display_transaction_log();
        break;

    case 10:
        printf("Closing Inventory System.....Goodbye!\n");
        exit(EXIT_SUCCESS);
        system("PAUSE");
        break;

    default:
        printf("\nInvalid choice! Please enter a valid option.\n");
        break;
    }
}

/*Utility function for safe integer input*/
int get_int_input(const char* prompt) {
    char buffer[100];
    int value;

    while (1) {
        printf("\n%s: ", prompt);
        fgets(buffer, sizeof(buffer), stdin);

        if (sscanf(buffer, "%d", &value) == 1) {
            return value;
        }
        printf("Invalid input! Please enter a number.\n");
    }
}

/*UI helper function*/
void print_header(const char* text) {
    printf("\n=================================");
    printf("\n%s", text);
    printf("\n=================================");
}