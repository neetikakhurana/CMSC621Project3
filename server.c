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

#define MAXDATASIZE 1024

int main(int argc, char **argv)
{
	if(argc<2)
	{
		fprintf(stderr, "Usage server1 <coordinator port>\n");
		exit(1);
	}
	char buffer[MAXDATASIZE];
	int port=atoi(argv[1]);
	int sock, ret, len;
    int one = 1;
    struct sockaddr_in serv_addr, our_addr;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	return -1;

    //
    // allow the local host to reuse the port if the server is
    // terminated prematurely
    //
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(one)) < 0) 
    {
        perror("SO_REUSEADDR error");
    }
    
       
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
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

 struct sockaddr_in rem_addr;
        int new_fd;
        int ready = 0;
        char transaction[100];
        len = sizeof(rem_addr);

        new_fd = accept(sock, (struct sockaddr*)&rem_addr, &len);
        
		if(new_fd<0)
		{
			fprintf(stderr, "Error accepting socket\n");
		} 

		int n=read(new_fd, buffer, MAXDATASIZE);
		if(n<0){
			fprintf(stderr, "Error reading\n");
		}
        printf("in server%s\n", buffer);
        strcpy(transaction,buffer);

        //send an acknowledgement that you are ready to commit
        bzero(buffer,MAXDATASIZE);
        strcpy(buffer,"OK");
        n=write(new_fd, buffer, MAXDATASIZE);
        if(n<0)
        {
        	fprintf(stderr, "ERROR sending ready to commit message to coordinator\n");
        }


        //receive the commit msg
        bzero(buffer,MAXDATASIZE);
        n=read(new_fd,buffer,MAXDATASIZE);
        if(n<0){
        	fprintf(stderr, "ERror reading commit msg from coordinator\n");
        }

        //parse transaction and perform the task
        char *p;
        printf("%s\n", transaction);
        p=strtok(transaction," ");
        char command[10];
        strcpy(command,p);

                printf("%s\n", command);

       

        if(strcmp(command,"CREATE")==0)
        {
        	//create an account
        	printf("CREATE\n");
        	 p=strtok(NULL," ");
        int acc=atoi(p);
       /* p=strtok(NULL," ");
        int val=atoi(p);*/
        }
        else if(strcmp(command,"QUERY")==0)
        {
        	//query for account balance
        }

        else if(strcmp(command,"UPDATE")==0)
        {
        	//update a record
        }
        else if(strcmp(command,"QUIT")==0){
        	//quit
        }

        close(sock);
        
}