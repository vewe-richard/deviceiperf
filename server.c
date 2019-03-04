#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include "common.h"

struct Server{
    int started;
    int listsocket;
};

struct Server svr = {0, -1}, *gSvr = &svr;

static void * threadFunc(void * arg)
{
    long clintConnt;
    unsigned char data[PACKET_LEN];
    int ret;

    clintConnt = (long)arg;

    sprintf(data, "socket id: %ld\n", clintConnt);
    while(gSvr->started)
    {
        ret = write(clintConnt, data, sizeof(data));
        if(ret < 0)
        {
            mylog("Server thread> write failed %d", ret);
            close(clintConnt);
            break;
        }
    }
}

static void * startserver(void * arg)
{
    int clintConnt = 0;
    struct sockaddr_in ipOfServer;

    mylog("startserver\n");
    gSvr->listsocket = socket(AF_INET, SOCK_STREAM, 0); // creating socket
    memset(&ipOfServer, '0', sizeof(ipOfServer));
    ipOfServer.sin_family = AF_INET;
    ipOfServer.sin_addr.s_addr = htonl(INADDR_ANY);
    ipOfServer.sin_port = htons(2016);              // this is the port number of running server
    bind(gSvr->listsocket, (struct sockaddr*)&ipOfServer , sizeof(ipOfServer));
    listen(gSvr->listsocket , 20);
     
    while(gSvr->started)
    {
        int err;
        pthread_t threadId;

        mylog("Server> Waitting for client to call in");
        clintConnt = accept(gSvr->listsocket, (struct sockaddr*)NULL, NULL);
        if(clintConnt < 0)
        {
            mylog("Server> accept failed\n");
            continue;
        }


        err = pthread_create(&threadId, NULL, &threadFunc, (void *)(long)clintConnt);
        if (err)
        {
            mylog("Server> create pthread for client failed\n");
        }
    }
}

void stopserver(void)
{
    mylog("stopserver\n");
    if(gSvr->listsocket > 0)
    {
        close(gSvr->listsocket);
    }
    gSvr->listsocket = -1;
}

void server(char *cmd)
{
    if(strstr(cmd, "start"))
    {
        pthread_t threadId;
        if(gSvr->started)
        {
            mylog("server had already been started\n");
            return;
        }
        gSvr->started = 1;
        
        pthread_create(&threadId, NULL, &startserver, NULL);
    }
    else if(strstr(cmd, "stop"))
    {
        gSvr->started = 0;
        stopserver();
    }
    else
    {
        mylog("Unknown command");
    }
}


