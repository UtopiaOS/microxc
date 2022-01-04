//
// Created by Diego Magdaleno on 11/14/20.
//

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <plist/plist.h>
#include <sys/stat.h>
//#include <TargetConditionals.h>
#include "validators.h"
#include "typedefs.h"
#include "getters.h"
#include "errors.h"

static char *additional_remover(char *input) {
	char *dest = input;
	char *src = input;
	while (*src) {
		if (isdigit(*src) || ispunct(*src)) {
			src++;
			continue;
		}
		*dest++ = *src++;
	}
	*dest = '\0';
	return input;
}

/**
 * @func get_sdk_info -- fetch config info from a toolchain's info.ini
 * @arg path - path to sdk's info.ini
 * @return: struct containing sdk config info
 */
sdk_config *
get_sdk_info(const char *path, int *err) {
	int error;
	uint32_t readSize = 0;
	struct stat fileStat;
	char *plistBuf = NULL;
	plist_t plistData = NULL;

	char *sdkSettingsPath = NULL;
	asprintf(&sdkSettingsPath, "%s/SDKSettings.plist", path);
	FILE *targetPlist = fopen(sdkSettingsPath, "rb");
	if (!targetPlist) {
		if (err) { *err = ERROR_GETTING_TOOLCHAIN; }
		return NULL;
	}
	memset(&fileStat, '\0', sizeof(struct stat));
	fstat(fileno(targetPlist), &fileStat);
	plistBuf = (char *) (malloc(sizeof(char) * (fileStat.st_size + 1)));
	if (plistBuf == NULL) {
		if (err) { *err = ERROR_ALLOCATING_MEMORY; }
		free(plistBuf);
		return NULL;
	}
	readSize = fread(plistBuf, sizeof(char), fileStat.st_size, targetPlist);
	plist_from_bin(plistBuf, readSize, &plistData);
	fclose(targetPlist);

	/* If we got to this point, this means we successfully passed our plist file
	 * now our next objective is reading the dict that is located on the var
	 * targetPlist, and getting the necessary information to init our struct
	 */

	if (plistData == NULL) {
		printf("here");
		if (err) { *err = ERROR_GETTING_SDK; }
		return NULL;
	}


	/* Value declaration of expected values to return from plist */
	char *tmpName = NULL;
	char *canonicalName = NULL;
	char *version = NULL;
	char *deploymentTarget = NULL;
	char *arch = NULL;
	plist_get_string_val(plist_dict_get_item(plistData, "CanonicalName"), &tmpName);
	canonicalName = additional_remover(tmpName);
	free(tmpName);
	if (canonicalName == NULL) {
		if (err) { *err = INVALID_KEY_PARSED; }
		return NULL;
	}
	plist_get_string_val(plist_dict_get_item(plistData, "DefaultDeploymentTarget"), &version);
	if (version == NULL) {
		if (err) { *err = INVALID_KEY_PARSED; }
		return NULL;
	}
	plist_get_string_val(plist_array_get_item(plist_dict_get_item(
			plist_dict_get_item(plist_dict_get_item(targetPlist, "SupportedTargets"), canonicalName), "Archs"), 0),
	                     &arch);
	if (arch == NULL) {
		if (err) { *err = INVALID_KEY_PARSED; }
		return NULL;
	}
	plist_get_string_val(plist_dict_get_item(plistData, "DefaultDeploymentTarget"), &deploymentTarget);
	if (deploymentTarget == NULL) {
		if (err) { *err = INVALID_KEY_PARSED; }
		return NULL;
	}
	sdk_config *config = initSdkConfig(canonicalName,
	                                   version,
	                                   arch,
	                                   deploymentTarget,
	                                   &error);
	if (config != NULL && err) {
		*err = ERROR_GETTING_SDK;
		return NULL;
	}
	return config;
}

default_config *
getDefaultInfo(int *err) {
	char *supposedName;
	int error;
#if TARGET_OS_IPHONE
	supposedName = "iPhoneOS";
#elif TARGET_OS_MAC
	supposedName = "MacOSX";
#else
	supposedName = "UtopiaOS";
#endif
	if (supposedName == NULL) {
		if (err) { *err = UNSUPPORTED_PLATFORM; }
		return NULL;
	}

	// TODO: Unhardcode this! This is only temporal!
	default_config *config = initDefaultConfig(supposedName, "XcodeDefault", &error);
	if (config == NULL && err) {
		*err = ERROR_GETTING_DEFAULT_CONFIG;
		return NULL;
	}
	return config;
}

char *
getToolchainPath(const char *developerDir, const char *name, int *err) {
	char *path = NULL;
	int error;
	path = (char *) malloc(PATH_MAX - 1);
	sprintf(path, "%s/Toolchains/%s.xctoolchain", developerDir, name);
	validateDirectoryPath(path, &error);
	if (error != SUCCESFUL_OPERATION) {
		if (err) { *err = error; }
		return NULL;
	}
	if (err) { *err = SUCCESFUL_OPERATION; }
	return path;

}

char *
getSdkPath(const char *developerDir, const char *name, int *err) {
	char *path = NULL;
	int error;
	path = (char *) malloc(PATH_MAX - 1);
	sprintf(path, "%s/Platforms/%s.Platform/Developer/SDKs/%s.sdk", developerDir, name, name);
	validateDirectoryPath(path, &error);
	if (error != SUCCESFUL_OPERATION) {
		if (err) { *err = error; }
		return NULL;
	}
	testSdkAuthenticity(path, &error);
	if (error != SUCCESFUL_OPERATION) {
		if (err) { *err = error; }
		return NULL;
	}
	if (err) { *err = SUCCESFUL_OPERATION; }
	return path;

}