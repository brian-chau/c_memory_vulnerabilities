#include <stdio.h>
#include <string.h>

int main(void) {
    char buff[15];
    int pass = 0;

    printf("Password: ");
    gets(buff);

    if ( strcmp(buff, "hunter2") ) {
        printf("Wrong Password\n");
    } else {
        pass = 1;
    }

    if ( pass ) {
        printf ("Success!\n");
    }

    return 0;
}