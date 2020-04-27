#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

int z;

int postOrderApply (char *path,int pathfun(char *path1));
int sizepathfun (char *path);


int main(int argc,char **argv) {
    FILE *file;
    int i = 1 ;
    size_t len;
    ssize_t n;
    char *argu1[1024];
    char buf[100];

    if(argc == 2){
        if(strcmp(argv[0],"bunedu") == 0 && strcmp(argv[1],"-z") == 0 ){
            scanf("%[^\n]%*c", buf);
            argu1[0] = argv[1];
            argu1[1] = buf;
            i = 3;

        }else{
            argu1[0] = argv[1];
            i++;

        }
    }else if(argc == 3){
        argu1[0] = argv[1];
        argu1[1] = argv[2];
        i = i + 2;
    }else{
        //n = getline( &buf, &len,stdin);
        scanf("%[^\n]%*c", buf);

        argu1[0] = strtok(buf, " ");
        if(argu1[0])
            i++;
        argu1[1] = strtok(NULL, " ");
        if(argu1[1])
            i++;
        
      
    }

	//Eger girilen parametre sayısı az veya fazla ise usage gosteriliyor
    if (i != 2  && i != 3) {
        fprintf(stdout, "Usage: %s \"buNeDu [-z] directory_name\"\n", argv[0]);
        return 0;
    }
    //Girilen parametere sayisi 2 ise ve path dogru verilmis ise islme yapılıyor
    if(i==2){
        /*if(strcmp(argv[0],"buNeDu")!=0){
            fprintf(stdout, "Usage: %s \"buNeDu [-z] directory_name\"\n", argv[0]);
            return 0;
        }*/
        z = 0;
        printf("Output of “buNeDu %s” don’t add subdirectory sizes:\n",argv[0]);
        if(postOrderApply(argu1[0],sizepathfun) == -1){
            printf("Can't read folder ");
            printf("%s\n",argu1[0]);
            return 0;
        }
        printf("%s\n",argu1[0]);

    }
    //Girilen parametre sayisi 3 ise ilk olarak -z argumanı dogru girilmismi kontrol ediliyor
    //Daha sonra path kontrol edilerek fonksiyon cagiriliyor
    if(i==3){
        /*if(strcmp(argv[1],"buNeDu") !=0 || strcmp(argv[1],"-z") !=0 ){
            fprintf(stdout, "Usage: %s \" buNeDu [-z] directory_name \"\n", argv[0]);
            return 0;
        }*/
        if(strcmp(argu1[0],"-z") !=0 ){
            fprintf(stdout, "Usage: %s \" buNeDu [-z] directory_name \"\n", argv[0]);
            return 0;
        }
        z=1;
        printf("Output of “buNeDu -z %s” gives total sizes:\n",argu1[1]);
        if(postOrderApply(argu1[1],sizepathfun) == -1){
            //printf("Can't read folder\n");
            printf("%s\n",argu1[1]);
            return 0;
        }
        printf("%s\n",argu1[1]);

    }
    fflush(0);
    return 0;
}


int postOrderApply (char *path,int pathfun(char *path1)){
    DIR *dir;
    struct dirent *entry;
    int sumSize =  0;

    if (!(dir = opendir(path))){
        printf("Cannot read folder");
        return -1;
    }
 
    while (1){
        char pathDir[1024];
        //donguyu durdurma kosulu
        if((entry = readdir(dir)) == NULL){
            printf("%d   ",sumSize);
            break;
        }
        //Gelen pathinn dosya olup olmadigi kontrol ediliyor
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            snprintf(pathDir, sizeof(pathDir), "%s/%s", path, entry->d_name);
            if(z == 1){
            	//Komut -z olarak cagirildiği zaman bu kosulun icine giriyor
                int temp = postOrderApply(pathDir,pathfun);
                if(temp == -1)
                    sumSize += 0;
                else
                    sumSize += temp;
            }else{
            	// -z olarak girilmedigi zaman
                postOrderApply(pathDir,pathfun);
            }
            printf("%s\n", pathDir);
        }
        //ozel dosyalarin komtelleri yapiliyor
        if(entry->d_type == DT_LNK || entry->d_type == DT_FIFO || entry->d_type == DT_BLK || entry->d_type == DT_CHR || entry->d_type == DT_SOCK ){
            printf("Special File %s\n",entry->d_name);
        }
        //Regular file ise boyutunun bulunmasi icin pathfun() fonksiyonu cagirmak gerekiyor.
        if(entry->d_type == DT_REG){
            snprintf(pathDir, sizeof(pathDir), "%s/%s", path, entry->d_name);
            sumSize += pathfun(pathDir);
        }
    }
    closedir(dir);
    return sumSize;
}
int sizepathfun (char *path){
	//Boyutu bulabilmek icin stat structı kullanildi
    struct stat fileStat;
    if(lstat(path,&fileStat) == -1)    
        return -1;

    return fileStat.st_size;
}


