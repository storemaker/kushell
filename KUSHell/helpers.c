//
//  helpers.c
//  KUSHell
//
//  Created by Jakub Taraba on 28/04/2022.
//

#include "helpers.h"



void print_current_time(void) {
    time_t rawtime;
    struct tm * timeinfo;
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    printf("%d:%d ", timeinfo->tm_hour, timeinfo->tm_min);
}

void print_help(void) {
    printf("KUSHell by Jakub Taraba (simple server-client shell)\n");
    printf("Options:\n");
    printf("-u  socket path\n");
    printf("-i  socket IP address\n");
    printf("-p  socket port\n");
    printf("-s  server mode\n");
    printf("-c  client mode\n");
    printf("-h  help meeeeeee\n\n");
    printf("Example:\n");
    printf("./kushell -u /socket/path -p 1337 -s");
}

ARGUMENTS *parse_cmd_args(int argc, char **argv) {
    int option = 0;
    ARGUMENTS *args = malloc(sizeof(ARGUMENTS));
    
    while ((option = getopt(argc, argv, "u:i:p:sch")) != -1) {
        switch (option) {
            case 'u':
                args->socket_path = malloc(sizeof(optarg));
                strcpy(args->socket_path, optarg);
                break;
            case 'i':
                args->socket_address = malloc(sizeof(optarg));
                strcpy(args->socket_address, optarg);
                break;
            case 'p':
                args->socket_port = atoi(optarg);
                break;
            case 'h':
                print_help();
                exit(EXIT_SUCCESS);
                break;
            case 's':
                args->mode = SERVER;
                break;
            case 'c':
                args->mode = CLIENT;
                break;
            default:
                print_help();
                exit(EXIT_SUCCESS);
                break;
        }
    }
    
    return args;
}
