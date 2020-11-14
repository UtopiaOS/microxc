/* xcrun - clone of apple's xcode xcrun utility
 *
 * Copyright (c) 2013-2014, Brian McKenzie <mckenzba@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the organization nor the names of its contributors may
 *     be used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <dirent.h>
#include <AppKit/AppKit.h>

#include "developer_path.h"
#include "verbose_printf.h"
#include "typedefs.h"
#include "config_handler.h"
#include "parsers.h"
#include "stripext.h"
#include "validators.h"
#include "logging_printf.h"

// Obj-c
#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>
#include <TargetConditionals.h>

/* General stuff */
#define TOOL_VERSION "1.0.0"

/* Output mode flags */
static int logging_mode = 0;
static int verbose_mode = 0;
static int finding_mode = 0;

/* Behavior mode flags */
static int explicit_sdk_mode = 0;
static int explicit_toolchain_mode = 0;
static int ios_deployment_target_set = 0;
static int macosx_deployment_target_set = 0;

/* Runtime info */
static char *developer_dir = NULL;
static char *current_sdk = NULL;
static char *current_toolchain = NULL;

/* Alternate behavior flags */
static char *alternate_sdk_path = NULL;
static char *alternate_toolchain_path = NULL;

/* Ways that this tool may be called */
static const char *multicall_tool_names[4] = {
	"xcrun",
	"xcrun_log",
	"xcrun_verbose",
	"xcrun_nocache"
};

/* Our program's name as called by the user */
static char *progname;

/**
 * @func usage -- Print helpful information about this program.
 */
static void usage(void)
{
	fprintf(stderr,
		"Usage: %s [options] <tool name> ... arguments ...\n"
		"\n"
		"Find and execute the named command line tool from the active developer directory.\n"
		"\n"
		"The active developer directory can be set using `xcode-select`, or via the\n"
		"DEVELOPER_DIR environment variable.\n"
		"\n"
		"Options:\n"
		"  -h, --help                   show this help message and exit\n"
		"  --version                    show the xcrun version\n"
		"  -v, --verbose                show verbose logging output\n"
		"  --sdk <sdk name>             find the tool for the given SDK name\n"
		"  --toolchain <name>           find the tool for the given toolchain\n"
		"  -l, --log                    show commands to be executed (with --run)\n"
		"  -f, --find                   only find and print the tool path\n"
		"  -r, --run                    find and execute the tool (the default behavior)\n"
#if 0
		"  -n, --no-cache               do not use the lookup cache (not implemented yet - does nothing)\n"
		"  -k, --kill-cache             invalidate all existing cache entries (not implemented yet - does nothing)\n"
#endif
		"  --show-sdk-path              show selected SDK install path\n"
		"  --show-sdk-version           show selected SDK version\n"
		"  --show-sdk-toolchain-path    show selected SDK toolchain path\n"
		"  --show-sdk-toolchain-version show selected SDK toolchain version\n\n"
		, progname);

	exit(0);
}

/**
 * @func version -- print out version info for this tool
 */
static void version(void)
{
	fprintf(stdout, "xcrun version %s\n", TOOL_VERSION);
	exit(0);
}

static toolchain_config get_toolchain_info(const char *path)
{
	toolchain_config config;
	NSURL *info_path;

	// lets handle the conversion from a CString to an NSString
	NSString *initial_path = [NSString stringWithCString:path encoding:NSUTF8StringEncoding];
	NSString *final_path = [NSString stringWithFormat:@"%@/SDKSettings.plist", initial_path];

	free(initial_path);

	info_path = [NSURL fileURLWithPath:final_path];

	NSString *target_sdk = [NSString stringWithCString:current_sdk encoding:NSUTF8StringEncoding];

	NSDictionary *dictSession = plist_parse(info_path, target_sdk);

	const char *version_char = [[dictSession valueForKey:@"version"] UTF8String];
	const char *name_char = [[dictSession valueForKey:@"name"] UTF8String];

	config.version = version_char;
	config.name = name_char;

	return config;

}

/**
 * @func get_sdk_info -- fetch config info from a toolchain's info.ini
 * @arg path - path to sdk's info.ini
 * @return: struct containing sdk config info
 */
