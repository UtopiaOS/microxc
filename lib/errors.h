//
// Created by Diego Magdaleno on 11/14/20.
//

#ifndef MICROXCODE_ERRORS_H
#define MICROXCODE_ERRORS_H

enum microxcode_error_state_t{
    NOT_AUTHENTIC = 3,
    UNABLE_TO_VALIDATE,
    NOT_VALID,
    UNABLE_TO_GET_DEV_PATH,
    ERROR_GETTING_TOOLCHAIN,
    ERROR_GETTING_SDK,
    UNSUPPORTED_PLATFORM,
    ERROR_ALLOCATING_MEMORY,
    ERROR_GETTING_DEFAULT_CONFIG,



    // Successful operations start at 30
    SUCCESFUL_OPERATION = 30,

};


#endif //MICROXCODE_ERRORS_H
