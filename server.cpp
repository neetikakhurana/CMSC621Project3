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
pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mut1=PTHREAD_MUTEX_INITIALIZER;


typedef struct {
            int acc_no; 
            float amount;
} rec;

rec record[500];

int main(int argc, char **argv)
{
    pthread_mutex_init(&mut,NULL);
        pthread_mutex_init(&mut1,NULL);
    if(argc<3)
    {
        fprintf(stderr, "Usage server <coordinator port> <server no>\n");
        exit(1);
    }
    char buffer[MAXDATASIZE];
    int port=atoi(argv[1]);
    int sock, ret;
    socklen_t len;
    int serverno=atoi(argv[2]);
    int one = 1, flag=1;
    FILE *myfile;
    struct sockaddr_in serv_addr, our_addr;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return -1;
    char filename[20];
    sprintf(filename, "Record%d.txt",serverno);
    printf("%s\n", filename);
    myfile = fopen(filename,"w");
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
    
    if (getsockname(sock,(struct sockaddr*)&our_addr, (socklen_t*)&len) < 0) {
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

        int sock_fd = accept(sock, (struct sockaddr*)&rem_addr, (socklen_t*)&len);
        if(sock_fd<0)
        {
            fprintf(stderr, "%d : Error accepting socket\n",serverno);
        } 

        int n;
        int i=0;
        int z=0;
        int randomacc=100;
        while(1)
        {
            n=read(sock_fd, buffer, MAXDATASIZE);
            if(n<0){
                fprintf(stderr, "%d: Error reading\n",serverno);
            }
            if(strlen(buffer)==0)
            {
                close(sock_fd);
                exit(1);
            }
            printf("%d : in server %s\n", serverno, buffer);

            //copy the transaction to "transaction" array
            strcpy(transaction,buffer);
            bzero(buffer,MAXDATASIZE);
            strcpy(buffer,"YES");

            //in order to check that the system works even when one of them do not response
            /*if(z==0 && serverno==1){
                //sending no delibretely
                printf("SEnding Empty delibretely........\n");
                bzero(buffer,MAXDATASIZE);
                z++;
            }*/


            //in order to check that the system restarts the application if one of them sends a NO
             /*if(z==0 && serverno==1){
                //sending no delibretely
                printf("SEnding Empty delibretely........\n");
                bzero(buffer,MAXDATASIZE);
                strcpy(buffer,"NO");
                z++;
            }*/
               
            int x=write(sock_fd, buffer,MAXDATASIZE);
            if(x<0)
            {
                fprintf(stderr, "Error writing yes/no to coordinator\n");
            }

            //receive the commit msg
            bzero(buffer,MAXDATASIZE);
            int h=read(sock_fd,buffer,MAXDATASIZE);
            if(h<0){
                fprintf(stderr, "Error reading commit/abort msg from coordinator\n");
            }
            printf("%d : commit%s\n", serverno,buffer);
           

            //check if it is global commit or abort
            if(strcmp(buffer,"COMMIT")==0)
            {
                usleep(200000);
                printf("Going to commit\n");
                

            //parse transaction and perform the task
            char *p;
            p=strtok(transaction," ");
            char command[10];
            strcpy(command,p);
            if(strcmp(command,"QUIT\n")!=0)
            {
                p=strtok(NULL," ");
            }
            char line[200];
            int num_lines=0;
          
            if(strcmp(strdup(command),"CREATE")==0)
            {
                //create an account id
               
                printf("CREATE\n");
               // pthread_mutex_lock(&mut);

                double amt=atof(p);
                
                if(amt<0)
                {
                    bzero(buffer,MAXDATASIZE);
                    sprintf(buffer,"ERR creating the account (Amount is negative)\r\n");
                    printf("buffer%s\n", buffer);
                }
                //return the account no
                else{
                    record[i].amount=amt;
                    record[i].acc_no=randomacc;
                    bzero(buffer,MAXDATASIZE);
                    sprintf(buffer,"OK %d\r\n",record[i].acc_no);
                    printf("buffer%s\n", buffer);
                    randomacc++;
                    i++;
                    fseek(myfile, 0, SEEK_SET);
                    for(int k=0;k<i;k++){
                        fprintf(myfile, "%d %.2f\n", record[k].acc_no, record[k].amount);   
                    }
                }
                int t=write(sock_fd,buffer, MAXDATASIZE);
                if(t<0){
                    fprintf(stderr, "%d : Error writing to coordinator\n", serverno);
                }
               // pthread_mutex_unlock(&mut);
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
                if(j<i && acc>=0){
                    bzero(buffer,MAXDATASIZE);
                    sprintf(buffer,"OK %.2f\r\n",record[j].amount);
                    fseek(myfile, 0, SEEK_SET);
                    for(int k=0;k<i;k++){
                    fprintf(myfile, "%d %.2f\n", record[k].acc_no, record[k].amount);   
                    }
                }
                else{
                    bzero(buffer,MAXDATASIZE);
                    sprintf(buffer,"ERR Account %d does not exist\r\n",acc);
                }
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
                int found=0;
                int j=0;
                for(j=0;j<i;j++){
                    if(record[j].acc_no==acc){
                        pthread_mutex_lock(&mut1);
                        found=1;
                        record[j].amount=amt;
                        pthread_mutex_unlock(&mut1);
                        break;
                    }
                }
              
                //return the account no
                bzero(buffer,MAXDATASIZE);
                if(found==1)
                {
                sprintf(buffer,"OK %.2f\r\n",record[j].amount);
                fseek(myfile, 0, SEEK_SET);
                for(int k=0;k<i;k++){
                fprintf(myfile, "%d %.2f\n", record[k].acc_no, record[k].amount);   
                }
                }
                else
                {
                    sprintf(buffer,"ERR Account %d does not exist\r\n",acc);
                }
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
                strcpy(buffer,"OK\r\n");
                printf("sending%s\n", buffer);
                int c=write(sock_fd, buffer, MAXDATASIZE);
                if(c<0)
                {
                    fprintf(stderr, "ERROR sending ready to commit message to coordinator\n");
                }
            }

            
        }
            else{
                //global abort, dont save data into file
                printf("Global abort\n");
                bzero(buffer,MAXDATASIZE);
                strcpy(buffer,"ABORT");
                int y=write(sock_fd, buffer, MAXDATASIZE);
                if(y<0)
                {
                    fprintf(stderr, "ERROR sending response to abort message to coordinator\n");
                }
            }
            
            bzero(buffer,MAXDATASIZE);
        }
        fflush(myfile);
        fclose(myfile);
        close(sock_fd);
        close(sock);
        
}