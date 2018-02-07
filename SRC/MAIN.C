#include <stdio.h>

#include "src/dospong.h"
#include "src/cubetest.h"
#include "src/pongmenu.h"

int main(int argc, char **argv)
{
    if(argc > 1) {
        if(0 == strcmp(argv[1], "cubetest")) {
            return cubetest_init();
        }
        else {
            printf("Unknown argument '%s'\n", argv[1]);
        }
    }
    else {
        return pong_menu_init();
    }

    return 1;
}
