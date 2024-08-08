/*
ioctl执行硬件控制，除了读写外的其他操作，比如锁门、弹出介质、设置波特率、设置比特位等
long (*unlocked_ioctl)(struct file *filp, unsigned int cmd, unsigned long arg)

命令构成：direction(方向) size(数据大小) type(幻数)   number(序数) 
		 |	   2bits	|	14bits		|	8bits	|	 8bits 	 |
				
宏：_IO(type,nr) _IOR(type,nr,size) _IOW(type,nr,size) _IOWR(type,nr,size)
	_IOC_DIR(nr) _IOC_TYPE(nr) _IOC_NR(nr) _IOC_SIZE(nr)
函数：
	access_ok()				//检查用户空间地址是否OK
	put_user() __put_user()	//向用户空间写数据
	get_user() __get_user()	//从用户空间接收数据
	capable()				//检查进程是否有权限
*/
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/slab.h>
#include<linux/uaccess.h>
#include<linux/sched.h>

#include<linux/semaphore.h>
#include<linux/mutex.h>

#include "hello_chr_locked.h"

#define LOCK_USE 1 //0:semaphore,1:mutex

#define HELLO_MAJOR 0
#define HELLO_NR_DEVS 2

int hello_major = HELLO_MAJOR;
int hello_minor = 0;

dev_t devt;      //高12位是主设备号，低20位是次设备号

int hello_nr_devs = HELLO_NR_DEVS;

module_param(hello_major, int, S_IRUGO);
module_param(hello_minor, int, S_IRUGO);
module_param(hello_nr_devs, int, S_IRUGO);

struct hello_char_dev{		//实际的字符设备结构，类似于面向对象的继承
	struct cdev cdev;
	char *c;
	int n;
	struct semaphore sema;
	struct mutex mtx;
};

struct hello_char_dev *hc_devp;
struct class *hc_cls;

int hc_open(struct inode *inode, struct file *filp)
{
	struct hello_char_dev *hc_dev;
	printk(KERN_INFO "%s open \n",current->comm);
	hc_dev = container_of(inode->i_cdev,struct hello_char_dev,cdev);  //获取设备结构体的地址
	filp->private_data = hc_dev;		//将设备结构地址放到文件描述符结构的私有数据中

	return 0;
}
ssize_t hc_read(struct file *filp, char __user *buf, size_t count,loff_t *f_pos)
{
	ssize_t retval=0;
	struct hello_char_dev *hc_dev=filp->private_data;
	//printk(KERN_INFO "read hc_dev %p\n",hc_dev);
	
	if(*f_pos >= hc_dev->n)
		goto out;
	if(*f_pos + count > hc_dev->n)
		count = hc_dev->n - *f_pos;
	
	if(copy_to_user(buf,hc_dev->c,count))
	{
		retval = -EFAULT;
		goto out;
	}

	*f_pos += count;
	return count;	
out:
	return retval;
}
ssize_t hc_write(struct file *filp, const char __user *buf, size_t count,loff_t *f_pos)
{
	struct hello_char_dev *hc_dev=filp->private_data;
	int retval = -ENOMEM;
	#if (LOCK_USE==0)
	if(down_interruptible(&hc_dev->sema))		//-EINTR
		return -ERESTARTSYS;
	#endif
	#if (LOCK_USE==1)
	if(mutex_lock_interruptible(&hc_dev->mtx))   //-EINTR
		return -ERESTARTSYS;
	#endif

	kfree(hc_dev->c);
	hc_dev->c=NULL;
	hc_dev->n=0;
	hc_dev->c = kzalloc(count,GFP_KERNEL);
	if(!hc_dev->c)
		goto out;
	if(copy_from_user(hc_dev->c,buf,count))
	{
		retval = -EFAULT;
		goto fail_copy;
	}
	hc_dev->n = count;
	
	#if (LOCK_USE==0)
	up(&hc_dev->sema);
	#endif
	#if (LOCK_USE==1)
	mutex_unlock(&hc_dev->mtx);
	#endif
	return count;	 
fail_copy:
	kfree(hc_dev->c);
out:
	#if (LOCK_USE==0)
	up(&hc_dev->sema);
	#endif
	#if (LOCK_USE==1)
	mutex_unlock(&hc_dev->mtx);
	#endif
	return retval;					//不能返回0，否则会不停的写
}
int hc_release(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "%s release\n",current->comm);
	return 0;
}

