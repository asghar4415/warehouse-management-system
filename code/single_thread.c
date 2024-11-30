#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define INITIAL_SIZE 1000
#define MAX_LINE_LENGTH 1024
#define LOW_STOCK_THRESHOLD 10
#define MAX_ITEMS 10000000

typedef struct {
    int id;
    char name[50];
    char category[50];  
    int quantity;
    float price;
} Item;

Item *items = NULL;
int itemCount = 0;
int itemCapacity = INITIAL_SIZE;

// Function prototypes
void loadDataFromFiles(); // Function to load data from all four CSV files
void loadData(const char *filename); 
void addItem(int id, const char *name, const char *category, int quantity, float price);
void deleteItem(int id);
void retrieveItem(int id);
void updateItem(int id, const char *name, const char *category, int quantity, float price);
void processBulkUpdates(int increment);
void searchItems(const char *keyword);
void sortItemsByPrice();
void logOperation(const char *operation, const char *details);
void exportData(const char *filename);
void stockAlert();
void displayMenu();
void printItems();
void viewItemsByCategory();

// Function to load data from an Excel-like CSV file
void loadDataFromFiles() {
  char filenames[20][50];

  // Construct filenames for all 20 data files (assuming a specific naming format)
  for (int i = 0; i < 20; i++) {
    sprintf(filenames[i], "warehouse_data_%d.csv", i + 1); 
  }

  for (int i = 0; i < 20; i++) {
    loadData(filenames[i]);
  }
}
// Function to load data from an Excel-like CSV file (unchanged)
void loadData(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        Item item;
        if (sscanf(line, "%d,%49[^,],%49[^,],%d,%f", &item.id, item.name, item.category, &item.quantity, &item.price) == 5) {
            if (itemCount >= itemCapacity) {
                itemCapacity *= 2;
                items = realloc(items, sizeof(Item) * itemCapacity);
                if (!items) {
                    perror("Memory allocation failed");
                    exit(EXIT_FAILURE);
                }
            }
            items[itemCount++] = item;
        }
    }

    fclose(file);
}

// Function to add an item to the dataset
void addItem(int id, const char *name, const char *category, int quantity, float price) {
    if (itemCount >= itemCapacity) {
        itemCapacity *= 2;
        items = realloc(items, sizeof(Item) * itemCapacity);
        if (!items) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
    }

    items[itemCount].id = id;
    strncpy(items[itemCount].name, name, sizeof(items[itemCount].name) - 1);
    strncpy(items[itemCount].category, category, sizeof(items[itemCount].category) - 1);
    items[itemCount].quantity = quantity;
    items[itemCount].price = price;
    itemCount++;
    printf("\nItem added successfully.\n");
}
// Function to delete an item by ID
void deleteItem(int id) {
    for (int i = 0; i < itemCount; i++) {
        if (items[i].id == id) {
            for (int j = i; j < itemCount - 1; j++) {
                items[j] = items[j + 1];
            }
            itemCount--;
            printf("\nItem deleted successfully.\n");
            return;
        }
    }
    printf("\nError: Item with ID %d not found.\n", id);
}

// Function to retrieve an item by ID
void retrieveItem(int id) {
    for (int i = 0; i < itemCount; i++) {
        if (items[i].id == id) {
            printf("\nItem Details:\n");
            printf("ID: %d\nName: %s\nCategory: %s\nQuantity: %d\nPrice: %.2f\n", 
                   items[i].id, items[i].name, items[i].category, items[i].quantity, items[i].price);
            return;
        }
    }
    printf("\nError: Item with ID %d not found.\n", id);
}

// Function to update an item's details
void updateItem(int id, const char *name, const char *category, int quantity, float price) {
    for (int i = 0; i < itemCount; i++) {
        if (items[i].id == id) {
            if (name) strncpy(items[i].name, name, sizeof(items[i].name) - 1);
            if (category) strncpy(items[i].category, category, sizeof(items[i].category) - 1);
            if (quantity >= 0) items[i].quantity = quantity;
            if (price >= 0) items[i].price = price;
            printf("\nItem updated successfully.\n");
            return;
        }
    }
    printf("\nError: Item with ID %d not found.\n", id);
}

