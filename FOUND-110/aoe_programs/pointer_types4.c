#include <stdio.h>

int main() {
    int i;

    char char_array{'a', 'b', 'c', 'd', 'e'};
    int int_array{1, 2, 3, 4, 5};

    void *void_pointer;

    for(i = 0; i < 5; i++) {
        printf("[char pointer] points to %p, which contains the char '%c'\n",
        void_pointer, *((char *) void_pointer));
        void_pointer = (void *) ((char *) void_pointer + 1);
    }

    for(i = 0; i < 5; i++) {
        printf("[integer pointer] points to %p, which contains the integer %d\n",
        void_pointer, *((int *) void_pointer));
        void_pointer = (void *) ((int *) void_pointer + 1);
    }
}