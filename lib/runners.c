//
// Created by Diego Magdaleno on 11/14/20.
//

#include "runners.h"
#include "getters.h"
#include "stripext.h"
#include "developer_path.h"
#include "validators.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>

/**
 * @func call_command -- Execute new process to replace this one.
 * @arg cmd - program's absolute path
 * @arg argc - number of arguments to be passed to new process
 * @arg argv - arguments to be passed to new process
 * @return: -1 on error, otherwise no return
 */
int
call_command(const char *cmd, const char *current_sdk, const char *current_toolchain, int argc, char *argv[])
{
    int i;
    char *envp[7] = { NULL };
    char *deployment_target = NULL;
    char *developer_path = get_developer_path();

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
    envp[0] = (char *)malloc(PATH_MAX - 1);
    envp[1] = (char *)malloc(PATH_MAX - 1);
    envp[2] = (char *)malloc(PATH_MAX - 1);
    envp[3] = (char *)malloc(PATH_MAX - 1);
    envp[4] = (char *)malloc(255);

    sprintf(envp[0], "SDKROOT=%s", get_sdk_path(developer_path, current_sdk));
    sprintf(envp[1], "PATH=%s/usr/bin:%s/usr/bin:%s", developer_path, get_toolchain_path(developer_path, current_toolchain), getenv("PATH"));
    sprintf(envp[2], "LD_LIBRARY_PATH=%s/usr/lib", get_toolchain_path(developer_path, current_toolchain));
    sprintf(envp[3], "HOME=%s", getenv("HOME"));



    if ((deployment_target = getenv("IOS_DEPLOYMENT_TARGET")) != NULL)
        sprintf(envp[4], "IOS_DEPLOYMENT_TARGET=%s", deployment_target);
    else if ((deployment_target = getenv("MACOSX_DEPLOYMENT_TARGET")) != NULL)
        sprintf(envp[4], "MACOSX_DEPLOYMENT_TARGET=%s", deployment_target);
    else {
        /* Use the deployment target info that is provided by the SDK. */
        if ((deployment_target = strdup(get_sdk_info(get_sdk_path(developer_path, current_sdk), current_sdk).deployment_target)) != NULL) {
            /* !THIS BLOCK REQUIRES MORE DEVELOPMENT TO BE REWRITTEN
            if (macosx_deployment_target_set == 1)
                sprintf(envp[5], "MACOSX_DEPLOYMENT_TARGET=%s", deployment_target);
            else if (ios_deployment_target_set == 1)
                sprintf(envp[5], "IOS_DEPLOYMENT_TARGET=%s", deployment_target);
            */
        } else {
            fprintf(stderr, "xcrun: error: failed to retrieve deployment target information for %s.sdk.\n", current_sdk);
            exit(1);
        }
    }

    /*if (logging_mode == 1) {
        logging_printf(stdout, "xcrun: info: invoking command:\n\t\"%s", cmd);
        for (i = 1; i < argc; i++)
            logging_printf(stdout, " %s", argv[i]);
        logging_printf(stdout, "\"\n");
    } */

    return execve(cmd, argv, envp);
}

/**
 * @func search_command -- Search a set of directories for a given command
 * @arg name - program's name
 * @arg dirs - set of directories to search, seperated by colons
 * @return: the program's absolute path on success, NULL on failure
 */
