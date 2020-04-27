#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

int main(int argc, char const *argv[]){

  int newLine = 0;

  char buffer[1024];
  char input[1024];
  char ch;
  FILE *file;
  DIR *dir;

  if(argc == 1) {
    
    file = stdin;

    while((fgets(buffer, MAX_CANON, file))) {
        newLine++;
    }

  }else{
    strcpy(input,argv[1]);

    if ((dir = opendir(input))){
        printf("This is not regular or special file !!!!\n");
        return -1;
    }

    file = fopen(input,"r");

    while(1){
      if(feof(file)){
        break;

      }
      ch = getc(file);
      if(ch == '\n')
        newLine++;

    }
}

  printf("\n%d\n",newLine);
  fclose(file);
  fflush(0);

	return 0;
}