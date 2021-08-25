#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
int main(int argc ,char* argv[])
{
	int n;
	char c,*buf="12345";
	int fd;
	while(1){
        fd = open("/dev/hc_dev0",O_RDWR);
		n=5;
	    n = write(fd,buf,n);
	    printf("%d:%s\n",n,buf);
		sleep(1);
	    close(fd);
	}

	return 0;
}
