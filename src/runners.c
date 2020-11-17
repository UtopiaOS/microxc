//
// Created by Diego Magdaleno on 11/14/20.
//

#include <stdio.h>
#include <stdlib.h>
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

extern char **environ;

void command(const char *cmd, int argc, char *argv[], int *err, ...) {
	char *sdk, *toolchain;
	int verbose;
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
	va_end(args);
	requestCommand((bool) verbose, cmd, sdk, toolchain, argc, argv, &error);
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
callCommand(bool verbose, const char *cmd, int argc, char **argv, int *err) {
	if (verbose) {
		loggingPrintf(stdout, "libxcselect: info: invoking command:\n\t\"%s", cmd);
		for (int i = 1; i < argc; i++)
			loggingPrintf(stdout, " %s", argv[i]);
		loggingPrintf(stdout, "\"\n");
	}
	/* We run with environ as the Apple implementation replicates the environment variables */
	if (execve(cmd, argv, environ) == -1) {
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
	char *delimiter = ":";    /* delimiter for directories in dirs argument */
	char **possible = NULL;
	int pathCount = 0, i;
	char *command = NULL;
	char *cmd;

	/* Get an array of all the possible places the command could be */
	char *abslPath = strtok(dirs, delimiter);
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
		asprintf(&cmd, "%s/%s", abslPath, name);
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
			if (err) { *err = SUCCESFUL_OPERATION; }
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
requestCommand(bool verbose, const char *name, const char *currentSdk, const char *currentToolchain,
               int argc,
               char **argv, int *err) {
	char *searchString;
	int error; /* Check if some error occurred in another function */

	/* Here we get our developerPath, if this is NULL
	 * we return so we stop executing the code and
	 * we also assign the proper error code
	*/
	char *developerPath = getDeveloperPath(&error);
	if (error != SUCCESFUL_OPERATION) {
		if (err) { *err = error; }
		return;
	}

	/* Now we populate our "searchString" with the developer path, currentSdk and currentToolchain
	 * /usr/bin paths, this are later separated in the searchCommand function. Since this is a memory
	 * allocation we check if asprintf returned -1 if so we free the memory and return an error
	 */
	int success = asprintf(&searchString, "%s/usr/bin:%s/usr/bin:%s/usr/bin:%s", developerPath, currentSdk,
	                       currentToolchain, getenv("PATH"));
	if (success == -1) {
		if (err) { *err = ERROR_ALLOCATING_MEMORY; }
		return;
	}

	/* Search each path entry in searchString until we find our program. */

	char *cmd = searchCommand(verbose, name, searchString, &error);
	if (cmd == NULL && error != SUCCESFUL_OPERATION) {
		if (err) { *err = PROGRAM_NOT_FOUND; }
		return;
	}
	callCommand(verbose, cmd, currentSdk, currentToolchain, argc, argv, &error);
	if (error != SUCCESFUL_OPERATION) {
		if (err) { *err = error; }
		return;
	}


	/* We have searched everywhere, but we haven't found our program. State why. */
	if (verbose) {
		if (err) { *err = PROGRAM_NOT_FOUND; }
		fprintf(stderr, "libxcselect: error: can't stat \'%s\' (errno=%s)\n", name, strerror(errno));
	}
	if (err) { *err = PROGRAM_NOT_FOUND; }
}
