//
//  client.h
//  KUSHell
//
//  Created by Jakub Taraba on 28/04/2022.
//

#ifndef client_h
#define client_h

#include <stdio.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include "helpers.h"
#define MAX_CLIENTS 32

extern int client_socket;
extern struct pollfd client_fd_list[2];
void init_client(ARGUMENTS *args);
void connect_to_server(ARGUMENTS *args);
void client_loop(void);

#endif /* client_h */