long hc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct hello_char_dev *hc_dev = filp->private_data;
	long retval = 0;
	int tmp,err=0;
	
	if (_IOC_TYPE(cmd) != HC_IOC_MAGIC) return -ENOTTY;	//检查幻数(返回值POSIX标准规定，也用-EINVAL)
	if (_IOC_NR(cmd) > HC_IOC_MAXNR) return -ENOTTY;	//检查命令编号
	
	if (_IOC_DIR(cmd) & _IOC_READ)		//涉及到用户空间与内核空间数据交互，判断读OK吗？
		err = !access_ok((void __user *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;
	
	switch(cmd){
		case HC_IOC_RESET:
			printk(KERN_INFO "ioctl reset\n");
			kfree(hc_dev->c);
			hc_dev->n=0;
			break;
		case HC_IOCP_GET_LENS:
			printk(KERN_INFO "ioctl get lens through pointer\n");
			retval = __put_user(hc_dev->n,(int __user *)arg);
			break;
		case HC_IOCV_GET_LENS:
			printk(KERN_INFO "ioctl get lens through value\n");
			return hc_dev->n;
			break;
		case HC_IOCP_SET_LENS:
			printk(KERN_INFO "ioctl set lens through pointer");
			if (! capable (CAP_SYS_ADMIN))
				return -EPERM;			
			retval = get_user(tmp,(int __user *)arg);			
			//hc_dev->n = min(hc_dev->n,tmp);
			if(hc_dev->n>tmp)
				hc_dev->n=tmp;
			printk(KERN_INFO " %d\n",hc_dev->n);
			break;
		case HC_IOCV_SET_LENS:
			printk(KERN_INFO "ioctl set lens through value");
			if (! capable (CAP_SYS_ADMIN))
				return -EPERM;			
			hc_dev->n = min(hc_dev->n,(int)arg);
			printk(KERN_INFO " %d\n",hc_dev->n);
			break;
		default:	//前面做了cmd的检查，这里可以不需要
			break;
	}
	
	return retval;
}

struct file_operations hc_fops = {		//字符设备的操作函数
	.owner =    THIS_MODULE,
	.read =     hc_read,
	.write =    hc_write,
	.open =     hc_open,
	.release =  hc_release,
	.unlocked_ioctl = hc_ioctl,			//还有一个compat_ioctl，用于32位程序运行于64位系统上
};

static int __init hello_init(void)	
{
	int ret,i;
	printk(KERN_INFO "---BEGIN HELLO LINUX MODULE---\n");
	if(hello_major){
		devt=MKDEV(hello_major,hello_minor);
		ret=register_chrdev_region(devt,hello_nr_devs,"hello_chr");	//使用指定的设备号分配
	}
	else{
		ret = alloc_chrdev_region(&devt,hello_minor,hello_nr_devs,"hello_chr");//动态分配主设备号
		hello_major = MAJOR(devt);
	}
	if (ret < 0) {
		printk(KERN_WARNING "hello: can't get major %d\n", hello_major);
		goto fail;
	}
	
	hc_devp = kzalloc(sizeof(struct hello_char_dev)*hello_nr_devs,GFP_KERNEL);  //给字符设备分配空间，这里hello_nr_devs为2
	if(!hc_devp)
	{
		printk(KERN_WARNING "alloc mem failed");
		ret = -ENOMEM;
		goto failure_kzalloc;		//内核常用goto处理错误
	}
	
	for(i=0;i<hello_nr_devs;i++){	
	#if (LOCK_USE==0)
		sema_init(&hc_devp[i].sema,1);  //初始化信号量
	#elif (LOCK_USE==1)
		mutex_init(&hc_devp[i].mtx);   //初始化互斥量
	#endif

		cdev_init(&hc_devp[i].cdev,&hc_fops);		//初始化字符设备结构
		hc_devp[i].cdev.owner = THIS_MODULE;
		ret = cdev_add(&hc_devp[i].cdev,MKDEV(hello_major,hello_minor+i),1);
		if(ret)
		{
			printk(KERN_WARNING"fail add hc_dev%d",i);
		}
	}	
	
	hc_cls = class_create(THIS_MODULE,"hc_dev");
	if(!hc_cls)
	{
		printk(KERN_WARNING"fail create class");
		ret = PTR_ERR(hc_cls);
		goto failure_class;
	}
	for(i=0;i<hello_nr_devs;i++){
		device_create(hc_cls,NULL,MKDEV(hello_major,hello_minor+i),NULL,"hc_dev%d",i);
	}	
	printk(KERN_INFO "---END HELLO LINUX MODULE---\n");
	return 0;

failure_class:
	kfree(hc_devp);
failure_kzalloc:		
	unregister_chrdev_region(devt,hello_nr_devs);
fail:
	return ret;	//返回错误，模块无法正常加载
}

static void __exit hello_exit(void)
{
	int i;	
	for(i=0;i<hello_nr_devs;i++)
	{
		device_destroy(hc_cls,MKDEV(hello_major,hello_minor+i));
	}
	class_destroy(hc_cls);
	for(i=0;i<hello_nr_devs;i++)
	{
		kfree(hc_devp[i].c);
		cdev_del(&hc_devp[i].cdev);
	}
	kfree(hc_devp);
	unregister_chrdev_region(devt,hello_nr_devs);	//移除模块时释放设备号		
	printk(KERN_INFO "GOODBYE LINUX\n");
}

module_init(hello_init);
module_exit(hello_exit);

//描述性定义
MODULE_LICENSE("Dual BSD/GPL");//许可 GPL、GPL v2、Dual MPL/GPL、Proprietary(专有)等,没有内核会提示
MODULE_AUTHOR("KGZ");		//作者
MODULE_VERSION("V1.0");  	//版本

////////
/*****补充     
1、compat_ioctl：支持64bit的driver必须要实现的ioctl，当有32bit的userspace application call 64bit kernel的IOCTL的时候，这个callback会被调用到。如果没有实现compat_ioctl，那么32位的用户程序在64位的kernel上执行ioctl时会返回错误：Not a typewriter

2、如果是64位的用户程序运行在64位的kernel上，调用的是unlocked_ioctl，如果是32位的APP运行在32位的kernel上，调用的也是unlocked_ioctl
*****/

