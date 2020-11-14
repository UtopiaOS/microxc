//
// Created by Diego Magdaleno on 11/14/20.
//

#ifndef MICROXCODE_RUNNERS_H
#define MICROXCODE_RUNNERS_H

int call_command(const char*, const char*, const char*, int argc, char *argv[]);
char* search_command(const char*, char*);
int request_command(const char*, const char*, const char*, int argc, char *argv[]);


#endif //MICROXCODE_RUNNERS_H