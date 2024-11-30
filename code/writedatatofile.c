#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_ITEMS 10000000 
#define NUM_FILES 20 

float randomFloat(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

const char* randomCategory() {
    const char* categories[] = {"Electronics", "Clothing", "Home Goods", "Books", "Toys", "Food"};
    int index = rand() % 6;
    return categories[index];
}

void generateRandomData(const char *filename, long long startID, long long numItems) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "ID,Name,Category,Quantity,Price\n");

    for (long long i = 0; i < numItems; i++) {
        long long id = startID + i; 
        char name[20];
        snprintf(name, sizeof(name), "Item_%lld", id); 
        int quantity = rand() % 1000 + 1; 
        float price = randomFloat(1.0, 100.0); 
        const char* category = randomCategory(); 

        fprintf(file, "%lld,%s,%s,%d,%.2f\n", id, name, category, quantity, price);
    }

    fclose(file);
    printf("Data written successfully to %s\n", filename);
}

int main() {
    srand(time(NULL)); 

    clock_t startTime = clock();

    long long itemsPerFile = MAX_ITEMS / NUM_FILES; 
    long long remainder = MAX_ITEMS % NUM_FILES;   

    for (int i = 0; i < NUM_FILES; i++) {
        char filename[50];
        snprintf(filename, sizeof(filename), "warehouse_data_%d.csv", i + 1); 

        long long startID = i * itemsPerFile + 1; 
        long long numItems = itemsPerFile;      

        if (i == NUM_FILES - 1) { 
            numItems += remainder;
        }

        printf("Generating data for file %s...\n", filename);
        generateRandomData(filename, startID, numItems);
    }

    clock_t endTime = clock();
    double totalTime = ((double)(endTime - startTime)) / CLOCKS_PER_SEC;

    printf("All files generated successfully.\n");
    printf("Total time taken: %.2f seconds\n", totalTime);

    return 0;
}
