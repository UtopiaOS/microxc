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
    int verbose, find_only;
    int error;
    default_config *our_config;

    char* developer_path;

    developer_path = get_developer_path(&error);

    if (error != SUCCESFUL_OPERATION && err) {
        *err = error;
        return;
    }

    va_list args;
    va_start(args, err);

    sdk = va_arg(args, char*);

    /* We have an SDK, now lets call our "get_sdk_path
     * function, if it is invalid this will return an error
    */
    if (sdk) {
        sdk = get_sdk_path(developer_path, sdk, &error);
        if (error != SUCCESFUL_OPERATION){
            if (err) {*err = error;}
        }
    } else {
        our_config = get_default_info(&error);
        if(error != SUCCESFUL_OPERATION && err) {
            *err = error;
            return;
        }
        sdk = get_sdk_path(developer_path, our_config->sdk, &error);
        if (error != SUCCESFUL_OPERATION && err) {
            *err = error;
            return;
        }
    }

    toolchain = va_arg(args, char*);
    if (!toolchain) {
        our_config = get_default_info(&error);
        if(error != SUCCESFUL_OPERATION && err) {
            *err = error;
            return;
        }
        toolchain = get_toolchain_path(developer_path, our_config->toolchain, &error);

        if (error != SUCCESFUL_OPERATION && err) {
            *err = error;
            return;
        }
    }

    verbose = va_arg(args, int);
    if (verbose != 1){
        verbose = 0;
    }

    find_only = va_arg(args, int);
    if (find_only != 1){
        find_only = 0;
    }

    va_end(args);

    request_command((bool)verbose, (bool)find_only, cmd, sdk, toolchain, argc, argv, &error);

}

/**
 * @func call_command -- Execute new process to replace this one.
 * @arg cmd - program's absolute path
 * @arg argc - number of arguments to be passed to new process
 * @arg argv - arguments to be passed to new process
 * @return: -1 on error, otherwise no return
 */
void
call_command(bool verbose, const char *cmd, const char *current_sdk, const char *current_toolchain, int argc,
             char *argv[], int *err) {
    int i;
    char *envp[7] = {NULL};
    int error;

    char *developer_path = get_developer_path(&error);

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
    envp[0] = (char *) malloc(PATH_MAX - 1);
    envp[1] = (char *) malloc(PATH_MAX - 1);
    envp[2] = (char *) malloc(PATH_MAX - 1);
    envp[3] = (char *) malloc(PATH_MAX - 1);

    sprintf(envp[0], "SDKROOT=%s", current_sdk);
    sprintf(envp[1], "PATH=%s/usr/bin:%s/usr/bin:%s", developer_path,
            current_toolchain, getenv("PATH"));
    sprintf(envp[2], "LD_LIBRARY_PATH=%s/usr/src", current_toolchain);
    sprintf(envp[3], "HOME=%s", getenv("HOME"));

    if (verbose) {
        logging_printf(stdout, "libxcselect: info: invoking command:\n\t\"%s", cmd);
        for (i = 1; i < argc; i++)
            logging_printf(stdout, " %s", argv[i]);
        logging_printf(stdout, "\"\n");
    }


    printf("%s", cmd);
    if (execve(cmd, argv, envp) == -1) {
        if (err) { *err = RUNNING_COMMAND_ERROR; }
        return;
    }

    if (err) { *err = SUCCESFUL_OPERATION; }
}

/**
 * @func search_command -- Search a set of directories for a given command
 * @arg name - program's name
 * @arg dirs - set of directories to search, separated by colons
 * @return: the program's absolute path on success, NULL on failure
 */
char *search_command(bool verbose, const char *name, char *dirs) {
    char *cmd = NULL;    /* command's absolute path */
    char *absl_path = NULL;        /* path entry to search */
    char delimiter[2] = ":";    /* delimiter for directories in dirs argument */

    /* Allocate space for the program's absolute path */
    cmd = (char *) malloc(PATH_MAX - 1);

    /* Search each path entry in dirs until we find our program. */
    absl_path = strtok(dirs, delimiter);
    while (absl_path != NULL) {
        if (verbose) {
            verbose_printf(stdout, "libxcselect: info: checking directory \'%s\' for command \'%s\'...\n", absl_path,
                           name);
        }

        /* Construct our program's absolute path. */
        sprintf(cmd, "%s/%s", absl_path, name);

        /* Does it exist? Is it an executable? */
        if (access(cmd, (F_OK | X_OK)) != (-1)) {
            if (verbose) {
                verbose_printf(stdout, "libxcselect: info: found command's absolute path: \'%s\'\n", cmd);
            }
            break;
        }

        /* If not, move onto the next entry.. */
        absl_path = strtok(NULL, delimiter);
    }

    return cmd;
}

/**
 * @func request_command - Request a program.
 * @arg name -- name of program
 * @arg argv -- arguments to be passed if program found
 * @return: -1 on failed search, 0 on successful search, no return on execute
 */
void
request_command(bool verbose, bool find_only, const char *name, const char *current_sdk, const char *current_toolchain,
                int argc,
                char *argv[], int *err) {
    char *cmd = NULL;    /* used to hold our command's absolute path */
    char search_string[PATH_MAX * 1024];    /* our search string */
    int error; /* Check if some error occurred in another function */

    char *developer_path = get_developer_path(&error);
    if (error != SUCCESFUL_OPERATION) {
        if (err) { *err = error; }
        return;
    }

    sprintf(search_string, "%s/usr/bin:", developer_path);

    /* Search each path entry in search_string until we find our program. */
    if ((cmd = search_command(verbose, name, search_string)) != NULL) {
        if (find_only) {
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
            call_command(verbose, cmd, current_sdk, current_toolchain, argc, argv, &error);
            if (error != SUCCESFUL_OPERATION) {
                printf("%d", error);
                if (err) { *err = error; }
                return;
            }
            /* NO REACH */
            if (verbose) {
                fprintf(stderr, "libxcselect: error: can't exec \'%s\' (errno=%s)\n", cmd, strerror(errno));
                if (err) { *err = EXECUTION_ERROR; }
                return;
            }
        }
    }

    /* We have searched everywhere, but we haven't found our program. State why. */
    if (verbose) {
        if (err) { *err = PROGRAM_NOT_FOUND; }
        fprintf(stderr, "libxcselect: error: can't stat \'%s\' (errno=%s)\n", name, strerror(errno));
        return;
    }

    if (err) { *err = PROGRAM_NOT_FOUND; }
}
