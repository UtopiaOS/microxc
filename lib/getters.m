//
// Created by Diego Magdaleno on 11/14/20.
//

#include <stdlib.h>
#include <stdio.h>
#include <Foundation/Foundation.h>

#include "validators.h"
#include "typedefs.h"
#include "getters.h"
#include "errors.h"

static NSDictionary *
plist_parse(NSURL *target_plist_path, NSString *target_name)
{
    if (target_plist_path == NULL || target_name == NULL) {
        return NULL;
    }

    NSData *data_of_file = [NSData dataWithContentsOfURL:target_plist_path];

    NSDictionary *parse_plist_dict = [NSPropertyListSerialization propertyListWithData:data_of_file
                                      options:NSPropertyListMutableContainers format:NULL error:NULL];

    NSString *canonical_name = [[[parse_plist_dict valueForKey:@"CanonicalName"]
                                 componentsSeparatedByCharactersInSet:[[NSCharacterSet letterCharacterSet] invertedSet]]
            componentsJoinedByString:@""];

    NSString *version = [parse_plist_dict valueForKey:@"DefaultDeploymentTarget"];

    // this is getting hardcoded until I refactor the code a little bit more
    NSString *toolchain = @"XcodeDefault";

    NSDictionary *supported_targets_defaults = [[parse_plist_dict valueForKey:@"SupportedTargets"] valueForKey:[target_name lowercaseString]];

    NSString *deployment_target = [supported_targets_defaults valueForKey:@"DefaultDeploymentTarget"];

    NSArray *architectures = [supported_targets_defaults valueForKey:@"Archs"];
    NSString *architecture = architectures[0];

    NSDictionary *our_dict = [NSDictionary alloc];

    our_dict = @{
            @"name": canonical_name,
            @"version": version,
            @"toolchain": toolchain,
            @"deployment_target": deployment_target,
            @"arch": architecture,
    };

    return our_dict;
}

toolchain_config
get_toolchain_info(const char *path, const char *current_sdk, int *err)
{

    microxcode_error_state_t error_state;
    toolchain_config config;
    NSURL *info_path;

    // lets handle the conversion from a CString to an NSString
    NSString *initial_path = [NSString stringWithCString:path encoding:NSUTF8StringEncoding];
    NSString *final_path = [NSString stringWithFormat:@"%@/SDKSettings.plist", initial_path];

    free(initial_path);

    info_path = [NSURL fileURLWithPath:final_path];

    NSString *target_sdk = [NSString stringWithCString:current_sdk encoding:NSUTF8StringEncoding];

    NSDictionary *dictSession = plist_parse(info_path, target_sdk);

    if (dictSession == NULL) {
        if (err) { *err = ERROR_GETTING_TOOLCHAIN; }
        return NULL;
    }

    config.version = [[dictSession valueForKey:@"version"] UTF8String];
    config.name = [[dictSession valueForKey:@"name"] UTF8String];

    return config;
}

/**
 * @func get_sdk_info -- fetch config info from a toolchain's info.ini
 * @arg path - path to sdk's info.ini
 * @return: struct containing sdk config info
 */
sdk_config
get_sdk_info(const char *path, const char *current_sdk, int *err)
{

    microxcode_error_state_t error_state;
    sdk_config config;
    NSURL *info_path;

    NSString *initial_path = [NSString stringWithCString:path encoding:NSUTF8StringEncoding];
    NSString *final_path = [NSString stringWithFormat:@"%@/SDKSettings.plist", initial_path];

    free(initial_path);

    info_path = [NSURL fileURLWithPath:final_path];

    NSString *target_sdk = [NSString stringWithCString:current_sdk encoding:NSUTF8StringEncoding];

    NSDictionary *dict_session = plist_parse(info_path, target_sdk);

    if (dict_session == NULL) {
        if (err) { *err = ERROR_GETTING_SDK; }
        return NULL;
    }

    config.version = [[dict_session valueForKey:@"version"] UTF8String];
    config.name = [[dict_session valueForKey:@"name"] UTF8String];
    config.default_arch = [[dict_session valueForKey:@"arch"] UTF8String];
    config.deployment_target = [[dict_session valueForKey:@"deployment_target"] UTF8String];
    config.toolchain = [[dict_session valueForKey:@"toolchain"] UTF8String];

    return config;

}

default_config
get_default_info(int *err)
{
    microxcode_error_state_t error_state;
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
        if (err) { *err = UNSUPPORTED_PLATFORM; }
        return NULL;
    }

    config.sdk = supposed_name;
    // TODO: Unhardcode this! This is only temporal!
    config.toolchain = "XcodeDefault";

    return config;

}

char *
get_toolchain_path(const char *developer_dir, const char *name, int *err)
{
    microxcode_error_state_t error_state;
    char *path = NULL;

    path = (char *) malloc(PATH_MAX - 1);

    sprintf(path, "%s/Toolchains/%s.xctoolchain", developer_dir, name);

    if (validate_directory_path(path) == 0) {
        return path;
    }

    if (err) { *err = ERROR_GETTING_TOOLCHAIN; }
    return NULL;
}

char *
get_sdk_path(const char *developer_dir, const char *name, int *err)
{
    microxcode_error_state_t error_state;
    char *path = NULL;

    path = (char *) malloc(PATH_MAX - 1);

    sprintf(path, "%s/Platforms/%s.Platform/Developer/SDKs/%s.sdk", developer_dir, name, name);
    if (validate_directory_path(path) == 0) {
        return path;
    }

    if (err) { *err = ERROR_GETTING_SDK; }
    return NULL;

}