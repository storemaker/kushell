#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "server.h"
#include "client.h"
#include "helpers.h"

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
  
    ARGUMENTS *args = parse_cmd_args(argc, argv);
    printf("%s %s %d %d", args->socket_path, args->socket_address, args->socket_port, args->mode);
    
    if (args->mode == SERVER) {
        init_server(args);
    }
    else if (args->mode == CLIENT) {
        init_client(args);
    }

  return EXIT_SUCCESS;
}
