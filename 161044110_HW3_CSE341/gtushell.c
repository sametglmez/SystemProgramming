#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>


#define BLANK_STRING " "

void executecmd(char *buffer,char *oldPath);
int parseCommand(const char *s, const char *delimiters, char *argvp[]);
void fillPipes(int numberOfCommand, char *tokens[], char *leftPipes[], char *rightPipes[]);
void help();
int readFromFile(char *buffer);
int writeToFile(int fd, char *buffer);
void sigintHandler(int sig_num);
//void child_trap(int sig) {fprintf(stderr, "Caught signal in CHILD.\n");}

int main (void) {
	char buffer[MAX_CANON];
	char tempBuffer[MAX_CANON];
	char oldPath[1024] = "";
	char exclamation[1024][1024];
	int counter = 0,exclamationIndex=0;
	int length;

	signal(SIGINT,sigintHandler);

	if(getcwd(oldPath,sizeof(oldPath)) == NULL)
      	perror("getcwd() error");

	while(1){

		fflush(stdout);
    	fflush(stdin);

		puts("gtushel>> ");

		/*if (fgets(buffer, MAX_CANON, stdin) == NULL)
			continue;*/

		scanf("%[^\n]%*c", buffer);

		if (strcmp(buffer,"exit") == 0){

			//fflush(stdout);
	    	//fflush(stdin);
			_exit(0);
		}

		length = strlen(buffer);

		if (buffer[length - 1] == '\n')
			buffer[length - 1] = 0;


		if(buffer[0] == '!' ){
			int j = 0;
			for(int i = 1 ; buffer[i] != '\0'; i++){
				tempBuffer[j] = buffer[i];
				j++;
			}
			sscanf(tempBuffer,"%d",&exclamationIndex);

			if((counter - exclamationIndex ) < 0){
				printf("Wrong Number ( N! )\n");
				continue;
			}
			strcpy(buffer,exclamation[counter - exclamationIndex]);
		}

		strcpy(exclamation[counter],buffer);
		executecmd(buffer,oldPath);
		counter++;
	}

	return 0;
}

void executecmd(char *buffer,char *oldPath) {
 	char *tokens[1024] = {};
 	char *leftPipes[1024];
 	char *rightPipes[1024];
 	char tempPath[200] = "";
    char cwd[1024];
    int numberOfCommand = 0,flag = 0;
    pid_t pid,pid2;
    int pipe1[2];
    int pipe2[2];
    int pipe3[2];


    numberOfCommand = parseCommand(buffer," ",tokens);

	if ( numberOfCommand <= 0) {
		fprintf(stderr, "Failed to parse command line\n");
		exit(1);
	}

	for(int i = 0 ; i < numberOfCommand; i++){

		//printf("----------> %s\n",tokens[i] );
		if(strstr(tokens[i], "|") != NULL){
			flag = 1;
		}
		if(strstr(tokens[i], "<") != NULL){
			flag = 2;
		}
		if(strstr(tokens[i], ">") != NULL){
			flag = 3;
		}
	}
	
	if(strcmp(tokens[0],"cd") == 0){
	    strcpy(tempPath,tokens[1]);

	    if(tokens[0] != "/"){
	        if (getcwd(cwd, sizeof(cwd)) == NULL)
      			perror("getcwd() error");
	        strcat(cwd,"/");
	        strcat(cwd,tempPath);
	        if(chdir(cwd) == -1 ){
	            fprintf(stdout,"there is not such a directory.\n");
	        }
	    }else{
	        if(chdir(tokens[0]) == -1){
	            fprintf(stdout,"there is not such a directory.\n");
	        }
	    }
    }else if(strcmp(tokens[0],"help") == 0){
	   help();

    }else if(flag == 1){
    	/* --------------- FOR ' | ' ----------------- */
    	memset(leftPipes,'\0',sizeof(leftPipes));
    	memset(leftPipes,'\0',sizeof(rightPipes));
    	fillPipes(numberOfCommand,tokens,leftPipes,rightPipes);
    	pipe(pipe1);

    	pid = fork();
		if (pid == -1){
			perror("Failed to fork child");

		}else if(pid == 0){
			dup2(pipe1[1],1); // redirect stdout to pipe
		    close(pipe1[0]);
		    strcat(oldPath,"/");
            strcat(oldPath,leftPipes[0]);
            execvp(oldPath,leftPipes);
			perror("Failed to execute command");
            exit(0);

		}else{
			close(pipe1[1]);
			
		}
		
		pid2 = fork();
		if (pid2 == -1){
			perror("Failed to fork child");

		}else if(pid2 == 0){
			dup2(pipe1[0],0); // get stdin from pipe

			strcat(oldPath,"/");
            strcat(oldPath,rightPipes[0]);
            execvp(oldPath,rightPipes);
			perror("Failed to execute command");
            exit(0);

		}else{
			close(pipe1[0]);
       		wait(NULL);
		}
        wait(NULL);
		
    }else if(flag == 2){
    	/* --------------- FOR ' < ' ----------------- */
    	memset(leftPipes,'\0',sizeof(leftPipes));
    	memset(rightPipes,'\0',sizeof(rightPipes));
    	//memset(rightPipes[0],'\0',sizeof(rightPipes[0]));
    	fillPipes(numberOfCommand,tokens,leftPipes,rightPipes);
    	pipe(pipe1);

    	pid = fork();
		if (pid == -1){
			perror("Failed to fork child");

		}else if(pid == 0){
			dup2(pipe1[1],1); // redirect stdout to pipe
		    close(pipe1[0]);

			readFromFile(rightPipes[0]);

            exit(0);

		}else{
			close(pipe1[1]);
			
		}
		
		pid2 = fork();
		if (pid2 == -1){
			perror("Failed to fork child");

		}else if(pid2 == 0){
			dup2(pipe1[0],0); // get stdin from pipe



			strcat(oldPath,"/");
            strcat(oldPath,leftPipes[0]);
            execvp(oldPath,leftPipes);
			perror("Failed to execute command");
            exit(0);

		}else{
			close(pipe1[0]);
			wait(NULL);
		}
		wait(NULL);
    	
    }else if(flag == 3){ 
    	memset(leftPipes,'\0',sizeof(leftPipes));
    	memset(rightPipes,'\0',sizeof(rightPipes));
    	/* --------------- FOR ' > ' ----------------- */
    	fillPipes(numberOfCommand,tokens,leftPipes,rightPipes);
    	
    	pipe(pipe1);

    	pid = fork();
		if (pid == -1){
			perror("Failed to fork child");

		}else if(pid == 0){

			dup2(pipe1[1],1); // redirect stdout to pipe
			
			strcat(oldPath,"/");
            strcat(oldPath,leftPipes[0]);
            execvp(oldPath,leftPipes);
			perror("Failed to execute command");

		    close(pipe1[0]);

            exit(0);

		}else{
			close(pipe1[1]);
			
		}

		pid2 = fork();

		if (pid2 == -1){
			perror("Failed to fork child");

		}else if(pid2 == 0){
			dup2(pipe1[0],0); // get stdin from pipe

			writeToFile(pipe1[0],rightPipes[0]);

            exit(0);

		}else{
			close(pipe1[0]);
			wait(NULL);
		}
		wait(NULL);

    }else {
    	pid = fork();
		if (pid == -1){
			perror("Failed to fork child");

		}else if(pid == 0){

			strcat(oldPath,"/");
            strcat(oldPath,tokens[0]);
            execvp(oldPath,tokens);
			perror("Failed to execute command");
            exit(0);

		} else{

			wait(NULL);

		}
    }
}

