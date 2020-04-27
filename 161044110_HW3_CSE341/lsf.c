#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

int sizepathfun (char *path);
int printResult(char *path,int flag);

int main(int argc, char const *argv[]){

	DIR *dir;
	struct stat fileStat;
    struct dirent *entry;
	char pathName[1024];
	char result[1024];
	FILE * fp;

    if(argc > 1){
        printf("ERROR !!!!!\n");
        exit(1);
    }
  
	if(getcwd(pathName,sizeof(pathName)) == NULL)
		printf("ERROR !!!!! %s \n",strerror(errno));

	if (!(dir = opendir(pathName))){
        printf("Cannot read folder !!!!!!!!!!");
        return -1;
    }

    while (1){
        char pathDir[1024];
        //donguyu durdurma kosulu
        if((entry = readdir(dir)) == NULL){
            break;
        }

        if(entry->d_type == DT_LNK || entry->d_type == DT_FIFO || entry->d_type == DT_BLK || entry->d_type == DT_CHR || entry->d_type == DT_SOCK ){
            if(printResult(entry->d_name,1) == -1 )
                return -1;
        }
        if(entry->d_type == DT_REG){
        	if(printResult(entry->d_name,0) == -1 )
                return -1;
        }

    }	
    fflush(0);
	return 0;
}
int printResult(char *path, int flag){
    struct stat fileStat;
    int sumSize = 0;

    if(lstat(path,&fileStat) == -1)    
                return -1;
            if(flag == 0)
                printf("R\t");
            else
                printf("S\t");
            printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
            printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
            printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
            printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
            printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
            printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
            printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
            printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
            printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
            printf("\t");
            sumSize = sizepathfun(path);
            printf("%10d\t",sumSize );
            printf("%s\n",path );
           
}

int sizepathfun (char *path){
	//Boyutu bulabilmek icin stat structı kullanildi
    struct stat fileStat;
    if(lstat(path,&fileStat) == -1)    
        return -1;

    return fileStat.st_size;
}