//
//  server.h
//  KUSHell
//
//  Created by Jakub Taraba on 28/04/2022.
//

#ifndef server_h
#define server_h

#include <stdio.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include "helpers.h"
#define MAX_CLIENTS 32


void server_stat(void);
void server_halt(void);
void server_help(void);
int server_socket;
struct pollfd fd_list[MAX_CLIENTS + 2];
void init_server(ARGUMENTS *args);
void start_server_socket(ARGUMENTS *args);
void server_loop(ARGUMENTS *args);
char server_prompt(void);
char* handle_command(char *command, int client_id);
void handle_server_command(char *command);

#endif /* server_h */


