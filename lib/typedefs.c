//
// Created by Diego Magdaleno on 11/14/20.
//

#include <stdlib.h>
#include <stdio.h>

#include "typedefs.h"
#include "errors.h"

toolchain_config *
init_toolchain_config(const char *version, const char *name, int *err) {
    toolchain_config *our_toolchain_config = malloc(sizeof(toolchain_config));

    if (our_toolchain_config != NULL && err) {
        *err = ERROR_ALLOCATING_MEMORY;
        free(our_toolchain_config);
        return NULL;
    }

    our_toolchain_config->name = name;
    our_toolchain_config->version = version;
    return our_toolchain_config;
}

sdk_config *
init_sdk_config(const char *name, const char *version, const char *toolchain, const char *default_arch,
                const char *deployment_target, int *err) {
    sdk_config *our_sdk_config = malloc(sizeof(sdk_config));

    if (our_sdk_config == NULL && err) {
        *err = ERROR_ALLOCATING_MEMORY;
        free(our_sdk_config);
        return NULL;
    }

    our_sdk_config->name = name;
    our_sdk_config->version = version;
    our_sdk_config->toolchain = toolchain;
    our_sdk_config->default_arch = default_arch;
    our_sdk_config->deployment_target = deployment_target;

    return our_sdk_config;
}

default_config *
init_default_config(const char* sdk, const char* toolchain, int *err) {
    default_config *our_default_config = malloc(sizeof(default_config));

    if (our_default_config == NULL && err){
        *err = ERROR_ALLOCATING_MEMORY;
        free(our_default_config);
        return NULL;
    }

    our_default_config->toolchain = toolchain;
    our_default_config->sdk = sdk;

    return our_default_config;
}