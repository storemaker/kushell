//
//  client.c
//  KUSHell
//
//  Created by Jakub Taraba on 28/04/2022.
//

#include "client.h"

void init_client(ARGUMENTS *args)
{
    connect_to_server(args);
    return;
}

void connect_to_server(ARGUMENTS *args)
{
    struct sockaddr_in servaddr;
    char *stringg = "hoj do pici";
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = (strlen(args->socket_address) > 0) ? inet_addr(args->socket_address) : htonl(INADDR_ANY);
    servaddr.sin_port = htons(args->socket_port);
    
    if (connect(client_socket, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
    
    write(client_socket, stringg, sizeof(stringg));
    close(client_socket);
}

