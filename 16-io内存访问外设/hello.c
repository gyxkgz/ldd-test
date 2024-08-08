/***********************************
	  @        @@@@      __/\__
	@||@@/	  @关注@     \ @@ /
	@||@/	   @@@@   	  /''\
	
			  科G栈
			   KGZ
***********************************/
/*
request_mem_region()
release_mem_region()
ioremap()
iounmap()
ioread32()   ioread8()/ioread16()
iowrite32()  iowrite8()/iowrite16()
*/

#include<linux/module.h>
#include<linux/io.h>

unsigned long gpio_base = 0x3f200000;
int gpio_len =0xb3;
struct timer_list t1;
int tdelay;
uint8_t flag=0;
void timer_fn(struct timer_list *t)
{
	if(flag)
		iowrite32(ioread32((void *)(gpio_base+0x1c))|(1<<4),(void*)(gpio_base+0x1c));
	else
		iowrite32(ioread32((void *)(gpio_base+0x28))|1<<4,(void*)(gpio_base+0x28));
	flag=!flag;
	mod_timer(&t1,jiffies+msecs_to_jiffies(1000));
}

static int __init hello_init(void)	
{
	printk(KERN_INFO "HELLO LINUX MODULE\n");
	// if (! request_mem_region(gpio_base,gpio_len , "gpio")) {
			// printk(KERN_INFO " can't get I/O mem address 0x%lx\n",
					// gpio_base);
			// return -ENODEV;
	// }
	gpio_base = (unsigned long)ioremap(gpio_base,gpio_len);
	iowrite32(ioread32((void *)gpio_base)|(1<<12),(void*)gpio_base);  //pin4 output
	printk(KERN_INFO"gpio remap base:0x%lx\n",gpio_base);
	printk(KERN_INFO"read %x %x %x\n",ioread8((void *)(gpio_base)),ioread16((void *)(gpio_base)),ioread32((void *)(gpio_base)));
	timer_setup(&t1,timer_fn,0);
	mod_timer(&t1,jiffies+msecs_to_jiffies(1000));
	
	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_INFO "GOODBYE LINUX\n");
	//release_mem_region(gpio_base,gpio_len);
	del_timer(&t1);
	iounmap((void *)gpio_base);
	
}
module_init(hello_init);
module_exit(hello_exit);
//描述性定义
MODULE_LICENSE("Dual BSD/GPL");//许可 GPL、GPL v2、Dual MPL/GPL、Proprietary(专有)等,没有内核会提示
MODULE_AUTHOR("KGZ");		//作者
MODULE_VERSION("V1.0");  	//版本
