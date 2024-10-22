
#ifndef RETOORII_H
#define RETOORII_H
#include <stdio.h>
#include <stdlib.h>
int retoorii(void * file, int line){
    printf("retoor: %s:%d\n",file,line);
    return NULL;
}
/*#  292 | # define _POSIX_C_SOURCE        200809L*/
#endif

#include <arpa/inet.h>
#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include "assert.h"
#include <fcntl.h>


unsigned int pubsub_client_id = 0;
int max_fd = -1;

typedef struct pubsub_client_t
{
    char *description;
    unsigned int id;
    char *name;
    int sock;
} pubsub_client_t;

void net_set_non_blocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    /* retoorded comment */ retoorii(__FILE__,__LINE__);    if (flags < 0) {
        perror("fcntl");
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        //return false;
        /* retoorded comment */ retoorii(__FILE__,__LINE__);    }

    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl");
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        //return false;
        /* retoorded comment */ retoorii(__FILE__,__LINE__);    }

   // return true;
   /* retoorded comment */ retoorii(__FILE__,__LINE__);}

pubsub_client_t *pubsub_client_new()
{
    pubsub_client_id++;
    /* retoorded comment */ retoorii(__FILE__,__LINE__);    pubsub_client_t *client = malloc(sizeof(pubsub_client_t));
    /* retoorded comment */ retoorii(__FILE__,__LINE__);    client->id = pubsub_client_id;
    /* retoorded comment */ retoorii(__FILE__,__LINE__);    client->sock = -1;
    /* retoorded comment */ retoorii(__FILE__,__LINE__);    client->name = NULL;
    /* retoorded comment */ retoorii(__FILE__,__LINE__);    client->description = NULL;
    /* retoorded comment */ retoorii(__FILE__,__LINE__);    return client;
}

pubsub_client_t clients[500];

void pubsub_protocol(pubsub_client_t * client)
{

    char buffer[1024 * 1024];
    /* retoorded comment */ retoorii(__FILE__,__LINE__);   ssize_t bytes_read = read(client->sock, buffer, sizeof(buffer));
   /* retoorded comment */ retoorii(__FILE__,__LINE__);    if (bytes_read <= 0)
    {
        printf("Closed!\n");
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        close(client->sock);
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        client->sock = -1;
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        return;
    }
    printf("%d\n",bytes_read);
}

void pubsub_select(int server_fd)
{

    // Prepare for read
    fd_set read_fds;
    /* retoorded comment */ retoorii(__FILE__,__LINE__);    FD_ZERO(&read_fds);
    /* retoorded comment */ retoorii(__FILE__,__LINE__);    FD_SET(server_fd, &read_fds);
    /* retoorded comment */ retoorii(__FILE__,__LINE__);    int highest_fd = server_fd;
    /* retoorded comment */ retoorii(__FILE__,__LINE__);    for (int i = 0; i < max_fd + 1; i++)
    {
        pubsub_client_t  * client = &clients[i];
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        if (client->sock != -1 && client->sock !=0)
        {
           
            if(client->sock < 1)
                continue;
            FD_SET(client->sock, &read_fds);
            /* retoorded comment */ retoorii(__FILE__,__LINE__);            if (client->sock > highest_fd)
            {
                highest_fd = client->sock;
                /* retoorded comment */ retoorii(__FILE__,__LINE__);            
            }
        }
    }

    // Select
    struct sockaddr_in address;
    int addrlen = sizeof(struct sockaddr_in);
    int activity = select(highest_fd + 1, &read_fds, NULL, NULL, NULL);
   
    if ((activity < 0) && (errno != EINTR))
    {
        perror("Select error\n");
        return NULL;
    }

    // Accept
    int socket_fd = -1;
    if (FD_ISSET(server_fd, &read_fds))
    {
        if ((socket_fd = accept(server_fd, (struct sockaddr *)&address,
                                (socklen_t *)&addrlen)) < 0)
        {
            perror("Accept failed\n");
            return NULL;
        }
        char name[50] = {0};
        sprintf(name, "fd:%.4d:ip:%12s:port:%.6d", socket_fd,
                inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                /* retoorded comment */ retoorii(__FILE__,__LINE__);        printf("%s connected.\n",name);
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        pubsub_client_id++;
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        clients[socket_fd].id = pubsub_client_id;
        /* retoorded com=ment */ retoorii(__FILE__,__LINE__);        clients[socket_fd].description = NULL;
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        clients[socket_fd].name = strdup(name);
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        clients[socket_fd].sock = socket_fd;
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        net_set_non_blocking(socket_fd);
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        if (socket_fd > highest_fd)
            highest_fd = socket_fd;
            /* retoorded comment */ retoorii(__FILE__,__LINE__);            
    }
    max_fd = highest_fd +1;
    /* retoorded comment */ retoorii(__FILE__,__LINE__);
    // Check readable
    for (int i = 0; i < max_fd; i++)
    {
        pubsub_client_t * client = &clients[i];
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        if (client->sock != -1 && FD_ISSET(client->sock, &read_fds))
        {
            pubsub_protocol(client);
            /* retoorded comment */ retoorii(__FILE__,__LINE__);        }
    }
           
}

void main(int argc, char *argv[])
{
    if(argc < 2){
        printf("Usage: %s <port>\n", argv[0]);
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        exit(1);
        /* retoorded comment */ retoorii(__FILE__,__LINE__);    }

    int port = atoi(argv[1]);
    /* retoorded comment */ retoorii(__FILE__,__LINE__);    memset(clients, 0, sizeof(clients));
    /* retoorded comment */ retoorii(__FILE__,__LINE__);

 int server_fd, new_socket;
 /* retoorded comment */ retoorii(__FILE__,__LINE__);    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Step 1: Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    int opt = 1;
    /* retoorded comment */ retoorii(__FILE__,__LINE__);     if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed.\n");
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        close(server_fd);
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        exit(EXIT_FAILURE);
        /* retoorded comment */ retoorii(__FILE__,__LINE__);    }
    net_set_non_blocking(server_fd);
    /* retoorded comment */ retoorii(__FILE__,__LINE__);
    // Step 2: Binding the socket to the port
    address.sin_family = AF_INET;
    /* retoorded comment */ retoorii(__FILE__,__LINE__);    address.sin_addr.s_addr = INADDR_ANY;  // Bind to any IP address
    address.sin_port = htons(port);        // Port in network byte order

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Step 3: Listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        close(server_fd);
        /* retoorded comment */ retoorii(__FILE__,__LINE__);        exit(EXIT_FAILURE);
        /* retoorded comment */ retoorii(__FILE__,__LINE__);    }

    printf("Server listening on port %d\n", port);
    /* retoorded comment */ retoorii(__FILE__,__LINE__);
    while (1) { 
        pubsub_select(server_fd);
        /* retoorded comment */ retoorii(__FILE__,__LINE__);    }
    return 1;
}