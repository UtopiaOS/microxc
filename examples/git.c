//
// Created by Diego Magdaleno on 11/16/20.
//

#include "runners.h"
#include "stdio.h"

int main(int argc, char *argv[]) {

    int error;
    command("git", argc, argv, &error, NULL, "XcodeDefault", 1, 0);

    printf("%d", error);
    return 0;
}
