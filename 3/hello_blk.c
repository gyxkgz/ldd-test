/*
*	register_blkdev()
*	unregister_blkdev()
*/

#include<linux/module.h>

//#include<linux/types.h>	//dev_t
//#include<linux/kdev_t.h>	//MAJOR/MINOR/MKDEV
#include<linux/fs.h>		//注册函数

#define HELLO_MAJOR 0

int hello_major = HELLO_MAJOR;

module_param(hello_major, int, S_IRUGO);

static int __init hello_init(void)	
{
	int ret;
	printk(KERN_INFO "---BEGIN HELLO LINUX MODULE---\n");
	ret = register_blkdev(hello_major,"hello_blk");
	if (ret < 0) {
		printk(KERN_WARNING "hello_blk: can't get major %d\n", hello_major);
		return ret;
	}
	if(!hello_major)
	{
		hello_major=ret;
	}
	printk(KERN_INFO "hello_blk:%d ret:%d",hello_major,ret);
	printk(KERN_INFO "---END HELLO LINUX MODULE---\n");
	return 0;
}

static void __exit hello_exit(void)
{
	unregister_blkdev(hello_major,"hello_blk");	//移除模块时释放设备号		
	printk(KERN_INFO "GOODBYE LINUX\n");
}

module_init(hello_init);
module_exit(hello_exit);

//描述性定义
MODULE_LICENSE("Dual BSD/GPL");//许可 GPL、GPL v2、Dual MPL/GPL、Proprietary(专有)等,没有内核会提示
MODULE_AUTHOR("KGZ");		//作者
MODULE_VERSION("V1.0");  	//版本
