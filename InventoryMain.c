#include "inventory.h"
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

/*===== CRUD functions =====*/

/*Function to add product to inventory*/
bool add_product(Product p) {
    /*Opening the file*/
    FILE* fptr = open_file("ab");
    /*Checking if file is open*/
    if (!fptr) {
        return false;
    }

    /*Writing product to file*/
    bool success = (fwrite(&p, sizeof(Product), 1, fptr) == 1);
    close_file(fptr);
    return success;
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

    Product p;
    bool found = false;
    long pos = 0;

    /*Searching for the product to update*/
    while (fread(&p, sizeof(Product), 1, fptr)) {
        if (p.id == id) {
            found = true;
            break;
        }
        pos = ftell(fptr);
    }
    /*Product found*/
    if (found) {
        fseek(fptr, pos - sizeof(Product), SEEK_SET);
        bool success = (fwrite(&new_data, sizeof(Product), 1, fptr) == 1);
        close_file(fptr);
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

    /*Deleting the product*/
    while (fread(&p, sizeof(Product), 1, src)) {
        if (p.id != id) {
            fwrite(&p, sizeof(Product), 1, dst);
        }
        else {
            deleted = true;
        }
    }

    close_file(src);
    fclose(dst);

    /*Checking if product is deleted*/
    if (deleted) {
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
    static int counter = 0;
    /*File pointer*/
    FILE* fptr = open_file("rb");
    /*Checking if file is open*/
    if (!fptr) {
        return counter++;
    }

    fseek(fptr, sizeof(Product), SEEK_END);
    Product last;
    if (fread(&last, sizeof(Product), 1, fptr)) {
        counter = last.id;
    }
    close_file(fptr);
    return counter++;
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
    char buffer[100]; // Buffer for numeric input

    /*Product name*/
    printf("Enter product name: ");
    fgets(p->name, MAX_NAME_LEN, stdin);
    p->name[strcspn(p->name, "\n")] = '\0';

    /*Product price*/
    printf("Enter price: ");
    fgets(buffer, sizeof(buffer), stdin);
    sscanf(buffer, "%f", &p->price);

    /*Product quantity*/
    printf("Enter quantity: ");
    fgets(buffer, sizeof(buffer), stdin);
    sscanf(buffer, "%d", &p->quantity);

    /*Product category*/
    printf("Enter category: ");
    fgets(p->category, MAX_CATEGORY_LEN, stdin);
    p->category[strcspn(p->category, "\n")] = '\0';

    /*Product id*/
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

    /*Checking if quantity is valid*/
    if (quantity <= 0) {
        printf("Error: Invalid restock quantity!\n");
        free(product);
        return;
    }

    product->quantity += quantity;

    /*Checking if product stock is updated*/
    if (update_product(id, *product)) {
        printf("Restocked %d units. New quantity: %d\n", quantity,
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
        printf("Sold %d units. Remaining stock: %d\n", quantity,
            product->quantity);
    }
    else {
        printf("Failed to update stock!\n");
    }

    free(product);
}