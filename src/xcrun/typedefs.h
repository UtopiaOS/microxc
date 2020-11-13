//
// Created by Diego Magdaleno on 11/13/20.
//

#ifndef MICROXCODE_TYPEDEFS_H
#define MICROXCODE_TYPEDEFS_H

/* Toolchain configuration struct */
typedef struct {
    const char *name;
    const char *version;
} toolchain_config;

/* SDK configuration struct */
typedef struct {
    const char *name;
    const char *version;
    const char *toolchain;
    const char *default_arch;
    const char *deployment_target;
} sdk_config;

/* xcrun default configuration struct */
typedef struct {
    const char *sdk;
    const char *toolchain;
} default_config;

#endif //MICROXCODE_TYPEDEFS_H
