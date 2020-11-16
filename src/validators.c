//
// Created by Diego Magdaleno on 11/14/20.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "validators.h"
#include "errors.h"

void
test_sdk_authenticity(const char *path, int *err) {
    char *fname = NULL;

    fname = (char *) malloc(PATH_MAX - 1);

    sprintf(fname, "%s/SDKSettings.plist", path);
    if (err) {
        *err = (access(fname, F_OK) == (-1)) ? NOT_AUTHENTIC : SUCCESFUL_OPERATION;
    }

    free(fname);
}

void
validate_directory_path(const char *dir, int *err) {
    struct stat fstat;

    if (stat(dir, &fstat) != 0) {
        if (err) { *err = UNABLE_TO_VALIDATE; }
        return;
    }

    if (S_ISDIR(fstat.st_mode) == 0) {
        if (err) { *err = NOT_VALID; }
        return;
    }

    if (err) { *err = SUCCESFUL_OPERATION; }

}
