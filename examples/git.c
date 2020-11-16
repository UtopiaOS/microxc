//
// Created by Diego Magdaleno on 11/16/20.
//

#include "runners.h"
#include "stdio.h"

int main(int argc, char *argv[]) {

    int error;
    command("git", argc, argv, &error, "iPhoneOS");

    printf("%d", error);
    return 0;
}
