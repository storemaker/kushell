//
//  helpers.h
//  KUSHell
//
//  Created by Jakub Taraba on 28/04/2022.
//

#ifndef helpers_h
#define helpers_h

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

typedef enum {SERVER, CLIENT} MODE;

typedef struct arguments {
    int socket_port;
    char *socket_path;
    char *socket_address;
    MODE mode;
} ARGUMENTS;

void print_current_time(void);
void print_help(void);
ARGUMENTS *parse_cmd_args(int argc, char **argv);

#endif /* helpers_h */


