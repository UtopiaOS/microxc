//
// Created by Diego Magdaleno on 11/16/20.
//

#include "runners.h"
#include "stdio.h"

int main(int argc, char *argv[]) {

    int error;
    command("brew", argc, argv, &error);

    printf("\n%d\n", error);
    return 0;
}
