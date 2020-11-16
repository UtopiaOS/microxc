//
// Created by Diego Magdaleno on 11/14/20.
//

#include <stdlib.h>
#include <stdio.h>

#include "typedefs.h"
#include "errors.h"

sdk_config *
init_sdk_config(const char *name, const char *version, const char *default_arch,
                const char *deployment_target, int *err) {
    sdk_config *our_sdk_config = malloc(sizeof(sdk_config));

    if (our_sdk_config == NULL && err) {
        *err = ERROR_ALLOCATING_MEMORY;
        free(our_sdk_config);
        return NULL;
    }

    our_sdk_config->name = name;
    our_sdk_config->version = version;
    our_sdk_config->default_arch = default_arch;
    our_sdk_config->deployment_target = deployment_target;

    return our_sdk_config;
}

default_config *
init_default_config(const char *sdk, const char *toolchain, int *err) {
    default_config *our_default_config = malloc(sizeof(default_config));

    if (our_default_config == NULL && err) {
        *err = ERROR_ALLOCATING_MEMORY;
        free(our_default_config);
        return NULL;
    }

    our_default_config->toolchain = toolchain;
    our_default_config->sdk = sdk;

    return our_default_config;
}