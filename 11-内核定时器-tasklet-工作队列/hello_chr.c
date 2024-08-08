/***********************************
	  @        @@@@      __/\__
	@||@@/	  @关注@     \ @@ /
	@||@/	   @@@@   	  /''\
	
			  科G栈
			   KGZ
***********************************/
/*
1、内核定时器
struct timer_list
timer_setup()
mod_timer()
del_timer()

2、tasklet
struct tasklet_struct
tasklet_init()
tasklet_hi_schedule()
tasklet_schedule()
tasklet_kill()

3、workqueue
alloc_workqueue()
destroy_workqueue()
struct work_struct
INIT_WORK()
queue_work()

struct delayed_work
INIT_DELAYED_WORK()
queue_delayed_work()	

4、其他
in_interrupt()
smp_processor_id()

构造下面的数据表，通过设备读取
  time   delta  inirq    pid   cpu command
4295601162    0     0      7559   0   cat
4295601162    0     1      10     0  ksoftirqd/0
4295601162    0     1      10     0  ksoftirqd/0
4295601162    0     1      10     0  ksoftirqd/0
4295601162    0     1      10     0  ksoftirqd/0
4295601162    0     1      10     0  ksoftirqd/0

./Documentation/core-api/workqueue.rst

*/
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/slab.h>
#include<linux/uaccess.h>

#include<linux/interrupt.h>  //包括了timer、tasklet和workqueue。
#include<linux/timer.h>		//定时器
#include<linux/workqueue.h>	//工作队列

#define DEFER_TEST 2  //(0:timer 1:tasklet 2:workqueue)
#define DELAY_WORK

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
	char * buff;
	int loops;
	int tdelay;
	unsigned long prej;
#if (DEFER_TEST==0)
	struct timer_list t1;
#elif (DEFER_TEST==1)
	struct tasklet_struct tsklt;
#else
#ifdef DELAY_WORK
	struct delayed_work work;
#else
	struct work_struct work;
#endif
#endif
};

struct hello_char_dev *hc_devp;
struct class *hc_cls;


DECLARE_WAIT_QUEUE_HEAD(wq);


int hc_open(struct inode *inode, struct file *filp)
{
	struct hello_char_dev *hc_dev;
	printk(KERN_INFO "%s open \n",current->comm);
	hc_dev = container_of(inode->i_cdev,struct hello_char_dev,cdev);  //获取设备结构体的地址
	filp->private_data = hc_dev;	
	return 0;
}
#if (DEFER_TEST==0)
//定时器回调函数
void timer_fn(struct timer_list *t)
{
	struct hello_char_dev *hc_dev =	container_of(t,struct hello_char_dev,t1);
	hc_dev->buff+=sprintf(hc_dev->buff,"%9ld  %3ld     %i  %6i     %i  %s\n",
			 jiffies, jiffies-hc_dev->prej, in_interrupt()? 1 : 0,
			 current->pid, smp_processor_id(), current->comm);
	if(--hc_dev->loops)
	{
		mod_timer(t,jiffies+hc_dev->tdelay);
		hc_dev->prej = jiffies;
	}
	else{
		del_timer(t);
		//del_timer_sync(t);
		wake_up_interruptible(&wq);
	}
}
ssize_t hc_read(struct file *filp, char __user *buf, size_t count,loff_t *f_pos)
{
	ssize_t retval=0;
	size_t cnt;
	char *buf1;
	struct hello_char_dev *hc_dev=filp->private_data;
	printk(KERN_INFO "timer defer test\n");
	if(*f_pos > 0)
		goto out;
	hc_dev->loops = 5;
	hc_dev->tdelay = 10;
	buf1 = kzalloc(400,GFP_KERNEL);
	hc_dev->buff = buf1;
	hc_dev->buff += sprintf(hc_dev->buff, "timer defer test\n");
	hc_dev->buff += sprintf(hc_dev->buff, "   time   delta  inirq    pid   cpu command\n");
	hc_dev->buff += sprintf(hc_dev->buff, "%9li  %3li     %i    %6i   %i   %s\n",
			jiffies, 0L, in_interrupt() ? 1 : 0,
			current->pid, smp_processor_id(), current->comm);
	timer_setup(&hc_dev->t1,timer_fn,0);
	mod_timer(&hc_dev->t1,jiffies+hc_dev->tdelay);
	hc_dev->prej = jiffies;
	
	wait_event_interruptible(wq, !hc_dev->loops);	

	cnt =hc_dev->buff - buf1;
	if(copy_to_user(buf,buf1,cnt))
	{
		retval = -EFAULT;
		goto out;
	}
	*f_pos += cnt;
	kfree(buf1);
	return cnt;	
out:
	return retval;
}
#elif (DEFER_TEST==1)
//tasklet 
void tasklet_fn(unsigned long data)
{
	struct hello_char_dev *hc_dev =(struct hello_char_dev *) data;
	hc_dev->buff+=sprintf(hc_dev->buff,"%9ld  %3ld     %i  %6i     %i  %s\n",
			 jiffies, jiffies-hc_dev->prej, in_interrupt()? 1 : 0,
			 current->pid, smp_processor_id(), current->comm);
	if (--hc_dev->loops) {
		hc_dev->prej = jiffies;
		tasklet_hi_schedule(&hc_dev->tsklt);
		//tasklet_schedule(&hc_dev->tsklt);
	} else {
		wake_up_interruptible(&wq);
	}
}
ssize_t hc_read(struct file *filp, char __user *buf, size_t count,loff_t *f_pos)
{
	ssize_t retval=0;
	int cnt;
	char * buf1;
	struct hello_char_dev *hc_dev=filp->private_data;
	printk(KERN_INFO "tasklet defer test\n");
	if(*f_pos > 0)
		goto out;
	hc_dev->loops = 5;
	buf1 = kzalloc(400,GFP_KERNEL);
	hc_dev->buff = buf1;
	hc_dev->buff += sprintf(hc_dev->buff, "tasklet defer test\n");
	hc_dev->buff += sprintf(hc_dev->buff, "   time   delta  inirq    pid   cpu command\n");
	hc_dev->buff += sprintf(hc_dev->buff, "%9li  %3li     %i    %6i   %i   %s\n",
			jiffies, 0L, in_interrupt() ? 1 : 0,
			current->pid, smp_processor_id(), current->comm);
	tasklet_init(&hc_dev->tsklt,tasklet_fn,(unsigned long)hc_dev);
	hc_dev->prej = jiffies;
	tasklet_hi_schedule(&hc_dev->tsklt);
	//tasklet_schedule(&hc_dev->tsklt);
	wait_event_interruptible(wq, !hc_dev->loops);	
	tasklet_kill(&hc_dev->tsklt);
	cnt =hc_dev->buff - buf1;
	if(copy_to_user(buf,buf1,cnt))
	{
		retval = -EFAULT;
		goto out;
	}
	*f_pos += cnt;
	kfree(buf1);
	return cnt;	
out:
	return retval;
}

