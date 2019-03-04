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
#include "common.h"

struct Client{
    char ip[30];
    int ID;
    pthread_t threadId;
    int started;
    struct Client *next;
    int socket;
};

struct GlobalSetting{
    struct Client *pclients;
    int gid;
};

struct GlobalSetting gs = {NULL, 0}, *gS = &gs;
static void * threadFunc(void * arg);

void dumpclients(void)
{
    struct Client *p;
    p = gS->pclients;
    mylog("Client: dump clients\n");
    while(p)
    {
        mylog("%d: %s, status %d\n", p->ID, p->ip, p->started);
        p = p->next;
    }
}

void addclient(struct Client *cli)
{
    struct Client *p;

    if(gS->pclients == NULL)
    {
        gS->pclients = cli;
        cli->next = NULL;
        dumpclients();
        return;
    }
    p = gS->pclients;
    while(p->next)
    {
        p = p->next;
    }
    p->next = cli;
    cli->next = NULL;
    dumpclients();
}

void removeclient(char *idstr)
{
    int id = atoi(idstr);
    struct Client *p, *prev;

    p = gS->pclients;
    prev = p;
    while(p)
    {
        if(p->ID == id)
        {
            if(p == gS->pclients)
            {
                gS->pclients = p->next;
            }
            else
            {
                prev->next = p->next;
            }
            if(p->socket > 0)
            {
                shutdown(p->socket, SHUT_RD);
                close(p->socket);
            }
            free(p);
            dumpclients();
            return;
        }
        prev = p;
        p = p->next;
    }
}

void startclient(char *ip)
{
    struct Client *cli;
    int err;

    cli = (struct Client *)malloc(sizeof(struct Client));
    strcpy(cli->ip, ip);
    cli->started = 1;
    cli->ID = gS->gid;
    gS->gid += 1;
    cli->socket = -1;

    err = pthread_create(&cli->threadId, NULL, &threadFunc, (void *)cli);
    if (err)
    {
        mylog("Client> start failed\n");
        return;
    }


    addclient(cli);

}

void stopclient(char *id)
{
    removeclient(id);
}

void client(char *cmd)
{
    if(strstr(cmd, "start"))
    {
        startclient(cmd + strlen("start") + 2);
    }
    else if(strstr(cmd, "stop"))
    {
        stopclient(cmd + strlen("stop") + 2);
    }
    else
    {
        mylog("Unknown client command\n");
    }
}

static void * threadFunc(void * arg)
{
    struct Client *cli = (struct Client *)arg;

    int n = 0;
    char dataReceived[2048];
    struct sockaddr_in ipOfServer;
    struct timeval prevtv, tv;
    struct timezone tz;
    long delta;
    long total;
     
    mylog("Client> enter client to connect %s\n", cli->ip);
    memset(dataReceived, '0' ,sizeof(dataReceived));
     
    if((cli->socket = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        mylog("Client> Socket not created \n");
        return;
    }

    ipOfServer.sin_family = AF_INET;
    ipOfServer.sin_port = htons(2016);
    ipOfServer.sin_addr.s_addr = inet_addr(cli->ip);

    if(connect(cli->socket, (struct sockaddr *)&ipOfServer, sizeof(ipOfServer))<0)
    {
        mylog("Client> Connection failed due to port and ip problems\n");
        return;
    }

    gettimeofday(&prevtv, &tz);
    total = 0;
    while((n = read(cli->socket, dataReceived, sizeof(dataReceived)-1)) > 0)
    {
        dataReceived[n] = 0;
        gettimeofday(&tv, &tz);
        delta = (tv.tv_sec-prevtv.tv_sec)*1000000 + tv.tv_usec-prevtv.tv_usec;
//        mylog("%d: %s", n, dataReceived);
        total += n;
        if(delta > 1000000){
            mylog("Client %d> delta: %ld, bs: %fKbps", cli->ID, delta, ((total * 8)*1000.0*1000.0/1024/delta));
            prevtv = tv;
            total = 0;
        }
    }

    if( n < 0)
    {
        mylog("Client> Standard input error \n");
    }
}


