//#include<linux/init.h>	//下面的头文件包含了，所以这里不调也可以
#include<linux/module.h>

static int __init hello_init(void)	//__init表示该函数只在初始化期间使用，模块装载后就扔掉，释放内存
{
	printk(KERN_INFO "HELLO LINUX MODULE\n");
	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_INFO "GOODBYE LINUX\n");
}

module_init(hello_init);
module_exit(hello_exit);

//描述性定义
MODULE_LICENSE("Dual BSD/GPL");//许可 GPL、GPL v2、Dual MPL/GPL、Proprietary(专有)等,没有内核会提示
MODULE_AUTHOR("KGZ");		//作者
MODULE_VERSION("V1.0");  	//版本
