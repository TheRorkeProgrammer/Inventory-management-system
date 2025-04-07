#include "include/inventory.h"
#include "include/backup-restore.h"
#include "include/ui.h"
#include <stdio.h>
#include <stdlib.h>

/*UI helper function*/
void print_header(const char* text) {
    printf("\n=================================");
    printf("\n%s", text);
    printf("\n=================================");
}

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
    printf("10. Create backup\n");
    printf("11. Restore backup\n");
    printf("12. Configure backup rotation\n");
    printf("13. Exit\n");
}

/*Backup rotation submenu*/
void display_rotation_submenu() {
    printf("\nBackup Rotation Settings:\n");
    printf("1. Set Maximum Backup Count\n");
    printf("2. Set Maximum Backup Age\n");
    printf("3. View Current Policy\n");
    printf("4. Run Rotation Now\n");
    printf("5. Return to Main Menu\n");
}

/*Handling rotation submenu function*/
void handle_rotation_settings() {
    RetentionPolicy policy = load_retention_config();
    int choice;

    do {
        display_rotation_submenu();
        choice = get_int_input("Enter rotation setting choice");

        switch (choice) {
        case 1: {
            int count = get_int_input("Enter maximum backups to retain");
            if (count > 0) {
                policy.retain_count = count;
                save_retention_policy(policy);
            }
            break;
        }
        case 2: {
            int days = get_int_input("Enter maximum backup age in days");
            if (days > 0) {
                policy.retain_days = days;
                save_retention_policy(policy);
            }
            break;
        }
        case 3: {
            printf("\nCurrent Retention Policy:\n");
            printf(" - Keep last %d backups\n", policy.retain_count);
            printf(" - Delete backups older than %d days\n", policy.retain_days);
            break;
        }

        case 4: {
            RetentionPolicy current_policy = load_retention_config();
            enforce_retention_policy(&current_policy);
            printf("\nRotation completed. Check rotation.log for details.\n");
            break;
        }

        case 5:
            return;
        default:
            printf("Invalid choice!\n");
        }
    } while (choice != 5);
}


/*Handling menu function*/
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
    case 8: {
        generate_report();
        break;
    }

          // Viewing transaction log
    case 9: {
        display_transaction_log();
        break;
    }

          // Create backup
    case 10: {
        if (create_backup()) {
            printf("\n Backup created successfully!\n");
        }
        else {
            printf("\n Backup creation failed!\n");
        }
        break;
    }

           // Restoring backup
    case 11: {
        list_backups();
        int backup_num = get_int_input("Enter backup number to restore");

        // Getting selected backup path
        DIR* dir = opendir(BACKUP_DIR);
        struct dirent* ent;
        int count = 0;
        char selected_path[295] = { 0 };

        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".bak")) {
                if (count++ == backup_num) {
                    snprintf(selected_path, sizeof(selected_path),
                        "%s / %s", BACKUP_DIR, ent->d_name);
                    break;
                }
            }
        }
        closedir(dir);

        if (selected_path[0] && restore_backup(selected_path)) {
            printf("\n Restoration successful!\n");
        }
        else {
            printf("\n Restoration failed!\n");
        }
        break;
    }

           // Rotation submenu
    case 12: {
        handle_rotation_settings();
        break;
    }

           // Exiting system
    case 13: {
        printf("Closing Inventory System.....Goodbye!\n");
        exit(EXIT_SUCCESS);
        system("PAUSE");
        break;
    }

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

        if (sscanf(buffer, "%d", &value) == 1 && value >= 0) {
            return value;
        }
        printf("Invalid input! Please enter a number.\n");
    }
}

