#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

int postOrderApply (char *path,int pathfun(char *path1));
int sizepathfun (char *path);
void printFile();
void sigintHandler(int sig_num);


char commonFifo[] = "161044110sizes";
int z = 0;


int main(int argc,char **argv){

	pid_t pid;
	int size = 0;
	int parent_id = getpid();
	char buffer[1024];
	int pfd[2];
	int commonFd = 0;
	int processSize = 0;
    
	
	
	signal(SIGINT,sigintHandler);
	signal(SIGTSTP,sigintHandler);

    if (argc != 2  && argc != 3) {
        fprintf(stdout, "Usage: %s \"buNeDuFPF [-z] directory_name\"\n", argv[0]);
        return 0;
    }

    if(argc==2){
    	char buffer2[1000];
    	char buffer3[1000];
        z = 0;
        printf("Output of “buNeDu %s” don’t add subdirectory sizes:\n",argv[1]);
        printf("PID      SIZE       PATH\n");
        if(mkfifo(commonFifo, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ) == -1){
	    	perror("Fifo error!!!!!!!!!!!!!!!!!!!!");
	    	exit(1);
		}

        pipe(pfd);
        pid = fork();
        if(pid == -1){
            perror("fork error :\n");
            exit(1);

         }else if(pid == 0){
         		while((( commonFd = open(commonFifo,O_WRONLY)) == -1 ) && (errno = EINTR));
	            size = postOrderApply(argv[1],sizepathfun);
	            strcpy(buffer2,argv[1]);
	            snprintf(buffer, sizeof(buffer), "%d", size);
	            sprintf(buffer3, "%ld      %s       %s\n",getpid(),buffer,buffer2);
	            write(commonFd,buffer3,sizeof(buffer3));
		   		memset(buffer,'\0',sizeof(buffer));
		   		close(commonFd);	       
            	exit(1);
            }else{            
            	int ret = 0;
		   		while((( commonFd = open(commonFifo,O_RDONLY)) == -1 ) && (errno = EINTR));
		   		while((ret = read(commonFd,buffer,sizeof(buffer))) != -1){
		   			sscanf(buffer, "%*s%s",buffer2);
		   			if(ret == 0)
		   				break;
		   			if(strcmp(buffer2,"-1"))
		   				processSize++;
		   			 printf("%s",buffer );
		   		
		   		}
		   		printf("%d child process created. Main process is %ld\n",processSize,getpid());
		   		wait(NULL);

		   		close(commonFd);
		   		unlink(commonFifo);
		   	}	
    }
    if(argc==3){
    	char buffer2[1024];
    	char buffer3[1024];
    	if(strcmp(argv[1],"-z") !=0 ){
            fprintf(stdout, "Usage: %s \" buNeDuFPF [-z] directory_name \"\n", argv[0]);
            return 0;
        }
        z=1;
        printf("Output of “buNeDu -z %s” gives total sizes:\n",argv[2]);
        printf("PID      SIZE       PATH\n");

        if(mkfifo(commonFifo, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ) == -1){
	    	perror("Fifo error!!!!!!!!!!!!!!!!!!!!");
	    	exit(1);
		}

        pipe(pfd);
        pid = fork();
        if(pid == -1){
            perror("fork error :\n");
            exit(1);

         }else if(pid == 0){
         		while((( commonFd = open(commonFifo,O_WRONLY)) == -1 ) && (errno = EINTR));
	            size = postOrderApply(argv[2],sizepathfun);
	            strcpy(buffer2,argv[2]);
	            snprintf(buffer, sizeof(buffer), "%d", size);
	            sprintf(buffer3, "%ld      %s       %s\n",getpid(),buffer,buffer2);
	            write(commonFd,buffer3,sizeof(buffer3));
		   		memset(buffer,'\0',sizeof(buffer));
		   		close(commonFd);
	       
            	exit(1);
            }else{            
            	int ret = 0;
		   		while((( commonFd = open(commonFifo,O_RDONLY)) == -1 ) && (errno = EINTR));
		   		while((ret = read(commonFd,buffer,sizeof(buffer))) != -1){
		   			sscanf(buffer, "%*s%s",buffer2);
		   			if(ret == 0)
		   				break;
		   			if(strcmp(buffer2,"-1"))
		   				processSize++;
		   			 printf("%s",buffer );
		   		
		   		}
		   		printf("%d child process created. Main process is %ld\n",processSize,getpid());
		   		wait(NULL);

		   		close(commonFd);
		   		unlink(commonFifo);
		   	}	
    }
    
    return 0;
}
int postOrderApply (char *path,int pathfun(char *path1)){

	DIR *dir;
    struct dirent *entry;
    int sumSize = 0,fifofd = 0;
    pid_t pid;
    char fifoName[1024];
    char buffer[1024];
    char buffer2[1024];
    char pathDir[1024];
    int pfd[2];
    int commonFb = 0;

   

	if (!(dir = opendir(path))){
        printf("Cannot read folder");
        return -1;
    }
    while (1){
        if((entry = readdir(dir)) == NULL){            
            break;
        }
        if (entry->d_type == DT_DIR) {
        	
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            while(((commonFb = open(commonFifo,  O_WRONLY | O_NONBLOCK) ) == -1) && (errno = EINTR));

           		snprintf(pathDir, sizeof(pathDir), "%s/%s", path, entry->d_name);
           		if (pipe (pfd)){
		            printf (stderr, "Pipe failed.\n");
		            return EXIT_FAILURE;
	            }
           		pid = fork();
           		
           		if(pid == -1){
           			perror("fork error :\n");
                    exit(1);
           		}else if(pid == 0){
           			int tempSize = 0;
           			closedir(dir);
           			tempSize = postOrderApply(pathDir,pathfun);
           			close(pfd[0]);
   					snprintf(buffer, sizeof(buffer), "%d", tempSize);
   					write(pfd[1], buffer,(strlen(buffer)));
   					memset(buffer,'\0',sizeof(buffer));
   					
   					close(pfd[1]);
   					exit(1);
   				} else {
   				  	int temp = 0;
   				  	
   				  	close(pfd[1]);
			   		read(pfd[0],buffer,sizeof(buffer));
			   		sscanf(buffer,"%d",&temp);
			   		memset(buffer,'\0',sizeof(buffer));

   					if(z == 1)
			   			sumSize += temp;
			   		
			   		sprintf(buffer2, "%ld      %d      %s\n", pid,temp,pathDir);
			   		write(commonFb,buffer2,sizeof(buffer2));
			   		memset(buffer2,'\0',sizeof(buffer2));	
			   		close(pfd[0]);	
			   		wait(NULL);
           		}
        }
        
        if(entry->d_type == DT_LNK || entry->d_type == DT_FIFO || entry->d_type == DT_BLK || entry->d_type == DT_CHR || entry->d_type == DT_SOCK ){
        	char buffer2[1024];
	   		while(((commonFb = open(commonFifo,  O_WRONLY | O_NONBLOCK) ) == -1) && (errno = EINTR));
        	sprintf(buffer2, "%ld      %d      Special_File_%s\n", getpid(),-1,entry->d_name);
	   		write(commonFb,buffer2,sizeof(buffer2));
	   		memset(buffer2,'\0',sizeof(buffer2));
	   		close(commonFb);        
        }
        if(entry->d_type == DT_REG){
            snprintf(pathDir, sizeof(pathDir), "%s/%s", path, entry->d_name);
            sumSize  += pathfun(pathDir);
        }
    }
    closedir(dir);   
	return sumSize;
}

int sizepathfun (char *path){
    struct stat fileStat;
    if(lstat(path,&fileStat) == -1)    
        return -1;

    return fileStat.st_size;
}

void sigintHandler(int sig_num) { 
	signal(SIGINT,sigintHandler);
	signal(SIGTSTP,sigintHandler);
	printf("\n Cannot be Terminated using Ctrl + c \n");
	fflush(stdout);
   
} 


	