#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>

#include "protocols.h"
int sock;
char buffer[MSG_SIZE];

//prints usage
void print_help(){
	printf("press one of the following keys:\n[1] \"ls\"\n[2] \"pwd\"\n");
	printf("[3] \"cd\"\n[4] \"file info\"\n[5] \"cat\"\n[q] \"quit\"\n");
}
//getting file info from server
void client_file_info(){
	char* io = malloc(3);
	write_msg(sock,4,NULL ,0);
	memset(buffer, 0, MSG_SIZE);
	int num_file = read_print(sock, buffer,0);
	printf("Which file do you want info about? ");
	io = fgets(io, 20, stdin);
	if(isdigit(io[0]) && atoi(io)<num_file){
		write_msg(sock, atoi(io), NULL, 0);
		memset(buffer,0,sizeof(buffer));
		read_msg(sock, buffer, 0);
		printf("%s \n",buffer);
	}else{
		write_msg(sock, -1,NULL,0);
		printf("invalid value\n");
	} 
	free(io);
}
//performs ls on the client side
void client_ls(){
	write_msg(sock,1,NULL ,0);
	memset(buffer, 0 ,sizeof(buffer));
	read_print(sock, buffer,0);
}
//performs cd on the client side
void client_cd(){
	char* io = malloc(3);
	write_msg(sock,3,NULL ,0);
	printf("[0] Up a directory\n[1] New directory(in current folder)\n[q] to quit this menu\nWhich option? ");
	io = memset(io, 0, sizeof(io));
	io = fgets(io, 20, stdin);
	if(strcmp(io, "0\n") == 0){ 
		write_msg(sock, 1, NULL,0);
		memset(buffer,0,MSG_SIZE);
		read_msg(sock, buffer, 0);
		if(strcmp(buffer,"1")==0){
			printf("already at top dir");
		}else{ 
			printf("current dir: %s\n",buffer);
		}
	}else if(strcmp(io, "1\n") == 0){ 
		write_msg(sock, 2, NULL, 0);
		memset(buffer, 0, MSG_SIZE);
		int num = read_print(sock, buffer,0);
		printf("which dir? ");
		memset(io, 0, sizeof(io));
		io = fgets(io, 20, stdin);
		if(isdigit(io[0]) && atoi(io)<num){
			write_msg(sock, atoi(io),NULL,0);
			memset(buffer,0, MSG_SIZE);
			read_msg(sock, buffer,0);
			printf("current dir: %s", buffer);
		}else printf("invalid value\n"); write_msg(sock, -1, NULL, 0);
	}else{
		write_msg(sock, -1, NULL, 0);
	}
	free(io);
}
//cat on clientside
void client_cat(){
	char* io = malloc(3);
	write_msg(sock, 5, NULL, 0);
	memset(buffer, 0, MSG_SIZE);
	read_msg(sock, buffer, 0);
	int num_file = atoi(buffer);
	for(int i = 0; i<num_file;i++){
		memset(buffer, 0, MSG_SIZE);
		read_msg(sock,buffer,0);
		printf("[%2d] %s\n", i+1, buffer);
	}
	printf("which file do you want to read? ");
	memset(io, 0, sizeof(io));
	io = fgets(io,20,stdin);
	if(isdigit(io[0]) && atoi(io) <= num_file){
		write_msg(sock, atoi(io), NULL, 1);
		memset(buffer, 0, MSG_SIZE);
		read_msg(sock, buffer, 0);	
		num_file = atoi(buffer);
		for(int i = 0; i<num_file;i++){
			memset(buffer, 0, MSG_SIZE);
			read_msg(sock,buffer,0);
			for(int j = 0; j<MSG_SIZE; j++){
				if(j!=MSG_SIZE)printf("%c",buffer[j]);
			}
		}
	}else{
		write_msg(sock, -1, NULL, 1);
		memset(buffer,0,MSG_SIZE);
		read_msg(sock, buffer, 0);
		printf("%s\n",buffer);
	}
	free(io);
}
//client main
int main(int argc, char* argv[]){
	if(argc < 3){
		printf("1st argument: <address(machine)>\n2. argument <port>\n");
		return 0;
	}
	int socket_fd;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo* result;
	int ret = getaddrinfo(argv[1], argv[2], &hints, &result);
	if (ret == -1) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
		exit(EXIT_FAILURE);
	}

	struct addrinfo* rp;
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (socket_fd == -1) {
			fprintf (stderr, "socket: %s\n", strerror(errno));
			exit(-1);
		}
		if (connect(socket_fd, rp->ai_addr, rp-> ai_addrlen) != -1) break; // Success
		else close(socket_fd); // Fail
	}
	freeaddrinfo(result);

	fprintf(stderr, "Connected to server\n");
	sock = socket_fd;
	char* io = malloc(3);
	while(1){
		printf("(? for help)->");
		io = fgets(io, 20, stdin);
		if(strcmp(io,"q\n") == 0){
			write_msg(sock,9,NULL,0);
			memset(buffer, 0,MSG_SIZE);
			read_msg(sock, buffer, 0);
			fprintf(stderr, "%s\n", buffer);
			break;
		}else if(strcmp(io,"?\n") == 0){
			print_help();
		}else if(strcmp(io,"1\n") == 0){//ls
			client_ls();
		}else if(strcmp(io,"2\n") == 0){//pwd
			write_msg(sock,2,NULL ,0);
			read_msg(sock, buffer, 0);
			fprintf(stderr, "%s", buffer);
		}else if(strcmp(io,"3\n") == 0){//cd
			client_cd();			
		}else if(strcmp(io,"4\n") == 0){//fileinfo
			client_file_info();
		}else if(strcmp(io,"5\n") == 0){//cat
			client_cat();
		}
		printf("\n");
	}
	free(io);
	close(sock);
	return 0;
}