#else
//workqueue

struct workqueue_struct *workq = NULL;

void workqueue_fn(struct work_struct *work)
{
#ifdef DELAY_WORK
	struct delayed_work *dwork = container_of(work,struct delayed_work,work);
	struct hello_char_dev *hc_dev =	container_of(dwork,struct hello_char_dev,work);
#else
	struct hello_char_dev *hc_dev =	container_of(work,struct hello_char_dev,work);
#endif
	hc_dev->buff+=sprintf(hc_dev->buff,"%9ld  %3ld     %i  %6i     %i  %s\n",
			 jiffies, jiffies-hc_dev->prej, in_interrupt()? 1 : 0,
			 current->pid, smp_processor_id(), current->comm);
	if (--hc_dev->loops) {
		hc_dev->prej = jiffies;
#ifdef DELAY_WORK
		queue_delayed_work(workq,dwork,hc_dev->tdelay);
#else
		queue_work(workq,work);
#endif
	} else {
		
		wake_up_interruptible(&wq);
	}
}
ssize_t hc_read(struct file *filp, char __user *buf, size_t count,loff_t *f_pos)
{
	ssize_t retval=0;
	int cnt;
	char * buf1;
	struct hello_char_dev *hc_dev=filp->private_data;
	printk(KERN_INFO "workqueue defer test\n");
	if(*f_pos > 0)
		goto out;
	workq = alloc_workqueue("workq",WQ_UNBOUND,0);
	hc_dev->loops = 5;
#ifdef DELAY_WORK
	hc_dev->tdelay = 10;
#endif
	buf1 = kzalloc(400,GFP_KERNEL);
	hc_dev->buff = buf1;
	hc_dev->buff += sprintf(hc_dev->buff, "workqueue defer test\n");
	hc_dev->buff += sprintf(hc_dev->buff, "   time   delta  inirq    pid   cpu command\n");
	hc_dev->buff += sprintf(hc_dev->buff, "%9li  %3li     %i    %6i   %i   %s\n",
			jiffies, 0L, in_interrupt() ? 1 : 0,
			current->pid, smp_processor_id(), current->comm);	
	hc_dev->prej = jiffies;	
#ifdef DELAY_WORK
	INIT_DELAYED_WORK(&hc_dev->work,workqueue_fn);
	queue_delayed_work(workq,&hc_dev->work,hc_dev->tdelay);
#else
	INIT_WORK(&hc_dev->work,workqueue_fn);
	queue_work(workq,&hc_dev->work);
#endif	

	wait_event_interruptible(wq, !hc_dev->loops);
	destroy_workqueue(workq);
	cnt =hc_dev->buff - buf1;
	if(copy_to_user(buf,buf1,cnt))
	{
		retval = -EFAULT;
		goto out;
	}
	*f_pos += cnt;
	kfree(buf1);
	return cnt;	
out:
	return retval;
}
	
#endif

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
