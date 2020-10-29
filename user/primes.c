#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(){
    int pp[2];
    pipe(pp);
    // int cnt = 0;
    for (int i=2; i<=35; ++i){
        write(pp[1], &i, sizeof(i));
        // ++cnt;
    }
    close(pp[1]);
    // printf("pp[0]:%d, pp[1]:%d\n", pp[0], pp[1]);

    int todo = 1;
    if(fork()==0){
        while (todo){
            // printf("start!\n");
            todo = 0;
            int n = 0, t, pc[2];
            int fir = 1;
            while (read(pp[0], &t, sizeof(t))){
                // printf("read: %d\n", t);
                if(fir) {
                    fir = 0;
                    n = t;
                    printf("prime %d\n", n);
                    pipe(pc);
                    // printf("pc[0]:%d, pc[1]:%d\n", pc[0], pc[1]);
                    continue;
                }

                if(t%n!=0){
                    todo = 1;
                    // printf("wirte t:%d\n", t);
                    write(pc[1], &t, sizeof(t));
                }
                
            }
            close(pp[0]);
            close(pc[1]);
            if(todo){
                pp[0] = pc[0];
                // printf("pp[0]:%d, pp[1]:%d\n", pp[0], pp[1]);
                if(fork() > 0) while(wait()>0);
            }
        }
    }
    else while(wait()>0);
    exit();
}