char *search_command(const char *name, char *dirs)
{
    char *cmd = NULL;	/* command's absolute path */
    char *absl_path = NULL;		/* path entry to search */
    char delimiter[2] = ":";	/* delimiter for directories in dirs argument */

    /* Allocate space for the program's absolute path */
    cmd = (char *)malloc(PATH_MAX - 1);

    /* Search each path entry in dirs until we find our program. */
    absl_path = strtok(dirs, delimiter);
    while (absl_path != NULL) {
        /*if (verbose_mode == 1) {
            verbose_printf(stdout, "xcrun: info: checking directory \'%s\' for command \'%s\'...\n", absl_path, name);
        }*/

        /* Construct our program's absolute path. */
        sprintf(cmd, "%s/%s", absl_path, name);

        /* Does it exist? Is it an executable? */
        if (access(cmd, (F_OK | X_OK)) != (-1)) {
            /*if (verbose_mode == 1) {
                verbose_printf(stdout, "xcrun: info: found command's absolute path: \'%s\'\n", cmd);
            }
             */
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
int request_command(const char *name, const char* current_sdk, const char* current_toolchain, int argc, char *argv[])
{
    char *cmd = NULL;	/* used to hold our command's absolute path */
    char *sdk_env = NULL;	/* used for passing SDKROOT in call_command */
    char *toolch_name = NULL;	/* toolchain name to be used with sdk */
    char *toolchain_env = NULL;	/* used for passing PATH in call_command */
    char search_string[PATH_MAX * 1024];	/* our search string */
    char *developer_path = get_developer_path();

    /*
     * If xcrun was called in a multicall state, we still want to specify current_sdk for SDKROOT and
     * current_toolchain for PATH.
     */

    if (current_sdk == NULL) {
        current_sdk = (char *)malloc(255);
        if ((sdk_env = getenv("SDKROOT")) != NULL)  {
            stripext((char *)current_sdk, basename(sdk_env));
        } else {
            current_sdk = strdup(get_default_info().sdk);
        }
    }

    if (current_toolchain == NULL) {
        current_toolchain = (char *)malloc(255);
        if ((toolchain_env = getenv("TOOLCHAINS")) != NULL) {
            stripext((char *)current_toolchain, basename(toolchain_env));
        } else {
            current_toolchain = strdup(get_default_info().toolchain);
        }
    }

    /* No matter the circumstance, search the developer dir. */
    sprintf(search_string, "%s/usr/bin:", developer_path);

    /* !THIS WHOLE BLOCK REQUIRES MORE INVESTIGATION TO BE REWRITTEN */

    /* If we implicitly specified an sdk, search the sdk and it's associated toolchain.
    toolch_name = strdup(get_sdk_info(get_sdk_path(developer_path,current_sdk), current_sdk).toolchain);
    sprintf((search_string + strlen(search_string)), "%s/usr/bin:%s/usr/bin", get_sdk_path(developer_path,current_sdk), get_toolchain_path(developer_path,toolch_name));
    goto do_search;

    * If we implicitly specified a toolchain, only search the toolchain.
        sprintf((search_string + strlen(search_string)), "%s/usr/bin", get_toolchain_path(current_toolchain));
        goto do_search;
    *  If we explicitly specified an SDK, append it to the search string.
    if (alternate_sdk_path != NULL) {
        sprintf((search_string + strlen(search_string)), "%s/usr/bin:", alternate_sdk_path);
        * We also want to append an associated toolchain if this is really an SDK folder.
        if (test_sdk_authenticity(alternate_sdk_path) == 1) {
            toolch_name = strdup(get_sdk_info(alternate_sdk_path).toolchain);
            sprintf((search_string + strlen(search_string)), "%s/usr/bin", get_toolchain_path(developer_path, toolch_name));
            * We now have a toolchain, so skip to search.
            goto do_search;
        }
    }

    * If we explicitly specified a toolchain, append it to the search string.
    if (alternate_toolchain_path != NULL)
        sprintf((search_string + strlen(search_string)), "%s/usr/bin", alternate_toolchain_path);

    * By default, we search our developer dir, our default sdk, and our default toolchain only.
    if (explicit_sdk_mode == 0 && explicit_toolchain_mode == 0 && alternate_toolchain_path == NULL && alternate_sdk_path == NULL)
        sprintf((search_string + strlen(search_string)), "%s/usr/bin:%s/usr/bin", get_sdk_path(current_sdk), get_toolchain_path(current_toolchain));
    !THIS WHOLE BLOCK REQUIRES MORE INVESTIGATION TO BE REWRITTEN */

    /* Search each path entry in search_string until we find our program. */
    do_search:
    if ((cmd = search_command(name, search_string)) != NULL) {
        /*! THIS BLOCK REQUIRES MORE RESEARCH TO BE REWRITTEN */
        // temporal solution so it compiles
        int finding_mode = 0;
        if (finding_mode == 1) {
            if (access(cmd, (F_OK | X_OK)) != (-1)) {
                fprintf(stdout, "%s\n", cmd);
                free(cmd);
                return 0;
            } else
                return -1;
        } else {
            call_command(cmd, current_sdk, current_toolchain, argc, argv);
            /* NOREACH */
            fprintf(stderr, "xcrun: error: can't exec \'%s\' (errno=%s)\n", cmd, strerror(errno));
            return -1;
        }
    }

    /* We have searched everywhere, but we haven't found our program. State why. */
    fprintf(stderr, "xcrun: error: can't stat \'%s\' (errno=%s)\n", name, strerror(errno));

    return -1;
}
