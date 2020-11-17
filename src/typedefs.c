//
// Created by Diego Magdaleno on 11/14/20.
//

#include <stdlib.h>
#include <stdio.h>
#include "typedefs.h"
#include "errors.h"

sdk_config *
initSdkConfig(const char *name, const char *version, const char *defaultArch,
              const char *deploymentTarget, int *err) {
	sdk_config *ourSdkConfig = malloc(sizeof(sdk_config));
	if (ourSdkConfig == NULL && err) {
		*err = ERROR_ALLOCATING_MEMORY;
		free(ourSdkConfig);
		return NULL;
	}
	ourSdkConfig->name = name;
	ourSdkConfig->version = version;
	ourSdkConfig->default_arch = defaultArch;
	ourSdkConfig->deployment_target = deploymentTarget;
	return ourSdkConfig;
}

default_config *
initDefaultConfig(const char *sdk, const char *toolchain, int *err) {
	default_config *ourDefaultConfig = malloc(sizeof(default_config));
	if (ourDefaultConfig == NULL && err) {
		*err = ERROR_ALLOCATING_MEMORY;
		free(ourDefaultConfig);
		return NULL;
	}
	ourDefaultConfig->toolchain = toolchain;
	ourDefaultConfig->sdk = sdk;
	return ourDefaultConfig;
}