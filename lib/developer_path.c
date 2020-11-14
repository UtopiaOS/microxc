#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syslimits.h>
#include <errno.h>

#include "errors.h"
#include "developer_path.h"

static char*
var_dev_symlink (const char* path)
{
	struct stat buf;
	int result;
	int is_link;
	char* symlink_name;
	int did_readlink;
	
	// The file is not a symlink
	is_link = stat(path, &buf);
	if (S_ISLNK(buf.st_mode) != 0) {
		return NULL;
	};

	result = (lstat(path, &buf)  == 0); {
		symlink_name = malloc(buf.st_size + 1);
		did_readlink = readlink(path, symlink_name, buf.st_size + 1);
		if (did_readlink != -1) {
		symlink_name[buf.st_size] = '\0';
		if (stat(symlink_name, &buf) == 0 ) {
			// return what it points to
			return symlink_name;
			}
		} else {
			return NULL;
		}
	}
	return NULL;
}

/**
 * @func get_developer_path -- retrieve current developer path
 * @return: string of current path on success, NULL string on failure
 */
char *
get_developer_path(int *err)
{
	microxcode_error_state_t error_state;
    char devpath[PATH_MAX - 1];
	char *value = NULL;

	if ((value = getenv("DEVELOPER_DIR")) != NULL){
		return value;
	}

	if ((value = var_dev_symlink("/var/db/xcode_select_link")) != NULL){
		return value;
	}

	if (err) = { *err = UNABLE_TO_GET_DEV_PATH }
	return value;
}