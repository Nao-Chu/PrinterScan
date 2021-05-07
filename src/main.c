#include "../inc/log.h"
#include "../inc/user.h"

#include <pthread.h>

int main(int argc,char **argv) 
{
	LogWrite(INFO,"\n %s","Run Start!!!");
	if (argc != 3){
		printf("user should input: vid pid\n");
		exit(0);
	}

	if(pthread_create(&scan_id,NULL,ScanJob,(void *)argv) == -1) {
              fprintf(stderr,"pthread scanJob create error!\n");
        }
	UserOper(argv);
	while (1);
	
	return 0;
}