static sdk_config get_sdk_info(const char *path)
{
	sdk_config config;
	NSURL *info_path;

	NSString *initial_path = [NSString stringWithCString:path encoding:NSUTF8StringEncoding];
	NSString *final_path = [NSString stringWithFormat:@"%@/SDKSettings.plist", initial_path];

	free(initial_path);

	info_path = [NSURL fileURLWithPath:final_path];

	NSString *target_sdk = [NSString stringWithCString:current_sdk encoding:NSUTF8StringEncoding];

	NSDictionary *dict_session = plist_parse(info_path, target_sdk);

	config.version = [[dict_session valueForKey:@"version"] UTF8String];
	config.name = [[dict_session valueForKey:@"name"] UTF8String];
	config.default_arch = [[dict_session valueForKey:@"arch"] UTF8String];
	config.deployment_target = [[dict_session valueForKey:@"deployment_target"] UTF8String];
	config.toolchain = [[dict_session valueForKey:@"toolchain"] UTF8String];

	return config;

}

/**
 * @func get_toolchain_path -- Return the specified toolchain path
 * @arg name - name of the toolchain
 * @return: absolute path of toolchain on success, exit on failure
 */
static char *get_toolchain_path(const char *name)
{
	char *path = NULL;
	char *devpath = NULL;

	devpath = developer_dir;
	path = (char *)malloc(PATH_MAX - 1);

	if (devpath != NULL) {
		sprintf(path, "%s/Toolchains/%s.xctoolchain", devpath, name);
		if (validate_directory_path(path) != (-1))
			return path;
		else {
			fprintf(stderr, "xcrun: error: \'%s\' is not a valid toolchain path.\n", path);
			free(path);
			exit(1);
		}
	} else {
		fprintf(stderr, "xcrun: error: failed to retrieve developer path, do you have it set?\n");
		free(path);
		exit(1);
	}
}

/**
 * @func get_sdk_path -- Return the specified sdk path
 * @arg name - name of the sdk
 * @return: absolute path of sdk on success, exit on failure
 */
static char *get_sdk_path(const char *name)
{
	char *path = NULL;
	char *devpath = NULL;

	devpath = developer_dir;
	path = (char *)malloc(PATH_MAX - 1);

	if (devpath != NULL) {
		sprintf(path, "%s/Platforms/%s.Platform/Developer/SDKs/%s.sdk", devpath, name, name);
		if (validate_directory_path(path) != (-1))
			return path;
		else {
			fprintf(stderr, "xcrun: error: \'%s\' is not a valid sdk path.\n", path);
			free(path);
			exit(1);
		}
	} else {
		fprintf(stderr, "xcrun: error: failed to retrieve developer path, do you have it set?\n");
		free(path);
		exit(1);
	}
}

/**
 * @func call_command -- Execute new process to replace this one.
 * @arg cmd - program's absolute path
 * @arg argc - number of arguments to be passed to new process
 * @arg argv - arguments to be passed to new process
 * @return: -1 on error, otherwise no return
 */
static int call_command(const char *cmd, int argc, char *argv[])
{
	int i;
	char *envp[7] = { NULL };
	char *deployment_target = NULL;

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

	sprintf(envp[0], "SDKROOT=%s", get_sdk_path(current_sdk));
	sprintf(envp[1], "PATH=%s/usr/bin:%s/usr/bin:%s", developer_dir, get_toolchain_path(current_toolchain), getenv("PATH"));
	sprintf(envp[2], "LD_LIBRARY_PATH=%s/usr/lib", get_toolchain_path(current_toolchain));
	sprintf(envp[3], "HOME=%s", getenv("HOME"));



	if ((deployment_target = getenv("IOS_DEPLOYMENT_TARGET")) != NULL)
		sprintf(envp[4], "IOS_DEPLOYMENT_TARGET=%s", deployment_target);
	else if ((deployment_target = getenv("MACOSX_DEPLOYMENT_TARGET")) != NULL)
		sprintf(envp[4], "MACOSX_DEPLOYMENT_TARGET=%s", deployment_target);
	else {
		/* Use the deployment target info that is provided by the SDK. */
		if ((deployment_target = strdup(get_sdk_info(get_sdk_path(current_sdk)).deployment_target)) != NULL) {
			if (macosx_deployment_target_set == 1)
				sprintf(envp[5], "MACOSX_DEPLOYMENT_TARGET=%s", deployment_target);
			else if (ios_deployment_target_set == 1)
				sprintf(envp[5], "IOS_DEPLOYMENT_TARGET=%s", deployment_target);
		} else {
			fprintf(stderr, "xcrun: error: failed to retrieve deployment target information for %s.sdk.\n", current_sdk);
			exit(1);
		}
	}

	if (logging_mode == 1) {
		logging_printf(stdout, "xcrun: info: invoking command:\n\t\"%s", cmd);
		for (i = 1; i < argc; i++)
			logging_printf(stdout, " %s", argv[i]);
		logging_printf(stdout, "\"\n");
	}

	return execve(cmd, argv, envp);
}

