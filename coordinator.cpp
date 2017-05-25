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
#include <map>

#define MAXDATASIZE 1024

void *connection_handler(void * sock);
int socketfd1,socketfd2,socketfd3;
char restart[MAXDATASIZE];
	char buffer[MAXDATASIZE];
	struct hostent *server;
	int i=0,n;
	int notWorking=0;
	pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER;

	char buffer1[MAXDATASIZE],buffer2[MAXDATASIZE],buffer3[MAXDATASIZE];
    int client_port, server1_port, server2_port, server3_port;

int main(int argc, char *argv[])
{
	//struct sockaddr_in server_addr;
    
    struct hostent *server;
	int *new_sock;
	u_int yes=1;
	pthread_mutex_init(&mut,NULL);

	if(argc<6)
	{
		fprintf(stderr, "Usage coordinator <hostname> <client port> <server1 port> <server2 port> <server3 port>\n");
		exit(1);
	}

	//assuming that all servers will work
	notWorking=0;

	client_port=atoi(argv[2]);
	server1_port=atoi(argv[3]);
	server2_port=atoi(argv[4]);
	server3_port=atoi(argv[5]);

	int sock, ret;
	socklen_t len;
    int one = 1;
    struct sockaddr_in serv_addr, our_addr;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	return -1;

    //
    // allow the local host to reuse the port if the server is
    // terminated prematurely
    //
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(one)) < 0) {
        fprintf(stderr,"SO_REUSEADDR error");
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
    
    if (getsockname(sock,(struct sockaddr*)&our_addr, (socklen_t*)&len) < 0) {
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
		fprintf(stderr, "Error connecting to server1\n");
		notWorking=1;
	}

	socketfd2 = socket(AF_INET, SOCK_STREAM, 0); //create end point for communication
	if(socketfd2 < 0)
	{
		fprintf(stderr, "Socket not formed \n");
		exit(0);
	}

	if(connect(socketfd2, (struct sockaddr *) &server_addr1, sizeof(server_addr1)) < 0)
	{
		fprintf(stderr, "Error connecting to server2\n");
		notWorking=2;
	}

	socketfd3 = socket(AF_INET, SOCK_STREAM, 0); //create end point for communication
	if(socketfd3 < 0)
	{
		fprintf(stderr, "Socket not formed \n");
		exit(0);
	}

	if(connect(socketfd3, (struct sockaddr *) &server_addr2, sizeof(server_addr2)) < 0)
	{
		fprintf(stderr, "Error connecting to server3\n");
		notWorking=3;
	}


     //accepting requests from client
    struct sockaddr_in rem_addr;
    int new_fd;
    int ready = 0;

    len = sizeof(rem_addr);
    pthread_t sniffer_thread;

    while(new_fd = accept(sock, (struct sockaddr*)&rem_addr, (socklen_t*)&len))
    {
        puts("Connection accepted");
        
	    new_sock = (int *)malloc(1);
	    *new_sock = new_fd;

	    printf("Socket no %d\n", new_fd);

	    bzero(buffer,MAXDATASIZE);
	    strcpy(buffer,"OK");
	    //send OK to client
	    int s=write(new_fd,buffer,MAXDATASIZE);
	    if(s<0){
	    	fprintf(stderr, "Error sending OK response to client\n");
	    }
	    else{
	    	printf("OK message sent to client\n");
	    }


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
	close(socketfd1);
	close(socketfd1);
	close(socketfd3);

	close(sock);
	exit(0);
}
int flag=0;

int count=0;


//thread to send the messages to the sequencer as well as multicast the message to other processes
void *connection_handler(void * sock)
{
	bzero(buffer, MAXDATASIZE);	
	printf("Accepting from client \n");
	int newsocket = *(int *)sock;
	int l=0;
	//read multiple requests from clients
	while(1)
	{

		if(flag==0)
		{
			n = read(newsocket, buffer, MAXDATASIZE);
			if(n < 0)
			{
				fprintf(stderr, "Error writing to client\n");
			}
			else
			{
				if(strlen(buffer)!=0){
					printf("Read successful from client %s\n", buffer);
				}
			}
				strcpy(restart,buffer);
			if(strlen(buffer)==0){
				if(l==10){			
					int j=write(newsocket,"OK\r\n",MAXDATASIZE);
					if(j<0){
						fprintf(stderr, "Error\n");
					}
				}
				l++;
				pthread_mutex_unlock(&mut);
			}
		}
		else
		{
			bzero(buffer,MAXDATASIZE);
			strcpy(buffer,restart);
		}
				pthread_mutex_lock(&mut);

		struct timeval timeout;     
    	timeout.tv_sec  = 0;
    	timeout.tv_usec = 1;

        if (setsockopt (socketfd1, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval)) < 0)
        {
            perror("setsockopt failed\n");
        }

        /*if(strlen(buffer)==0){
        	int j=write(newsocket,"OK",MAXDATASIZE);
        	if(j<0){
        		fprintf(stderr, "Error writing done message\n");
        	}
        	break;
        }*/

		//send command to server1
		if(notWorking!=1){
			int h=write(socketfd1, buffer, MAXDATASIZE);
			if(h < 0)
			{
				fprintf(stderr, "Error writing to server1\n");
			}
			else
			{
				printf("Write successful to server1 %s\n",buffer);
			}
		}

		//send command to server2
		if(notWorking!=2)
		{	  	
		  	int h=write(socketfd2, buffer, MAXDATASIZE);
			if(h < 0)
			{
				fprintf(stderr, "Error writing to server2\n");
			}
			else
			{
				printf("Write successful to server2 %s\n",buffer);
			}
		}

		//send command to server3
		if(notWorking!=3){
			int h=write(socketfd3, buffer, MAXDATASIZE);
			if(h < 0)
			{
				fprintf(stderr, "Error writing to server3\n");
			}
			else
			{
				printf("Write successful to server3 %s\n",buffer);
			}
		}


        if (setsockopt (socketfd1, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval)) < 0)
            {
            	perror("setsockopt failed\n");
            }

		//READ OK FROM SERVERS
		//server1
		if(notWorking!=1)
		{
			bzero(buffer1,MAXDATASIZE);
			int t=read(socketfd1, buffer1, MAXDATASIZE);
			if(t < 0)
			{
				fprintf(stderr, "Error reading from server1\n");
			}
			else
			{
				printf("Read successful from server1 %s\n",buffer1);
			}
		}

		//server2
		if(notWorking!=2)
		{
			bzero(buffer2,MAXDATASIZE);
			int t=read(socketfd2, buffer2, MAXDATASIZE);
			if(t < 0)
			{
				fprintf(stderr, "Error reading from server2\n");
			}
			else
			{
				printf("Read successful from server2 %s\n",buffer2);
			}
		}

		//server3
		if(notWorking!=3)
		{
			bzero(buffer3,MAXDATASIZE);
			int t=read(socketfd3, buffer3, MAXDATASIZE);
			if(t < 0)
			{
				fprintf(stderr, "Error reading from server3\n");
			}
			else
			{
				printf("REad successful from server3 %s\n",buffer3);
			}
		}
		/*char *p=strtok(buffer," ");
		printf("%s\n", p);
		if(strcmp(p,"QUIT")==0){
			break;
		}*/


		/**
		Implement timeout
		**/
		usleep(200000);

		if(flag==1 && (strlen(buffer1)!=0 || strlen(buffer2)!=0 || strlen(buffer3)!=0))
		{
				bzero(buffer, MAXDATASIZE);
				strcpy(buffer,"COMMIT");
				flag=0;
		}

		//when OK has been received from all the servers
		//check if response has been received from all of them
		//if not, then think of maintaining data consistency...............
		else if(strlen(buffer1)!=0 && strlen(buffer2)!=0 && strlen(buffer3)!=0)

		{
			//if all servers are ready to commit
			if(strcmp(buffer1,"NO")!=0 && strcmp(buffer2,"NO")!=0 && strcmp(buffer3,"NO")!=0){
				//send global commit to all of them
				bzero(buffer, MAXDATASIZE);
				strcpy(buffer,"COMMIT");
				flag=0;
			}
			else
			{
				//if even one of them sends a NO, ABORT the transaction and restart the transaction
				bzero(buffer,MAXDATASIZE);
				strcpy(buffer,"ABORT");
				flag=1;
			}
		}
		//if one of them do not reply/ one server crashes
		if(strlen(buffer1)==0){
			//didnt get response from socket 1
			notWorking=1;
			bzero(buffer, MAXDATASIZE);
			strcpy(buffer,"COMMIT");
			flag=0;
		}
		if(strlen(buffer2)==0)
		{
			//dint get response from socket 2
			notWorking=2;
			bzero(buffer, MAXDATASIZE);
			strcpy(buffer,"COMMIT");
			flag=0;
		}
		if(strlen(buffer3)==0){
			notWorking=3;
			bzero(buffer, MAXDATASIZE);
			strcpy(buffer,"COMMIT");
			flag=0;
		}
		
		if(notWorking!=1)
		{
			int t=write(socketfd1, buffer, MAXDATASIZE);
			if(t < 0)
			{
				fprintf(stderr, "Error writing to server1\n");
			}
			else
			{
				printf("Write successful to server1 %s\n",buffer);
			}
		}
		
		if(notWorking!=2)
		{
			int t=write(socketfd2, buffer, MAXDATASIZE);
			if(t < 0)
			{
				fprintf(stderr, "Error writing to server2\n");
			}
			else
			{
				printf("Write successful to server2 %s\n",buffer);
			}
		}

		if(notWorking!=3)
		{
			int t=write(socketfd3, buffer, MAXDATASIZE);
			if(t < 0)
			{
				fprintf(stderr, "Error writing to server3\n");
			}
			else
			{
				printf("Write successful to server3 %s\n",buffer);
			}

		}

		usleep(200000);

		//READ OK FROM SERVERS
		//server1
		char result[MAXDATASIZE];
		if(notWorking!=1){
			bzero(buffer1,MAXDATASIZE);
			bzero(result,MAXDATASIZE);
			int u=read(socketfd1, buffer1, MAXDATASIZE);
			if(u < 0)
			{
				fprintf(stderr, "Error reading from server1\n");
			}
			else
			{
				printf("Read successful from server1 %s\n",buffer1);
				strcpy(result,buffer1);
			}
			count++;
		}

		//server2
		if(notWorking!=2){
		bzero(buffer2,MAXDATASIZE);
		bzero(result,MAXDATASIZE);
		int t=read(socketfd2, buffer2, MAXDATASIZE);
		if(t < 0)
		{
			fprintf(stderr, "Error reading from server2\n");
		}
		else
		{
			printf("Read successful from server2 %s\n",buffer2);
			strcpy(result,buffer2);
		}
		count++;
	}
	
	//server3
	if(notWorking!=3){
		bzero(buffer3,MAXDATASIZE);
		bzero(result,MAXDATASIZE);
		int t=read(socketfd3, buffer3, MAXDATASIZE);
		if(t < 0)
		{
			fprintf(stderr, "Error reading from server3\n");
		}
		else
		{
			printf("REad successful from server3 %s\n",buffer3);
			strcpy(result,buffer3);
		}
		count++;
	}
		

		if(flag==0 && count>0){
		//write the response to the client
			int t=write(newsocket,result,MAXDATASIZE);
			if(t<0){
				fprintf(stderr, "Error writing final result to client\n");
			}
			else{
				printf("Written final result to client\n");
			}
		}
				
		pthread_mutex_unlock(&mut);
		bzero(buffer,MAXDATASIZE);
	}
}


