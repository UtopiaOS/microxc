#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "errors.h"
#include "developer_path.h"

static char *
varDevSymlink(const char *path) {
	struct stat buf;
	char *symlinkName;
	int didReadlink;

	// The file is not a symlink
	if (lstat(path, &buf) != 0) {
		return NULL;
	};
	symlinkName = malloc(buf.st_size + 1);
	didReadlink = readlink(path, symlinkName, buf.st_size + 1);
	if (didReadlink != -1) {
		symlinkName[buf.st_size] = '\0';
		if (stat(symlinkName, &buf) == 0) {
			// return what it points to
			return symlinkName;
		}
	} else {
		free(symlinkName);
		return NULL;
	}
	free(symlinkName);
	return NULL;
}

/**
 * @func getDeveloperPath -- retrieve current developer path
 * @return: string of current path on success, NULL string on failure
 */
char *
getDeveloperPath(int *err) {
	char *value = NULL;
	if ((value = getenv("DEVELOPER_DIR")) != NULL) {
		if (err) { *err = SUCCESFUL_OPERATION; }
		return value;
	}
	if ((value = varDevSymlink("/var/db/xcode_select_link")) != NULL) {
		if (err) { *err = SUCCESFUL_OPERATION; }
		return value;
	}
	if (err) { *err = UNABLE_TO_GET_DEV_PATH; }
	return value;
}