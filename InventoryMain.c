#include "Inventory.h"
#include <stdio.h>
#include <stdlib.h>

/*===== File operations =====*/

/*Function to create and open the file*/
FILE* open_file(const char* mode) {
    /*file pointer */
    FILE* fptr = fopen(FILENAME, mode);
    /*Checking if file is open*/
    if (fptr == NULL) {
        return NULL;
    }
    return fptr;
}

/*Function to close the file when program is closed*/
void close_file(FILE* fptr) {
    if (fptr != NULL) {
        fclose(fptr);
    }
}

/*Helper function for file existence check*/
bool file_exists() {
    FILE* fptr = fopen(FILENAME, "rb");
    /*Checking if file exists*/
    if (fptr) {
        fclose(fptr);
        return true;
    }
    return false;
}

/*=== Core Features ===*/

/*===== CRUD functions =====*/
/*Function to add product to inventory*/
bool add_product(Product p) {
    /*Opening the file*/
    FILE* fptr = open_file("ab");
    /*Checking if file is open*/
    if (!fptr) {
        return false;
    }

    // Addding transaction logging for adding a product
    Transaction t = {
        .timestamp = time(NULL),
        .type = ADD,
        .product_id = p.id,
        .quantity_change = p.quantity,
        .price_change = p.price,
        .user = "system"
    };
    snprintf(t.description, 100, "Added new product: %s", p.name);


    /*Writing product to file*/
    bool success = (fwrite(&p, sizeof(Product), 1, fptr) == 1);
    close_file(fptr);


    /*Logging if write is successful*/
    if (success) {
        log_transaction(t);
        return success;
    }
    return false;
}

/*Function to search for search for specific product*/
Product* get_product(int id) {
    /*File pointer*/
    FILE* fptr = open_file("rb");
    /*Checking if file is open*/
    if (!fptr) {
        return NULL;
    }

    /*Searching for product*/
    Product* result = malloc(sizeof(Product));
    if (!result) {
        close_file(fptr);
        return NULL;
    }

    while (fread(result, sizeof(Product), 1, fptr)) {
        if (result->id == id) {
            close_file(fptr);
            return result;
        }
    }

    close_file(fptr);
    free(result);
    return NULL;
};

/*Function to update specific product data*/
bool update_product(int id, Product new_data) {
    /*File pointer*/
    FILE* fptr = open_file("rb+");
    if (!fptr) {
        return false;
    }

    Product old;
    bool found = false;
    long pos = 0;
    Transaction t = {
        .timestamp = time(NULL),
        .type = UPDATE,
        .product_id = id,
        .user = "system"
    };

    /*Searching for the product to update*/
    while (fread(&old, sizeof(Product), 1, fptr)) {
        if (old.id == id) {
            found = true;
            break;
        }
        pos = ftell(fptr);
    }
    /*Product found*/
    if (found) {

        // Calculating changes
        t.quantity_change = new_data.quantity - old.quantity;
        t.price_change = new_data.price - old.price;
        snprintf(t.description, 100, "Updated %s | Qty %d -> %d | Price $%.2f -> $%.2f",
            old.name,
            old.quantity, new_data.quantity,
            old.price, new_data.price);

        fseek(fptr, pos - sizeof(Product), SEEK_SET);
        bool success = (fwrite(&new_data, sizeof(Product), 1, fptr) == 1);
        close_file(fptr);

        if (success) {
            log_transaction(t);
        }
        return success;
    }

    /*If failed*/
    close_file(fptr);
    return false;
}

/*Function to delete a specific product*/
bool delete_product(int id) {
    /*File pointer*/
    FILE* src = open_file("rb");
    FILE* dst = fopen(TEMP_FILE, "wb");
    if (!src || !dst) {
        if (src) {
            close_file(src);
        }

        if (dst) {
            fclose(dst);
        }
        return false;
    }

    Product p;
    bool deleted = false;
    Transaction t = {
        .timestamp = time(NULL),
        .type = DELETE,
        .product_id = id,
        .user = "system"
    };

    /*Deleting the product*/
    while (fread(&p, sizeof(Product), 1, src)) {
        if (p.id == id) {
            /*Recording details before deletion*/
            t.quantity_change = -p.quantity;
            snprintf(t.description, 100, "Deleted product: %s", p.name);
            deleted = true;
        }
        else {
            fwrite(&p, sizeof(Product), 1, dst);
        }
    }

    close_file(src);
    fclose(dst);

    /*Checking if product is deleted*/
    if (deleted) {
        log_transaction(t);
        remove(FILENAME);
        rename(TEMP_FILE, FILENAME);
        return true;
    }

    remove(TEMP_FILE);
    return false;
}

