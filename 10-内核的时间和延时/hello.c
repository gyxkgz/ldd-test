/***********************************
	  @        @@@@      __/\__
	@||@@/	  @关注@     \ @@ /
	@||@/	   @@@@   	  /''\
	
			  科G栈
			   KGZ
***********************************/
/*
HZ：100-1000之间
jiffies:系统时钟中断计数器

在比较jiffies时使用下面的宏，可以避免32位溢出的问题。
time_after(a,b)		a>b?		
time_before(a,b)	a<b?
time_after_eq(a,b)	a>=b?
time_before_eq(a,b)	a<=b?

jiffies与常用时间之间的转换
jiffies_to_msecs()
jiffies_to_usecs()
msecs_to_jiffies()
usecs_to_jiffies()

jiffies_to_timespec64()
timespec64_to_jiffies()

延时
wait_event_timeout()
wait_event_interruptible_timeout()
set_current_state()
schedule_timeout()

ndelay()  大于1000用下一个，均是忙等待
udelay()
mdelay()	如果用到了它，那么你可能需要考虑使用msleep

休眠延时
usleep_range()10us以上 20ms以下  
msleep()	
msleep_interruptible()
ssleep()

fsleep(unsigned long usecs)  新内核版本（5.13.2有）

Documentation/timers/timers-howto.rst
*/
#include<linux/module.h>
#include<linux/jiffies.h>
#include<linux/sched.h>
#include<linux/delay.h>
unsigned long j,t1,diff;
struct timespec64 ts64;
static int __init hello_init(void)	
{
	wait_queue_head_t wait;
	init_waitqueue_head(&wait);
	
	printk(KERN_INFO "HELLO LINUX MODULE\n");
	j=jiffies;
	t1=j+msecs_to_jiffies(1000);
	printk(KERN_INFO "j=%ld t1=%ld\n af:%d bf:%d afeq:%d beeq:%d\n",j,t1,time_after(j,t1),time_before(j,t1),time_after_eq(j,t1),time_before_eq(j,t1));
	//忙等待，不推荐使用，浪费系统性能，有可能让系统进入死循环出不来（比如禁止了中断）
	printk(KERN_INFO "忙等待延时1s\n");
	while(time_before(jiffies,t1))
	{
	}
	//等待队列延时
	printk(KERN_INFO "等待队列延时1s\n");
	wait_event_interruptible_timeout(wait,0,msecs_to_jiffies(1000));
	//schedule_timeout延时
	printk(KERN_INFO "schedule_timeout延时1s\n");
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(msecs_to_jiffies(1000));
	//短延时函数
	printk(KERN_INFO "短延时\n");
	mdelay(1000);
	//休眠延时
	printk(KERN_INFO "usleep_range延时\n");
	usleep_range(10000,15000);
	printk(KERN_INFO "ssleep延时\n");
	ssleep(1);
	
	diff = jiffies - j;
	printk(KERN_INFO"diff=%ld,time ms=%d us=%d\n",diff,jiffies_to_msecs(diff),jiffies_to_usecs(diff));
	printk(KERN_INFO "系统开机到现在的时间：\n");
	jiffies_to_timespec64(jiffies-INITIAL_JIFFIES,&ts64);
	printk(KERN_INFO"sec:%lld+ns:%ld\n",ts64.tv_sec,ts64.tv_nsec);

	
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
