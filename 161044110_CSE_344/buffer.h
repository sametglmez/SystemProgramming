#define BUFSIZE 100

typedef struct {
	int infd;
	int outfd;
	char filename[1024];
} buffer_t;

int getitem(buffer_t *itemp);
int putitem(buffer_t item);
int getdone(int *flag);
int setdone(void);



