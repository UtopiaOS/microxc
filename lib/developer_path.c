#include <stdio.h>
#include <stdlib.h>
#include "developer_path.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syslimits.h>
#include <errno.h>

#define SDK_CFG ".xcdev.dat"

char* var_dev_symlink (const char* path){
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
static char *get_developer_path(void)
{
	FILE *fp = NULL;
	char devpath[PATH_MAX - 1];
	char *pathtocfg = NULL;
	char *cfg_path = NULL;
	char *value = NULL;

	if ((value = getenv("DEVELOPER_DIR")) != NULL){
		return value;
	}

	if ((value = var_dev_symlink("/var/db/xcode_select_link")) != NULL){
		return value;
	}

	memset(devpath, 0, sizeof(devpath));

	if ((pathtocfg = getenv("HOME")) == NULL) {
		fprintf(stderr, "xcode-select: error: failed to read HOME environment variable.\n");
		return NULL;
	}

	cfg_path = (char *)malloc((strlen(pathtocfg) + sizeof(SDK_CFG)));

	strcat(pathtocfg, "/");
	strcat(cfg_path, strcat(pathtocfg, SDK_CFG));

	if ((fp = fopen(cfg_path, "r")) != NULL) {
		fseek(fp, SEEK_SET, 0);
		(void)fread(devpath, (PATH_MAX - 1), 1, fp);
		value = devpath;
		fclose(fp);
	} else {
		fprintf(stderr, "xcode-select: error: unable to read configuration file. (errno=%s)\n", strerror(errno));
		return NULL;
	}

	free(cfg_path);

	return value;
}