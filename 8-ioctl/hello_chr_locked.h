#ifndef _HELLO_CHR_LOCKED_H_
#define _HELLO_CHR_LOCKED_H_

#define HC_IOC_MAGIC 0x81		//Documentation/userspace-api/ioctl/ioctl-number.rst

#define HC_IOC_RESET		_IO(HC_IOC_MAGIC,0)
#define HC_IOCP_GET_LENS	_IOR(HC_IOC_MAGIC,1,int)	//通过指针返回
#define HC_IOCV_GET_LENS	_IO(HC_IOC_MAGIC,2)		//通过返回值返回
#define HC_IOCP_SET_LENS 	_IOW(HC_IOC_MAGIC,3,int)	//通过指针设置
#define HC_IOCV_SET_LENS	_IO(HC_IOC_MAGIC,4)		//通过值设置
 
/*正常情况不会混用，要不都按值传递，要不都用指针，这里只是演示*/

#define HC_IOC_MAXNR 4
#endif