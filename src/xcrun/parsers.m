//
// Created by Diego Magdaleno on 11/13/20.
//

#include <Foundation/Foundation.h>
#include "parsers.h"

NSDictionary*
plist_parse(NSURL *target_plist_path, NSString *target_name)
{
    NSData *data_of_file = [NSData dataWithContentsOfURL:target_plist_path];
    NSDictionary *parse_plist_dict = [NSPropertyListSerialization propertyListWithData:data_of_file
    options:NSPropertyListMutableContainers format:NULL error:NULL];

    NSString *canonical_name = [[parse_plist_dict valueForKey:@"CanonicalName"]
                                componentsSeparatedByCharactersInSet:[[NSChracter letterCharacterSet]invertedSet]
                                componentsJoinedByString:@""];

    NSString *version = [parse_plist_dict valueForKey:@"DefaultDeploymentTarget"];

    // this is getting hardcoded until I refactor the code a little bit more
    NSString *toolchain = "XcodeDefault";

    NSDictionary *supported_targets_defaults = [[parse_plist_dict valueForKey@"SupportedTargets"] valueForKey:target_name];

    NSString *deployment_target = [supported_targets_defaults valueForKey:@"DefaultDeploymentTarget"];

    NSString *architecture = [supported_targets_defaults valueForKey:@"Archs"][0];

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


