#include <stdio.h>
#include <string.h>
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <time.h>  //for timestamp 


int main(int argc, char **argv) {
    if(argc < 3)   
	{
		fprintf(stderr, "Usage client <hostname> <port number>\n");
		exit(1);
	}
    
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

    char buffer[256];

   
    	strcpy(buffer, "CREATE 50.00");
    	
    	int n = write(socketfd, buffer, strlen(buffer));
		if(n < 0)
		{	
			fprintf(stderr, "Error with writing to coordinator\n");
			exit(1);	
		}
		
    close(socketfd);
	exit(0);
}