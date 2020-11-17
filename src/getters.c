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
#include <TargetConditionals.h>
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
	uint32_t read_size = 0;
	struct stat file_stat;
	char *plist_buf = NULL;
	plist_t plist_data = NULL;

	char *sdk_settings_path = NULL;
	asprintf(&sdk_settings_path, "%s/SDKSettings.plist", path);
	FILE *target_plist = fopen(sdk_settings_path, "rb");
	if (!target_plist) {
		if (err) { *err = ERROR_GETTING_TOOLCHAIN; }
		return NULL;
	}
	memset(&file_stat, '\0', sizeof(struct stat));
	fstat(fileno(target_plist), &file_stat);
	plist_buf = (char *) (malloc(sizeof(char) * (file_stat.st_size + 1)));
	if (plist_buf == NULL) {
		if (err) { *err = ERROR_ALLOCATING_MEMORY; }
		free(plist_buf);
		return NULL;
	}
	read_size = fread(plist_buf, sizeof(char), file_stat.st_size, target_plist);
	plist_from_bin(plist_buf, read_size, &plist_data);
	fclose(target_plist);

	/* If we got to this point, this means we successfully passed our plist file
	 * now our next objective is reading the dict that is located on the var
	 * target_plist, and getting the necessary information to init our struct
	 */

	if (plist_data == NULL) {
		if (err) { *err = ERROR_GETTING_SDK; }
		return NULL;
	}


	/* Value declaration of expected values to return from plist */
	char *tmp_name = NULL;
	char *canonical_name = NULL;
	char *version = NULL;
	char *deployment_target = NULL;
	char *arch = NULL;
	plist_get_string_val(plist_dict_get_item(plist_data, "CanonicalName"), &tmp_name);
	canonical_name = additional_remover(tmp_name);
	free(tmp_name);
	if (canonical_name == NULL) {
		if (err) { *err = INVALID_KEY_PARSED; }
		return NULL;
	}
	plist_get_string_val(plist_dict_get_item(plist_data, "DefaultDeploymentTarget"), &version);
	if (version == NULL) {
		if (err) { *err = INVALID_KEY_PARSED; }
		return NULL;
	}
	plist_get_string_val(plist_array_get_item(plist_dict_get_item(
			plist_dict_get_item(plist_dict_get_item(target_plist, "SupportedTargets"), canonical_name), "Archs"), 0),
	                     &arch);
	if (arch == NULL) {
		if (err) { *err = INVALID_KEY_PARSED; }
		return NULL;
	}
	plist_get_string_val(plist_dict_get_item(plist_data, "DefaultDeploymentTarget"), &deployment_target);
	if (deployment_target == NULL) {
		if (err) { *err = INVALID_KEY_PARSED; }
		return NULL;
	}
	sdk_config *config = init_sdk_config(canonical_name,
	                                     version,
	                                     arch,
	                                     deployment_target,
	                                     &error);
	if (config != NULL && err) {
		*err = ERROR_GETTING_SDK;
		return NULL;
	}
	return config;
}

default_config *
get_default_info(int *err) {
	char *supposed_name;
	int error;
#if TARGET_OS_IPHONE
	supposed_name = "iPhoneOS";
#elif TARGET_OS_MAC
	supposed_name = "MacOSX";
#else
	supposed_name = NULL;
#endif
	if (supposed_name == NULL) {
		if (err) { *err = UNSUPPORTED_PLATFORM; }
		return NULL;
	}

	// TODO: Unhardcode this! This is only temporal!
	default_config *config = init_default_config(supposed_name, "XcodeDefault", &error);
	if (config == NULL && err) {
		*err = ERROR_GETTING_DEFAULT_CONFIG;
		return NULL;
	}
	return config;
}

char *
get_toolchain_path(const char *developer_dir, const char *name, int *err) {
	char *path = NULL;
	int error;
	path = (char *) malloc(PATH_MAX - 1);
	sprintf(path, "%s/Toolchains/%s.xctoolchain", developer_dir, name);
	validate_directory_path(path, &error);
	if (error != SUCCESFUL_OPERATION) {
		if (err) { *err = error; }
		return NULL;
	}
	if (err) { *err = SUCCESFUL_OPERATION; }
	return path;

}

char *
get_sdk_path(const char *developer_dir, const char *name, int *err) {
	char *path = NULL;
	int error;
	path = (char *) malloc(PATH_MAX - 1);
	sprintf(path, "%s/Platforms/%s.Platform/Developer/SDKs/%s.sdk", developer_dir, name, name);
	validate_directory_path(path, &error);
	if (error != SUCCESFUL_OPERATION) {
		if (err) { *err = error; }
		return NULL;
	}
	test_sdk_authenticity(path, &error);
	if (error != SUCCESFUL_OPERATION) {
		if (err) { *err = error; }
		return NULL;
	}
	if (err) { *err = SUCCESFUL_OPERATION; }
	return path;

}