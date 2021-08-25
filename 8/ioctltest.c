#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<sys/ioctl.h>
#include<errno.h>
#include"hello_chr_locked.h"
int main(int argc ,char* argv[])
{
	int n,retval=0;
	int fd;
	fd = open("/dev/hc_dev0",O_RDWR);
	switch(argv[1][0])
	{
		case '0':
			ioctl(fd,HC_IOC_RESET);
	    	printf("reset hc\n");
			break;
		case '1':
			ioctl(fd,HC_IOCP_GET_LENS,&n);
	    	printf("get lens pointer, %d\n",n);
			break;
		case '2':
			n = ioctl(fd,HC_IOCV_GET_LENS);
	    	printf("get lens value, %d\n",n);
			break;
		case '3':
			n=argv[2][0]-'0';
			retval = ioctl(fd,HC_IOCP_SET_LENS,&n);
	    	printf("set lens value, %d %s\n",n,strerror(errno));
	 		break;
		case '4':
			n=argv[2][0]-'0';
			retval = ioctl(fd,HC_IOCV_SET_LENS,n);
	    	printf("set lens value, %d %s\n",n,strerror(errno));
	 		break;
	}
	close(fd);
	
	return 0;
}
