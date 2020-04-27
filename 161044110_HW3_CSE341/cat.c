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

    if(argc == 1){
      file = stdin;
      char buf[1024];
      
      while(fgets(buf,1024,file) != NULL){       
        printf("%s",buf);
        fflush(stdout);
      }

   }else{
      strcpy(input,argv[1]);

      file = fopen(input,"r");
      ch = getc(file);

      while( ch  != EOF){

        printf("%c",ch );
        ch = getc(file);

      }
    }


	fclose(file);
  fflush(0);
	return 0;
}