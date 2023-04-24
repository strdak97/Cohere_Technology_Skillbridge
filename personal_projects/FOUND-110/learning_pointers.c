#include <stdio.h>
#include <string.h>

int main() {
    char string_array[26];
    char *bgn_pointer; //Points to start of array
    char *mid_pointer; //Points to middle of array
    char *end_pointer; //Points to end of array

    strcpy(string_array, "Computer, Keyboard, Mouse\n");
    bgn_pointer = string_array; 
    mid_pointer = string_array + 10;
    end_pointer = string_array + 20;

    strcpy(mid_pointer, "SkiBoard, Mouse\n");
    strcpy(end_pointer, "Mices\n");

    printf(bgn_pointer); //This will show the edited strings because all 3 pointer refer to the same memory address 
                         //instead of copying the data as a whole
}