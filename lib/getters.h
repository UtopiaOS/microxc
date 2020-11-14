//
// Created by Diego Magdaleno on 11/14/20.
//

#ifndef MICROXCODE_GETTERS_H
#define MICROXCODE_GETTERS_H
#include "typedefs.h"

char *get_toolchain_path(const char*, const char*);
char *get_sdk_path(const char*, const char*);
toolchain_config get_toolchain_info(const char*, const char*);
sdk_config get_sdk_info(const char*, const char*);
default_config get_default_info();

#endif //MICROXCODE_GETTERS_H