//
// Created by Diego Magdaleno on 11/13/20.
//

#include "config_handler.h"
#include <TargetConditionals.h>
#include <stdio.h>
#include <stdlib.h>

/* Get the default "Config"
 * Before I rewrote this code, this was supposed to be
 * a set of values at /etc/xcrun.ini, however
 * this is no longer necessary as we are not parsing inis
 * so you might wonder, what is our default config now?
 * Well our default config now is the SDK and the toolchain (if it has one)
 * of the platform we are running on
 *
*/


default_config get_default_info()
{
    char *supposed_name;
    default_config config;

    #if TARGET_OS_IPHONE
        supposed_name = "iPhoneOS";
    #elif TARGET_OS_MAC
        supposed_name = "MacOSX";
    #else
        supposed_name = NULL;
    #endif

        if (supposed_name == NULL) {
            fprintf(stderr, "xcrun: Failed to retrieve default SDK supposed name, platform not supported!");
            exit(1);
        }

        config.sdk = supposed_name;
        config.toolchain = "XcodeDefault";

        return config;

}