#include <stdio.h>
#include <malloc.h>

int main() {
    char * a_ptr = (char *) malloc(16 * sizeof(char));
    printf("a_ptr address before free: %p\n", a_ptr);
    free(a_ptr);
    printf("a_ptr address after free:  %p\n\n", a_ptr);

    printf("Overwriting bytes 8 and 9 of a_ptr's tcache_entry struct, which corresponds to 'the first two bytes of the pointer called \"key\" in the tcache_entry struct.\n");
    a_ptr[8] = 1;
    a_ptr[9] = 2;
    free(a_ptr);

    char * b_ptr = (char *) malloc(16 * sizeof(char));
    printf("b_ptr address: %p\n", b_ptr);
    char * c_ptr = (char *) malloc(16 * sizeof(char));
    printf("c_ptr address: %p\n\n", c_ptr);

    return 0;
}
