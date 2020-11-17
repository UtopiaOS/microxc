//
// Created by Diego Magdaleno on 11/14/20.
//

#ifndef MICROXCODE_RUNNERS_H
#define MICROXCODE_RUNNERS_H

#include <stdbool.h>

void callCommand(bool, const char *, int argc, char **argv, int *);

char *searchCommand(bool, const char *, char *, int *);

void requestCommand(bool, const char *, const char *, const char *, int argc, char **argv, int *);

void command(const char *cmd, int argc, char *argv[], int *, ...);
#endif //MICROXCODE_RUNNERS_H