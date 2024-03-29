#include <stdarg.h>
#include <stdio.h>
#include "verbose_printf.h"

void verbosePrintf(FILE *fp, const char *str, ...) {
	va_list args;
	va_start(args, str);
	vfprintf(fp, str, args);
	va_end(args);
}