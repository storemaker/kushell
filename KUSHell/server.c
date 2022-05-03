//
//  server.c
//  KUSHell
//
//  Created by Jakub Taraba on 28/04/2022.
//

#include "server.h"

#define READ_END 0
#define WRITE_END 1

int num_of_tokens;
int num_of_commands;
struct pollfd fd_list[MAX_CLIENTS + 2];
int server_socket;
int piped_command;
int redirection;
int redirection_in;
int redirection_out;
int epoch[34];

void start_server_socket(ARGUMENTS *args) {
    
    if (args->socket_path && strlen(args->socket_path) > 0) {
        struct sockaddr_un local_unix;
        server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        local_unix.sun_family = AF_UNIX;
        strcpy(local_unix.sun_path, args->socket_path);
        
        // 3. Binding port number
        if (bind(server_socket,(struct sockaddr *)&local_unix, sizeof(local_unix)) < 0) {
            perror("Server bind failed.\n");
            exit(EXIT_FAILURE);
        }
        
        printf("Socket binded...\n");
    }
    else {
        struct sockaddr_in local;
        server_socket = socket(AF_INET,SOCK_STREAM,0);
        
        if ( server_socket < 0 ) {
            perror("Socket failed to create...\n");
            exit(EXIT_FAILURE);
        }
        
        printf("Socket created...\n");
        
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
        
        printf("Socket binded...\n");
    }
    

    // 4. Get a listening socket
    if (listen(server_socket, 32) < 0) {
        perror("Server listen failed.\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening...\n");
    
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

void server_help()
{
    printf("Available commands as server:\n- halt => end the server and close all sockets\n- stat => show connected clients\n");
    return;
}

void server_halt()
{
    for (int i = 0; i < MAX_CLIENTS+2; i++) {
        if (fd_list[i].fd != -1 && i > 0)
            close(fd_list[i].fd);
        fd_list[i].fd = -1;
    }
    exit(EXIT_SUCCESS);
}

void server_stat()
{
    printf("Connected clients:\n");
    
    for (int i = 2; i < MAX_CLIENTS; i++) {
        if (fd_list[i].fd != -1)
            printf("Client ID: %d | Socket FD: %d\n", i-2, i);
    }
    
    printf("Server listening sockets FD: %d\n", 1);
    
    return;
}


char **tokenizer(char *input)
{
    char *copied_input = malloc(strlen(input)*sizeof(char));
    strcpy(copied_input, input);
    int bufsize = 64, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;
    //char delim = " ";
    num_of_tokens = 0;
    num_of_commands = 0;
    
    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    token = strtok(copied_input, " ");
    while (token != NULL) {
        tokens[position] = token;
        if ( strcmp("|", token) == 0 || strcmp(";", token) == 0) num_of_commands++;
        position++;
        
        if (position >= bufsize) {
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        
        token = strtok(NULL, " ");
    }
    tokens[position] = NULL;
    num_of_tokens = position;
    num_of_commands++;
    
    // TODO: escaping
    /*
     for (int i = 0; i < position; i++) {
     
     }*/
    
    
    return tokens;
}

COMMAND **parse_tokens(char **tokens)
{
    COMMAND **commands = malloc(num_of_commands * sizeof(COMMAND*));
    int j = 0;
    
    for (int i = 0; i < num_of_tokens; i++) {
        
        if (strcmp(">", tokens[i]) == 0) {
            redirection_out = 1;
            strcpy(commands[j]->filename, tokens[i+1]);
            commands[j]->argv[num_of_tokens-1] = NULL;
            commands[j]->argv[num_of_tokens-2] = NULL;
            break;
        }
        
        if (strcmp("<", tokens[i]) == 0) {
            redirection_in = 1;
            strcpy(commands[j]->filename, tokens[i+1]);
            commands[j]->argv[num_of_tokens-1] = NULL;
            commands[j]->argv[num_of_tokens-2] = NULL;
            break;
        }
        
        if (i == 0) {
            commands[j] = malloc(sizeof(COMMAND));
            commands[j]->program_name = malloc(strlen(tokens[i])*sizeof(char));
            commands[j]->argv = malloc(2*sizeof(char*));
            strcpy(commands[j]->program_name, tokens[i]);
            commands[j]->argv[0] = malloc(strlen(tokens[i])*sizeof(char));
            strcpy(commands[j]->argv[0], tokens[i]);
            commands[j]->argc = 1;
            continue;
        }
        else if ((strcmp("|", tokens[i]) == 0 || strcmp(";", tokens[i]) == 0) && i+1 < num_of_tokens) {
            if ((strcmp("|", tokens[i]) == 0)) piped_command = 1;
            j++;
            commands[j] = malloc(sizeof(COMMAND));
            commands[j]->program_name = malloc(strlen(tokens[i+1])*sizeof(char));
            commands[j]->argv = malloc(2*sizeof(char*));
            
            strcpy(commands[j]->program_name, tokens[i+1]);
            
            commands[j]->argv[0] = malloc(strlen(tokens[i+1])*sizeof(char));
            strcpy(commands[j]->argv[0], tokens[i+1]);
            commands[j]->argc = 1;
            i++;
            continue;
        }
        
        if (commands[j]->argc > 2) {
            realloc(commands[j]->argv, commands[j]->argc);
        }
        
        commands[j]->argv[commands[j]->argc] = malloc(strlen(tokens[i])*sizeof(char));
        strcpy(commands[j]->argv[commands[j]->argc], tokens[i]);
        commands[j]->argc++;
    }
    
    for (int i = 0; i < num_of_commands; i++) {
        realloc(commands[i]->argv, commands[i]->argc+1);
        commands[i]->argv[commands[i]->argc] = NULL;
    }
    
    return commands;
}

void handle_server_command(char *command)
{
    
    if (strcmp(command, "help") == 0) {
        server_help();
    }
    else if (strcmp(command, "halt") == 0) {
        server_halt();
    }
    else if (strcmp(command, "stat") == 0) {
        server_stat();
    } else {
        printf("Unknown command.\n");
    }
    
    
    return;
}

char *execute_commands(COMMAND **commands, int client_id)
{
    int i;
    
    if (num_of_commands == 1 && redirection_in != 1 && redirection_out != 1) {
        int pid = fork();
        // child process;
        if (pid == 0) {
            // duplicate the output to pipe
            dup2(fd_list[client_id].fd, STDOUT_FILENO);
            close(fd_list[client_id].fd);
            execvp(commands[0]->program_name, commands[0]->argv);
        }
        else {
            //close(fd_list[client_id].fd);
            waitpid( pid, NULL, 0 );
        }
    }
    else if (num_of_commands == 2 && piped_command == 0) {
        for (i = 0; i < 2; i++) {
            int pid = fork();
            // child process;
            if (pid == 0) {
                // duplicate the output to pipe
                dup2(fd_list[client_id].fd, STDOUT_FILENO);
                close(fd_list[client_id].fd);
                execvp(commands[i]->program_name, commands[i]->argv);
            }
            else {
                //close(fd_list[client_id].fd);
                waitpid( pid, NULL, 0 );
            }
        }
    }
    else if (piped_command == 1) {
        
        //printf("piped\n");
        pid_t pid;
        int fd[2];
        
        pipe(fd);
        pid = fork();
        
        if(pid==0)
        {
            dup2(fd[WRITE_END], STDOUT_FILENO);
            close(fd[READ_END]);
            close(fd[WRITE_END]);
            execvp(commands[0]->program_name, commands[0]->argv);
            exit(1);
        }
        else
        {
            pid=fork();
            
            if(pid==0)
            {
                dup2(fd[READ_END], STDIN_FILENO);
                dup2(fd_list[client_id].fd, STDOUT_FILENO);
                close(fd_list[client_id].fd);
                close(fd[WRITE_END]);
                close(fd[READ_END]);
                execvp(commands[1]->program_name, commands[1]->argv);
                exit(1);
            }
            else
            {
                int status;
                close(fd[READ_END]);
                close(fd[WRITE_END]);
                waitpid(pid, &status, 0);
            }
        }
    }
    else if (redirection_out == 1) {
        int pid = fork();
        // child process;
        if (pid == 0) {
            FILE *filee;
            filee = fopen(commands[0]->filename, "w");
            // duplicate the output to pipe
            dup2(fileno(filee), STDOUT_FILENO);
            close(fileno(filee));
            execvp(commands[0]->program_name, commands[0]->argv);
        }
        else {
            //close(fd_list[client_id].fd);
            waitpid( pid, NULL, 0 );
        }
        write(fd_list[client_id].fd, "\n", 1);
    }
    else if (redirection_in == 1) {
        int pid = fork();
        // child process;
        if (pid == 0) {
            FILE *filee;
            filee = fopen(commands[0]->filename, "r");
            // duplicate the output to pipe
            dup2(fileno(filee), STDIN_FILENO);
            close(fileno(filee));
            execvp(commands[0]->program_name, commands[0]->argv);
        }
        else {
            //close(fd_list[client_id].fd);
            waitpid( pid, NULL, 0 );
        }
        write(fd_list[client_id].fd, "\n", 1);
    }
    else {
        char *syntax_error = "Syntax error (multiple commands not supported)";
        write(fd_list[client_id].fd, syntax_error, strlen(syntax_error)+1);
    }
    
    return NULL;
}

void close_client_connection(int client_id)
{
    if(fd_list[client_id].fd == -1) {
        return;
    }
    
    //close the socket and reset FD for future use
    close(fd_list[client_id].fd);
    fd_list[client_id].fd = -1;
    fd_list[client_id].revents = 0;
    fd_list[client_id].events = 0;
    
    //TODO: release client IP from connected_clients array
    
}

void handle_command(char *command, int client_id)
{
    char *output = NULL;
    int i,j;
    
    if (strlen(command) == 0)
        return; // return but do not close client socket
    
    
    // the client exited program, kill the socket afterwards
    if (!(strcmp(command, "quit"))) {
        close_client_connection(client_id);
        return;
    }
    
    num_of_tokens = 0;
    num_of_commands = 0;
    redirection = 0;
    piped_command = 0;
    redirection_in = 0;
    redirection_out = 0;
    char **tokens = tokenizer(command);
    COMMAND **commands = parse_tokens(tokens);
    
    output = execute_commands(commands, client_id);
    
    // release commands to avoid memory problems
    // with new incoming commands
    for (i = 0; i < num_of_commands; i++) {
        /*for (j = 0; j < commands[i]->argc+1; j++) {
            free(commands[i]->argv[j]);
        }*/
        free(commands[i]->program_name);
        free(commands[i]);
    }
    
    
    free(tokens);
    
    return; // do not close client socket
}



void server_loop(ARGUMENTS *args)
{
    int i;
    char server_command[1024];
    char client_command[1024];
    char *exec_output;
    FILE *logfile;
    
    logfile = fopen(args->log_file, "a");
    
    print_prompt();
    while(1) {
        
        //4. Start calling poll and wait for the file descriptor set of interest to be ready
        switch( poll(fd_list, MAX_CLIENTS + 2, 1000) ) {
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
                // stdin
                if((fd_list[0].revents & POLLIN)) {
                    fgets(server_command, 1024, stdin);
                    server_command[strcspn(server_command, "\n")] = 0;
                    handle_server_command(server_command);
                    print_prompt();
                    
                    if(args->log_file)
                        fprintf(logfile, "server command: %s\n", server_command);
                }
                
                // server socket
                if(fd_list[1].fd == server_socket && (fd_list[1].revents & POLLIN)) {
                    
                    // 1. Provide a connection acceptance service if the listening socket is ready to read
                    struct sockaddr_in client;
                    socklen_t len = sizeof(client);
                    int new_sock = accept(server_socket,(struct sockaddr *)&client,&len);
                    
                    if(new_sock < 0) {
                        perror("Client accept failed...\n ");
                        continue;
                    }
                    
                    // find free FDs
                    for(i = 0; i < MAX_CLIENTS; i++)
                        if( fd_list[i].fd == -1 )//Place the first value in the array at - 1
                            break;
                    
                    if(i < MAX_CLIENTS) {
                        // add new client socket to list
                        fd_list[i].fd = new_sock;
                        fd_list[i].events = POLLIN;
                    }
                    else {
                        // close the connection if MAX_CLIENTS is reached
                        close(new_sock);
                    }
                    printf("\nNew connection accepted!\n");
                    print_prompt();
                    continue;
                }
                
                
                for(i = 2; i < MAX_CLIENTS + 2; i++) {
                    
                    // skip empty FDs
                    if(fd_list[i].fd == -1)
                        continue;
                    
                    //2. At this point, we are concerned with ordinary file descriptors.
                    //   Provide services to read data at this time
                    if( fd_list[i].revents & POLLIN ) {
                        
                        ssize_t s = read(fd_list[i].fd, client_command, sizeof(client_command)-1);
                        if( s < 0 ) {
                            printf("read fail...\n");
                            continue;
                        }
                        else if (s > 0) {
                            //buf[s] = 0;
                            printf("\nClient[%d] command: %s\n", i-2, client_command);
                            
                            if (args->log_file)
                                fprintf(logfile, "client[%d]: %s\n", i-2, client_command);
                            
                            handle_command(client_command, i);
                        
                            
                            print_prompt();
                        }
                        
                        memset(client_command, 0, 1024);
                    }
                } // end for
                
                //
                
                // TODO: keep-alive check
                //printf("keepalive-check here\n");
                
                break;
            } // end default case
        } // end switch
       
        
        
        
        
    } // end while loop
    
    
    fclose(logfile);
        
    return;
}

void init_server(ARGUMENTS *args) {
    start_server_socket(args);
    server_loop(args);
    return;
}
