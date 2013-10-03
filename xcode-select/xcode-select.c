#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#define TOOL_VERSION "0.0.1"
#define DARWINSDK_CFG ".darwinsdk.dat"

void usage(char *prog)
{

	fprintf(stderr, "%s%s%s%s%s%s%s",
			"Usage: xcode-select -print-path\n",
			"   or: xcode-select -switch <darwinsdk_folder_path>\n",
			"   or: xcode-select -version\n",
			"Arguments:\n",
			"   -print-path                     Prints the path of the current DarwinSDK folder\n",
			"   -switch <xcode_folder_path>     Sets the path for the current DarwinSDK folder\n",
			"   -version                        Prints xcode-select version information\n");

	exit(1);
}

char *get_sdk_path(void)
{
	FILE *fp = NULL;
	char devpath[PATH_MAX - 1];
	char *pathtocfg = NULL;
	char *darwincfg_path = NULL;
	char *value = NULL;

	memset(devpath, 0, sizeof(devpath));

	if ((pathtocfg = getenv("HOME")) == NULL) {
		fprintf(stderr, "xcode-select: error: failed to read HOME variable.\n");
		return NULL;
	}

	darwincfg_path = (char *)malloc((strlen(pathtocfg) + sizeof(DARWINSDK_CFG)));

	strcat(pathtocfg, "/");
	strcat(darwincfg_path, strcat(pathtocfg, DARWINSDK_CFG));

	if ((fp = fopen(darwincfg_path, "r")) != NULL) {
		fseek(fp, SEEK_SET, 0);
		fread(devpath, (PATH_MAX), 1, fp);
		value = devpath;
		fclose(fp);
	} else {
		fprintf(stderr, "xcode-select: error: unable to read configuration file. (errno=%s)\n", strerror(errno));
		return NULL;
	}

	free(darwincfg_path);

	return value;
}

int set_sdk_path(const char *path)
{
	FILE *fp = NULL;
	char *pathtocfg = NULL;
	char *darwincfg_path = NULL;

	if ((pathtocfg = getenv("HOME")) == NULL) {
		fprintf(stderr, "xcode-select: error: failed to read HOME variable.\n");
		return -1;
	}

        darwincfg_path = (char *)malloc((strlen(pathtocfg) + 8));

        strcat(pathtocfg, "/");
	strcat(darwincfg_path, strcat(pathtocfg, DARWINSDK_CFG));

	if ((fp = fopen(darwincfg_path, "w+")) != NULL) {
		fwrite(path, 1, strlen(path), fp);
		fclose(fp);
	} else {
		fprintf(stderr, "xcode-select: error: unable to open configuration file. (errno=%s)\n", strerror(errno));
		return 1;
	}

	free(darwincfg_path);

	return 0;
}

int main(int argc, char *argv[])
{
	char *path;

	if (argc < 2)
		usage(argv[0]);

	if (strcmp(argv[1], "-help") == 0)
		usage(argv[0]);
	else if (strcmp(argv[1], "-switch") == 0)
		return set_sdk_path(argv[2]);
	else if (strcmp(argv[1], "-print-path") == 0) {
		if ((path = get_sdk_path()) != NULL)
			printf("%s\n", path);
	} else if (strcmp(argv[1], "-version") == 0) {
		printf("xcode-select version %s\n", TOOL_VERSION);
	} else
		usage(argv[0]);

	return 0;
}
