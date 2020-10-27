#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(){
    int p0[2];
    pipe(p0);
    for (int i=2; i<=35; ++i){
        write(p0[1], i, 4);
    }
    if(fork()==0){
        star:
        int *pl = p0;
        int n; read(pl[0], n, 4);
        printf("prime %d", n);
        int t;
        int pr[2]; pipe(pr);
        bool todo = false;
        while(read(pl[0], t, 4) > 0){
            if(t%n!=0){
                todo = true;
                write(pr[1], t, 4);
            }
        }
        pl = pr;
        if(todo){
            // todo = false;
            if(fork()==0) goto star;
        }
    }
    exit();
}