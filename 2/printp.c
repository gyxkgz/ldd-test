
#include<linux/module.h>

extern char * hi;
extern void prt(void);

static int __init printp_init(void)	
{
	printk(KERN_INFO "printp:%s",hi);
	prt();
	return 0;
}

static void __exit printp_exit(void)
{
}

module_init(printp_init);
module_exit(printp_exit);

//描述性定义
MODULE_LICENSE("Dual BSD/GPL");//许可 GPL、GPL v2、Dual MPL/GPL、Proprietary(专有)等,没有内核会提示
MODULE_AUTHOR("KGZ");		//作者
MODULE_VERSION("V1.0");  	//版本
