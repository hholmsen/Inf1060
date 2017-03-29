#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>


#define MSG_SIZE (sizeof(uint8_t)+7*(sizeof(uint32_t))) //29 bytes

#define S_MSG (sizeof(uint16_t)); //2 bytes

int read_msg(int sockn, char* msg, int f);
int write_msg(int sockn, int len, char* msg, int f);
int read_print(int sockn, char* msg,int f);