void fillPipes(int numberOfCommand, char *tokens[], char *leftPipes[], char *rightPipes[]){
	int flag = 0 ;
	int j = 0 ;

	for(int i = 0 ; i < numberOfCommand; i++){
		if((strstr(tokens[i], "|" ) != NULL) | (strstr(tokens[i], "<" ) != NULL) | (strstr(tokens[i], ">" ) != NULL)){
			flag = 1;
			j = 0;
			continue;
		}
		if(flag == 0){
			//printf("-------------< Left1 \n");

			leftPipes[j] = tokens[i];
			//printf("-------------< Left2 \n");
			j++;
		}else if(flag == 1){
			rightPipes[j] = tokens[i];
			j++;
		}
	}
}

int parseCommand(const char *s, const char *delimiters, char *argvp[]) {
	int error;
	int i,k;
	int numtokens;
	char temp[1024] = "", t[1024] = "";
	char *flag;


	if ((s == NULL) || (argvp == NULL)) {
		errno = EINVAL;
		return -1;
	}

	for(k = 0 ; s[k] == ' ' ; k++);

	/*if ((temp = malloc(strlen(s) - k)) == NULL)
		return -1;
	if ((t = malloc(strlen(s) - k)) == NULL)
		return -1;*/

	for(int j = 0 ; s[k] != '\0' ; j++){
		temp[j] = s[k];
		k++;
	}

	strcpy(t,temp);

	numtokens = 0;

	flag = strtok(t," \n\t");

	while(flag != NULL){
		argvp[numtokens] = flag;
		numtokens++;
		flag = strtok(NULL," \n\t");
	}
	//free(t);
	//free(temp);
	argvp[numtokens] = NULL;
	return numtokens;
}

void help(){
    printf("- ls; which will list file type, access rights, file size and file name of all files in the present working\n"
                   "directory\n");
    printf("- pwd; which will print the present working directory\n") ;
    printf("- cd; which will change the present working directory to the location provided as argument\n");
    printf("- help; which will print the list of supported commands\n");
    printf("- cat; which will print on standard output the contents of the file provided to it as argument\n");
    printf("- wc; which will print on standard output the number of lines in the file provided to it as argument\n");
    printf("- exit; which will exit the shell\n");
}

int readFromFile(char *buffer){
	FILE *fp;
	char ch;

	fp = fopen(buffer,"r");

	if(fp == NULL)
		return -1;

	while((ch = fgetc(fp)) != EOF)
      printf("%c", ch);

 
   fclose(fp);
   return 0;
}
int writeToFile(int fd, char *buffer){
	FILE *fp;
	FILE *fd2;
	char ch;

	fd2 = fdopen(fd,"r");
	fp = fopen(buffer,"w");

	if(fp == NULL)
		return -1;

	while((ch = fgetc(fd2)) != EOF){
		fprintf(fp,"%c",ch );
	}

   fclose(fp);
   fclose(fd2);
   return 0;

}
void sigintHandler(int sig_num) { 

	if(sig_num == SIGINT){
		printf("Signal is founded in the SHELL!! \n");
		exit(0);
	}
	if(sig_num == SIGTSTP){
		printf("Signal is founded in the SHELL!! \n ");
		exit(0);
	}
   
}