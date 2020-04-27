#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include "buffer.h"

#define st_mtime st_mtim.tv_sec


char pathName[1024];
char ip[1024];
//char message[1000];
int portNumber;
int sizeOfArr = 10;
int counter = 0;
buffer_t message;
buffer_t *bufArr;
buffer_t *result;

void increase(buffer_t** data){
    *data = realloc(*data, (sizeOfArr*2) * sizeof( buffer_t));
    sizeOfArr = sizeOfArr*2;
}

int postOrderApply (char *path){
    DIR *dir;
    struct dirent *entry;
    int sumSize =  0;
    struct stat fileStat;

   
    if (!(dir = opendir(path))){
        printf("Cannot read folder");
        return -1;
    }
 
    while (1){
        char pathDir[1024];
        //donguyu durdurma kosulu

        if((entry = readdir(dir)) == NULL){
            break;
        }
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(pathDir, sizeof(pathDir), "%s/%s", path, entry->d_name);
        lstat(pathDir,&fileStat);

        if(S_ISDIR(fileStat.st_mode)){

            strcpy(message.filename,pathDir);
            strcpy(message.mod_time,(char*) ctime(&fileStat.st_atime));
            message.sizeFile = fileStat.st_size;
            message.type = 2;
            bufArr[counter] = message;
            counter++;

            if(counter == (sizeOfArr-1)){
                increase(&bufArr);
            }            
            postOrderApply(pathDir);

        }else if(S_ISREG(fileStat.st_mode)){
            strcpy(message.filename,pathDir);
            strcpy(message.mod_time,(char*) ctime(&fileStat.st_atime));
            message.sizeFile = fileStat.st_size;
            message.type = 1;


            bufArr[counter] = message;
            counter++;

            if(counter == (sizeOfArr-1)){
                increase(&bufArr);
            }
            memset(pathDir, '\0', sizeof pathDir);
        }else{
            continue;
            /**/
        }
    }
    closedir(dir);
    return sumSize;
}

void mainProgram(){

    printf("In thread\n");

    while(1){


        int filefd;
        ssize_t read_return;
        char pathDir[1024];
        struct stat fileStat;
        int clientSocket;
        struct sockaddr_in serverAddr;
        socklen_t addr_size;
       
        
        // Create the socket. 
        clientSocket = socket(PF_INET, SOCK_STREAM, 0);
        //Configure settings of the server address
        // Address family is Internet 
        serverAddr.sin_family = AF_INET;
        //Set port number, using htons function 
        serverAddr.sin_port = htons(portNumber);
        //Set IP address to localhost
        serverAddr.sin_addr.s_addr = inet_addr(ip);

        memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
        //Connect the socket to the server using the address
        addr_size = sizeof serverAddr;
        connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);

        //free(bufArr);
        bufArr = malloc(sizeOfArr*sizeof(buffer_t));

        lstat(pathName,&fileStat);

        if(S_ISDIR(fileStat.st_mode)){

            strcpy(message.filename,pathName);
            strcpy(message.mod_time,(char*) ctime(&fileStat.st_atime));
            message.sizeFile = fileStat.st_size;
            message.type = 2;
            bufArr[counter] = message;
            counter++;

        }else{
            printf("This is not a directory !!!1\n");
            exit(1);
        }

       
        postOrderApply(pathName);


        if( send(clientSocket ,&counter ,sizeof(int) , 0) < 0){

            printf("Send failed\n");
        
        }
        for(int i = 0 ; i < counter ; i++){
            send(clientSocket ,bufArr[i].filename ,sizeof(bufArr[i].filename) , 0);
        }
        for(int i = 0 ; i < counter ; i++){
            send(clientSocket ,bufArr[i].mod_time ,sizeof(bufArr[i].mod_time) , 0);
        }
        for(int i = 0 ; i < counter ; i++){
            send(clientSocket ,&bufArr[i].type ,sizeof(int) , 0);
        }
        for(int i = 0 ; i < counter ; i++){
            send(clientSocket ,&bufArr[i].sizeFile ,sizeof(off_t) , 0);
        }

        /*if( send(clientSocket ,(void**)bufArr ,counter*sizeof(buffer_t) , 0) < 0){

            printf("Send failed\n");
        
        }*/


        recv(clientSocket ,&counter, sizeof(int) , 0);

        result = malloc(counter*sizeof(buffer_t));

        for(int i = 0 ; i < counter ; i++)
            recv(clientSocket , (void*)result[i].filename, sizeof(result[i].filename) , 0);
        for(int i = 0 ; i < counter ; i++)
            recv(clientSocket , (void*)result[i].mod_time, sizeof(result[i].mod_time) , 0);
        for(int i = 0 ; i < counter ; i++)
            recv(clientSocket ,&result[i].type, sizeof(int) , 0);
        for(int i = 0 ; i < counter ; i++)
            recv(clientSocket ,&result[i].sizeFile, sizeof(off_t) , 0);

 
        for(int i = 0 ; i < counter ; i++){

            memset(pathDir, '\0', sizeof pathDir);
            strcpy(pathDir,result[i].filename);

            if(result[i].type == 2){
                continue;
            }
           
           
            char buffer[1024];
            filefd = open(pathDir, O_RDONLY);
            if (filefd == -1) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            int size = result[i].sizeFile;

            while (1) {
                memset(buffer,0,1024);

                if(size - 1024 >= 0){
                    read_return = read(filefd, buffer, 1024);
                    if (write(clientSocket, buffer,1024) == -1) {

                        perror("write");
                        exit(EXIT_FAILURE);
                    }
                    size -= 1024;
                }else{
                    read(filefd, buffer, size);
                    if (write(clientSocket, buffer,size) == -1) {

                        perror("write");
                        exit(EXIT_FAILURE);
                    }
                    memset(buffer,0,size);
                    break;
                }
            }
            close(filefd);
    }

    free(bufArr);
    free(result);
    close(clientSocket);
    counter = 0;
    }

    
}

int main(int argc, char const *argv[]){

    pthread_t tid;

    if(argc != 4){
        fprintf(stdout,"Usage : %s \" Client [dirName] [ipAdress] [portnumber]\"\n",argv[0]);
        return 0;
    }

    portNumber = atoi(argv[3]);
    strcpy(ip,argv[2]);
    strcpy(pathName,argv[1]);

    mainProgram();

    
    return 0;
}