/*===== Helper functions =====*/
/*Function to generate id for product*/
int generate_id() {
    static int max_id = -1; // -1 indicates not initialized

    if (max_id == -1) { // Initialize max_id once
        max_id = 0;
        FILE* fptr = fopen(FILENAME, "rb");
        if (fptr) {
            Product p;
            while (fread(&p, sizeof(Product), 1, fptr)) {
                if (p.id > max_id) {
                    max_id = p.id;
                }
            }
            fclose(fptr);
        }
        max_id++; // Next ID is max existing +1
    }

    int new_id = max_id;
    max_id++;
    return new_id;
}

/*Function to check if product exists*/
bool product_exists(int id) {
    Product* p = get_product(id);
    bool exists = (p != NULL);
    free(p);
    return exists;
}

/*Function to input product data*/
void input_product_data(Product* p) {
    char buffer[100];
    int valid = 0;

    // Product name
    printf("Enter product name: ");
    fgets(p->name, MAX_NAME_LEN, stdin);
    p->name[strcspn(p->name, "\n")] = '\0';

    // Product price with validation
    do {
        printf("Enter price: ");
        fgets(buffer, sizeof(buffer), stdin);
        valid = sscanf(buffer, "%f", &p->price);
        if (valid != 1 || p->price <= 0) {
            printf("Invalid price. Please enter a positive number.\n");
            valid = 0;
        }
    } while (!valid);

    // Product quantity with validation
    do {
        printf("Enter quantity: ");
        fgets(buffer, sizeof(buffer), stdin);
        valid = sscanf(buffer, "%d", &p->quantity);
        if (valid != 1 || p->quantity < 0) {
            printf("Invalid quantity. Please enter a non-negative integer.\n");
            valid = 0;
        }
    } while (!valid);

    // Product category
    printf("Enter category: ");
    fgets(p->category, MAX_CATEGORY_LEN, stdin);
    p->category[strcspn(p->category, "\n")] = '\0';

    // Generate ID (fixed function ensures uniqueness)
    p->id = generate_id();
}

/*===== Display And Reporting functions =====*/
/*Function to display a specific product*/
void display_product(Product p) {
    printf("\nID: %d\n", p.id);
    printf("Name: %s\n", p.name);
    printf("Price: $%.2f\n", p.price);
    printf("Quantity: %d\n", p.quantity);
    printf("Category: %s\n", p.category);
    printf("------------------------------\n");
}

/*Function to display all products in inventory system*/
void display_all_products() {
    /*FILE pointer*/
    FILE* fptr = open_file("rb");
    /*Checking if file is open*/
    if (!fptr) {
        printf("\nNo Products found!\n");
        return;
    }

    Product p;
    printf("\n=== Inventory Listing ===\n");
    while (fread(&p, sizeof(Product), 1, fptr)) {
        display_product(p);
    }
    close_file(fptr);
}

/*Function to generate a report of all products in inventory system*/
void generate_report() {
    /*File pointer*/
    FILE* fptr = open_file("rb");
    /*Checking if file is open*/
    if (!fptr) {
        printf("\nInventory is empty!\n");
        return;
    }

    Product p;
    float total_value = 0.0;
    int low_stock_count = 0;
    int out_of_stock_count = 0;

    /*Inventory report*/
    printf("\n=== Inventory Report ===\n");
    printf("%-5s %-20s %-10s %-8s %-15s\n",
        "ID", "Name", "Price", "Qty", "Category");

    while (fread(&p, sizeof(Product), 1, fptr)) {
        /*Table view*/
        printf("%-5d %-20s $%-9.2f %-8d %-15s\n",
            p.id, p.name, p.price, p.quantity, p.category);

        /*Calculating totals*/
        total_value += p.price * p.quantity;

        if (p.quantity == 0) {
            out_of_stock_count++;
        }

        if (p.quantity < 10 && p.quantity > 0) {
            low_stock_count++;
        }
    }

    /*Summary*/
    printf("\nSummary\n");
    printf("Total Inventory Value: %.2f\n", total_value);
    printf("Low Stock Items (<10): %d\n", low_stock_count);
    printf("Out-of-Stock Items: %d\n", out_of_stock_count);

    close_file(fptr);
}

