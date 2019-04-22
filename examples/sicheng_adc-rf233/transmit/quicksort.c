// https://www.geeksforgeeks.org/quick-sort/

#include "quicksort.h"

void printArray(uint16_t arr[], unsigned size) {
  unsigned i; 
  for (i = 0; i < size; i++)
    printf("%d ", arr[i]);
  printf("\n");
}

// A utility function to swap two elements
void swap(uint16_t* a, uint16_t* b) {
  uint16_t t = *a;
  *a = *b;
  *b = t;
}

unsigned partition (uint16_t arr[], unsigned low, unsigned high) {
  int pivot = arr[high]; 
  unsigned i = (low - 1); 

  for (unsigned j = low; j <= high - 1; j++) {
    if (arr[j] <= pivot) {
      i++;
      swap(&arr[i], &arr[j]);
    }
  }
  swap(&arr[i + 1], &arr[high]); 
  return (i + 1);
}

void quickSort(uint16_t arr[], unsigned low, unsigned high) {
  if (low < high) {
    /* pi is partitioning index, arr[p] is now
       at right place */
    unsigned pi = partition(arr, low, high);

    // Separately sort elements before
    // partition and after partition
    quickSort(arr, low, pi - 1);
    quickSort(arr, pi + 1, high);
  }
}
