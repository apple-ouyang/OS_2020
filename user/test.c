#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    int p[2];
    pipe(p);
    printf("p[0]:%d, p[1]:%d\n", p[0], p[1]);
    int pd[2];
    pd[0] = dup(p[0]);
    pd[1] = dup(p[1]);
    printf("pd[0]:%d, pd[1]:%d\n", pd[0], pd[1]);
    exit();
}