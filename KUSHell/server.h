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
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>
#define MAX_CLIENTS 32


typedef struct command {
    char *program_name;
    char **argv;
    int argc;
    char filename[1024];
} COMMAND;



void server_stat(void);
void server_halt(void);
void server_help(void);
void init_server(ARGUMENTS *args);
void start_server_socket(ARGUMENTS *args);
void server_loop(ARGUMENTS *args);
void close_client_connection(int client_id);
char server_prompt(void);
void handle_command(char *command, int client_id);
void handle_server_command(char *command);
char **tokenizer(char *input);
COMMAND **parse_tokens(char **tokens);
char *execute_commands(COMMAND **commands, int client_id);
extern int num_of_tokens;
extern int num_of_commands;
extern struct pollfd fd_list[MAX_CLIENTS + 2];
extern int server_socket;
extern int piped_command;
extern int redirection;
extern int redirection_in;
extern int redirection_out;

#endif /* server_h */


