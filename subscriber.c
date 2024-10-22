#include "pubsubc.h"
#include <stdbool.h>


int main() {

    

    int s2 = pubsub_connect("127.0.0.1",9999);
  //pubsub_name(s2,"aaa");
      pubsub_subscribe(s2,"aaa");
        
    while(true){
        //pubsub_publish(s,"aaa","test",5);
        char data[1024];
        if(pubsub_read_raw(s2,data,sizeof(data)) <= 0){
            exit(1);
        }
       // if(pubsub_message_type(data) == PUBSUB_PUBLISH)

        printf("%s\n",data);
        
       // ssize_t bytes_read = pubsub_read(s,data,sizeof(data));
        //printf("%s\n",data);
    }

//    }

}