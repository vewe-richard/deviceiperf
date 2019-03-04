#ifndef __COMMON_H_
#define __COMMON_H_

void mylog(const char* format, ...); 
void server(char *cmd);
void client(char *cmd);

#define PACKET_LEN      1600
#endif
