#include <stdlib.h>
#include <stdio.h>

char *number_to_string(int number) {
  char *str_ptr;
  str_ptr = (char *)malloc((sizeof(number) + 1) * sizeof(str_ptr));
  sprintf(str_ptr, "%d", number);
  
  return str_ptr; // memory will be freed
}