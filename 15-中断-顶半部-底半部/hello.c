/***********************************
	  @        @@@@      __/\__
	@||@@/	  @关注@     \ @@ /
	@||@/	   @@@@   	  /''\
	
			  科G栈
			   KGZ
***********************************/
/*
request_irq()
free_irq()
typedef irqreturn_t (*irq_handler_t)(int, void *);
enable_irq()
disable_irq()

local_irq_enable()
local_irq_restore()
local_irq_disable()
local_irq_save()

*/

#include<linux/module.h>
#include<linux/gpio.h>
#include<linux/interrupt.h>
#include<linux/proc_fs.h>
#include<linux/uaccess.h>

static struct work_struct work;
unsigned long flags;
void workqueue_fn(struct work_struct *work)			//下半部/底半部
{
	printk("hello workqueue\n");
}

static irqreturn_t irq_handler(int irq,void *dev)	//上半部/顶半部
{
	static int n=0;
	printk("get irq%d int %d\n",irq,++n);
	schedule_work(&work);
	return IRQ_HANDLED;
}

ssize_t hp_write(struct file * filp, const char __user * buff, size_t count, loff_t * f_pos)
{
	char a;
	get_user(a,buff);
	if(a=='0')
	{
		printk("disable irq\n");
		disable_irq(gpio_to_irq(12));
		//local_irq_disable();
		//local_irq_save(flags);
	}
	else
	{
		printk("enable irq\n");
		enable_irq(gpio_to_irq(12));
		//local_irq_enable();
		//local_irq_restore(flags);		
	}
	return count;	
}

struct file_operations hp_ops = {
	.write = hp_write,
};


static int __init hello_init(void)	
{
	int err;
	printk(KERN_INFO "HELLO LINUX MODULE\n");
	proc_create("hello_proc",0777,NULL,&hp_ops);
	INIT_WORK(&work,workqueue_fn);
	err = request_irq(gpio_to_irq(12),irq_handler,IRQ_TYPE_EDGE_BOTH,"hello-int",NULL);
    if(err<0)
    {
        printk("irq_request failed\n");
		remove_proc_entry("hello_proc",NULL);
        return err;
    }	
	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_INFO "GOODBYE LINUX\n");
	free_irq(gpio_to_irq(12),NULL);
	remove_proc_entry("hello_proc",NULL);
	
}

module_init(hello_init);
module_exit(hello_exit);

//描述性定义
MODULE_LICENSE("Dual BSD/GPL");//许可 GPL、GPL v2、Dual MPL/GPL、Proprietary(专有)等,没有内核会提示
MODULE_AUTHOR("KGZ");		//作者
MODULE_VERSION("V1.0");  	//版本
