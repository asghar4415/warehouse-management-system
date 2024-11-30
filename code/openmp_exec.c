#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <omp.h>

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
void loadDataFromFiles();
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
void calculateTotalValue();

// Function to load data from all files
void loadDataFromFiles() {
    char filenames[20][50];

    #pragma omp parallel for // Parallelize the filename creation step
    for (int i = 0; i < 20; i++) {
        sprintf(filenames[i], "warehouse_data_%d.csv", i + 1);
    }

    #pragma omp parallel for // Parallelize loading data from each file
    for (int i = 0; i < 20; i++) {
        loadData(filenames[i]);
    }
}

// Function to load data from a CSV file
void loadData(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    #pragma omp parallel private(line) // Parallelize reading lines
    {
        while (fgets(line, sizeof(line), file)) {
            Item item;
            if (sscanf(line, "%d,%49[^,],%49[^,],%d,%f", &item.id, item.name, item.category, &item.quantity, &item.price) == 5) {
                #pragma omp critical // Ensure thread safety for adding items
                {
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
        }
    }
    fclose(file);
}

// Function to add an item to the dataset
void addItem(int id, const char *name, const char *category, int quantity, float price) {
    #pragma omp critical // Ensure thread safety when adding an item
    {
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
    }
    printf("\nItem added successfully.\n");
}

// Function to delete an item by ID
void deleteItem(int id) {
    int itemFound = 0;
    #pragma omp parallel for
    for (int i = 0; i < itemCount; i++) {
        if (items[i].id == id) {
            #pragma omp critical
            {
                for (int j = i; j < itemCount - 1; j++) {
                    items[j] = items[j + 1];
                }
                itemCount--;
                itemFound = 1;
            }
        }
    }
    if (itemFound) {
        printf("\nItem deleted successfully.\n");
    } else {
        printf("\nError: Item with ID %d not found.\n", id);
    }
}


// Function to retrieve an item by ID
void retrieveItem(int id) {
    int itemFound = 0;
    #pragma omp parallel for
    for (int i = 0; i < itemCount; i++) {
        if (items[i].id == id) {
            #pragma omp critical
            {
                printf("\nItem Details:\n");
                printf("ID: %d\nName: %s\nCategory: %s\nQuantity: %d\nPrice: %.2f\n",
                       items[i].id, items[i].name, items[i].category, items[i].quantity, items[i].price);
                itemFound = 1;
            }
        }
    }
    if (!itemFound) {
        printf("\nError: Item with ID %d not found.\n", id);
    }
}


// Function to update an item's details
void updateItem(int id, const char *name, const char *category, int quantity, float price) {
    int itemFound = 0;
    #pragma omp parallel for
    for (int i = 0; i < itemCount; i++) {
        if (items[i].id == id) {
            #pragma omp critical
            {
                if (name) strncpy(items[i].name, name, sizeof(items[i].name) - 1);
                if (category) strncpy(items[i].category, category, sizeof(items[i].category) - 1);
                if (quantity >= 0) items[i].quantity = quantity;
                if (price >= 0) items[i].price = price;
                itemFound = 1;
            }
        }
    }
    if (itemFound) {
        printf("\nItem updated successfully.\n");
    } else {
        printf("\nError: Item with ID %d not found.\n", id);
    }
}

// Function to process bulk updates
void processBulkUpdates(int increment) {
    clock_t start_time, end_time;
    double processing_time;
    double sleep_time = 0.0;

    start_time = clock();

    #pragma omp parallel for // Parallelize item quantity update
    for (int i = 0; i < itemCount; i++) {
        items[i].quantity += increment;
        if (i % 100000 == 0) {
            #pragma omp critical // Synchronize messages for progress
            {
                printf("\nProcessing done for %d items.\n", i);
            }
            sleep(2);
            sleep_time += 2.0;
        }
    }

    end_time = clock();
    processing_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    double total_time = processing_time + sleep_time;

    printf("\nBulk update completed: Incremented all quantities by %d.\n", increment);
    printf("Total processing time: %.2f seconds\n", total_time);
}

// Function to search for items by name (partial match)
void searchItems(const char *keyword) {
    printf("\nSearch Results for '%s':\n", keyword);
    int found = 0;
    #pragma omp parallel for reduction(+:found) // Parallelize search and use reduction to track found items
    for (int i = 0; i < itemCount; i++) {
        if (strstr(items[i].name, keyword)) {
            #pragma omp critical // Synchronize printing search results
            {
                printf("ID: %d | Name: %s | Category: %s | Quantity: %d | Price: %.2f\n",
                       items[i].id, items[i].name, items[i].category, items[i].quantity, items[i].price);
            }
            found++;
        }
    }
    if (found == 0) {
        printf("No items found matching '%s'.\n", keyword);
    }
}

// Function to sort items by price using bubble sort (parallelized)
void sortItemsByPrice() {
    #pragma omp parallel for // Parallelize outer loop for bubble sort
    for (int i = 0; i < itemCount - 1; i++) {
        #pragma omp parallel for // Parallelize inner loop for comparisons and swaps
        for (int j = 0; j < itemCount - i - 1; j++) {
            if (items[j].price > items[j + 1].price) {
                #pragma omp critical // Ensure thread safety for swapping
                {
                    Item temp = items[j];
                    items[j] = items[j + 1];
                    items[j + 1] = temp;
                }
            }
        }
    }
    printf("\nItems sorted by price.\n");
}

// Function to log operations to a file
void logOperation(const char *operation, const char *details) {
    #pragma omp critical // Ensure thread safety for writing logs
    {
        FILE *logFile = fopen("warehouse_log.txt", "a");
        if (!logFile) {
            perror("Error opening log file");
            exit(EXIT_FAILURE);
        }
        fprintf(logFile, "%s: %s\n", operation, details);
        fclose(logFile);
    }
}

// Function to export data to a CSV file
void exportData(const char *filename) {
    #pragma omp parallel for // Parallelize exporting data
    for (int i = 0; i < itemCount; i++) {
        #pragma omp critical // Synchronize file writing
        {
            FILE *file = fopen(filename, "w");
            if (!file) {
                perror("Error opening file for export");
                exit(EXIT_FAILURE);
            }
            fprintf(file, "%d,%s,%s,%d,%.2f\n", items[i].id, items[i].name, items[i].category, items[i].quantity, items[i].price);
            fclose(file);
        }
    }
}

// Function to display items with low stock alert
void stockAlert() {
    printf("\nStock Alert: Low stock items (quantity < %d):\n", LOW_STOCK_THRESHOLD);
    #pragma omp parallel for // Parallelize stock alert checking
    for (int i = 0; i < itemCount; i++) {
        if (items[i].quantity < LOW_STOCK_THRESHOLD) {
            #pragma omp critical // Synchronize printing alert messages
            {
                printf("ID: %d | Name: %s | Category: %s | Quantity: %d | Price: %.2f\n",
                       items[i].id, items[i].name, items[i].category, items[i].quantity, items[i].price);
            }
        }
    }
}

// Function to display the menu
void displayMenu() {
    printf("\nWarehouse Management System Menu:\n");
    printf("1. Add Item\n");
    printf("2. Delete Item\n");
    printf("3. Retrieve Item\n");
    printf("4. Update Item\n");
    printf("5. Process Bulk Updates\n");
    printf("6. Search Items\n");
    printf("7. Sort Items by Price\n");
    printf("8. Export Data\n");
    printf("9. Stock Alert\n");
    printf("10. Print All Items\n");
    printf("11. View Items by Category\n");
    printf("12. Calculate Total Value\n");
    printf("0. Exit\n");
}

// Function to print all items
void printItems() {
    printf("\nAll Items in the Warehouse:\n");
    #pragma omp parallel for // Parallelize item printing
    for (int i = 0; i < itemCount; i++) {
        #pragma omp critical // Synchronize printing item details
        {
            printf("ID: %d | Name: %s | Category: %s | Quantity: %d | Price: %.2f\n",
                   items[i].id, items[i].name, items[i].category, items[i].quantity, items[i].price);
        }
    }
}

// Function to view items by category
void viewItemsByCategory() {
    char category[50];
    printf("\nEnter category to view items: ");
    scanf("%49s", category);

    printf("\nItems in category '%s':\n", category);
    #pragma omp parallel for // Parallelize category filtering
    for (int i = 0; i < itemCount; i++) {
        if (strcmp(items[i].category, category) == 0) {
            #pragma omp critical // Synchronize printing category results
            {
                printf("ID: %d | Name: %s | Quantity: %d | Price: %.2f\n",
                       items[i].id, items[i].name, items[i].quantity, items[i].price);
            }
        }
    }
}

// Function to calculate and display the total value of all items
void calculateTotalValue() {
    clock_t start_time, end_time;
    double processing_time;

    start_time = clock(); // Start timer

    float totalValue = 0.0;

    // Parallelize the loop to calculate the total value without the sleep delay
    #pragma omp parallel for reduction(+:totalValue)
    for (int i = 0; i < itemCount; i++) {
        totalValue += items[i].quantity * items[i].price;
    }



    end_time = clock(); // End timer
    processing_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;


    printf("\nTotal value of all items: %.2f\n", totalValue);

    printf("Processing time: %.2f seconds\n", processing_time);
}


// Function to parallelize the main loop for menu interaction
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
        printf("\nEnter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: {
                int id, quantity;
                float price;
                char name[50], category[50];
                printf("Enter ID: ");
                scanf("%d", &id);
                printf("Enter Name: ");
                scanf("%49s", name);
                printf("Enter Category: ");
                scanf("%49s", category);
                printf("Enter Quantity: ");
                scanf("%d", &quantity);
                printf("Enter Price: ");
                scanf("%f", &price);
                addItem(id, name, category, quantity, price);
                break;
            }
            case 2: {
                int id;
                printf("Enter ID of item to delete: ");
                scanf("%d", &id);
                deleteItem(id);
                break;
            }
            case 3: {
                int id;
                printf("Enter ID of item to retrieve: ");
                scanf("%d", &id);
                retrieveItem(id);
                break;
            }
            case 4: {
                int id, quantity;
                float price;
                char name[50], category[50];
                printf("Enter ID of item to update: ");
                scanf("%d", &id);
                printf("Enter new Name: ");
                scanf("%49s", name);
                printf("Enter new Category: ");
                scanf("%49s", category);
                printf("Enter new Quantity: ");
                scanf("%d", &quantity);
                printf("Enter new Price: ");
                scanf("%f", &price);
                updateItem(id, name, category, quantity, price);
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
                scanf("%49s", keyword);
                searchItems(keyword);
                break;
            }
            case 7: {
                sortItemsByPrice();
                break;
            }
            case 8: {
                char filename[50];
                printf("Enter filename to export data: ");
                scanf("%49s", filename);
                exportData(filename);
                break;
            }
            case 9: {
                stockAlert();
                break;
            }
            case 10: {
                printItems();
                break;
            }
            case 11: {
                viewItemsByCategory();
                break;
            }
            case 12: {
                calculateTotalValue();
                break;
            }
            case 0:
                printf("Exiting program.\n");
                break;
            default:
                printf("Invalid choice, please try again.\n");
        }
    } while (choice != 0);


    free(items);    
    return 0;
}
