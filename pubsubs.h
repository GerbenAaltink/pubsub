#ifndef PUBSUBS_H
#define PUBSUBS_H
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
#include <stdbool.h>
#include <ctype.h>
#include <signal.h>


#ifndef PUBSUBS_DEBUG
unsigned int pubsubs_debug = 0;
#else 
unsigned int pubsubs_debug = 1;
#endif

unsigned int pubsub_client_id = 0;
int max_fd = -1;

typedef struct pubsub_client_t
{
    char *description;
    unsigned int id;
    char *name;
    char *write_to;
    ssize_t write_to_length;
    char *publish_message;
    ssize_t publish_message_length;
    char *publish_message_safe;
    char *read_from;
    ssize_t read_from_length;
    int sock;
} pubsub_client_t;

void net_set_non_blocking(int sock)
{
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0)
    {
        perror("fcntl");
        // return false;
    }

    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        perror("fcntl");
        // return false;
    }

    // return true;
}

pubsub_client_t *pubsub_client_new()
{
    pubsub_client_id++;
    pubsub_client_t *client = malloc(sizeof(pubsub_client_t));
    client->id = pubsub_client_id;
    client->sock = -1;
    client->name = NULL;
    client->description = NULL;
    return client;
}

pubsub_client_t clients[500];

char *strchr_binary(char *data, char c)
{
    char *ptr = data;
    while (*ptr != c)
        ptr++;
    return ptr;
}

bool is_human_safe(char c)
{
    if (isalnum(c))
        return true;
    if (isdigit(c))
        return true;
    if (strchr(".-+/*!@#$%%^&^_ ", c))
        return true;
    return false;
}

char *human_safe(char *data, unsigned int length, size_t size_limit)
{
    char *result = (char *)calloc(size_limit * 2 + 100, sizeof(char));
    
    
    size_t new_length = 0;
    result[0] = 0;
    for (unsigned int i = 0; i < length && i < size_limit; i++)
    {
        char c = data[i];

        if (c && !is_human_safe(c))
            c = '.';

        result[i] = c;
        new_length++;
        if (!c)
            break;
    }
    result[new_length] = 0;

    strcat(result, " (");
    char bytes_str[40] = {0};
    sprintf(bytes_str, "%lu", size_limit);
    strcat(result, bytes_str);
    strcat(result, " bytes)");
    return result;
}

bool pubsub_protocol(pubsub_client_t *client)
{

    char buffer[1024 * 1024];
    memset(buffer,0,sizeof(buffer));
    ssize_t bytes_read = read(client->sock, buffer, sizeof(buffer));
    if (bytes_read <= 0)
    {
        return false;
    }
    buffer[bytes_read] = 0;

    char *ptr = buffer;
    char channel[1024] = {0};
    while (*ptr)
    {
        if (*ptr == 's' && *(ptr + 1) == ':')
        {
            ptr++;
            ptr++;
            client->read_from = (char *)malloc(1024);
            char *end = strchr(ptr, '\n');
            client->read_from_length = end - ptr;
            memcpy(client->read_from, ptr, client->read_from_length);
            if(pubsubs_debug)
            printf("%s set read channel to %s\n", client->name, client->read_from);
            ptr = end + client->read_from_length;   
            
        }
        else if (*ptr == 'n' && *(ptr + 1) == ':')
        {
            ptr++;
            ptr++;
            char *name_old = client->name;
            char *name_new = (char *)malloc(1024);
            char *end = strchr(ptr, '\n');
            size_t length = end - ptr;
            memcpy(name_new, ptr, length);
            name_new[length] = 0;
            if(pubsubs_debug)
            printf("%s set name to %s\n", name_old, name_new);
            free(name_old);
            client->name = name_new;
            ptr = end + length; // + 1 == 
        }
        /*
        else if(*ptr == 'r' && *(ptr + 1) == ':'){
        ptr++;
        ptr++;
        client->read_from = (char *)malloc(1024);
        char * end = strchr(ptr,'\n');
        client->read_from_length = end - ptr;
        memcpy(client->read_from,ptr, client->read_from_length);
        printf("%s set receive channel to %s\n",client->name,client->read_from);
        ptr = end + client->read_from_length + 1; // + 1 == \n
        }


        else*/
        else if (*ptr == 'p' && *(ptr + 1) == ':')
        {
            ptr++;
            ptr++;
            if(client->write_to)
                free(client->write_to);
            client->write_to = (char *)malloc(1024);
            char *end = strchr(ptr, '\n');
            client->write_to_length = end - ptr;
            memcpy(client->write_to, ptr, client->write_to_length);
            client->write_to[client->write_to_length] = 0;
            if(pubsubs_debug)
            printf("%s set write channel to %s\n", client->name, client->write_to);
            ptr = end + client->write_to_length;
            
        }
        else if (client->write_to && *ptr == 'm' && *(ptr + 1) == ':')
        {
            ptr++;
            ptr++;
            if (client->publish_message){
                free(client->publish_message);
                free(client->publish_message_safe);
            }
            char *end = strchr_binary(ptr, '\n');
            client->publish_message_length = end - ptr;
            client->publish_message = (char *)malloc(client->publish_message_length + 1);
            
            memcpy(client->publish_message, ptr, client->publish_message_length);
            client->publish_message[client->publish_message_length] = 0;
            
            printf("HIERRR\n");
            client->publish_message_safe = human_safe(client->publish_message, 10, client->publish_message_length);
            
            if(pubsubs_debug)
            printf("%s publishing to %s: %s\n", client->name, client->write_to, client->publish_message_safe);
            


            for(unsigned int i = 0; i < max_fd; i++){
                if(clients[i].id < 1)
                    continue;
                if(!clients[i].read_from)
                    continue; 
                if(clients[i].sock == client->sock)
                    continue;

                if(!strcmp(clients[i].read_from,client->write_to)){
                    if(write(clients[i].sock,buffer,bytes_read) != bytes_read){
                        printf("ERROR\n");
                    }
                }
            }
         //   stripped_data[bytes_read-1] = '\n';
          
            if(pubsubs_debug)
            printf("%s published to %s: %s\n", client->name, client->write_to, client->publish_message_safe);
            ptr = end + client->publish_message_length;

        }
        else
        {
           
            __attribute__((unused)) ssize_t sent_bytes = write(client->sock,"Error, closing connection.",strlen("Error, closing connection."));
            printf("e:%d\n", *ptr);
            return false;
        }
    }
    
    return write(client->sock,"+",2) > 0;
}

