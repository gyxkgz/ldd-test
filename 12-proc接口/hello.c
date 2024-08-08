/***********************************
	  @        @@@@      __/\__
	@||@@/	  @关注@     \ @@ /
	@||@/	   @@@@   	  /''\
	
			  科G栈
			   KGZ
***********************************/
/*
struct proc_ops
proc_create()

struct seq_operations
proc_create_seq()

remove_proc_entry
*/
#include<linux/module.h>
#include<linux/uaccess.h>

#include<linux/string.h>


#define PROC_DEBUG

#ifdef PROC_DEBUG
#include<linux/proc_fs.h>
#include<linux/seq_file.h>
#endif


char * str = "hello proc\n";
#ifdef PROC_DEBUG
int hp_open(struct inode * inode, struct file * filp)
{
	printk(KERN_INFO"open %ld\n",strlen(str));
	return 0;
}

ssize_t hp_read(struct file * filp, char __user * buff, size_t count, loff_t * f_pos)
{
	ssize_t retval=0;
	int n = strlen(str);
	if(*f_pos >= n)
		goto out;
	if(*f_pos + count > n)
		count = n - *f_pos;
	
	if(copy_to_user(buff,str,count))
	{
		retval = -EFAULT;
		goto out;
	}

	*f_pos += count;
	return count;	
out:
	return retval;
}

struct proc_ops hp_ops = {
	.proc_open = hp_open,
	.proc_read = hp_read,
};

void * hp_seq_start (struct seq_file *m, loff_t *pos)
{
	printk(KERN_INFO"seq start\n");
	if(*pos >= strlen(str))
		return NULL;
	return &str[*pos];
}
void hp_seq_stop(struct seq_file *m, void *v)
{
	printk(KERN_INFO"seq stop\n");
}

void * hp_seq_next (struct seq_file *m, void *v, loff_t *pos)
{
	printk(KERN_INFO"seq next\n");
	(*pos)++;
	if(*pos >= strlen(str))
		return NULL;
	return &str[*pos];
}

int hp_seq_show (struct seq_file *m, void *v)
{
	printk(KERN_INFO"seq show\n");
	seq_putc(m,*(char*)v);
	return 0;
}

const struct seq_operations seq_ops={
	.start = hp_seq_start,
	.stop = hp_seq_stop,
	.next = hp_seq_next,
	.show = hp_seq_show,
};
#endif


static int __init hello_init(void)	
{
	printk(KERN_INFO "HELLO LINUX MODULE\n");
#ifdef PROC_DEBUG
	proc_create("hello_proc",0,NULL,&hp_ops);
	proc_create_seq("hello_seq_proc",0,NULL,&seq_ops);
#endif
	return 0;
}

static void __exit hello_exit(void)
{
#ifdef PROC_DEBUG
	remove_proc_entry("hello_proc",NULL);
	remove_proc_entry("hello_seq_proc",NULL);
#endif
	printk(KERN_INFO "GOODBYE LINUX\n");
}

module_init(hello_init);
module_exit(hello_exit);

//描述性定义
MODULE_LICENSE("Dual BSD/GPL");//许可 GPL、GPL v2、Dual MPL/GPL、Proprietary(专有)等,没有内核会提示
MODULE_AUTHOR("KGZ");		//作者
MODULE_VERSION("V1.0");  	//版本
