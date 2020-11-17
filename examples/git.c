//
// Created by Diego Magdaleno on 11/16/20.
//

#include "runners.h"
#include "stdio.h"

int main(int argc, char *argv[]) {

    int error;
    command("clang", argc, argv, &error, NULL, NULL, 0, 0);

    printf("\n%d\n", error);
    return 0;
}
