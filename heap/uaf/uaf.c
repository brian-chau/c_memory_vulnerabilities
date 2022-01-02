#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <unistd.h>

// Summary: In this application, the user would expect that it would only
//          accept the password "hunter2". However, it accepts any password,
//          and will print the contents of "file.txt" regardless of what the
//          user inputs.
//
//          Why? When we free the memory for "user_input," it lets the app
//          re-use that address, so when we malloc again for "authenticated",
//          "user_input" and "authenticated" would end up sharing the
//          same heap memory address. As a result, when "user_input" is 
//          re-used in the second "scanf", it fills the heap memory address 
//          for "authenticated" with data intended for "user_input," resulting
//          in the "if(*authenticated)" condition (which checks if "authenticated"
//          has data, which it now does) to pass. This is called a use-after free 
//          vulnerability.
//
//          The %p format specifier tells "printf" to display the address 
//          of the malloc'd variable, so we can see that both "user_input"
//          and "authenticated" have the same address.
//          Source: pwn.college YouTube video = https://youtu.be/Cr9IeGQxFoc
//
int main() {
    char *user_input = (char*) malloc(8);                    // vuln here
    printf("Name: ");
    scanf("%7s", user_input);
    printf("Hello %s!\n", user_input);
    printf("user_input address: %p\n", user_input);
    free(user_input);

    long *authenticated = (long*) malloc(8);                 // vuln here
    printf("authenticated address: %p\n", authenticated);
    *authenticated = 0;

    printf("Password: ");
    scanf("%7s", user_input);                               // vuln here

    if(strcmp(user_input, "hunter2") == 0) {
        *authenticated = 1;
    }
    if (*authenticated) {                                   // vuln here
        sendfile(STDOUT_FILENO, open("file.txt", O_RDONLY), 0, 128);
    }
}