void processBulkUpdates(int increment) {
    clock_t start_time, end_time;
    double processing_time;
    double sleep_time = 0.0; // To track total sleep time

    start_time = clock();

    for (int i = 0; i < itemCount; i++) {
        items[i].quantity += increment;
        if (i % 100000 == 0) {
            printf("\nProcessing done for %d items.\n", i);
            sleep(2);  // Sleep for 3 seconds
            sleep_time += 2.0; // Add sleep time
        }
    }

    end_time = clock();
    processing_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    // Include sleep time in total processing time
    double total_time = processing_time + sleep_time;

    printf("\nBulk update completed: Incremented all quantities by %d.\n", increment);
    printf("Total processing time: %.2f seconds\n", total_time);

}


// Function to search for items by name (partial match)
void searchItems(const char *keyword) {
    printf("\nSearch Results for '%s':\n", keyword);
    int found = 0;
    for (int i = 0; i < itemCount; i++) {
        if (strstr(items[i].name, keyword)) {
            printf("ID: %d | Name: %s | Category: %s | Quantity: %d | Price: %.2f\n", 
                   items[i].id, items[i].name, items[i].category, items[i].quantity, items[i].price);
            found = 1;
        }
    }
    if (!found) {
        printf("No items found matching '%s'.\n", keyword);
    }
}

// Function to sort items by price
void sortItemsByPrice() {
    for (int i = 0; i < itemCount - 1; i++) {
        for (int j = 0; j < itemCount - i - 1; j++) {
            if (items[j].price > items[j + 1].price) {
                Item temp = items[j];
                items[j] = items[j + 1];
                items[j + 1] = temp;
            }
        }
    }
    printf("\nItems sorted by price.\n");
}

// Function to log operations to a file
void logOperation(const char *operation, const char *details) {
    FILE *logFile = fopen("warehouse_log.txt", "a");
    if (!logFile) {
        perror("Error opening log file");
        return;
    }
    fprintf(logFile, "%s: %s\n", operation, details);
    fclose(logFile);
}

// Function to export data to a file
void exportData(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error exporting data");
        return;
    }

    for (int i = 0; i < itemCount; i++) {
        fprintf(file, "%d,%s,%s,%d,%.2f\n", items[i].id, items[i].name, items[i].category, items[i].quantity, items[i].price);
    }
    fclose(file);
    printf("\nData exported successfully to %s.\n", filename);
}

// Function to alert low stock
void stockAlert() {
    printf("\nLow Stock Alert:\n");
    int found = 0;
    for (int i = 0; i < itemCount; i++) {
        if (items[i].quantity < LOW_STOCK_THRESHOLD) {
            printf("ID: %d | Name: %s | Category: %s | Quantity: %d\n", 
                   items[i].id, items[i].name, items[i].category, items[i].quantity);
            found = 1;
        }
    }
    if (!found) {
        printf("No items with low stock.\n");
    }
}

// Function to display items
void printItems() {
    printf("\n================= Current Warehouse Items =================\n");
    printf("| %-5s | %-15s | %-15s | %-10s | %-10s |\n", "ID", "Name", "Category", "Quantity", "Price");
    printf("|----------------------------------------------------------|\n");
    for (int i = 0; i < itemCount; i++) {
        printf("| %-5d | %-15s | %-15s | %-10d | %-10.2f |\n", 
               items[i].id, items[i].name, items[i].category, items[i].quantity, items[i].price);
    }
    printf("===========================================================\n");
}

// Function to calculate and display the total price of all items
void calculateTotalValue() {
    clock_t start_time, end_time;
    double processing_time;

    start_time = clock(); 

    float totalValue = 0.0;
    for (int i = 0; i < itemCount; i++) {
        totalValue += items[i].quantity * items[i].price;


    }

    end_time = clock(); // End timer
    processing_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;



    printf("\nTotal value of all items: %.2f\n", totalValue);
    printf("Processing time: %.2f seconds\n", processing_time   );
}




