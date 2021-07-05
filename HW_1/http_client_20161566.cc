#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<netdb.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define MAX_INPUT 100
#define MAX_DATA 1024
//purpose: parse input, return 0 if unvalide input
//input: user input(http://~~)
//output: number of parsed elements + parsed result

int parse_input(char *input, char **return_hostname, char **return_port,char **return_filepath){
		int return_value=1;
		int length=strlen(input);
		char *temp;
		char *hostname= *return_hostname;
		char *port = *return_port;
		char *filepath = *return_filepath;
		char http[8]="http://";
		bool is_there_port = true;
		bool is_there_filepath = true;

		//check if it starts with http
		temp = (char*)malloc(sizeof(char)*length);
		strcpy(temp,input);
		temp[7]='\0';	
		if(strcmp(http,temp)){
				free(temp);
				return 0;
		}		
		//start parsing
		strcpy(temp, input+7);

		//is there port number?
		if(!strchr(temp,':')){
				is_there_port=false;
				strcpy(port,"80");
		}
		
		//is there file path?
		if(!strchr(temp,'/')){
				is_there_filepath=false;
				strcpy(filepath,"");
		}

		if(temp[strlen(temp)-1]=='/'){
				is_there_filepath=false;
				temp[strlen(temp)-1]='\0';
				strcpy(filepath,"");
		}

		if(is_there_port && is_there_filepath){ //host + port +filepath
				hostname = strtok(temp,":");
				port = strtok(NULL,"/");
				filepath = strtok(NULL," ");
				return_value=3;
		}
		else if(is_there_port){ // host + port
				hostname=strtok(temp,":");
				port=strtok(NULL," ");
				return_value=2;
		}
		else if(is_there_filepath){ //host + filepath
				hostname = strtok(temp,"/");
				filepath = strtok(NULL," ");
				return_value=2;
		}
		else{ // host
				strcpy(hostname,temp);
				return_value=1;
		}	
		
		strcpy(*return_hostname,hostname);
		strcpy(*return_port,port);
		strcpy(*return_filepath,filepath);
		return return_value;
}



int main(int argc, char *argv[]){
		int sockfd;
		int numbytes;
		int rv;
		int buf_last=0;
		char buf[MAX_DATA];
		char *print_buffer;
		char *temp;
		char *temp2;
		int content_length=0;

		bool header_flag=false;
		bool status_code_flag=false;
		bool content_length_flag=false;
		
		char *hostname=(char*)malloc(sizeof(char)*MAX_INPUT);
		char *port=(char*)malloc(sizeof(char)*MAX_INPUT);
		char *filepath=(char*)malloc(sizeof(char)*MAX_INPUT);

		struct addrinfo hints;
		struct addrinfo *servinfo;
		FILE *fp;

		//check if input is right
		if(argc!=2 || !parse_input(argv[1],&hostname,&port,&filepath)){
				fprintf(stderr,"usage: http_client http://hostname[:port][/path/to/file]\n");
				exit(1);
		}
		//------------------------------------------- connecting!
		memset(&hints,0,sizeof(hints));
		hints.ai_family=AF_UNSPEC;
		hints.ai_socktype=SOCK_STREAM;

		//get the server information using host name(DNS)
		if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0){ 
				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); 
				return 1;
		}
		//setup socket
		if((sockfd=socket(servinfo->ai_family,servinfo->ai_socktype, servinfo->ai_protocol))==-1){
				perror("client: socket");
				return 2;
		}
		//try TCP connection
		if(connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
				close(sockfd);
				perror("connect");
				exit(1);
		}
		//----------------------------------------- connecting finish!


		freeaddrinfo(servinfo);

		//----------------------------------------- creating HTTP request
		//first line
		memset(buf,0,MAX_DATA);
		strcpy(buf+buf_last,"GET");
		buf_last += strlen("GET");
		buf[buf_last++]=' ';
		buf[buf_last++]='/';

		strcpy(buf+buf_last,filepath);
		buf_last += strlen(filepath);
		buf[buf_last++]=' ';

		strcpy(buf+buf_last,"HTTP/1.1");
		buf_last += strlen("HTTP/1.1");
		buf[buf_last++]='\r';
		buf[buf_last++]='\n';
		
		//second line
		strcpy(buf+buf_last,"Host:");
		buf_last += strlen("Host:");
		buf[buf_last++]=' ';

		strcpy(buf+buf_last,hostname);
		buf_last += strlen(hostname);
		buf[buf_last++]=':';

		strcpy(buf+buf_last,port);
		buf_last += strlen(port);
		buf[buf_last++]='\r';
		buf[buf_last++]='\n';
		
		buf[buf_last++]='\r';
		buf[buf_last++]='\n';
		
		//-------------------------- created HTTP request!
				
		if(send(sockfd,buf,strlen(buf),0)==-1){
				perror("send");
				close(sockfd);
				exit(1);
		}
		
		//--------------------------------- receive result
		fp=fopen("20161566.out","w");
		print_buffer=(char*)malloc(sizeof(char)*MAX_DATA);
		temp=(char*)malloc(sizeof(char)*MAX_DATA);
		temp2=(char*)malloc(sizeof(char)*MAX_DATA);
		memset(temp,0,MAX_DATA);
		memset(print_buffer,0,MAX_DATA);
		
		int saved_bytes=0;
		while((numbytes=recv(sockfd,buf,MAX_DATA-1,0)) >= 0){
				buf[numbytes]='\0';
				saved_bytes+=numbytes;
				//header not finished yet!
				if(!header_flag){
						//Does header end in this buffer?
						if(strstr(buf,"\r\n\r\n")){
							header_flag=true;
						}
						//This is the first buffer!
						if(!status_code_flag){
								status_code_flag=true;
								strcpy(print_buffer,buf);
								strtok(print_buffer,"\r\n");
						}
						//Is there Content-length?
						if(!content_length_flag){
								strcpy(temp2,buf);
								if(temp=strstr(temp2,"content-length")){
										content_length_flag=true;
										strtok(temp,": ");
										temp = strtok(NULL,"\r\n ");
										content_length=atoi(temp);
								}
								if(temp=strstr(temp2,"Content-Length")){
										content_length_flag=true;
										strtok(temp,": ");
										temp = strtok(NULL,"\r\n ");
										content_length=atoi(temp);
								}
						}
						if(header_flag){
							if(!content_length_flag)
									break;
							strcpy(temp2,buf);
							temp=strstr(temp2,"\r\n\r\n");
							saved_bytes+=(strlen(temp)-strlen(temp2)-4);
							fprintf(fp,"%s",temp+4);
						}
				}		
				else
					fprintf(fp,"%s",buf);

		
				memset(buf,0,MAX_DATA);
				if(saved_bytes==content_length)
						break;
		}
		if(numbytes==-1){
				perror("recv");
				close(sockfd);
				fclose(fp);
				free(print_buffer);
				free(hostname);
				free(port);
				free(filepath);
				exit(1);
		}
		printf("%s\n",print_buffer);
		//if there is no content_length
		if(!content_length_flag){
				printf("Content-Length not specified.\n");
		}
		else{
				printf("%d bytes written to 20161566.out\n",content_length);
		}
		
		//---------------------------------- received data
		free(print_buffer);
		free(hostname);
		free(port);
		free(filepath);
		
		fclose(fp);
		close(sockfd);
		return 0;
}