void pubsub_select(int server_fd)
{

    // Prepare for read
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &read_fds);
    int highest_fd = server_fd;
    for (int i = 0; i < max_fd + 1; i++)
    {
        pubsub_client_t *client = &clients[i];
        if (client->sock != -1 && client->sock != 0)
        {
                printf("%d\n",client->sock);
            FD_SET(client->sock, &read_fds);
            if (client->sock > highest_fd)
            {
                highest_fd = client->sock;
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
        return;
    }

    // Accept
    int socket_fd = -1;
    if (FD_ISSET(server_fd, &read_fds))
    {
        if ((socket_fd = accept(server_fd, (struct sockaddr *)&address,
                                (socklen_t *)&addrlen)) < 0)
        {
            perror("Accept failed\n");
            return;
        }
        char name[50] = {0};
        sprintf(name, "fd:%.4d:ip:%12s:port:%.6d", socket_fd,
                inet_ntoa(address.sin_addr), ntohs(address.sin_port));
        printf("%s connected.\n", name);
        pubsub_client_id++;
        clients[socket_fd].id = pubsub_client_id;
        clients[socket_fd].description = NULL;
        clients[socket_fd].write_to = NULL;
        clients[socket_fd].write_to_length = 0;
        clients[socket_fd].publish_message = NULL;
        clients[socket_fd].publish_message_length = 0;
        clients[socket_fd].name = strdup(name);
        clients[socket_fd].sock = socket_fd;
      //  net_set_non_blocking(socket_fd);
        if (socket_fd > highest_fd)
            highest_fd = socket_fd;
    }
    max_fd = highest_fd + 1;

    // Check readable
    for (int i = 0; i < max_fd; i++)
    {
        pubsub_client_t *client = &clients[i];
        if (FD_ISSET(client->sock, &read_fds))
        {
            if (!pubsub_protocol(client))
            {
                printf("%s: disconnected.\n", client->name);
                close(client->sock);
                client->sock = -1;
            }
        }
    }
}

int pubsub_listen(int port){
    signal(SIGPIPE, SIG_IGN); 
    memset(clients, 0, sizeof(clients));

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Step 1: Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("Setsockopt failed.\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    net_set_non_blocking(server_fd);

    // Step 2: Binding the socket to the port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Bind to any IP address
    address.sin_port = htons(port);       // Port in network byte order

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Step 3: Listening for incoming connections
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    } 
    return server_fd;
}

#endif 