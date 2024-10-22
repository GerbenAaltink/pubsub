#include "pubsubc.h"
#include <stdbool.h>


int main() {

    int s = pubsub_connect("127.0.0.1",9999);

    char number[100];
    unsigned int inc = 0;
    while(true){
        number[0] = 0;
        sprintf(number,"%u",inc);
        inc++;

     //  pubsub_subscribe(s,"aaa");
       pubsub_publish(s,"aaa",number,strlen(number));
        
        char data[1024];
       // pubsub_read(s2,data,sizeof(data));
        
       // ssize_t bytes_read = pubsub_read(s,data,sizeof(data));
        //printf("%s\n",data);


    }

}