void viewItemsByCategory() {
    char category[50];
    printf("Enter category name: ");
    if (!fgets(category, sizeof(category), stdin)) {
        printf("Invalid input.\n");
        return;
    }
    strtok(category, "\n"); // Remove newline

    int found = 0;
    printf("\nItems in Category '%s':\n", category);
    printf("| %-5s | %-15s | %-10s | %-10s |\n", "ID", "Name", "Quantity", "Price");
    printf("|----------------------------------------------------------|\n");
    for (int i = 0; i < itemCount; i++) {
        if (strcmp(items[i].category, category) == 0) {
            printf("| %-5d | %-15s | %-10d | %-10.2f |\n", 
                   items[i].id, items[i].name, items[i].quantity, items[i].price);
            found = 1;
        }
    }
    if (!found) {
        printf("No items found in category '%s'.\n", category);
    }
    printf("===========================================================\n");
}

void displayMenu() {
    printf("\n============= Warehouse Management System =============\n");
    printf("1. Add Item\n");
    printf("2. Delete Item\n");
    printf("3. Retrieve Item\n");
    printf("4. Update Item\n");
    printf("5. Process Bulk Updates\n");
    printf("6. Search for Item\n");
    printf("7. Sort Items by Price\n");
    printf("8. View Stock Alerts\n");
    printf("9. Print All Items\n");
    printf("10. Export Data to CSV\n");
    printf("11. View Items by Category\n");
    printf("12. Calculate Total Value of All Items\n");

    printf("13. Exit\n");
    printf("=========================================================\n");
}

int main() {
    items = malloc(sizeof(Item) * itemCapacity);
    if (!items) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    printf("Loading data from warehouse data files...\n");
    loadDataFromFiles(); // Use the new loadDataFromFiles function
    printf("Data loaded successfully.\n");


    int choice;
    do {
        displayMenu();
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Clear newline from buffer

        switch (choice) {
            case 1: {
                int id, quantity;
                float price;
                char name[50], category[50];

                printf("Enter item ID: ");
                scanf("%d", &id);
                getchar();
                printf("Enter item name: ");
                fgets(name, sizeof(name), stdin);
                strtok(name, "\n");
                printf("Enter category: ");
                fgets(category, sizeof(category), stdin);
                strtok(category, "\n");
                printf("Enter quantity: ");
                scanf("%d", &quantity);
                printf("Enter price: ");
                scanf("%f", &price);

                addItem(id, name, category, quantity, price);
                break;
            }
            case 2: {
                int id;
                printf("Enter item ID to delete: ");
                scanf("%d", &id);
                deleteItem(id);
                break;
            }
            case 3: {
                int id;
                printf("Enter item ID to retrieve: ");
                scanf("%d", &id);
                retrieveItem(id);
                break;
            }
            case 4: {
                int id, quantity;
                float price;
                char name[50], category[50];

                printf("Enter item ID to update: ");
                scanf("%d", &id);
                getchar();
                printf("Enter new item name (leave empty to keep current): ");
                fgets(name, sizeof(name), stdin);
                strtok(name, "\n");
                printf("Enter new category (leave empty to keep current): ");
                fgets(category, sizeof(category), stdin);
                strtok(category, "\n");
                printf("Enter new quantity (enter -1 to keep current): ");
                scanf("%d", &quantity);
                printf("Enter new price (enter -1 to keep current): ");
                scanf("%f", &price);

                updateItem(id, name[0] ? name : NULL, category[0] ? category : NULL, quantity >= 0 ? quantity : -1, price >= 0 ? price : -1);
                break;
            }
            case 5: {
                int increment;
                printf("Enter increment value for bulk update: ");
                scanf("%d", &increment);
                processBulkUpdates(increment);
                break;
            }
            case 6: {
                char keyword[50];
                printf("Enter keyword to search: ");
                fgets(keyword, sizeof(keyword), stdin);
                strtok(keyword, "\n");
                searchItems(keyword);
                break;
            }
            case 7:
                sortItemsByPrice();
                break;
            case 8:
                stockAlert();
                break;
            case 9:
                printItems();
                break;
            case 10: {
                char filename[50];
                printf("Enter filename to export data: ");
                fgets(filename, sizeof(filename), stdin);
                strtok(filename, "\n");
                exportData(filename);
                break;
            }
            case 11:
                viewItemsByCategory();
                break;
                case 12:
    calculateTotalValue();
    break;
            case 13:
                printf("See you another time. Bye ! \n");
                break;
            default:
                printf("Invalid choice, please try again.\n");
        }
    } while (choice != 12);

    free(items);
    return 0;
}