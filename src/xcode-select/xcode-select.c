/* xcode-select - clone of apple's xcode-select utility
 *
 * Copyright (c) 2013, Brian McKenzie <mckenzba@gmail.com>
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
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdbool.h>
#include "developer_path.h"


#define TOOL_VERSION "1.0.0"
#define SDK_CFG ".xcdev.dat"

extern int errno;

/**
 * @func usage -- Print helpful information about this tool.
 * @arg prog - name of this tool
 */
static void usage(void)
{

	fprintf(stderr,
			"Usage: xcode-select -print-path\n"
			"   or: xcode-select -switch <sdk_folder_path>\n"
			"   or: xcode-select -version\n"
			"Arguments:\n"
			"   -print-path                     Prints the path of the current SDK folder\n"
			"   -switch <xcode_folder_path>     Sets the path for the current SDK folder\n"
			"   -version                        Prints xcode-select version information\n\n");

	exit(1);
}

/**
 * @func usage -- Print the tool version.
 */
static void version(void)
{
	fprintf(stdout, "xcode-select version %s\n", TOOL_VERSION);

	exit(0);
}

/**
 * @func validate_directory_path -- validate if requested directory path exists and if it contains a valid required files
 * @arg dir - directory to validate
 * @return: 0 on success, 1 on failure
 */
static int validate_directory_path(const char *dir)
{
	struct stat fstat;
	int retval = 1;

	if (stat(dir, &fstat) != 0)
		fprintf(stderr, "xcode-select: error: unable to validate directory \'%s\' (errno=%s)\n", dir, strerror(errno));
	else {
		if (S_ISDIR(fstat.st_mode) == 0)
			fprintf(stderr, "xcode-select: error: \'%s\' is not a directory, please try a different path\n", dir);
		else
			retval = 0;
	}

	return retval;
}

/**
 * @func set_developer_path -- set the current developer path
 * @arg path - path to set
 * @return: 0 on success, -1 on failure
 */
static int set_developer_path(const char *path)
{
	FILE *fp = NULL;
	char *pathtocfg = NULL;
	char *cfg_path = NULL;

	int success; 

	/* This tool normally, does a thing where it tries to set it on a config file at the home directory
	* However, this isn't the way Apple does it, so what are we going to do?
	* to keep compat with this legacy code, what we are gonna do is the following:
	* - Check if we can create a file in /var/db/xcode_select_link
	* - If we fail, we fallback to this legacy code but notify the user, that they might want to elevate privileges
	* - If everything was successful lets just return 0 so we don't execute the legacy code.
	*/ 

	success = symlink(path, "/var/db/xcode_select_link");
	if (success != 0) {
		if (errno == EEXIST) {
			int did_unlink;
			did_unlink = unlink("/var/db/xcode_select_link");
			if (did_unlink != 0){
				fprintf(stderr,"xcode-select: warning: The following error occurred when trying to symlink: %s, this might be because you are not root! Will fallback to default code. In case you wish to handle this operation re-run this program as root!.\n", strerror(errno));
			}
			symlink(path, "/var/db/xcode_select_link");
			return 0;
		}
		fprintf(stderr, "xcode-select: warning: The following error occurred when trying to symlink: %s, this might be because you are not root! Will fallback to default code. In case you wish to handle this operation re-run this program as root!.\n", strerror(errno));
	}

	if ((pathtocfg = getenv("HOME")) == NULL) {
		fprintf(stderr, "xcode-select: error: failed to read HOME variable.\n");
		return -1;
	}

    cfg_path = (char *)malloc((strlen(pathtocfg) + sizeof(SDK_CFG)));

    strcat(pathtocfg, "/");
	strcat(cfg_path, strcat(pathtocfg, SDK_CFG));

	if ((fp = fopen(cfg_path, "w+")) != NULL) {
		fwrite(path, 1, strlen(path), fp);
		fclose(fp);
	} else {
		fprintf(stderr, "xcode-select: error: unable to open configuration file. (errno=%s)\n", strerror(errno));
		return -1;
	}

	free(cfg_path);

	return 0;
}

int main(int argc, char *argv[])
{
	int ch;
	char *path = NULL;

	if (argc < 2)
		usage();

	static int help_f, version_f, switch_f, printpath_f;
	help_f = version_f = switch_f = printpath_f = 0;

	static struct option options[] = {
		{ "help", no_argument, 0, 'h' },
		{ "version", no_argument, 0, 'v' },
		{ "switch", required_argument, 0, 's' },
		{ "print-path", no_argument, 0, 'p' },
		{ NULL, 0, 0, 0 }
	};

	while ((ch = getopt_long_only(argc, argv, "hvs:p", options, NULL)) != (-1)) {
		switch (ch) {
			case 'h':
				help_f = 1;
				break;
			case 'v':
				version_f = 1;
				break;
			case 's':
				switch_f = 1;
				path = optarg;
				break;
			case 'p':
				printpath_f = 1;
				break;
			case '?':
			default:
				help_f = 1;
		}
	}

	if (help_f == 1)
		usage();

	if (version_f == 1)
		version();

	if (switch_f == 1) {
		if (validate_directory_path(path) == 0)
			return set_developer_path(path);
		else
			return -1;
	}

	if (printpath_f == 1) {
		path = get_developer_path();
		fprintf(stdout, "%s\n", path);
	}

	return 0;
}
