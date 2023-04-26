#include <stdbool.h>

bool firstNonConsecutive (const int arr[], const int length, int *first) {
  int test = arr[0]; //assigns test var to first num in array
  for(int i = 0; i < length; i++) {
    if(test != arr[i]) {
      *first = arr[i];
      return true;
    }
    else {
      test = arr[i];
      test++;
    }
  }
  return false;
}