/*Function to give an alert on low stock*/
void low_stock_alert(int threshold) {
    /*File pointer*/
    FILE* fptr = open_file("rb");
    /*Checking if file is open*/
    if (!fptr) {
        return;
    }

    Product p;
    bool alert_shown = false;

    /*Low stock alert*/
    printf("\n=== Low Stock Alert (Threshold: %d ===\n)", threshold);
    while (fread(&p, sizeof(Product), 1, fptr)) {
        if (p.quantity < threshold) {
            printf("! %s (ID: %d) - Only %d left!\n",
                p.name, p.id, p.quantity);
            alert_shown = true;
        }
    }

    /*If items are not below threshold*/
    if (!alert_shown) {
        printf("No items below stock threshold\n");
    }
    close_file(fptr);
}

/*===== Inventory operations =====*/
/*Function to restock a specific product*/
void restock_product(int id, int quantity) {
    Product* product = get_product(id);
    /*Checking if product is found*/
    if (!product) {
        printf("Error: Product ID %d not found!\n", id);
        return;
    }

    Transaction t = {
        .timestamp = time(NULL),
        .type = RESTOCK,
        .product_id = id,
        .quantity_change = quantity,
        .user = "system"
    };
    snprintf(t.description, 100, "Restocked %d units of %s",
        quantity, product->name);

    /*Checking if quantity is valid*/
    if (quantity <= 0) {
        printf("Error: Invalid restock quantity!\n");
        free(product);
        return;
    }

    product->quantity += quantity;

    /*Checking if product stock is updated*/
    if (update_product(id, *product)) {
        log_transaction(t);
        printf("Restock successfull!\n"
            "Restocked %d units. New quantity: %d\n", quantity,
            product->quantity);
    }
    else {
        printf("Failed to update stock!\n");
    }

    free(product);
}

/*Function that will sell a product*/
void sell_product(int id, int quantity) {
    /*Searching for the product*/
    Product* product = get_product(id);
    /*Checking if product is found*/
    if (!product) {
        printf("Error: Product ID %d not found!\n", id);
        return;
    }

    Transaction t = {
        .timestamp = time(NULL),
        .type = SALE,
        .product_id = id,
        .quantity_change = -quantity,
        .user = "system"
    };
    snprintf(t.description, 100, "Sold %d units of %s",
        quantity, product->name);

    /*Checking if product stock is still available*/
    if (quantity <= 0) {
        printf("Error: Invalid sale quantity!\n");
        free(product);
        return;
    }

    /*Selling if product units re available*/
    if (product->quantity < quantity) {
        printf("Error: Only %d units available!\n", product->quantity);
        free(product);
        return;
    }

    product->quantity -= quantity;

    /*Updating product stock when it is sold*/
    if (update_product(id, *product)) {
        log_transaction(t);
        printf("Sale Successful!\n"
            "Sold %d units. Remaining stock: %d\n", quantity,
            product->quantity);
    }
    else {
        printf("Sale Failed!\n");
    }

    free(product);
}

/*==================================================================*/

/*=== Logging ===*/
/*Code for logging transactions*/
void log_transaction(Transaction t) {
    /*Opening file*/
    FILE* fptr = fopen(LOG_FILE, "ab");
    if (!fptr) {
        printf("Transaction log error");
    }
    fwrite(&t, sizeof(Transaction), 1, fptr);
    fclose(fptr);
}

/*Displaying transactions*/
void display_transaction_log() {
    /*Opening file*/
    FILE* fptr = fopen(LOG_FILE, "rb");
    if (!fptr) {
        printf("\nNo transactions recorded\n");
        return;
    }

    Transaction t;
    char time_buf[20], price_buf[20];

    // Displaying transaction log
    printf("\n=== Transaction Log ===\n");
    printf("%-19s | %-8s | %-6s | %-5s | %s\n",
        "Timestamp", "Type", "Qty", "ID", "Description");

    while (fread(&t, sizeof(Transaction), 1, fptr)) {
        strftime(time_buf, 20, "%Y-%m-%d: %H:%M%S", localtime(&t.timestamp));

        // Formatting price change
        if (t.price_change != 0) {
            snprintf(price_buf, 20, "$%.2f", t.price_change);
        }
        else {
            strcpy(price_buf, "-");
        }

        printf("%-19s | %-8s | %+6d | %-5d | %s\n",
            time_buf,
            (const char* []) {
            "ADD", "UPDATE", "DELETE", "RESTOCK", "SALE"
        }[t.type],
                t.quantity_change,
                t.product_id,
                t.description);
    }
    fclose(fptr);
}