#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
int main(int argc ,char* argv[])
{
	int n;
	char c,buf[20];
	int fd;
    memset(buf,0,20);
	while(1){
        fd = open("/dev/hc_dev0",O_RDWR);
		n=5;
	    n = read(fd,buf,n);
	    printf("%d:%s\n",n,buf);
		memset(buf,0,n);
		sleep(1);
	    close(fd);
	}

	return 0;
}
