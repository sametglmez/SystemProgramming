#define BUFSIZE 100

typedef struct {
	off_t sizeFile;
	int type;
	//int outfd;
	char filename[1024];
	char mod_time[30];
} buffer_t;



