#ifndef INVENTORY_H
#define INVENTORY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// Constants
#define MAX_NAME_LEN 50
#define MAX_CATEGORY_LEN 30
#define FILENAME "inventory.dat"
#define TEMP_FILE "temp.dat"
#define LOG_FILE "transactions.log"

// Product structure
typedef struct {
    int id; // unique product ID
    char name[MAX_NAME_LEN]; // Product name
    float price; // unit price
    int quantity; // Current stock quantity 
    char category[MAX_CATEGORY_LEN]; // Product category
} Product;

/*Transaction structures*/
typedef enum { ADD, UPDATE, DELETE, RESTOCK, SALE }
TransactionType;

typedef struct {
    time_t timestamp;
    TransactionType type;
    int product_id;
    char user[30];
    int quantity_change;
    float price_change;
    char description[100];
} Transaction;

/*Function prototypes*/
/*=== File Operations ===*/
FILE* open_file(const char* mode);
void close_file(FILE* fptr);

/*=== Core CRUD operations ===*/
bool add_product(Product p);
bool update_product(int id, Product new_data);
bool delete_product(int id);
Product* get_product(int id);

/*=== Display & Reporting ===*/
void display_all_products();
void display_product(Product p);
void generate_report();
void low_stock_alert(int threshold);

/*=== Inventory operations ===*/
void restock_product(int id, int quantity);
void sell_product(int id, int quantity);

/*=== Logging ===*/
void log_transaction(Transaction t);
void display_transaction_log();

/*=== Utility Functions ===*/
int generate_id();
bool product_exists(int id);
void input_product_data(Product* p);

#endif // !INVENTORY_H