/**
 * @func search_command -- Search a set of directories for a given command
 * @arg name - program's name
 * @arg dirs - set of directories to search, seperated by colons
 * @return: the program's absolute path on success, NULL on failure
 */
static char *search_command(const char *name, char *dirs)
{
	char *cmd = NULL;	/* command's absolute path */
	char *absl_path = NULL;		/* path entry to search */
	char delimiter[2] = ":";	/* delimiter for directories in dirs argument */

	/* Allocate space for the program's absolute path */
	cmd = (char *)malloc(PATH_MAX - 1);

	/* Search each path entry in dirs until we find our program. */
	absl_path = strtok(dirs, delimiter);
	while (absl_path != NULL) {
	    if (verbose_mode == 1) {
            verbose_printf(stdout, "xcrun: info: checking directory \'%s\' for command \'%s\'...\n", absl_path, name);
	    }

		/* Construct our program's absolute path. */
		sprintf(cmd, "%s/%s", absl_path, name);

		/* Does it exist? Is it an executable? */
		if (access(cmd, (F_OK | X_OK)) != (-1)) {
		    if (verbose_mode == 1) {
                verbose_printf(stdout, "xcrun: info: found command's absolute path: \'%s\'\n", cmd);
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
static int request_command(const char *name, int argc, char *argv[])
{
	char *cmd = NULL;	/* used to hold our command's absolute path */
	char *sdk_env = NULL;	/* used for passing SDKROOT in call_command */
	char *toolch_name = NULL;	/* toolchain name to be used with sdk */
	char *toolchain_env = NULL;	/* used for passing PATH in call_command */
	char search_string[PATH_MAX * 1024];	/* our search string */

	/*
	 * If xcrun was called in a multicall state, we still want to specify current_sdk for SDKROOT and
	 * current_toolchain for PATH.
	 */

	if (current_sdk == NULL) {
		current_sdk = (char *)malloc(255);
		if ((sdk_env = getenv("SDKROOT")) != NULL)  {
			stripext(current_sdk, basename(sdk_env));
		} else {
			current_sdk = strdup(get_default_info().sdk);
		}	
	}

	if (current_toolchain == NULL) {
		current_toolchain = (char *)malloc(255);
		if ((toolchain_env = getenv("TOOLCHAINS")) != NULL) {
            stripext(current_toolchain, basename(toolchain_env));
		} else {
		    current_toolchain = strdup(get_default_info().toolchain);
        }
	}

	/* No matter the circumstance, search the developer dir. */
	sprintf(search_string, "%s/usr/bin:", developer_dir);

	/* If we implicitly specified an sdk, search the sdk and it's associated toolchain. */
	if (explicit_sdk_mode == 1) {
		toolch_name = strdup(get_sdk_info(get_sdk_path(current_sdk)).toolchain);
		sprintf((search_string + strlen(search_string)), "%s/usr/bin:%s/usr/bin", get_sdk_path(current_sdk), get_toolchain_path(toolch_name));
		goto do_search;
	}

	/* If we implicitly specified a toolchain, only search the toolchain. */
	if (explicit_toolchain_mode == 1) {
		sprintf((search_string + strlen(search_string)), "%s/usr/bin", get_toolchain_path(current_toolchain));
		goto do_search;
	}

	/* If we explicitly specified an SDK, append it to the search string. */
	if (alternate_sdk_path != NULL) {
		sprintf((search_string + strlen(search_string)), "%s/usr/bin:", alternate_sdk_path);
		/* We also want to append an associated toolchain if this is really an SDK folder. */
		if (test_sdk_authenticity(alternate_sdk_path) == 1) {
			toolch_name = strdup(get_sdk_info(alternate_sdk_path).toolchain);
			sprintf((search_string + strlen(search_string)), "%s/usr/bin", get_toolchain_path(toolch_name));
			/* We now have a toolchain, so skip to search. */
			goto do_search;
		}
	}

	/* If we explicitly specified a toolchain, append it to the search string. */
	if (alternate_toolchain_path != NULL)
		sprintf((search_string + strlen(search_string)), "%s/usr/bin", alternate_toolchain_path);

	/* By default, we search our developer dir, our default sdk, and our default toolchain only. */
	if (explicit_sdk_mode == 0 && explicit_toolchain_mode == 0 && alternate_toolchain_path == NULL && alternate_sdk_path == NULL)
		sprintf((search_string + strlen(search_string)), "%s/usr/bin:%s/usr/bin", get_sdk_path(current_sdk), get_toolchain_path(current_toolchain));

	/* Search each path entry in search_string until we find our program. */
do_search:
	if ((cmd = search_command(name, search_string)) != NULL) {
		if (finding_mode == 1) {
			if (access(cmd, (F_OK | X_OK)) != (-1)) {
				fprintf(stdout, "%s\n", cmd);
				free(cmd);
				return 0;
			} else
				return -1;
		} else {
			call_command(cmd, argc, argv);
			/* NOREACH */
			fprintf(stderr, "xcrun: error: can't exec \'%s\' (errno=%s)\n", cmd, strerror(errno));
			return -1;
		}
	}

	/* We have searched everywhere, but we haven't found our program. State why. */
	fprintf(stderr, "xcrun: error: can't stat \'%s\' (errno=%s)\n", name, strerror(errno));

	return -1;
}

/**
 * @func xcrun_main -- xcrun's main routine
 * @arg argc - number of arguments passed by user
 * @arg argv - array of arguments passed by user
 * @return: 0 (or none) on success, 1 on failure
 */
static int xcrun_main(int argc, char *argv[])
{
	int ch;
	int retval = 1;
	int optindex = 0;
	int argc_offset = 0;
	char *sdk = NULL;
	char *toolchain = NULL;
	char *tool_called = NULL;

	char *sdk_env = NULL;
	char *toolchain_env = NULL;

	static int help_f, verbose_f, log_f, find_f, run_f, nocache_f, killcache_f, version_f, sdk_f, toolchain_f, ssdkp_f, ssdkv_f, ssdkpp_f, ssdkpv_f;
	help_f = verbose_f = log_f = find_f = run_f = nocache_f = killcache_f = version_f = sdk_f = toolchain_f = ssdkp_f = ssdkv_f = ssdkpp_f = ssdkpv_f = 0;

	/* Supported options */
	static struct option options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "version", no_argument, &version_f, 1 },
		{ "verbose", no_argument, 0, 'v' },
		{ "sdk", required_argument, &sdk_f, 1 },
		{ "toolchain", required_argument, &toolchain_f, 1 },
		{ "log", no_argument, 0, 'l' },
		{ "find", required_argument, 0, 'f' },
		{ "run", required_argument, 0, 'r' },
		{ "no-cache", no_argument, 0, 'n' },
		{ "kill-cache", no_argument, 0, 'k' },
		{ "show-sdk-path", no_argument, &ssdkp_f, 1 },
		{ "show-sdk-version", no_argument, &ssdkv_f, 1 },
		{ "show-sdk-toolchain-path", no_argument, &ssdkpp_f, 1 },
		{ "show-sdk-toolchain-version", no_argument, &ssdkpv_f, 1 },
		{ NULL, 0, 0, 0 }
	};

	/* Print help if nothing is specified */
	if (argc < 2)
		usage();

	/* Only parse arguments if they are given */
	if (*(*(argv + 1)) == '-') {
		if (strcmp(argv[1], "-") == 0 || strcmp(argv[1], "--") == 0)
			usage();
		while ((ch = getopt_long_only(argc, argv, "+hvlr:f:nk", options, &optindex)) != (-1)) {
			switch (ch) {
				case 'h':
					help_f = 1;
					break;
				case 'v':
					verbose_f = 1;
					break;
				case 'l':
					log_f = 1;
					break;
				case 'r':
					run_f = 1;
					tool_called = basename(optarg);
					++argc_offset;
					break;
				case 'f':
					find_f = 1;
					tool_called = basename(optarg);
					++argc_offset;
					break;
				case 'n':
					nocache_f = 1;
					break;
				case 'k':
					killcache_f = 1;
					break;
				case 0: /* long-only options */
					switch (optindex) {
						case 1: /* --version */
							break;
						case 3: /* --sdk */
							if (*optarg != '-') {
								++argc_offset;
								sdk = optarg;
								/* we support absolute paths and short names */
								if (*sdk == '/') {
									if (validate_directory_path(sdk) != (-1))
										alternate_sdk_path = sdk;
									else
										exit(1);
								} else {
									current_sdk = (char *)malloc(255);
									explicit_sdk_mode = 1;
									stripext(current_sdk, sdk);
								}
							} else {
								fprintf(stderr, "xcrun: error: sdk flag requires an argument.\n");
								exit(1);
							}
							break;
						case 4: /* --toolchain */
							if (*optarg != '-') {
								++argc_offset;
								toolchain = optarg;
								/* we support absolute paths and short names */
								if (*toolchain == '/') {
									if (validate_directory_path(toolchain) != (-1))
										alternate_toolchain_path = toolchain;
									else
										exit(1);
								} else {
									current_toolchain = (char *)malloc(255);
									explicit_toolchain_mode = 1;
									stripext(current_toolchain, toolchain);
								}
							} else {
								fprintf(stderr, "xcrun: error: toolchain flag requires an argument.\n");
								exit(1);
							}
							break;
						case 10: /* --show-sdk-path */
							break;
						case 11: /* --show-sdk-version */
							break;
						case 12: /* --snow-sdk-target-triple */
							break;
						case 13: /* --show-sdk-toolchain-path */
							break;
						case 14: /* --show-sdk-toolchain-version */
							break;
					}
					break;
				case '?':
				default:
					help_f = 1;
					break;
			}

			++argc_offset;

			/* We don't want to parse any more arguments after these are set. */
			if (ch == 'f' || ch == 'r')
				break;
		}
	} else { /* We are just executing a program. */
		tool_called = basename(argv[1]);
		++argc_offset;
	}

	/* The last non-option argument may be the command called. */
	if (optind < argc && ((run_f == 0 || find_f == 0) && tool_called == NULL)) {
		tool_called = basename(argv[optind++]);
		++argc_offset;
	}

	/* Don't continue if we are missing arguments. */
	if ((verbose_f == 1 || log_f == 1) && tool_called == NULL) {
		fprintf(stderr, "xcrun: error: specified arguments require -r or -f arguments.\n");
		exit(1);
	}

	/* Print help? */
	if (help_f == 1 || argc < 2)
		usage();

	/* Print version? */
	if (version_f == 1)
		version();

	/* If our SDK and/or Toolchain hasn't been specified, fall back to environment or defaults. */
	if (current_sdk == NULL) {
		current_sdk = (char *)malloc(255);
		if ((sdk_env = getenv("SDKROOT")) != NULL)
			stripext(current_sdk, basename(sdk_env));
		else
			current_sdk = strdup(get_default_info().sdk);
	}

	if (current_toolchain == NULL) {
		current_toolchain = (char *)malloc(255);
		if ((toolchain_env = getenv("TOOLCHAINS")) != NULL)
			stripext(current_toolchain, basename(toolchain_env));
		else
			current_toolchain = strdup(get_default_info().toolchain);
	}

	/* Show SDK path? */
	if (ssdkp_f == 1) {
		printf("%s\n", get_sdk_path(current_sdk));
		exit(0);
	}

	/* Show SDK version? */
	if (ssdkv_f == 1) {
		printf("%s SDK version %s\n", get_sdk_info(get_sdk_path(current_sdk)).name, get_sdk_info(get_sdk_path(current_sdk)).version);
		exit(0);
	}

	/* Show SDK toolchain path? */
	if (ssdkpp_f == 1) {
		printf("%s\n", get_toolchain_path(current_toolchain));
		exit(0);
	}

	/* Show SDK toolchain version? */
	if (ssdkpv_f == 1) {
		printf("%s SDK Toolchain version %s (%s)\n", get_sdk_info(get_sdk_path(current_sdk)).name, get_toolchain_info(get_toolchain_path(current_toolchain)).version, get_toolchain_info(get_toolchain_path(current_toolchain)).name);
		exit(0);
	}

	/* Clear the lookup cache? */
	if (killcache_f == 1)
		fprintf(stderr, "xcrun: warning: --kill-cache not supported.\n");

	/* Don't use the lookup cache? */
	if (nocache_f == 1)
		fprintf(stderr, "xcrun: warning: --no-cache not supported.\n");

	/* Turn on verbose mode? */
	if (verbose_f == 1)
		verbose_mode = 1;

	/* Turn on logging mode? */
	if (log_f == 1)
		logging_mode = 1;

	/* Before we continue, double check if we have a tool to call. */
	if (tool_called == NULL) {
		fprintf(stderr, "xcrun: error: no tool specified.\n");
		exit(1);
	}

	/* Search for program? */
	if (find_f == 1) {
		finding_mode = 1;
		if (request_command(tool_called, 0, NULL) != -1)
			retval = 0;
		else {
			fprintf(stderr, "xcrun: error: unable to locate command \'%s\' (errno=%s)\n", tool_called, strerror(errno));
			exit(1);
		}
	}

	/* Search and execute program. (default behavior) */
	if (find_f != 1) {
		if (request_command(tool_called, (argc - argc_offset),  (argv += ((argc - argc_offset) - (argc - argc_offset) + (argc_offset)))) != -1)
			retval = -1; /* NOREACH */
		else {
			fprintf(stderr, "xcrun: error: failed to execute command \'%s\'. aborting.\n", tool_called);
			exit(1);
		}
	}

	return retval;
}

/**
 * @func get_multicall_state -- Return a number that is associated to a given multicall state.
 * @arg cmd - command that binary is being called
 * @arg state - char array containing a set of possible "multicall states"
 * @arg state_size - number of elements in state array
 * @return: a number from 1 to state_size (first enrty to last entry found in state array), or -1 if one isn't found
 */
static int get_multicall_state(const char *cmd, const char *state[], int state_size)
{
	int i;

	for (i = 0; i < (state_size - 1); i++) {
		if (strcmp(cmd, state[i]) == 0)
			return (i + 1);
	}

	return -1;
}

int main(int argc, char *argv[])
{
	int retval = 1;
	int call_state;
	char *this_tool = NULL;

	/* Strip out any path name that may have been passed into argv[0] */
	this_tool = basename(argv[0]);
	progname = this_tool;

	/* Get our developer dir */
	developer_dir = get_developer_path();

	/* Check if we are being treated as a multi-call binary. */
	call_state = get_multicall_state(this_tool, multicall_tool_names, 4);

	/* Execute based on the state that we were called in. */
	switch (call_state) {
		case 1: /* xcrun */
			retval = xcrun_main(argc, argv);
			break;
		case 2: /* xcrun_log */
			retval = xcrun_main(argc, argv);
			break;
		case 3: /* xcrun_verbose */
			retval = xcrun_main(argc, argv);
			break;
		case 4: /* xcrun_nocache */
			retval = xcrun_main(argc, argv);
			break;
		case -1:
		default: /* called as tool name */
			/* Locate and execute the command */
			if (request_command(this_tool, argc, argv) != -1)
				retval = -1; /* NOREACH */
			else {
				fprintf(stderr, "xcrun: error: failed to execute command \'%s\'. aborting.\n", this_tool);
				exit(1);
			}
			break;
	}

	return retval;
}
