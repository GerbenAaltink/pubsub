#ifdef PUBSUB_DEBUG
unsigned pubsub_debug = 1
#else 
unsigned pubsub_debug = 0
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
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>
#include "pubsubs.h"
#include "pubsubc.h"
#endif
