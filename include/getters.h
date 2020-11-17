//
// Created by Diego Magdaleno on 11/14/20.
//

#ifndef MICROXCODE_GETTERS_H
#define MICROXCODE_GETTERS_H

#include "typedefs.h"

char *getToolchainPath(const char *, const char *, int *);

char *getSdkPath(const char *, const char *, int *);

sdk_config *get_sdk_info(const char *, int *);

default_config *getDefaultInfo(int *);
#endif //MICROXCODE_GETTERS_H