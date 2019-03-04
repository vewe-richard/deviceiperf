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

struct MyCommand{
    int socket;
};

struct MyCommand gCommand = {-1,};

void cmdconnect(char *ipaddr);
void sendcommand(char *cmd);


void run(char * command)
{
    if(strstr(command, "connect") == command)
    {
        char * pc = command + strlen("connect") + 1;
        cmdconnect(pc);
    }
    else if(strcmp(command, "exit") == 0)
    {
        if(gCommand.socket > 0)
        {
            shutdown(gCommand.socket, SHUT_RD);
            close(gCommand.socket);
        }
        exit(0);
    }
    else
    {
        sendcommand(command);
    }

}

int main(int argc, const char * argv[])
{
    char str[100];
    while(1)
    {
        usleep(10000);
        printf("enter command:");
        gets(str);
        run(str);
    }
    return 0;
}

void sendcommand(char *cmd)
{
    int ret;

    if(gCommand.socket < 0)
    {
        printf("please connect first, command send failed\n");
        return;
    }

    ret = write(gCommand.socket, cmd, strlen(cmd));
    if(ret != strlen(cmd))
    {
        printf("Warning: send command failed\n");
    }
    printf("send command done\n");
}

void * threadFunc(void * arg)
{
    int n;
    char dataReceived[1024];

    while((n = read(gCommand.socket, dataReceived, sizeof(dataReceived)-1)) > 0)
    {
        dataReceived[n] = 0;
        printf("MYLOG: %s\n", dataReceived);
    }
}



void cmdconnect(char *ipaddr)
{
    struct sockaddr_in ipOfServer;
    int err;
    pthread_t threadId;

    if((gCommand.socket = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        printf("Error: Socket not created \n");
        return;
    }

    ipOfServer.sin_family = AF_INET;
    ipOfServer.sin_port = htons(2017);
    ipOfServer.sin_addr.s_addr = inet_addr(ipaddr);

    if(connect(gCommand.socket, (struct sockaddr *)&ipOfServer, sizeof(ipOfServer))<0)
    {
        printf("Error: Connection failed due to port and ip problems\n");
        return;
    }
    printf("connect done\n");

    err = pthread_create(&threadId, NULL, &threadFunc, NULL);
    if (err)
    {
        printf("Error: create thread\n");
    }
    else
    {
        printf("create log thread done\n");
    }
}

