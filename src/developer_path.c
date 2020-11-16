#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "errors.h"
#include "developer_path.h"

static char *
var_dev_symlink(const char *path) {
    struct stat buf;
    char *symlink_name;
    int did_readlink;

    // The file is not a symlink
    if (lstat(path, &buf) != 0) {
        return NULL;
    };

    symlink_name = malloc(buf.st_size + 1);
    did_readlink = readlink(path, symlink_name, buf.st_size + 1);
    if (did_readlink != -1) {
        symlink_name[buf.st_size] = '\0';
        if (stat(symlink_name, &buf) == 0) {
            // return what it points to
            return symlink_name;
        }
    } else {
        free(symlink_name);
        return NULL;
    }

    free(symlink_name);
    return NULL;
}

/**
 * @func get_developer_path -- retrieve current developer path
 * @return: string of current path on success, NULL string on failure
 */
char *
get_developer_path(int *err) {
    char *value = NULL;

    if ((value = getenv("DEVELOPER_DIR")) != NULL) {
        if (err) { *err = SUCCESFUL_OPERATION; }
        return value;
    }

    if ((value = var_dev_symlink("/var/db/xcode_select_link")) != NULL) {
        if (err) { *err = SUCCESFUL_OPERATION; }
        return value;
    }

    if (err) { *err = UNABLE_TO_GET_DEV_PATH; }
    return value;
}