#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argv, int **argc){
	int pp[2], cp[2];
	pipe(pp);
	pipe(cp);

	if(fork()==0){
		char buf[10];
		read(pp[0], buf, 5);
		printf("%d: received %s",getpid(), buf);
		write(cp[1], "pong\n", 5);
		return 0;
	}else{
		char buf[10];
		write(pp[1], "ping\n", 5);
		wait();
		read(cp[0], buf, 5);
		printf("%d: received %s",getpid(), buf);
	}	
	return 0;
}
