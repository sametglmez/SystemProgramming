#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#include "buffer.h"


static void *producer(void *arg);
static void *consumer(void * arg);
int initconsumer(pthread_t *tconsumer);
int initproducer(pthread_t *tproducer);

static char sourceDirectory[1024];
static char destinationDirectory[1024];


int main(int argc, char const *argv[]){

	pthread_t ptid,ctid;
	int numberconsumers;
	int numberproducers;
	int sleeptime;
	pthread_t *tidc;
	pthread_t *tidp;
	int i = 0;
	int error;

	strcpy(sourceDirectory,argv[3]);
	strcpy(destinationDirectory,argv[4]);
	

	sleeptime = atoi(argv[2]);
	numberproducers = 1;
	numberconsumers = atoi(argv[1]);

	tidp = (pthread_t *)calloc(numberproducers, sizeof(pthread_t));
	if (tidp == NULL) {
		perror("Failed to allocate space for producer IDs");
		return 1;
	}

	tidc = (pthread_t *)calloc(numberconsumers, sizeof(pthread_t));

	if (tidc == NULL) {
		perror("Failed to allocate space for consumer IDs");
		return 1;
	}

	for (i = 0; i < numberconsumers; i++){
		printf("-----------------< %d\n",i );
		initconsumer(tidc+i);
	}

	
	initproducer(tidp+i);


	sleep(100);
		
	if (error = setdone()) {
		fprintf(stderr, "Failed to set done indicator:%s\n", strerror(error));
		return 1;
	}

	for (i = 0; i < numberproducers; i++)
		pthread_join(tidp[i], NULL);
			

	for (i = 0; i < numberconsumers; i++)
		pthread_join(tidc[i], NULL);
	
	

	return 0;
}



int initconsumer(pthread_t *tconsumer) {
	int error;
	error = pthread_create(tconsumer, NULL, consumer, NULL);
	return error;
}

int initproducer(pthread_t *tproducer) {
	int error;
	error = pthread_create(tproducer, NULL, producer, NULL);
	return error;
}



static void *consumer(void *arg) {
	int error;
	buffer_t nextitem;
	double value;
	buffer_t destBuffer;


	while(1){
		if(getitem(&destBuffer))
			break;

		printf("Copied : %s \n",destBuffer.filename );

		int err,n;
		unsigned char buf[4096];

		 while (1) {
	        err = read(destBuffer.infd, buf, 4096);
	        if (err == -1) {
	            perror("Error : ");
	            exit(1);
	        }
	        n = err;

	        if (n == 0) 
	        	break;

	        err = write(destBuffer.outfd, buf, n);
	        if (err == -1) {
	            printf("Error writing to file.\n");
	            exit(1);
	        }
	    }

	    close(destBuffer.outfd);
	    close(destBuffer.infd);

	}
	return NULL;
}


static void *producer(void *arg) {
	int error;
	buffer_t item;
	int localdone = 0;
	int inFd;
	char ch;
	int outFd;
	buffer_t sourceBuffer;


	DIR *dir,*dir1;
    struct dirent *entry;
    int sumSize =  0;

   
    if (!(dir = opendir(sourceDirectory))){
        printf("Cannot read folder");
        return NULL;
    }
 
    while (!localdone){
        char pathDir[1024];
        char temppathDir[1024];
        char temppathDir1[1024];
        //donguyu durdurma kosulu
        if((entry = readdir(dir)) == NULL){
        	//error = 1;
            break;
        }
        //Gelen pathinn dosya olup olmadigi kontrol ediliyor
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            snprintf(pathDir, sizeof(pathDir), "%s/%s", sourceDirectory, entry->d_name);
            producer(pathDir);
            printf("%s\n", pathDir);
        
        }else{

        	snprintf(temppathDir1, sizeof(temppathDir1), "%s/%s", sourceDirectory, entry->d_name);
        	inFd = open(temppathDir1, O_RDONLY);

			snprintf(temppathDir, sizeof(temppathDir), "%s/%s", destinationDirectory, entry->d_name);

			outFd = open(temppathDir,O_WRONLY | O_TRUNC | O_CREAT);
			
			sourceBuffer.infd = inFd;
			sourceBuffer.outfd = outFd;
			strcpy(sourceBuffer.filename,entry->d_name);
			if(putitem(sourceBuffer)){
				break;
			}
			if (error = getdone(&localdone)){
				break;
			}
			
        }
    }
	return NULL;
}



