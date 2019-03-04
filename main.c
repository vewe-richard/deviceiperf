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
#include <stdarg.h>
#include "common.h"

struct DeviceIPerf{
    int socket;
};

struct DeviceIPerf dip = {-1,}, *gDip = &dip;
void commandproc(char *command);

void mylog(const char* format, ...) { 
    va_list vl; 
    char buf[1024];

    va_start(vl, format); 
    vsprintf(buf, format, vl); 
    va_end(vl); 

    fprintf(stdout, "%s\n", buf);

    if(gDip->socket < 0) return;

    write(gDip->socket, buf, strlen(buf));
}

int main(int argc, const char * argv[])
{
    int clintListn = 0, clintConnt = 0;
    struct sockaddr_in ipOfServer;

    clintListn = socket(AF_INET, SOCK_STREAM, 0); // creating socket
    memset(&ipOfServer, '0', sizeof(ipOfServer));
    ipOfServer.sin_family = AF_INET;
    ipOfServer.sin_addr.s_addr = htonl(INADDR_ANY);
    ipOfServer.sin_port = htons(2017);              // this is the port number of running server
    bind(clintListn, (struct sockaddr*)&ipOfServer , sizeof(ipOfServer));
    listen(clintListn , 20);
     
    while(1)
    {
        int n;
        char dataReceived[1024];
        
        printf("listening ...\n");
        clintConnt = accept(clintListn, (struct sockaddr*)NULL, NULL);
        printf("accept one connection %d\n", clintConnt);
        gDip->socket = clintConnt;
        while((n = read(clintConnt, dataReceived, sizeof(dataReceived)-1)) > 0)
        {
            dataReceived[n] = 0;
            mylog("receive command: %s", dataReceived);
            commandproc(dataReceived);
        }
        shutdown(clintConnt, SHUT_RD);
        close(clintConnt);
        gDip->socket = -1;
    }
    return 0;
}

void commandproc(char *command)
{
    if(strstr(command, "server"))
    {
        server(command + strlen("server"));
    }
    else if(strstr(command, "client"))
    {
        client(command + strlen("client"));
    }
}









