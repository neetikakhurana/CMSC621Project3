#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAXDATASIZE 256

void *connection_handler(void * sock);
int socketfd1,socketfd2,socketfd3;


int main(int argc, char *argv[])
{
	//struct sockaddr_in server_addr;
    
    int client_port, server1_port, server2_port, server3_port;
    struct hostent *server;
	char buffer[MAXDATASIZE];
	int *new_sock;
	u_int yes=1;

	if(argc<6)
	{
		fprintf(stderr, "Usage coordinator <hostname> <client port> <server1 port> <server2 port> <server3 port>\n");
		exit(1);
	}

	client_port=atoi(argv[2]);
	server1_port=atoi(argv[3]);
	server2_port=atoi(argv[4]);
	server3_port=atoi(argv[5]);

	int sock, ret,len;
    int one = 1;
    struct sockaddr_in serv_addr, our_addr;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	return -1;

    //
    // allow the local host to reuse the port if the server is
    // terminated prematurely
    //
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(one)) < 0) {
        perror("SO_REUSEADDR error");
    }
    
        
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(client_port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if ((bind(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))) < 0) {
        perror("cannot bind address");
		close(sock);
		return -1;
    }

    len = sizeof(our_addr);
    
    if (getsockname(sock,(struct sockaddr*)&our_addr, &len) < 0) {
		perror("cannot get socket name");
        close(sock);
		return -1;
    }

    
    listen(sock, 5);

 	struct hostent *server1,*server2,*server3;

	server1 = gethostbyname(argv[1]);
	server2 = gethostbyname(argv[1]);
	server3 = gethostbyname(argv[1]);

     //connect to the three servers
    struct sockaddr_in server_addr;

	server_addr.sin_family = AF_INET;
	bcopy((char *)server1->h_addr, (char *)&server_addr.sin_addr.s_addr,server1->h_length);
	server_addr.sin_port = htons(server1_port);

	struct sockaddr_in server_addr1;

	server_addr1.sin_family = AF_INET;
	bcopy((char *)server2->h_addr, (char *)&server_addr1.sin_addr.s_addr,server2->h_length);
	server_addr1.sin_port = htons(server2_port);

	struct sockaddr_in server_addr2;

	server_addr2.sin_family = AF_INET;
	bcopy((char *)server3->h_addr, (char *)&server_addr2.sin_addr.s_addr,server3->h_length);
	server_addr2.sin_port = htons(server3_port);


	socketfd1 = socket(AF_INET, SOCK_STREAM, 0); //create end point for communication
	if(socketfd1 < 0)
	{
		fprintf(stderr, "Socket not formed \n");
		exit(0);
	}

	if(connect(socketfd1, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	{
		fprintf(stderr, "Error connecting\n");
	}

	socketfd2 = socket(AF_INET, SOCK_STREAM, 0); //create end point for communication
	if(socketfd2 < 0)
	{
		fprintf(stderr, "Socket not formed \n");
		exit(0);
	}

	if(connect(socketfd2, (struct sockaddr *) &server_addr1, sizeof(server_addr1)) < 0)
	{
		fprintf(stderr, "Error connecting\n");
	}

	socketfd3 = socket(AF_INET, SOCK_STREAM, 0); //create end point for communication
	if(socketfd3 < 0)
	{
		fprintf(stderr, "Socket not formed \n");
		exit(0);
	}

	if(connect(socketfd3, (struct sockaddr *) &server_addr2, sizeof(server_addr2)) < 0)
	{
		fprintf(stderr, "Error connecting\n");
	}


     //accepting requests from client
    struct sockaddr_in rem_addr;
    int new_fd;
    int ready = 0;

    len = sizeof(rem_addr);
    pthread_t sniffer_thread;

    while(new_fd = accept(sock, (struct sockaddr*)&rem_addr, &len))
    {
        puts("Connection accepted");
        
	    new_sock = (int *)malloc(1);
	    *new_sock = new_fd;

	    if(pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
	    {
	        perror("could not create thread");
	        return 1;
	    }
    }



	for(int i = 0; i < 100; i++)
	{
		pthread_join(sniffer_thread,NULL);
	}

	close(sock);
	exit(0);
}

//thread to send the messages to the sequencer as well as multicast the message to other processes
void *connection_handler(void * sock)
{
	char buffer[MAXDATASIZE];
	int newsocket = *(int *)sock;
	struct hostent *server;
	int i=0;
	bzero(buffer, MAXDATASIZE);	
	printf("Accepting from sequencer \n");
	
	int n = read(newsocket, buffer, MAXDATASIZE);
	if(n < 0)
	{
		fprintf(stderr, "Error writing to socket\n");
	}
	else
	{
		printf("Read successful from client %s\n", buffer);
	}

	n=write(socketfd1, buffer, MAXDATASIZE);
	if(n < 0)
	{
		fprintf(stderr, "Error writing to server1\n");
	}
	else
	{
		printf("Write successful to server1\n");
	}
  	
  	n=write(socketfd2, buffer, MAXDATASIZE);
	if(n < 0)
	{
		fprintf(stderr, "Error writing to server2\n");
	}
	else
	{
		printf("Write successful to server2\n");
	}
	
	n=write(socketfd3, buffer, MAXDATASIZE);
	if(n < 0)
	{
		fprintf(stderr, "Error writing to server3\n");
	}
	else
	{
		printf("Write successful to server3\n");
	}

	//READ OK FROM SERVERS
	char buffer1[MAXDATASIZE],buffer2[MAXDATASIZE],buffer3[MAXDATASIZE];
	n=read(socketfd1, buffer1, MAXDATASIZE);
	if(n < 0)
	{
		fprintf(stderr, "Error reading from server1\n");
	}
	else
	{
		printf("Read successful from server1 %s\n",buffer1);
	}

	n=read(socketfd2, buffer2, MAXDATASIZE);
	if(n < 0)
	{
		fprintf(stderr, "Error reading from server1\n");
	}
	else
	{
		printf("Read successful from server2 %s\n",buffer2);
	}

	n=read(socketfd3, buffer3, MAXDATASIZE);
	if(n < 0)
	{
		fprintf(stderr, "Error reading from server1\n");
	}
	else
	{
		printf("REad successful from server3 %s\n",buffer3);
	}

	//when OK has been received from all the servers

	if(buffer1!=NULL && buffer2!=NULL && buffer3!=NULL)

	{
		bzero(buffer, MAXDATASIZE);
		strcpy(buffer,"COMMIT");
		n=write(socketfd1, buffer, MAXDATASIZE);
		if(n < 0)
		{
			fprintf(stderr, "Error writing to server2\n");
		}
		else
		{
			printf("Write successful to server2 %s\n",buffer);
		}
		
		n=write(socketfd2, buffer, MAXDATASIZE);
		if(n < 0)
		{
			fprintf(stderr, "Error writing to server3\n");
		}
		else
		{
			printf("Write successful to server3 %s\n",buffer);
		}
		n=write(socketfd3, buffer, MAXDATASIZE);
		if(n < 0)
		{
			fprintf(stderr, "Error writing to server3\n");
		}
		else
		{
			printf("Write successful to server3 %s\n",buffer);
		}

	}
}


