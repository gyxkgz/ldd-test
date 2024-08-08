/*
	class_create()		//创建类  会在/sys/class/下创建文件夹
	class_destroy()
	
	device_create()		//在我们创建的类下创建设备，有了这步就会在/dev生成设备节点
	device_destroy()
*/
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/slab.h>
//#include <linux/device.h>   //cdev里边有

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
	char c;	
};

struct hello_char_dev *hc_devp;
struct class *hc_cls;

int hc_open(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "open hc_dev%d %d\n",iminor(inode),MINOR(inode->i_cdev->dev));
	return 0;
}
ssize_t hc_read(struct file *filp, char __user *buf, size_t count,loff_t *f_pos)
{
	printk(KERN_INFO "read hc_dev\n");
	return 0;
}
ssize_t hc_write(struct file *filp, const char __user *buf, size_t count,loff_t *f_pos)
{
	printk(KERN_INFO "write hc_dev\n");
	return count;					//不能返回0，否则会不停的写
}

int hc_release(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "release hc_dev\n");
	return 0;
}

struct file_operations hc_fops = {		//字符设备的操作函数
	.owner =    THIS_MODULE,
	.read =     hc_read,
	.write =    hc_write,
	.open =     hc_open,
	.release =  hc_release,
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
		cdev_init(&hc_devp[i].cdev,&hc_fops);		//初始化字符设备结构
		hc_devp[i].cdev.owner = THIS_MODULE;
		ret = cdev_add(&hc_devp[i].cdev,MKDEV(hello_major,hello_minor+i),1);  //添加该字符设备到系统中
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
		cdev_del(&hc_devp[i].cdev);
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

/*
 内核中定义了struct class结构体，顾名思义，一个struct class结构体类型变量对应一个类，
 内核同时提供了class_create(…)函数， 可以用它来创建一个类，这个类存放于sysfs下面，
 一旦创建好了这个类，再调用device_create(…)函数来在/dev目录下创建相应的设备节点。
 这样，加载模块的时候，用户空间中的udev会自动响应device_create(…)函数，
 去/sysfs下寻找对应的类从而创建设备节点。
 */
