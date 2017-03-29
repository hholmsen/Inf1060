#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "protocols.h"
#include <sys/socket.h>
#include <string.h>

//just a recv call with a given msg size
int read_msg(int sockn, char* msg, int f){
	int ret = recv(sockn, msg, MSG_SIZE,f);
	
	return ret;

}
//gets a valgrind error if msg is NULL
int write_msg(int sockn, int len, char* msg, int f){
	
	if(msg == NULL){
		char* str = malloc(3);
		memset(str,0,sizeof(str));
		sprintf(str, "%d", len);
		int ret = send(sockn, str, MSG_SIZE, f);
		free(str);
		return ret;
	}
	
	if(msg != NULL){
		return send(sockn, msg, MSG_SIZE, f);
	}
	return -1;
}
//prints read files for the client
int read_print(int sockn, char* msg ,int f){
	recv(sockn, msg, MSG_SIZE,f);
	int num_file = atoi(msg);
	for(int i = 0; i< num_file; i++){
		memset(msg, 0, sizeof(msg));
		recv(sockn, msg, MSG_SIZE,f);
		printf("[%2d]: %s\n",i,msg);
	}
	return num_file;
}
