#include <stdio.h>
#include <string.h>
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>  //for timestamp 

#define MAXDATASIZE 1024

int main(int argc, char **argv) {
    if(argc < 3)   
	{
		fprintf(stderr, "Usage client <hostname> <port number>\n");
		exit(1);
	}

    FILE * filehandle;
    struct hostent *server;
	int portno;
	portno = atoi(argv[2]);
	server = gethostbyname(argv[1]);
	if(server == NULL)
	{
		fprintf(stderr, "No such host exists\n");
		exit(1);
	}

	struct sockaddr_in server_addr;

	server_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr,server->h_length);
	server_addr.sin_port = htons(portno);	

	int socketfd = socket(AF_INET, SOCK_STREAM, 0); //create end point for communication
	if(socketfd < 0)
	{
		fprintf(stderr, "Socket not formed \n");
		exit(0);
	}

	if(connect(socketfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	{
		fprintf(stderr, "Error connecting\n");
	}

    char buffer[MAXDATASIZE];
    int n=read(socketfd,buffer,MAXDATASIZE);
    if(n<0){
    	fprintf(stderr, "Error receiving OK from coordinator\n");
    }
    else{
    	printf("%s\n", buffer);
    }
    int ch;
    /*filehandle = fopen("Test.txt","r");    
    if(filehandle == 0){
        printf("Unable to open file");
        exit(1);
    }*/
    bzero(buffer,MAXDATASIZE);

	   	
    	do{
	   		printf("Enter the command\n");
	   		fgets(buffer,MAXDATASIZE,stdin);
	    	for(int index=0; index<strlen(buffer); index++){
		        if(islower(buffer[index])){
		            buffer[index] = toupper(buffer[index]);
		        } else {
		            buffer[index] = buffer[index];
		        }
		    }
		    printf("%s\n", buffer);
	    	int t = write(socketfd, buffer, strlen(buffer));
			if(t < 0)
			{	
				fprintf(stderr, "Error with writing to coordinator\n");
				exit(1);	
			}
			bzero(buffer, MAXDATASIZE);
			t=read(socketfd,buffer,MAXDATASIZE);
			if(t<0)
			{
				fprintf(stderr, "Error reading the final response\n");
			}
			else{
				printf("Received : %s\n", buffer);
				if(strcmp(buffer,"OK\r\n")==0)
				{
					printf("Connection closed by foreign host.\n");
					//close(socketfd);
					ch=0;
					//exit(1);
				}
				scanf("%d",&ch);
				bzero(buffer,MAXDATASIZE);
			}
			
		}while(ch!=0);//fgets(buffer,MAXDATASIZE,filehandle));
	//fclose(filehandle);
    close(socketfd);
	exit(0);
}