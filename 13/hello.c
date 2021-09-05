/***********************************
	  @        @@@@      __/\__
	@||@@/	  @关注@     \ @@ /
	@||@/	   @@@@   	  /''\
	
			  科G栈
			   KGZ
***********************************/
/*
1.
kmalloc()		一般千字节以下
kzalloc()
kfree()
2.
struct kmem_cache		slab分配器/专用高速缓存  速度快 利用率高
kmem_cache_create()
kmem_cache_alloc()			
kmem_cache_free()
kmem_cache_destroy()
3.
__get_free_page()		大块内存，按页分配
__get_free_pages()
get_zeroed_page()
free_page()
free_pages()
4.
vmalloc()	虚拟地址连续，物理地址不连续，效率不高，
vfree()		用在分配大的连续的、只在软件中使用的，用于缓存的内存区域
5.
others
*/
#include<linux/module.h>
#include<linux/slab.h>
#include<linux/gfp.h>
#include<linux/vmalloc.h>
char * kmlcp;
struct kmem_cache *h_cache;
char * kmemcp;
char * frpgp;
char * vmlcp;
static int  hello_init(void)	
{
	printk(KERN_INFO "HELLO LINUX MODULE\n");
//1
	kmlcp = kmalloc(1024,GFP_KERNEL);    //常用flag有GFP_KERNEL和GFP_ATOMIC
	if(!kmlcp)
	{
		return -ENOMEM;
	}
	printk(KERN_INFO"kmalloc get addr:%p\n",kmlcp);
//2
	h_cache = kmem_cache_create("h_cache",512,0,SLAB_HWCACHE_ALIGN|SLAB_POISON,NULL);
	if(!h_cache)
	{
		kfree(kmlcp);
		return -ENOMEM;
	}
	kmemcp = kmem_cache_alloc(h_cache,GFP_KERNEL);
	if(!kmemcp)
	{
		//do something
		return -ENOMEM;
	}
	printk(KERN_INFO"kmem_cache get addr:%p\n",kmemcp);
//3
	frpgp =(void *) __get_free_pages(GFP_KERNEL,0);    //第二个参数是页面数的对数值，0:1 1:2 2:4 3:8
	if(!frpgp)
	{
		//do something
		return -ENOMEM;
	}
	printk(KERN_INFO"free pages get addr:%p\n",frpgp);
//4
	vmlcp = vmalloc(PAGE_SIZE<<4);   //大空间
	if(!vmlcp)
	{
		//do something
		return -ENOMEM;
	}
	printk(KERN_INFO"vmalloc get addr:%p\n",vmlcp);
	
	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_INFO "GOODBYE LINUX\n");
//1
	kfree(kmlcp);
//2
	kmem_cache_free(h_cache,kmemcp);
	kmem_cache_destroy(h_cache);
//3
	free_pages((unsigned long)frpgp,0);
//4
	vfree(vmlcp);
}

module_init(hello_init);
module_exit(hello_exit);

//描述性定义
MODULE_LICENSE("Dual BSD/GPL");//许可 GPL、GPL v2、Dual MPL/GPL、Proprietary(专有)等,没有内核会提示
MODULE_AUTHOR("KGZ");		//作者
MODULE_VERSION("V1.0");  	//版本
