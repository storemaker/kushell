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
    
    // FD for client's STDIN
    fd_list[0].fd = 0;
    fd_list[0].events = (POLLIN | POLLPRI);
    
    // FD for new connections
    fd_list[1].fd = client_socket;
    fd_list[1].events = (POLLIN | POLLPRI);
    
    client_loop();
}

void client_loop()
{
    int i = 0;
    
    while(1) {
        //4. Start calling poll and wait for the file descriptor set of interest to be ready
        switch( poll(fd_list, MAX_CLIENTS, 3000) ) {
            case 0: {
                //printf("timeout...\n");
                continue;
            }
            case -1: {
                printf("poll fail...\n");
                continue;
            }
            default: { // poll is successfull
                //   If it is a listener file descriptor, call accept to accept a new connection
                //   If it is a normal file descriptor, read is called to read the data
                for(i = 0; i < 2; i++) {
                    // stdin
                    if(i == 0 && (fd_list[0].revents & POLLIN)) {
                        //server_prompt();
                        
                    }
                    // server socket
                    if(i == 1 && (fd_list[1].revents & POLLIN)) {
                        char buf[1024];
                        read(fd_list[1].fd,buf,sizeof(buf)-1);
                        printf("server: %s\n", buf);
                    }
                    
                } // end for
                
                break;
            } // end default case
        } // end switch
    } // end while loop
    
    
    return;
}

void connect_to_server(ARGUMENTS *args)
{
    struct sockaddr_in servaddr;
        
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = (strlen(args->socket_address) > 0) ? inet_addr(args->socket_address) : htonl(INADDR_ANY);
    servaddr.sin_port = htons(args->socket_port);
    
    if (connect(client_socket, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("Connection with the server failed...\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("connected to the server..\n");
}

