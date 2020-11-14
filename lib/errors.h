//
// Created by Diego Magdaleno on 11/14/20.
//

#ifndef MICROXCODE_ERRORS_H
#define MICROXCODE_ERRORS_H

typedef enum {
    NOT_AUTHENTIC = 3,
    UNABLE_TO_VALIDATE,
    NOT_VALID,
    UNABLE_TO_GET_DEV_PATH,
    ERROR_GETTING_TOOLCHAIN,
    ERROR_GETTING_SDK,
    UNSUPPORTED_PLATFORM,
} microxcode_error_state_t;


#endif //MICROXCODE_ERRORS_H
