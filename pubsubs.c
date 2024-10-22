#define PUBSUBS_DEBUG
#include "pubsubs.h"

int main(int argc, char *argv[])
{
    setbuf(stdout,NULL);
    if (argc < 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    int server_fd = pubsub_listen(atoi(argv[1]));
    printf("Server listening on port %s\n", argv[1]);

    while (1)
    {
        pubsub_select(server_fd);
    }
    return 1;
}