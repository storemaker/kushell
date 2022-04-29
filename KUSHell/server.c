//
//  server.c
//  KUSHell
//
//  Created by Jakub Taraba on 28/04/2022.
//

#include "server.h"


void start_server_socket(ARGUMENTS *args) {
    struct sockaddr_in local;
    
    server_socket = socket(AF_INET,SOCK_STREAM,0);
    
    if ( server_socket < 0 ) {
        perror("Socket failed to create...\n");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = (strlen(args->socket_address) > 0) ? inet_addr(args->socket_address) : htonl(INADDR_ANY);
    local.sin_port = htons(args->socket_port);
    
    // 3. Binding port number
    if (bind(server_socket,(struct sockaddr *)&local, sizeof(local)) < 0) {
        perror("Server bind failed.\n");
        exit(EXIT_FAILURE);
    }

    // 4. Get a listening socket
    if (listen(server_socket, 32) < 0) {
        perror("Server listen failed.\n");
        exit(EXIT_FAILURE);
    }
    
    // init FDs
    for(int i = 0; i < MAX_CLIENTS; i++) {
        fd_list[i].fd = -1;// File descriptor
        fd_list[i].events = 0;// Set of events to monitor
        fd_list[i].revents = 0;// Ready Event Set of Concerned Descriptors
    }
    
    // FD for server's STDIN
    fd_list[0].fd = 0;
    fd_list[0].events = (POLLIN | POLLPRI);
    
    // FD for new connections
    fd_list[1].fd = server_socket;
    fd_list[1].events = (POLLIN | POLLPRI);
 
}

void handle_communication(ARGUMENTS *args)
{
    int i = 0;
    while(1) {
        //4. Start calling poll and wait for the file descriptor set of interest to be ready
        switch( poll(fd_list, MAX_CLIENTS, 3000) ) {
            case 0: {
                printf("timeout...\n");
                continue;
            }
            case -1: {
                printf("poll fail...\n");
                continue;
            }
            default: { // poll is successfull
                //   If it is a listener file descriptor, call accept to accept a new connection
                //   If it is a normal file descriptor, read is called to read the data
                for(i = 0; i < MAX_CLIENTS; i++) {
                    
                    // skip uninitialized FDs
                    if(fd_list[i].fd == -1)
                        continue;
                    
                    // server socket
                    if(fd_list[i].fd == server_socket && (fd_list[i].revents & POLLIN)) {
                        
                        // 1. Provide a connection acceptance service if the listening socket is ready to read
                        struct sockaddr_in client;
                        socklen_t len = sizeof(client);
                        int new_sock = accept(listen_sock,(struct sockaddr *)&client,&len);
                        if(new_sock < 0)
                        {
                            perror("accept fail...\n ");
                            continue;
                        }
                        //After obtaining the new file descriptor, add the file descriptor to the array for the next time you care about the file descriptor
                        int i = 0;
                        for( ; i < num; i++ )
                        {
                            if( fd_list[i].fd == -1 )//Place the first value in the array at - 1
                                break;
                        }
                        if( i < num )
                        {
                            fd_list[i].fd= new_sock;
                            fd_list[i].events = POLLIN;
                        }
                        else
                        {
                            close(new_sock);
                        }
                        printf("get a new link![%s:%d]\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
                        continue;
                    }
                    
                    //2. At this point, we are concerned with ordinary file descriptors.
                    //   Provide services to read data at this time
                    if( fd_list[i].revents & POLLIN ) {
                        char buf[1024];
                        ssize_t s = read(fd_list[i].fd,buf,sizeof(buf)-1);
                        if( s < 0 )
                        {
                            printf("read fail...\n");
                            continue;
                        }
                        else if( s == 0 )
                        {
                            printf("client quit...\n");
                            close(fd_list[i].fd);
                            fd_list[i].fd = -1;
                        }
                        else
                        {
                            buf[s] = 0;
                            printf("client# %s\n",buf);
                        }
                    }
                }
            }
                break;
        }
    }
        

    return;
}

void init_server(ARGUMENTS *args) {
    start_server_socket(args);
    handle_communication(args);
    return;
}
