#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <netdb.h>

#include "buffer.h"

buffer_t client_message;
buffer_t message;
buffer_t *bufArr;
buffer_t *tempBufArr;
buffer_t *result;
int sizeOfArr = 10;
int x = 0;

int counter;
int checkCounter = 0;

char directory[1024];
char directory1[1024];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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

            tempBufArr[checkCounter] = message;
            checkCounter++;
        
            postOrderApply(pathDir);

        }else if(S_ISREG(fileStat.st_mode)){
            strcpy(message.filename,pathDir);
            strcpy(message.mod_time,(char*) ctime(&fileStat.st_atime));
            message.sizeFile = fileStat.st_size;
            message.type = 1;


            tempBufArr[checkCounter] = message;
            checkCounter++;

            memset(pathDir, '\0', sizeof pathDir);
        }else{
            //continue;
            /**/
        }
    }
    closedir(dir);
    return sumSize;
}





void * socketThread(void *arg){

    char file_path[1024];
    char temp_path[1024];
    int newSocket = *((int *)arg);
    int filefd;
    ssize_t read_return;
    ssize_t byte = 0;
    struct stat fileStat;
    FILE * fp;
    

    
   //pthread_mutex_lock(&lock);

    fp = fopen ("log.txt","a");

    recv(newSocket ,&counter, sizeof(int) , 0);


    bufArr = malloc(counter*sizeof(buffer_t));
    tempBufArr = malloc(5000*sizeof(buffer_t));
    result = malloc(counter*sizeof(buffer_t));

    for(int i = 0 ; i < counter ; i++)
        recv(newSocket , (void*)bufArr[i].filename, sizeof(bufArr[i].filename) , 0);
    for(int i = 0 ; i < counter ; i++)
        recv(newSocket , (void*)bufArr[i].mod_time, sizeof(bufArr[i].mod_time) , 0);
    for(int i = 0 ; i < counter ; i++)
        recv(newSocket ,&bufArr[i].type, sizeof(int) , 0);
    for(int i = 0 ; i < counter ; i++)
        recv(newSocket ,&bufArr[i].sizeFile, sizeof(off_t) , 0);

    lstat(directory1,&fileStat);

    if(S_ISDIR(fileStat.st_mode)){

        strcpy(message.filename,directory1);
        strcpy(message.mod_time,(char*) ctime(&fileStat.st_atime));
        message.sizeFile = fileStat.st_size;
        message.type = 2;
        tempBufArr[checkCounter] = message;
        checkCounter++;

    }else{
        printf("This is not a directory !!!1\n");
        exit(1);
    }

    postOrderApply(directory1);

    int flag = 0;
    buffer_t *tempResult;


    tempResult = malloc(5000*sizeof(buffer_t));
    int y = 0;

    for(int i = 0 ; i < checkCounter ; i++){
        for(int j = 0 ; j < counter ; j++){
            char arr[1024];
            strcpy(arr,directory);
            strcat(arr,bufArr[j].filename);
            if(strcmp(arr,tempBufArr[i].filename) != 0){
                tempResult[y] = tempBufArr[i];
                y++;
                break;
            }
        }
    }

    for(int i = 0 ; i < counter ; i++){
        /*if(bufArr[i].type == 2)
            continue;*/
        for(int j = 0 ; j < checkCounter ; j++){
            if(tempBufArr[j].type == 2)
                continue;
            char arr[1024];
            strcpy(arr,directory);
            strcat(arr,bufArr[i].filename);
            if(strcmp(arr,tempBufArr[j].filename) == 0){
                flag = 1;

                if(strcmp(tempBufArr[j].mod_time,bufArr[i].mod_time) < 0){
                    fprintf(fp, "%s Değiştirildi \n",bufArr[i].filename );
                    printf("%s\n",bufArr[i].filename);
                    result[x] = bufArr[i];
                    x++;  
                }
                break;
            }
        }
        if(flag == 0){
            fprintf(fp, "%s Eklendi \n",bufArr[i].filename );
            result[x] = bufArr[i];
            x++;
        }else
            flag = 0;
    }


    fclose(fp);

    if( send(newSocket ,&x ,sizeof(int) , 0) < 0){

        printf("Send failed\n");
    
    }

    for(int i = 0 ; i < x ; i++){
        send(newSocket ,result[i].filename ,sizeof(result[i].filename) , 0);
    }
    for(int i = 0 ; i < x ; i++){
        send(newSocket ,result[i].mod_time ,sizeof(result[i].mod_time) , 0);
    }
    for(int i = 0 ; i < x ; i++){
        send(newSocket ,&result[i].type ,sizeof(int) , 0);
    }
    for(int i = 0 ; i < x ; i++){
        send(newSocket ,&result[i].sizeFile ,sizeof(off_t) , 0);
    }

    for(int i = 0 ; i < x ; i++){

        memset(file_path, '\0', sizeof file_path);
        strcpy(file_path,directory);
        strcat(file_path,result[i].filename);

        if(result[i].type == 2){
            mkdir(file_path, 0777);
            continue;
        }

        filefd = open(file_path,O_WRONLY | O_CREAT | O_TRUNC,S_IRUSR | S_IWUSR);
        if (filefd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        char buffer[1024];
        byte = result[i].sizeFile;

        while (byte >= 1024){
            memset(buffer,0,1024);
            read_return = read(newSocket, buffer,1024);

            byte -= 1024;
            if (read_return == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }
            if (write(filefd,buffer,1024) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        } 
        
        read_return = read(newSocket, buffer,byte);
        if (write(filefd,buffer,byte) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
       close(filefd);
    }   
    sizeOfArr = 10;
    checkCounter = 0;
    counter = 0;
    x = 0;
    //pthread_mutex_unlock(&lock);
        
    
    pthread_exit(NULL);

}

int main(int argc, char const *argv[]){

    int serverSocket, newSocket;
    int threadPoolSize;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;

    if(argc != 4){
        fprintf(stdout,"Usage : %s \" server [directory] [threadPoolSize] [portnumber]\"\n",argv[0]);
        return 0;
    }

    threadPoolSize = atoi(argv[2]);
    strcpy(directory,argv[1]);
    strcpy(directory1,argv[1]);
    strcat(directory,"/");


    char hostbuffer[256]; 
    char *IPbuffer; 
    struct hostent *host_entry;
    gethostname(hostbuffer, sizeof(hostbuffer));
    host_entry = gethostbyname(hostbuffer);
    IPbuffer = inet_ntoa(*( (struct in_addr*) host_entry->h_addr_list[0])); 
  

   
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;

    serverAddr.sin_port = htons(8080);

    serverAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);


    bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    if(listen(serverSocket,threadPoolSize)==0)
        printf("Listening\n");
    else
        printf("Error\n");


    pthread_t tid[threadPoolSize+10];
    int i = 0;
    while(1){
        addr_size = sizeof serverStorage;
        newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);

        if(pthread_create(&tid[i], NULL, socketThread, &newSocket) != 0 )
            printf("Failed to create thread\n");
        i++;
        if( i >= threadPoolSize){
            i = 0;
            while(i < threadPoolSize){
                pthread_join(tid[i++],NULL);

            }
            i = 0;
        }
    }
    
    return 0;
}