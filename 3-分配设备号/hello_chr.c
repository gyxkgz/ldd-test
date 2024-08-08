/*
*  	register_chrdev_region()
*	alloc_chrdev_region()
*	unregister_chrdev_region()
	MAJOR()
	MINOR()
	MKDEV()
*/
#include<linux/module.h>

//#include<linux/types.h>	//dev_t
//#include<linux/kdev_t.h>	//MAJOR/MINOR/MKDEV
#include<linux/fs.h>		//注册函数

#define HELLO_MAJOR 0
#define HELLO_NR_DEVS 2

int hello_major = HELLO_MAJOR;
int hello_minor = 0;

dev_t hello_dev;      //高12位是主设备号，低20位是次设备号

int hello_nr_devs = HELLO_NR_DEVS;

module_param(hello_major, int, S_IRUGO);
module_param(hello_minor, int, S_IRUGO);
module_param(hello_nr_devs, int, S_IRUGO);

static int __init hello_init(void)	
{
	int ret;
	printk(KERN_INFO "---BEGIN HELLO LINUX MODULE---\n");
	if(hello_major){
		hello_dev=MKDEV(hello_major,hello_minor);
		ret=register_chrdev_region(hello_dev,hello_nr_devs,"hello_chr");	//使用指定的设备号分配
	}
	else{
		ret = alloc_chrdev_region(&hello_dev,hello_minor,hello_nr_devs,"hello_chr");//动态分配主设备号
		hello_major = MAJOR(hello_dev);
	}
	if (ret < 0) {
		printk(KERN_WARNING "hello: can't get major %d\n", hello_major);
		return ret;
	}
	printk(KERN_INFO "hello_chr:%d hello_dev:%x",hello_major,hello_dev);
	printk(KERN_INFO "---END HELLO LINUX MODULE---\n");
	return 0;
}

static void __exit hello_exit(void)
{
	unregister_chrdev_region(hello_dev,hello_nr_devs);	//移除模块时释放设备号		
	printk(KERN_INFO "GOODBYE LINUX\n");
}

module_init(hello_init);
module_exit(hello_exit);

//描述性定义
MODULE_LICENSE("Dual BSD/GPL");//许可 GPL、GPL v2、Dual MPL/GPL、Proprietary(专有)等,没有内核会提示
MODULE_AUTHOR("KGZ");		//作者
MODULE_VERSION("V1.0");  	//版本
