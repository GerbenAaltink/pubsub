#ifndef PUBSUBC_H
#define PUBSUBC_H

#ifndef PUBSUBC_DEBUG
unsigned int pubsubc_debug = 0;
#else 
unsigned int pubsubc_debug = 1;
#endif

#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>

typedef enum pubsub_message_type_t
{
    PUBSUB_UNKNOWN = 0,
    PUBSUB_PUBLISH = 10,
    PUBSUB_SUBSCRIBE = 20,
    PUBSUB_UNSUBSCRIBE = 30
} pubsub_message_type_t;

size_t pubsub_escape(const unsigned char * data, unsigned char * result, size_t length){
    result[0] = 0;
    size_t pos = 0;
    size_t new_size = 0;
    while(length != pos){
        if(*data == '\n'){
            *result = '\\';
            result++;
            new_size++;
        }else if(*data ==  '\\'){
            *result = '\\';
            result++;
            new_size++;
        }
        new_size++;
        *result = *data ;
        result++;
        data++;
        pos++;
    }
}
size_t pubsub_unescape(const unsigned char * data, unsigned char * result, size_t length){
    result[0] = 0;
    size_t pos = 0;
    size_t new_length = length;
    while(length != pos){
        if(*data == '\\'){
            data++;
            new_length--;
            //result++;
        }
        
        *result = *data ;
        result++;
        data++;
        pos++;
    }
    return new_length;
}

int pubsub_connect(const char *host, unsigned int port)
{

    char port_str[10] = {0};
    sprintf(port_str, "%d", port);
    int status;
    int socket_fd = -1;
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *p;
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return false;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(host, port_str, &hints, &res)) != 0)
    {
        return -1;
    }

    for (p = res; p != NULL; p = p->ai_next)
    {
        if ((socket_fd =
                 socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            continue;
        }

        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(socket_fd);
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);
    return socket_fd;
}

bool pubsub_ack(int s){
    char data[3];
    read(s,data,3);

    bool result = data[0] == '+';
    if(result == false){
        printf(data);
    }
    return result;
}
ssize_t pubsub_write(int s, unsigned char * data, size_t length){
    char * escaped = (char *)malloc(length * 2);
    size_t new_length = pubsub_escape(data,escaped,length);
    if(write(s,data,new_length) > 0){
        free(escaped);
        bool result = pubsub_ack(s);
        if(!result){
            printf("ERROR\n");
        }else{
            printf("SUCCES\n");
        }
        return result;
    }
    return false;
}

ssize_t pubsub_name(int s, unsigned char *name)
{
    char bytes[strlen(name) + 10];
    bytes[0] = 0;
    sprintf(bytes,"n:%s\n",name);
    pubsub_write(s,(unsigned char *)bytes,strlen(bytes));
    
        return true; 
}
bool pubsub_publish(int s, char *channel, unsigned char *message, size_t length)
{

    char bytes[strlen(channel)  + strlen(message)+ 20];
    memset(bytes,0,sizeof(bytes));
    sprintf(bytes,"p:%s\n",channel);
    pubsub_write(s,(unsigned char *)bytes,strlen(bytes)+1);
    
    memset(bytes,0,sizeof(bytes));  
    sprintf(bytes,"m:%s\n",message);
      pubsub_write(s,(unsigned char *)bytes,strlen(bytes)+1);
    return true;
}
bool pubsub_subscribe(int s, char *channel)
{
    char bytes[strlen(channel) + 10];
    bytes[0] = 0;
    sprintf(bytes,"s:%s\n",channel);
    pubsub_write(s,(unsigned char *)bytes,strlen(bytes) + 1);
    return true;
}
ssize_t pubsub_read(int s, unsigned char *buffer, size_t buffer_size)
{
    unsigned char * subbuffer = (unsigned char *)malloc(buffer_size);

    ssize_t bytes_read = read(s, subbuffer, buffer_size);
    if(bytes_read < 1)
    {
        free(subbuffer);
        return bytes_read;
    }
    ssize_t new_size = pubsub_unescape(subbuffer,buffer,bytes_read);
    buffer[new_size] = 0;
    free(subbuffer);
    return new_size;
}
ssize_t pubsub_read_raw(int s,unsigned char * buffer,size_t buffer_size){
    char subbuffer[buffer_size];
    size_t new_buffer_size = pubsub_read(s,subbuffer,buffer_size) - 3;
    for(unsigned int i = 2; i < buffer_size - 3; i++){
        buffer[i-2] = subbuffer[i];
    }
    buffer[buffer_size - 3] = 0;
    return new_buffer_size;
}


pubsub_message_type_t pubsub_message_type(char *data)
{
    if (data[0] == 'p')
        return PUBSUB_PUBLISH;
    if (data[0] == 's')
        return PUBSUB_SUBSCRIBE;
    if (data[0] == 'u')
        return PUBSUB_UNSUBSCRIBE;
    return PUBSUB_UNKNOWN;
}
#endif