//
// Created by Diego Magdaleno on 11/14/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <dirent.h>
#include "runners.h"
#include "getters.h"
#include "developer_path.h"
#include "errors.h"
#include "verbose_printf.h"
#include "logging_printf.h"

void command(const char *cmd, int argc, char *argv[], int *err, ...) {
	char *sdk, *toolchain;
	int verbose, findOnly;
	int error;
	default_config *ourConfig;

	char *developerPath;
	developerPath = getDeveloperPath(&error);
	if (error != SUCCESFUL_OPERATION && err) {
		*err = error;
		return;
	}
	va_list args;
	va_start(args, err);
	sdk = va_arg(args, char*);

	/* We have an SDK, now lets call our "getSdkPath
	 * function, if it is invalid this will return an error
	*/
	if (sdk) {
		sdk = getSdkPath(developerPath, sdk, &error);
		if (error != SUCCESFUL_OPERATION) {
			if (err) { *err = error; }
		}
	} else {
		ourConfig = getDefaultInfo(&error);
		if (error != SUCCESFUL_OPERATION && err) {
			*err = error;
			return;
		}
		sdk = getSdkPath(developerPath, ourConfig->sdk, &error);
		if (error != SUCCESFUL_OPERATION && err) {
			*err = error;
			return;
		}
	}
	toolchain = va_arg(args, char*);
	if (toolchain) {
		toolchain = getToolchainPath(developerPath, toolchain, &error);
		if (error != SUCCESFUL_OPERATION) {
			if (err) { *err = error; }
		}
	} else {
		ourConfig = getDefaultInfo(&error);
		if (error != SUCCESFUL_OPERATION && err) {
			*err = error;
			return;
		}
		toolchain = getToolchainPath(developerPath, ourConfig->toolchain, &error);
		if (error != SUCCESFUL_OPERATION && err) {
			*err = error;
			return;
		}
	}
	verbose = va_arg(args, int);
	findOnly = va_arg(args, int);
	va_end(args);
	requestCommand((bool) verbose, (bool) findOnly, cmd, sdk, toolchain, argc, argv, &error);
	if (err) { *err = error; }

}

/**
 * @func callCommand -- Execute new process to replace this one.
 * @arg cmd - program's absolute path
 * @arg argc - number of arguments to be passed to new process
 * @arg argv - arguments to be passed to new process
 * @return: -1 on error, otherwise no return
 */
void
callCommand(bool verbose, const char *cmd, const char *currentSdk, const char *currentToolchain, int argc,
            char **argv, int *err) {
	int i;
	char *envp[5] = {NULL};
	int error;

	char *developerPath = getDeveloperPath(&error);
	if (error != SUCCESFUL_OPERATION && err) {
		*err = error;
		return;
	}

	/*
	 * Pass SDKROOT, PATH, HOME, LD_LIBRARY_PATH, TARGET_TRIPLE, and MACOSX_DEPLOYMENT_TARGET to the called program's environment.
	 *
	 * > SDKROOT is used for when programs such as clang need to know the location of the sdk.
	 * > PATH is used for when programs such as clang need to call on another program (such as the linker).
	 * > HOME is used for recursive calls to xcrun (such as when xcrun calls a script calling xcrun ect).
	 * > LD_LIBRARY_PATH is used for when tools needs to access libraries that are specific to the toolchain.
	 * > TARGET_TRIPLE is used for clang/clang++ cross compilation when building on a foreign host.
	 * > {MACOSX|IOS}_DEPLOYMENT_TARGET is used for tools like ld that need to set the minimum compatibility
	 *   version number for a linked binary.
	 */

	asprintf(&envp[0], "SDKROOT=%s", currentSdk);
	asprintf(&envp[1], "PATH=%s/usr/bin:%s/usr/bin:%s", developerPath, currentToolchain, getenv("PATH"));
	asprintf(&envp[2], "LD_LIBRARY_PATH=%s/usr/src", currentToolchain);
	asprintf(&envp[3], "HOME=%s", getenv("HOME"));
	if (verbose) {
		loggingPrintf(stdout, "libxcselect: info: invoking command:\n\t\"%s", cmd);
		for (i = 1; i < argc; i++)
			loggingPrintf(stdout, " %s", argv[i]);
		loggingPrintf(stdout, "\"\n");
	}
	if (execve(cmd, argv, envp) == -1) {
		if (err) { *err = RUNNING_COMMAND_ERROR; }
		return;
	}
	if (err) { *err = SUCCESFUL_OPERATION; }
}

