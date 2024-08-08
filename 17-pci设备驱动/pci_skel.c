/***********************************
	  @        @@@@      __/\__
	@||@@/	  @关注@     \ @@ /
	@||@/	   @@@@   	  /''\
	
			  科G栈
			   KGZ
***********************************/

/*
struct pci_device_id	驱动支持的设备
PCI_DEVICE()
PCI_DEVICE_CLASS()
MODULE_DEVICE_TABLE()   导出pci_device_id结构体到用户空间，使热插拔和模块装载系统知道什么模块针对什么硬件设备

struct pci_driver
pci_register_driver()		注册
pci_unregister_driver()		注销

pci_enable_device()      激活/初始化pci设备，比如唤醒设备、读写配置信息等
pci_disable_device()

pci_read_config_byte()
pci_read_config_word()
pci_read_config_dword()
pci_resource_start()	获取区域信息(bar info) pci支持6个区域（io端口/io内存）
pci_resource_end()
pci_resource_flags()

pci_request_regions()	跟request_mem_region()一样
pci_release_regions()

pci_ioremap_bar()	跟ioremap一样，作了必要的检查

pci_set_drvdata()	设置驱动私有数据
pci_get_drvdata()	获取驱动私有数据


*/

#include <linux/module.h>
#include <linux/pci.h>

struct pci_card
{
   //端口读写变量
   resource_size_t io;
   long range,flags;
   void __iomem *ioaddr;
   int irq;
};

static struct pci_device_id ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x100e) },
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_80332_0) },
	{ 0, }  //最后一组是0，表示结束
};
MODULE_DEVICE_TABLE(pci, ids);

void skel_get_configs(struct pci_dev *dev)
{
	uint8_t val1;
	uint16_t val2;
	uint32_t val4;

	pci_read_config_word(dev,PCI_VENDOR_ID, &val2);
	printk("vendorID:%x",val2);
	pci_read_config_word(dev,PCI_DEVICE_ID, &val2);
	printk("deviceID:%x",val2);
	pci_read_config_byte(dev, PCI_REVISION_ID, &val1);
	printk("revisionID:%x",val1);
	pci_read_config_dword(dev,PCI_CLASS_REVISION, &val4);
	printk("class:%x",val4);
}
/* 设备中断服务*/
static irqreturn_t mypci_interrupt(int irq, void *dev_id)
{
   struct pci_card *mypci = (struct pci_card *)dev_id;
   printk("irq = %d,mypci_irq = %d\n",irq,mypci->irq);
   return IRQ_HANDLED;
}

static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	int retval = 0;
	struct pci_card *mypci;
	printk("probe func\n"); 
	if(pci_enable_device(dev))
	{
		printk (KERN_ERR "IO Error.\n");
		return -EIO;
	}
	mypci = kmalloc(sizeof(struct pci_card),GFP_KERNEL);
   if(!mypci)
   {
      printk("In %s,kmalloc err!",__func__);
      return -ENOMEM;
   }

   mypci->irq = dev->irq;
   if(mypci->irq < 0)
   {
      printk("IRQ is %d, it's invalid!\n",mypci->irq);
      goto out_mypci;
   }

   mypci->io = pci_resource_start(dev, 0);
   mypci->range = pci_resource_end(dev, 0) - mypci->io + 1;
   mypci->flags = pci_resource_flags(dev,0);
   printk("start %llx %lx %lx\n",mypci->io,mypci->range,mypci->flags);
   printk("PCI base addr 0 is io%s.\n",(mypci->flags & IORESOURCE_MEM)? "mem":"port");

  //retval=request_mem_region(mypci->io,mypci->range, "pci_skel");
   retval = pci_request_regions(dev,"pci_skel");
   if(retval)
   {
      printk("PCI request regions err!\n");
      goto out_mypci;
   }
   mypci->ioaddr = pci_ioremap_bar(dev,0);
   //mypci->ioaddr = ioremap(mypci->io,mypci->range);  这里变量的类型与函数参数的类型必须一致，否则会出错
   if(!mypci->ioaddr)
   {
      printk("ioremap err!\n");
      retval = -ENOMEM;
      goto out_regions;
   }
   //申请中断IRQ并设定中断服务子函数
   retval = request_irq(mypci->irq, mypci_interrupt, IRQF_SHARED, "pci_skel", mypci);
   if(retval)
   {
      printk (KERN_ERR "Can't get assigned IRQ %d.\n",mypci->irq);
      goto out_iounmap;
   }
   pci_set_drvdata(dev,mypci);
   printk("Probe succeeds.PCIE ioport addr start at %llX, mypci->ioaddr is 0x%p,interrupt No. %d.\n",mypci->io,mypci->ioaddr,mypci->irq);
 	skel_get_configs(dev);
   return 0;
  
out_iounmap:
	iounmap(mypci->ioaddr);
out_regions:
	pci_release_regions(dev);
out_mypci:
	kfree(mypci);
	return retval;
}

/* 移除PCI设备 */
static void remove(struct pci_dev *dev)
{
   struct pci_card *mypci = pci_get_drvdata(dev);
   free_irq (mypci->irq, mypci);
   iounmap(mypci->ioaddr);
   //release_mem_region(mypci->io,mypci->range);
   pci_release_regions(dev);
   kfree(mypci);
   pci_disable_device(dev);
   printk("Device is removed successfully.\n");
}

static struct pci_driver pci_driver = {
	.name = "pci_skel",
	.id_table = ids,
	.probe = probe,
	.remove = remove,
};

static int __init pci_skel_init(void)
{

	printk("HELLO PCI\n");
	return pci_register_driver(&pci_driver);
}

static void __exit pci_skel_exit(void)
{

	printk("GOODBYE PCI\n");
	pci_unregister_driver(&pci_driver);
}

MODULE_LICENSE("GPL");

module_init(pci_skel_init);
module_exit(pci_skel_exit);

/****
PCI本质上就是一种总线，具体的PCI设备可以是字符设备、网络设备、USB等，所以PCI设备驱动应该包含两部分：

1.PCI驱动
2.根据需求的设备驱动
*/
