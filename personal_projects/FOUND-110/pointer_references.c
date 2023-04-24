#include <stdio.h>
#include <string.h>

int main() {
    int number = 7;
    int *num_ptr;

    num_ptr = &number; //stores mem address of number in num_ptr

    printf("number = %d\n", number);
    printf("number mem address = 0x%08x\n", &number); //ampersand references mem address and not actual value of variable
    printf("num_ptr mem address = 0x%08x\n", &num_ptr);
    printf("num_ptr pointing address = 0x%08x\n", num_ptr); //shows mem address where pointer is being stored
    printf("num_ptr value stored = %d\n", *num_ptr); 
}