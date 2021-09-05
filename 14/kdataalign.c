/***********************************
	  @        @@@@      __/\__
	@||@@/	  @关注@     \ @@ /
	@||@/	   @@@@   	  /''\
	
			  科G栈
			   KGZ
***********************************/
/*
自然对齐：在数据项大小的整数倍的地址处存储数据项
字节对齐可以提高CPU的访问效率	
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/utsname.h>
#include <linux/errno.h>


struct c   {char c;  char      t;} c;
struct s   {char c;  short     t;} s;
struct i   {char c;  int       t;} i;
struct l   {char c;  long      t;} l;
struct ll  {char c;  long long t;} ll;
struct p   {char c;  void *    t;} p;
struct u1b {char c;  __u8      t;} u1b;
struct u2b {char c;  __u16     t;} u2b;
struct u4b {char c;  __u32     t;} u4b;
struct u8b {char c;  __u64     t;} u8b;

struct {
	u16 id;
	u8  a;
	u64 lun;
	u16 reserved1;
	u32 reserved2;
}__attribute__((packed)) scsi;

struct {
	u16 id;
	u8  a;
	u64 lun;
	u16 reserved1;
	u32 reserved2;
} scsi1;

static void data_cleanup(void)
{
	/* never called */
}

static int data_init(void)
{
	/* print information and return an error */
	printk("arch  Align:  char  short  int  long   ptr long-long "
		" u8 u16 u32 u64\n");
	printk("%-12s  %3i   %3i   %3i   %3i   %3i   %3i      "
		"%3i %3i %3i %3i\n",
		init_uts_ns.name.machine,
		/* note that gcc can subtract void * values, but it's not ansi */
		(int)((void *)(&c.t)   - (void *)&c),
		(int)((void *)(&s.t)   - (void *)&s),
		(int)((void *)(&i.t)   - (void *)&i),
		(int)((void *)(&l.t)   - (void *)&l),
		(int)((void *)(&p.t)   - (void *)&p),
		(int)((void *)(&ll.t)  - (void *)&ll),
		(int)((void *)(&u1b.t) - (void *)&u1b),
		(int)((void *)(&u2b.t) - (void *)&u2b),
		(int)((void *)(&u4b.t) - (void *)&u4b),
		(int)((void *)(&u8b.t) - (void *)&u8b));
	//printk("%lx %lx %lx %lx %lx %lx %lx %lx %lx %lx \n",(unsigned long)&c,(unsigned long)&s,(unsigned long)&i,(unsigned long)&l,(unsigned long)&p,(unsigned long)&ll,(unsigned long)&u1b,(unsigned long)&u2b,(unsigned long)&u4b,(unsigned long)&u8b);
	printk("packed %i unpacked %i\n",(int)sizeof(scsi),(int)sizeof(scsi1));
	printk("      id		      a		       lun	        reserved1	 reserved2\n");
	printk("scsi  %lx %lx %lx %lx %lx",(unsigned long)&scsi.id,(unsigned long)&scsi.a,(unsigned long)&scsi.lun,(unsigned long)&scsi.reserved1,(unsigned long)&scsi.reserved2);
	printk("scsi1 %lx %lx %lx %lx %lx\n",(unsigned long)&scsi1.id,(unsigned long)&scsi1.a,(unsigned long)&scsi1.lun,(unsigned long)&scsi1.reserved1,(unsigned long)&scsi1.reserved2);
	
	return -ENODEV;
}

module_init(data_init);
module_exit(data_cleanup);

MODULE_LICENSE("Dual BSD/GPL");
