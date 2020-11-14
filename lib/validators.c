//
// Created by Diego Magdaleno on 11/14/20.
//

#include "validators.h"
#include "errors.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

int
test_sdk_authenticity(const char *path)
{
    microxcode_error_state_t error_state;
    char *fname = NULL;

    fname = (char *)malloc(PATH_MAX - 1);

    sprintf(fname, "%s/SDKSettings.plist", path);
    if (access(fname, F_OK) != -1){
        error_state = NOT_AUTHENTIC;
        return error_state;
    }

    free(fname);

    return error_state;
}

int
validate_directory_path(const char *dir)
{
    struct stat fstat;
    microxcode_error_state_t error_state;

    if (stat(dir, &fstat) != 0){
        error_state = UNABLE_TO_VALIDATE;
    } else {
        if (S_ISDIR(fstat.st_mode) == 0){
            error_state = NOT_VALID;
        }
    }

    return error_state;
}