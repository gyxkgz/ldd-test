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
	while((c=getchar())!='x'){
	    fd = open("/dev/hc_dev0",O_RDWR);
		printf("input a num:\n");
	    scanf("%d",&n);
		printf("get num:%d\n",n);
	    n = read(fd,buf,n);
	    printf("%d:%s\n",n,buf);
		write(fd,"hello",5);
		memset(buf,0,n);
			close(fd);
	}

	return 0;
}
