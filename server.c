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

typedef struct {
            int acc_no; 
            int amount;
} rec;

rec record[200];

int main(int argc, char **argv)
{
	if(argc<3)
	{
		fprintf(stderr, "Usage server <coordinator port> <server no>\n");
		exit(1);
	}
	char buffer[MAXDATASIZE];
	int port=atoi(argv[1]);
	int sock, ret, len;
    int serverno=atoi(argv[2]);
    int one = 1;
    File *myfile;
    struct sockaddr_in serv_addr, our_addr;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	return -1;
    char filename[20];
    strcpy(filename, "Record%d",serverno);
    myfile = fopen(filename,"r+");
    if(myfile==0){
        fprintf(stderr, "%d : Unable to open file\n", serverno);
    }
    //
    // allow the local host to reuse the port if the server is
    // terminated prematurely
    //
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(one)) < 0) 
    {
        perror("%d : SO_REUSEADDR error",serverno);
    }
    
       
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if ((bind(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))) < 0) {
        perror("%d : cannot bind address",serverno);
		close(sock);
		return -1;
    }

    len = sizeof(our_addr);
    
    if (getsockname(sock,(struct sockaddr*)&our_addr, &len) < 0) {
		perror("%d : cannot get socket name",serverno);
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
			fprintf(stderr, "%d : Error accepting socket\n",serverno);
		} 

		int n=read(new_fd, buffer, MAXDATASIZE);
		if(n<0){
			fprintf(stderr, "%d: Error reading\n",serverno);
		}
        printf("%d : in server%s\n", serverno, buffer);
        strcpy(transaction,buffer);

        //parse transaction and perform the task
        char *p;
        printf("%s\n", transaction);
        p=strtok(transaction," ");
        char command[10];
        strcpy(command,p);

        printf("%d : %s\n", serverno, command);
        char line[200];
        int i=0;
        int randomacc=100;
        int num_lines=0;
       while (fgets(line,200,myfile)) {
                char *q;
            q = strtok(line," ");
            record[i].acc_no = atoi(q);

            q = strtok(NULL," ");
            record[i].amount = atoi(q);
            if(line!=NULL){
                num_lines++;
                            i++;

            }
    }

        if(strcmp(command,"CREATE")==0)
        {
            //create an account id
            printf("CREATE\n");
            p=strtok(NULL," ");
            int amt=atoi(p);
            record[i].acc_no=randomacc;
            randomacc++;
            record[i].amount=amt;
            i++;
            fseek(myfile, num_lines, SEEK_SET);
            num_lines++;
            fprintf(myfile, "%d %d\n", record[i].acc_no, record[i].amount);
             //return the account no
            bzero(buffer,MAXDATASIZE);
            strcpy(buffer,"OK %d",record[j-1].acc_no);
            n=write(new_fd,buffer, MAXDATASIZE);
            if(n<0){
                fprintf(stderr, "%d : Error writing to coordinator\n", serverno);
            }
       /* p=strtok(NULL," ");
        int val=atoi(p);*/
        }
        else if(strcmp(command,"QUERY")==0)
        {
            //query for account balance
            printf("QUERY\n");
            p=strtok(NULL," ");
            int acc=atoi(p);
           
            int j=0;
            for(j=0;j<i;j++){
                if(record[j].acc_no==acc){
                    break;
                }
            }
            //return the account no
            bzero(buffer,MAXDATASIZE);
            strcpy(buffer,"OK %d",record[j-1].amount);
            n=write(new_fd,buffer, MAXDATASIZE);
            if(n<0){
                fprintf(stderr, "%d : Error writing to coordinator\n", serverno);
            }
        }

        else if(strcmp(command,"UPDATE")==0)
        {
            //update a record
            printf("UPDATE\n");
            p=strtok(NULL," ");
            int acc=atoi(p);
            p=strtok(NULL," ");
            int amt=atoi(p);
            int j=0;
            for(j=0;j<i;j++){
                if(record[j].acc_no==acc){
                    record[j].amount=amt;
                    break;
                }
            }
            fseek(myfile, num_lines, SEEK_SET);
            fprintf(myfile, "%d %d\n", record[j-1].acc_no, record[j-1].amount);
             //return the account no
            bzero(buffer,MAXDATASIZE);
            strcpy(buffer,"OK %d",record[j-1].amount);
            n=write(new_fd,buffer, MAXDATASIZE);
            if(n<0){
                fprintf(stderr, "%d : Error writing to coordinator\n", serverno);
            }
        }
        else if(strcmp(command,"QUIT")==0){
            //quit
        }


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



        close(sock);
        
}