
#include<linux/module.h>
//#include<linux/moduleparam.h>
//#include<linux/stat.h>

#define CNT 1

static int cnt=CNT;
static char *hi="Hi,Linux\n";
static int arr[]={1,2,3,4,5,6};
static int nums=sizeof(arr)/sizeof(int);

module_param(cnt,int,S_IRUGO);   //类型支持布尔(bool invbool)、
				//字符指针(charp)、
				//整形(int long short uint ulong ushort)
module_param(hi,charp,S_IRUGO);
module_param_array(arr,int,&nums,S_IRUGO);	//赋值不能超过数组大小，否则报错


static void prt(void)
{
 printk(KERN_INFO "this is hello module\n");
}

static int __init hello_init(void)	
{
	int i;
	printk(KERN_INFO "HELLO LINUX MODULE\n");
	for( i=0;i<cnt;i++)
	{
		printk(KERN_INFO "%d:%s",i,hi);		
	}
	for( i=0;i<6;i++)
	{
		printk(KERN_INFO "%d ",arr[i]);
	}
	printk(KERN_INFO "nums:%d\n",nums);
	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_INFO "GOODBYE LINUX\n");
}

module_init(hello_init);
module_exit(hello_exit);


EXPORT_SYMBOL(hi);
EXPORT_SYMBOL(prt);

//描述性定义
MODULE_LICENSE("Dual BSD/GPL");//许可 GPL、GPL v2、Dual MPL/GPL、Proprietary(专有)等,没有内核会提示
MODULE_AUTHOR("KGZ");		//作者
MODULE_VERSION("V1.0");  	//版本
