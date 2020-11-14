//
// Created by Diego Magdaleno on 11/14/20.
//

#include "logging_printf.h"

#include <stdio.h>
#include <stdarg.h>

void logging_printf(FILE *fp, const char *str, ...)
{
    va_list args;

    va_start(args, str);
    vfprintf(fp, str, args);
    va_end(args);
}