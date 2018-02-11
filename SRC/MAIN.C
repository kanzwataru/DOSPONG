#include <stdio.h>
#include <conio.h>

#include "src/dospong.h"
#include "src/cubetest.h"
#include "src/pongmenu.h"
#include "src/pcinput.h"

void test_pcinput(void)
{
    unsigned char a;
    do {
        a = read_scancode();
        printf("%c (%u)\n", a, a);
    } while(a != 1); /* ESC */
}

void test_getch(void)
{
    unsigned char a;
    do {
        a = getch();
        printf("%c (%u)\n", a, a);
    } while(a != 27); /* ESC */
}

int main(int argc, char **argv)
{
    if(argc > 1) {
        if(0 == strcmp(argv[1], "cubetest")) {
            return cubetest_init();
        }
        else if(0 == strcmp(argv[1], "pcinput")) {
            test_pcinput();
        }
        else if(0 == strcmp(argv[1], "getch")) {
            test_getch();
        }
        else {
            printf("Unknown argument '%s'\n", argv[1]);
        }
    }
    else {
        return pong_menu_init();
    }

    return 0;
}
