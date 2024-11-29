#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_ITEMS 1000000 // Define the number of items to generate

// Function to generate a random float between two values
float randomFloat(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

// Function to generate a random category
const char* randomCategory() {
    const char* categories[] = {"Electronics", "Clothing", "Home Goods", "Books", "Toys", "Food"};
    int index = rand() % 6;
    return categories[index];
}

// Function to generate random data and write it to a CSV file
void generateRandomData(const char *filename, int numItems) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    // Write the header row
    fprintf(file, "ID,Name,Category,Quantity,Price\n");

    // Generate and write random data
    for (int i = 0; i < numItems; i++) {
        int id = i + 1; // Sequential ID starting from 1
        char name[20];
        snprintf(name, sizeof(name), "Item_%d", id); // Generate item name
        int quantity = rand() % 1000 + 1; // Random quantity between 1 and 1000
        float price = randomFloat(1.0, 100.0); // Random price between 1.0 and 100.0
        const char* category = randomCategory(); // Random category

        // Write the data row
        fprintf(file, "%d,%s,%s,%d,%.2f\n", id, name, category, quantity, price);
    }

    fclose(file);
    printf("Data generated successfully and saved to %s\n", filename);
}

int main() {
    srand(time(NULL)); // Seed the random number generator

    const char *filename = "warehouse_data.csv";
    printf("Generating random warehouse data...\n");

    generateRandomData(filename, MAX_ITEMS);

    printf("Random data generation complete.\n");
    return 0;
}