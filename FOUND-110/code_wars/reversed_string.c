#include <string.h>
char *strrev (char *string)
{
    int length = 0;
    char *start = 0;
    char *end = 0;
    char temp = 0;

    length = strlen(string);
    start = string;
    end = string;

    //  the end_ptr is moved to the last character  
    for (int i = 0; i < length - 1; i++)
        end++;

    //swapping 
    for (int i = 0; i < length / 2; i++) {
        temp = *end;
        *end = *start;
        *start = temp;
        start++;
        end--;
    }
    return string; // reverse the string in place and return it
}