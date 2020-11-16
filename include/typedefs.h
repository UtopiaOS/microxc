//
// Created by Diego Magdaleno on 11/13/20.
//

#ifndef MICROXCODE_TYPEDEFS_H
#define MICROXCODE_TYPEDEFS_H

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

sdk_config *init_sdk_config(const char *, const char *, const char *, const char *, const char *, int *);

default_config *init_default_config(const char *, const char *, int *);

#endif //MICROXCODE_TYPEDEFS_H