/**
 * @func searchCommand -- Search a set of directories for a given command
 * @arg name - program's name
 * @arg dirs - set of directories to search, separated by colons
 * @return: the program's absolute path on success, NULL on failure
 */
char *searchCommand(bool verbose, const char *name, char *dirs, int *err) {
	char *cmd = NULL;    /* command's absolute path */
	char *abslPath = NULL;        /* path entry to search */
	char delimiter[2] = ":";    /* delimiter for directories in dirs argument */
	char **possible = NULL;
	int pathCount = 0, i;
	char *command = NULL;
	cmd = (char *) malloc(PATH_MAX - 1);

	/* Get an array of all the possible places the command could be */
	abslPath = strtok(dirs, delimiter);
	while (abslPath != NULL) {
		possible = realloc(possible, sizeof(char *) * ++pathCount); /* Resize our "array" */

		if (possible == NULL) {
			if (err) { *err = ERROR_ALLOCATING_MEMORY; }
			return NULL; /*Memory allocation failed */
		}
		if (verbose) {
			verbosePrintf(stderr, "libxcselect: info: checking directory \'%s\' for command \'%s\'...\n", abslPath,
			              name);
		}

		/* Construct our program's absolute path, and append it to array */
		sprintf(cmd, "%s/%s", abslPath, name);
		possible[pathCount - 1] = cmd;

		/* Move to the next entry */
		abslPath = strtok(NULL, delimiter);
	}

	/* Free the cmd allocation as we don't need it anymore */
	free(cmd);

	/* reallocate one extra element for the last NULL */
	possible = realloc(possible, sizeof(char *) * (pathCount + 1));
	possible[pathCount] = 0;

	/* Iterate over the array, until we find a command that meets the criteria */
	for (i = 0; i < (pathCount + 1); ++i) {
		if (access(possible[i], (F_OK | X_OK)) != (-1)) {
			if (verbose) {
				verbosePrintf(stdout, "libxcselect: info: found command's absolute path: \'%s\'\n", possible[i]);
			}
			command = possible[i];
			break;
		}
	}
	free(possible);
	return command;
}

/**
 * @func requestCommand - Request a program.
 * @arg name -- name of program
 * @arg argv -- arguments to be passed if program found
 * @return: -1 on failed search, 0 on successful search, no return on execute
 */
void
requestCommand(bool verbose, bool findOnly, const char *name, const char *currentSdk, const char *currentToolchain,
               int argc,
               char **argv, int *err) {
	char *cmd = NULL;    /* used to hold our command's absolute path */
	char searchString[PATH_MAX * 1024];    /* our search string */
	int error; /* Check if some error occurred in another function */

	char *developerPath = getDeveloperPath(&error);
	if (error != SUCCESFUL_OPERATION) {
		if (err) { *err = error; }
		return;
	}
	sprintf(searchString, "%s/usr/bin:%s/usr/bin:%s/usr/bin", developerPath, currentSdk, currentToolchain);

	/* Search each path entry in searchString until we find our program. */
	if ((cmd = searchCommand(verbose, name, searchString, &error)) != NULL) {
		if (findOnly) {
			if (access(cmd, (F_OK | X_OK)) != (-1)) {
				fprintf(stdout, "%s\n", cmd);
				free(cmd);
				if (err) { *err = SUCCESFUL_OPERATION; }
				return;
			} else {
				if (err) { *err = PROGRAM_NOT_FOUND; }
				free(cmd);
				return;
			}
		} else {
			callCommand(verbose, cmd, currentSdk, currentToolchain, argc, argv, &error);
			if (error != SUCCESFUL_OPERATION) {
				if (err) { *err = error; }
				return;
			}
			/* NO REACH */
			if (verbose) {
				fprintf(stderr, "libxcselect: error: can't exec \'%s\' (errno=%s)\n", cmd, strerror(errno));
				if (err) { *err = EXECUTION_ERROR; }
			}
		}
	}

	/* We have searched everywhere, but we haven't found our program. State why. */
	if (verbose) {
		if (err) { *err = PROGRAM_NOT_FOUND; }
		fprintf(stderr, "libxcselect: error: can't stat \'%s\' (errno=%s)\n", name, strerror(errno));
	}
	if (err) { *err = PROGRAM_NOT_FOUND; }
}
