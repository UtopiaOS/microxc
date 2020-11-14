//
// Created by Diego Magdaleno on 11/14/20.
//

#include "getters.h"
#include <stdlib.h>
#include <stdio.h>
#include "validators.h"

char*
get_toolchain_path(const char* developer_dir, const char* name)
{
    char *path = NULL;

    path = (char *)malloc(PATH_MAX - 1);

    sprintf(path, "%s/Toolchains/%s.xctoolchain", developer_dir, name);

    if (validate_directory_path(path) == 0){
        return path;
    }

    return NULL;
}

char *
get_sdk_path(const char *developer_dir, const char* name)
{
    char *path = NULL;

    path = (char *)malloc(PATH_MAX - 1);

    sprintf(path, "%s/Platforms/%s.Platform/Developer/SDKs/%s.sdk", developer_dir, name, name);
    if (validate_directory_path(path) == 0) {
        return path;
    }

    return NULL;

}