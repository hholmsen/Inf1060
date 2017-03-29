#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <dirent.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>

#include "protocols.h"

/*
KANDIDATNR: 182 
*/
char buf[PATH_MAX];
char buf2[PATH_MAX];
char* current_dir = buf; //the working directory of the machine...
char* current_visible_dir = buf2;//the directory currently visible to the client
int name; //fd
//sets path and visible path, and makes a unique dir for the client
void init_client(){
	getcwd(current_dir, PATH_MAX);
	buf2[0] = '.';
	buf2[1] = '/';
	/*char dir[PATH_MAX];
	strcpy(dir, current_dir);
	strcat(dir, "/");
	strcat(dir, "Client");
	char str[3];
	memset(str,0,sizeof(str));
	sprintf(str, "%d", name);
	strcat(dir, str);
	mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);*/
	//i dont't set this directory as the default directory
	//because it would be very difficult to test the program
	//is not performed(commented out) because this was not necessary accordig to a piazza post
	
}
int is_dir(char* file);
//first count the files in the directory, sends number of files to client
//then sends each file to the client
int server_ls(int type){
	int num_file = 0;
	DIR* dp;
	struct dirent* ep; 
	dp = opendir(current_dir);
	int i = 0;
	if(dp != NULL){
		while((ep = readdir(dp))){
			if(type == 0 ){
				num_file++;
			}else if(is_dir(ep->d_name)&&type==1 && i>2){
				num_file++;
			}else if(!is_dir(ep->d_name)&&type == 2){
				num_file++;
			}i++;
		}
	}i=0;
	if(closedir(dp) != 0){
		fprintf(stderr, "error on closedir\n");
	}
	struct dirent* e;
	DIR* d;
	write_msg(name, num_file,NULL, 0);
	d = opendir(current_dir);
	if(d != NULL){	
		while((e = readdir(d))){
			if(type == 0){
				write_msg(name, 0, e->d_name, 0);
			}else if(is_dir(e->d_name)&&type == 1&&i>2){
				write_msg(name, 0, e->d_name, 0);
			}else if(!is_dir(e->d_name)&&type==2){
				write_msg(name, 0, e->d_name, 0);
			}i++;
		}
	}
	if(closedir(d) != 0){
		fprintf(stderr, "error on closedir\n");
	}
	return num_file;
}
//calculates the length of a file, then sends it one message at a time containig part of the file
void server_cat(){
	server_ls(2);
	char read_buf[3];
	memset(read_buf, 0, sizeof(read_buf));
	read_msg(name, read_buf, 0);
	if(atoi(read_buf) >= 0){
		int filen = atoi(read_buf);
		int file_nr =0;
		DIR* dp;
		struct dirent* ep; 
		dp = opendir(current_dir);
		if(dp != NULL){
			while((ep = readdir(dp))){
				if(!is_dir(ep->d_name)) {file_nr++;}
				if(file_nr == filen) {break;}
			}
		}
		char msg[MSG_SIZE];
		FILE* file;
		char file_name[PATH_MAX];
		memset(file_name, 0,sizeof(file_name));
		strcat(file_name, current_dir);
		strcat(file_name,"/");
		strcat(file_name,ep->d_name);
		if(closedir(dp) != 0){
			fprintf(stderr, "error closing dir\n");
		}
		if((file = fopen(file_name, "r")) == NULL){
			perror("fopen");
		}
		if(file != NULL){
			int i =0;
			int num_char =0;
			char ch;
			while((ch = fgetc(file)) != EOF){
				num_char++;
			}
			int num_pck = ((num_char)/(MSG_SIZE))+1;
			write_msg(name,num_pck, NULL,0); 
			ch = 0;
			i = 0;
			fclose(file);
			file = fopen(file_name, "r");
			
			while((ch = fgetc(file)) != EOF){
				if(isascii(ch)){//printable
					msg[i] = ch;
				}else{
					msg[i] ='.';
				}i++;
				if(i == MSG_SIZE){
					write_msg(name, 0,msg, 0);
					for(int g = 0; g<MSG_SIZE; g++) msg[g] =0;
					i=0;
				}
			}
			write_msg(name, 0,msg,0);
			if(fclose(file) != 0){
				fprintf(stderr, "error on closefile\n");
			}
		}else{
			write_msg(name, 0, "nope",0);
			fprintf(stderr, "FILENOTFOUND!\n");
		}
	}else{
		write_msg(name, 0, "invalid input\n",0);
	}
}
//writes current_visible_dir 
void server_pwd(){
	write_msg(name, 0, current_visible_dir,0);
}
//goes up an directory more or less...
void remove_last_dir(){
	int last_f_slash;
	for(int i = 0; i<PATH_MAX; i++){
		if(current_dir[i] == '/'){
			last_f_slash = i;
		}
	}
	for(int i = last_f_slash; i< PATH_MAX; i++){
		current_dir[i] = 0;
	}
	int num_f = 0;
	for(int i = 0; i<PATH_MAX; i++){
		if(current_visible_dir[i] == '/'){
			last_f_slash = i+1;
			num_f++;
		}
	}
	if(num_f > 1)last_f_slash--;
	for(int i = last_f_slash; i<PATH_MAX; i++){
		current_visible_dir[i] =0;
	}
}
//check if a file is a dir
int is_dir(char* file){
	char* copy = current_dir;
	char buffer[PATH_MAX];
	memset(buffer, 0, PATH_MAX);
	strcat(buffer,copy);
	strcat(buffer, "/");
	strcat(buffer, file);
	struct stat sb;
	if(stat(buffer, &sb) == -1){
		perror("stat");
	}
	if(S_ISDIR(sb.st_mode)){
		return 1;
	}
	return 0;
}
//performs the cd command goes either up or down in the directories but not above "./"
void server_cd(){ 
	char val[2];
	memset(val, 0, 2);
	read_msg(name, val, 0);
	if(strcmp(val, "1")==0){
		if(strcmp(current_visible_dir, "./") == 0){
			write_msg(name, 1,NULL, 0);//is at top dir
		}else{
			remove_last_dir();
			write_msg(name,0 , current_visible_dir,0);
		}
	}
	else if(strcmp(val, "2")==0){
		server_ls(1);
		char which[3];
		memset(which,0,sizeof(which));
		read_msg(name, which, 0);
		if(atoi(which) != -1){
			DIR* de;
			struct dirent* ee; 
			de = opendir(current_dir);
			if(de != NULL){
				int i = 0;
				int j = 0;
				while((ee = readdir(de))){
					if(is_dir(ee->d_name)){
						if(i==atoi(which)&&j>=2){//sets the new directory
							fprintf(stderr, "changed dir: %s\n", ee->d_name);
							char buffer_one[PATH_MAX] ;
							char buffer_two[PATH_MAX] ;
							
							memset(buffer_one, 0, PATH_MAX);
							memset(buffer_two, 0, PATH_MAX);

							for(int j=0; j<PATH_MAX;j++){
								buffer_one[j] = buf[j];
								buffer_two[j] = buf2[j];
							}
							if(strcmp(current_visible_dir, "./") != 0){
								strcat(buffer_two, "/");
							}
							strcat(buffer_one, "/");
							strcat(buffer_one, ee->d_name);
							strcat(buffer_two, ee->d_name);
							for (int j = 0; j<PATH_MAX;j++){
								buf[j]=buffer_one[j];
								buf2[j]=buffer_two[j];
							}
							fprintf(stderr, "%s\n",current_dir);
							fprintf(stderr, "%s\n",current_visible_dir);
							write_msg(name, 0, current_visible_dir, 0);

							break;
						}j++;
						if(j>2){
							i++;
						}
					}
				}
			}
			if(closedir(de) != 0){
				fprintf(stderr, "error closing dir\n");
			}
		}
	}else{
		fprintf(stderr,"false input\n");
	}
}
//finds a file and return the stats about the file
void file_info(){
	int num_file =server_ls(0);
	
	char bufer[3];
	read_msg(name, bufer, 0);
	int file = atoi(bufer);
	if(file != -1){
		struct dirent* e; 
		DIR* d;
		char str[3];
		sprintf(str, "%d",num_file);
		d = opendir(current_dir);
		int i = 0;
		if(d != NULL){	
			while((e = readdir(d))){
				if(file == i){
					struct stat sb;
					char buffer[PATH_MAX];
					memset(buffer, 0, PATH_MAX);
					strcat(buffer, current_dir);
					strcat(buffer, "/");
					strcat(buffer, e->d_name);
					if(lstat(buffer, &sb) == -1){
						write_msg(name, 0,"no such file!\n",0);
						perror("lstat");
					}
					char* send = malloc(sizeof(e->d_name)+13);
					memset(send, 0, sizeof(send));
					strcat(send, e->d_name);
					strcat(send, " is ");
					
					if(S_ISDIR(sb.st_mode)) strcat(send, "directory");
					else if(S_ISLNK(sb.st_mode))strcat(send, "link");
					else if(S_ISREG(sb.st_mode)) strcat(send, "regular");
					else strcat(send, "special");
					
					write_msg(name, 0, send, 0);
					free(send); 
					break;
				}
				i++;
			}
		}
		closedir(d);
	}
}
//rick handles the zombies waits on signal from terminated children
void rick(int sig){
	while(waitpid((pid_t)(-1), 0, WNOHANG)>0){}
	fprintf(stderr, "The zombie is terminated!\n");
}
//main function
int main(int argc, char* argv[]){
	if(argc < 2){
		printf("USAGE: \n");
		printf("\t first argument: listening port\n");
		printf("\t no second argument!\n");
		exit(-1);
	}
	int listenfd;
	if((listenfd = socket(AF_INET, SOCK_STREAM,0)) < 0){
		perror("socket");
		return -1;
	}
	struct sockaddr_in sa, ca; 
	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons(atoi(argv[1]));
	
	int yes = 1; 
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))<0){
		perror("setsockopt");
		return -1;
	}
	if(bind(listenfd, (struct sockaddr*)&sa, sizeof(struct sockaddr_in)) == -1){
		perror("bind");
		return -1;
	}
	if(listen(listenfd ,0)<0){
		perror("listen");
		return -1;
	}
	fprintf(stderr, "Listening on port %s\n", argv[1]);

	while(1){
		//to handle zombies!!
		struct sigaction sact;
		sact.sa_handler = &rick;
		sigemptyset(&sact.sa_mask);
		sact.sa_flags = SA_RESTART | SA_NOCLDSTOP;
		if(sigaction(SIGCHLD, &sact,0) == -1){
			perror(0);
			exit(1);
		}
		//done handeling zombies :)
		socklen_t sl = sizeof(struct sockaddr_in);
		int acc =accept(listenfd, (struct sockaddr* )&ca, &sl);
		if (acc == -1){
			perror("accept");
			return -1;
		}
		pid_t pid = fork();
		if(pid == -1){
			perror("fork");
			return -1;
		}else if(pid == 0){//client
			char bufer[2];
			int close = 0;
			fprintf(stderr, "Client connected: %d\n", acc);
			init_client();
			while(1){
				clock_t start = clock(), diff; //to account for sudden disconnects... so that the server ends and we dont't get an orphan
				name = acc;
				read_msg(acc, bufer, 0);
				if(strcmp("9",bufer) == 0){
					write_msg(acc, 0,"Disconnecting from server", 0);
					fprintf(stderr, "%d, disconnecting\n",acc);
					exit(1);
				}else if(strcmp("1",bufer) == 0){
					server_ls(0);
				}else if(strcmp("2",bufer) == 0){
					server_pwd();
				}else if(strcmp("3",bufer) == 0){
					server_cd();
				}else if(strcmp("4",bufer) == 0){
					file_info();
				}else if(strcmp("5",bufer) == 0){
					server_cat();
				}
				diff = clock() -start; //calculates time spent in whileloop
				int msec = diff * 1000 /CLOCKS_PER_SEC;
				if(msec<1 && close == 200){//solution for handeling orphans hen CTRL+C is pressed or another runtime error
					fprintf(stderr, "closing: %d\n", acc);
					exit(1);
				}if(close == 200){
					close = 0;
				}
				close++;
			}
		}else{
			close(acc);
		}
	}
}