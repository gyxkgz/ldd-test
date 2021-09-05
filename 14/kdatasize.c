/***********************************
	  @        @@@@      __/\__
	@||@@/	  @关注@     \ @@ /
	@||@/	   @@@@   	  /''\
	
			  科G栈
			   KGZ
***********************************/
/*
	三大类：C标准（int），大小确定（u32），特定内核对象（pid_t）
	不同的架构，基础类型大小可能不同，主要区别在long和指针上
	
	可移植性：-Wall，消除所有警告;使用uint32_t等标准类型;页大小为PAGE_SIZE,不要假设4K
	
	大小端：cpu_to_le32() le32_to_cpu()
			cpu_to_be32() be32_to_cpu()
			......
			htonl() 	ntohl()
			htons()		ntohs()
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/utsname.h>
#include <linux/errno.h>

static void data_cleanup(void)
{
	/* never called */
}

int data_init(void)
{
	ssize_t n=90888;
	/* print information and return an error */
	printk("arch   Size:  char  short  int  long   ptr long-long "
		" u8 u16 u32 u64\n");
	printk("%-12s  %3i   %3i   %3i   %3i   %3i   %3i      "
		"%3i %3i %3i %3i\n",
		init_uts_ns.name.machine,
		(int)sizeof(char), (int)sizeof(short), (int)sizeof(int),
		(int)sizeof(long),
		(int)sizeof(void *), (int)sizeof(long long), (int)sizeof(__u8),
		(int)sizeof(__u16), (int)sizeof(__u32), (int)sizeof(__u64));
	printk("%i, %li, %i, %li\n",(int)sizeof(pid_t),(long)current->pid,(int)sizeof(ssize_t),(long)n);
	printk("le32:%x be32:%x htonl:%x ntohl:%x\n",	cpu_to_le32(0x1234abcd),
													cpu_to_be32(0x1234abcd),
													htonl(0x1234abcd),
													ntohl(0x1234abcd));
	return -ENODEV;
}

module_init(data_init);
module_exit(data_cleanup);

MODULE_LICENSE("Dual BSD/GPL");
