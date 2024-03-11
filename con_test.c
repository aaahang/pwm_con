#include "stdio.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

union 
{
    struct {
        int pwm,con;
    }ik;
    char uk[16];
}pwm_k;
int main(int argc,char **argv)
{
    printf("line is %d\n",__LINE__);
    int fd = open(argv[1],O_WRONLY);
    if(fd <0 )
    {
        printf(" open is  error");
        exit(1);
    }
    printf("line is %d\n",__LINE__);
    while(1)
    {
        printf("line is %d\n",__LINE__);
        scanf("%d,%d",&pwm_k.ik.pwm,&pwm_k.ik.con);
        printf("pwm is %d  con is %d\n",pwm_k.ik.pwm,pwm_k.ik.con);
        write(fd,pwm_k.uk,16);
        sleep(1);
    }
    return 0;
}