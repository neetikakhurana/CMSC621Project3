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
            float amount;
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
    int one = 1, flag=1;
    FILE *myfile;
    struct sockaddr_in serv_addr, our_addr;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	return -1;
    char filename[20];
    sprintf(filename, "Record%d.txt",serverno);
    printf("%s\n", filename);
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
        fprintf(stderr,"%d : SO_REUSEADDR error",serverno);
    }
    
       
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if ((bind(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))) < 0) {
        fprintf(stderr,"%d : cannot bind address",serverno);
		close(sock);
		return -1;
    }

    len = sizeof(our_addr);
    
    if (getsockname(sock,(struct sockaddr*)&our_addr, &len) < 0) {
		fprintf(stderr,"%d : cannot get socket name",serverno);
        close(sock);
		return -1;
    }

    
    listen(sock, 5);

 struct sockaddr_in rem_addr;
        int new_fd;
        int ready = 0;
        char transaction[100];
        len = sizeof(rem_addr);

        int sock_fd = accept(sock, (struct sockaddr*)&rem_addr, &len);
        //new_fd = sock_fd;
		if(sock_fd<0)
		{
			fprintf(stderr, "%d : Error accepting socket\n",serverno);
		} 

        int n;
        int i=0;
        int randomacc=100;
		while(1)
        {
    		n=read(sock_fd, buffer, MAXDATASIZE);
            //new_fd = sock_fd;
            if(n<0){
    			fprintf(stderr, "%d: Error reading\n",serverno);
    		}

            printf("%d : in server %s\n", serverno, buffer);
            strcpy(transaction,buffer);

            //parse transaction and perform the task
            char *p;
            p=strtok(transaction," ");
            char command[10];
            strcpy(command,p);
            if(strcmp(command,"QUIT\n")!=0){
            p=strtok(NULL," ");
        }
            char line[200];
            
            int num_lines=0;
          /* while (fgets(line,200,myfile)) {
                    char *q;
                q = strtok(line," ");
                record[i].acc_no = atoi(q);

                q = strtok(NULL," ");
                record[i].amount = atof(q);
                if(line!=NULL){
                    num_lines++;
                                i++;

                }
        }*/

            if(strcmp(command,"CREATE")==0)
            {
                //create an account id
               
                printf("CREATE\n");
                double amt=atof(p);
                            record[i].amount=amt;
                printf("%f %f %d\n", amt,record[i].amount,i);
                record[i].acc_no=randomacc;
                printf("%d %d\n", randomacc, record[i].acc_no);

                
                //fseek(myfile, num_lines-1, SEEK_SET);
                //num_lines++;
                //fprintf(myfile, "%d %f\n", record[i-1].acc_no, record[i-1].amount);
                 //return the account no
                bzero(buffer,MAXDATASIZE);
                sprintf(buffer,"OK %d",record[i].acc_no);
                printf("buffer%s\n", buffer);
                randomacc++;
                i++;
                int t=write(sock_fd,buffer, MAXDATASIZE);
                if(t<0){
                    fprintf(stderr, "%d : Error writing to coordinator\n", serverno);
                }
            }
            else if(strcmp(command,"QUERY")==0)
            {
                //query for account balance
                printf("QUERY\n");
                int acc=atoi(p);
               
                int j=0;
                for(j=0;j<i;j++){
                    if(record[j].acc_no==acc){
                        printf("%d\n", acc);
                        break;
                    }
                }
                //return the account no
                if(j<i){
                    bzero(buffer,MAXDATASIZE);
                    sprintf(buffer,"OK %f",record[j].amount);
                }
                else{
                    bzero(buffer,MAXDATASIZE);
                    sprintf(buffer,"ERR Account %d does not exist",acc);
                }
                //fprintf(buffer,"OK %d",record[j-1].amount);
                int a=write(sock_fd,buffer, MAXDATASIZE);
                if(a<0){
                    fprintf(stderr, "%d : Error writing to coordinator\n", serverno);
                }
            }

            else if(strcmp(command,"UPDATE")==0)
            {
                //update a record
                printf("UPDATE\n");
                int acc=atoi(p);
                p=strtok(NULL," ");
                double amt=atof(p);
                printf("amt %f %d",amt,i);
                int j=0;
                for(j=0;j<i;j++){
                    printf("%d\n", record[j].acc_no);
                    if(record[j].acc_no==acc){
                        record[j].amount=amt;
                        break;
                    }
                }
              //  fseek(myfile, num_lines, SEEK_SET);
              //  sprintf(buffer, "%d %f\n", record[j-1].acc_no, record[j-1].amount);
                 //return the account no
                bzero(buffer,MAXDATASIZE);
                //buffer="OK "+record[j-1].amount;
                sprintf(buffer,"OK %f",record[j].amount);
                printf("SEnding %s\n", buffer);
                int b=write(sock_fd,buffer, MAXDATASIZE);
                if(b<0){
                    fprintf(stderr, "%d : Error writing to coordinator\n", serverno);
                }
            }
            else if(strcmp(command,"QUIT\n")==0){
                //quit
                printf("QUIT\n");
                bzero(buffer,MAXDATASIZE);
                strcpy(buffer,"OK");
                printf("sending%s\n", buffer);
                int c=write(sock_fd, buffer, MAXDATASIZE);
                if(c<0)
                {
                    fprintf(stderr, "ERROR sending ready to commit message to coordinator\n");
                }
              //  flag=0;
            }

            //receive the commit msg
            bzero(buffer,MAXDATASIZE);
            int h=read(sock_fd,buffer,MAXDATASIZE);
            if(h<0){
            	fprintf(stderr, "Error reading commit msg from coordinator\n");
            }
            printf("%d : commit%s\n", serverno,buffer);
            bzero(buffer,MAXDATASIZE);
        }
        close(sock_fd);
        close(sock);
        
}