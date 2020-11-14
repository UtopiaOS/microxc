//
// Created by Diego Magdaleno on 11/14/20.
//

#include "sdk_authenticity.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

int
test_sdk_authenticity(const char *path)
{
    int return_val = 0;
    char *fname = NULL;

    fname = (char *)malloc(PATH_MAX - 1);

    sprintf(fname, "%s/SDKSettings.plist", path);
    if (access(fname, F_OK) != -1){
    return_val = -1;
    }

    free(fname);

    return return_val;
}