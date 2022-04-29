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
#include <netinet/in.h>
#include "helpers.h"
#define MAX_CLIENTS 32

int client_socket;
void init_client(ARGUMENTS *args);
void connect_to_server(ARGUMENTS *args);

#endif /* client_h */


