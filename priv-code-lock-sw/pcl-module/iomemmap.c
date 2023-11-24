/**
 * @file		iomemmap.c
 * @brief		Linux Kernel Module Template
 *
 * @author		T. Ngtk
 * @copyright	Copyright (c) 2017 T. Ngtk
 *
 * @par License
 *	Released under the MIT and GPL Licenses.
 *	- https://github.com/ngtkt0909/linux-kernel-module-template/blob/master/LICENSE-MIT
 *	- https://github.com/ngtkt0909/linux-kernel-module-template/blob/master/LICENSE-GPL
 */

#include <linux/module.h>	/* MODULE_*, module_* */
#include <linux/fs.h>		/* file_operations, alloc_chrdev_region, unregister_chrdev_region */
#include <linux/cdev.h>		/* cdev, dev_init(), cdev_add(), cdev_del() */
#include <linux/device.h>	/* class_create(), class_destroy(), device_create(), device_destroy() */
#include <linux/slab.h>		/* kmalloc(), kfree() */
#include <asm/uaccess.h>	/* copy_from_user(), copy_to_user() */
#include <linux/io.h>		/* ioremap_nocache(), iounmap() */

/*------------------------------------------------------------------------------
	Prototype Declaration
------------------------------------------------------------------------------*/
static int iomemmapInit(void);
static void iomemmapExit(void);
static int iomemmapOpen(struct inode *inode, struct file *filep);
static int iomemmapRelease(struct inode *inode, struct file *filep);
static ssize_t iomemmapWrite(struct file *filep, const char __user *buf, size_t count, loff_t *f_pos);
static ssize_t iomemmapRead(struct file *filep, char __user *buf, size_t count, loff_t *f_pos);

static int sRegisterDev(void);
static void sUnregisterDev(void);

/*------------------------------------------------------------------------------
	Defined Macros
------------------------------------------------------------------------------*/
#define D_DEV_NAME		"05.iomemmap"		/**< device name */
#define D_DEV_MAJOR		(0)					/**< major# */
#define D_DEV_MINOR		(0)					/**< minor# */
#define D_DEV_NUM		(1)					/**< number of device */
#define D_BUF_SIZE		(32)				/**< buffer size (for sample-code) */

/*------------------------------------------------------------------------------
	Type Definition
------------------------------------------------------------------------------*/
/** @brief private data */
typedef struct t_private_data {
	int minor;								/**< minor# */
} T_PRIVATE_DATA;

/*------------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------------*/
static struct class *g_class;				/**< device class */
static struct cdev *g_cdev_array;			/**< charactor devices */
static int g_dev_major = D_DEV_MAJOR;		/**< major# */
static int g_dev_minor = D_DEV_MINOR;		/**< minor# */
static int g_buf[D_DEV_NUM][D_BUF_SIZE];	/**< buffer (for sample-code) */
static unsigned int g_addr[D_DEV_NUM];		/**< ioremap physical address */
static unsigned int g_size[D_DEV_NUM];		/**< ioremap size */

/** file operations */
static struct file_operations g_fops = {
	.open    = iomemmapOpen,
	.release = iomemmapRelease,
	.write   = iomemmapWrite,
	.read    = iomemmapRead,
};

/*------------------------------------------------------------------------------
	Macro Calls
------------------------------------------------------------------------------*/
MODULE_AUTHOR("T. Ngtk");
MODULE_LICENSE("Dual MIT/GPL");

module_init(iomemmapInit);
module_exit(iomemmapExit);

module_param(g_dev_major, int, S_IRUSR | S_IRGRP | S_IROTH);
module_param(g_dev_minor, int, S_IRUSR | S_IRGRP | S_IROTH);

/*------------------------------------------------------------------------------
	Functions (External)
------------------------------------------------------------------------------*/
/**
 * @brief Kernel Module Init
 *
 * @param nothing
 *
 * @retval 0		success
 * @retval others	failure
 */
static int iomemmapInit(void)
{
	int ret;

	printk(KERN_INFO "%s loading ...\n", D_DEV_NAME);

	/* register devices */
	if ((ret = sRegisterDev()) != 0) {
		printk(KERN_ERR "register_dev() failed\n");
		return ret;
	}

	return 0;
}

/**
 * @brief Kernel Module Exit
 *
 * @param nothing
 *
 * @retval nothing
 */
static void iomemmapExit(void)
{
	printk(KERN_INFO "%s unloading ...\n", D_DEV_NAME);

	/* unregister devices */
	sUnregisterDev();
}

/**
 * @brief Kernel Module Open : open()
 *
 * @param [in]		inode	inode structure
 * @param [in,out]	filep	file structure
 *
 * @retval 0		success
 * @retval others	failure
 */
static int iomemmapOpen(struct inode *inode, struct file *filep)
{
	T_PRIVATE_DATA *info;

	printk(KERN_INFO "%s opening ...\n", D_DEV_NAME);

	/* allocate private data */
	info = (T_PRIVATE_DATA *) kmalloc(sizeof(T_PRIVATE_DATA), GFP_KERNEL);

	/* store minor# into private data */
	info->minor = MINOR(inode->i_rdev);
	filep->private_data = (void *)info;

	return 0;
}

/**
 * @brief Kernel Module Release : close()
 *
 * @param [in]	inode	inode structure
 * @param [in]	filep	file structure
 *
 * @retval 0		success
 * @retval others	failure
 */
static int iomemmapRelease(struct inode *inode, struct file *filep)
{
	T_PRIVATE_DATA *info = (T_PRIVATE_DATA *)filep->private_data;

	printk(KERN_INFO "%s releasing ...\n", D_DEV_NAME);

	/* deallocate private data */
	kfree(info);

	return 0;
}

/**
 * @brief Kernel Module Write : write()
 *
 * @param [in]		filep	file structure
 * @param [in]		buf		buffer address (user)
 * @param [in]		count	write data size
 * @param [in,out]	f_pos	file position
 *
 * @return	number of write byte
 */
static ssize_t iomemmapWrite(struct file *filep, const char __user *buf, size_t count, loff_t *f_pos)
{
	int minor = ((T_PRIVATE_DATA *)(filep->private_data))->minor;

	printk(KERN_INFO "%s writing ...\n", D_DEV_NAME);

	if (count > D_BUF_SIZE) {
		printk(KERN_INFO "%s write data overflow\n", D_DEV_NAME);
		count = D_BUF_SIZE;
	}

	if (copy_from_user(g_buf[minor], buf, count)) {
		return -EFAULT;
	}

	/* parse wrote string */
	if (sscanf((const char *)g_buf[minor], "%x,%x", &g_addr[minor], &g_size[minor]) != 2) {
		return -EFAULT;
	}

	return count;
}

/**
 * @brief Kernel Module Read : read()
 *
 * @param [in]		filep	file structure
 * @param [out]		buf		buffer address (user)
 * @param [in]		count	read data size
 * @param [in,out]	f_pos	file position
 *
 * @return	number of read byte
 */
static ssize_t iomemmapRead(struct file *filep, char __user *buf, size_t count, loff_t *f_pos)
{
	int minor = ((T_PRIVATE_DATA *)(filep->private_data))->minor;
	void __iomem *addr_map;
	volatile uint8_t *addr_base;
	size_t count_rd = 0;
	long i;

	printk(KERN_INFO "%s reading ...\n", D_DEV_NAME);

	/* remap I/O memory into kernel address space */
//	if (!request_mem_region(g_addr[minor], g_size[minor], D_DEV_NAME)) {
//		printk(KERN_INFO "%s request_mem_region failed. %x, %x\n", D_DEV_NAME, g_addr[minor], g_size[minor]);
//		return -EBUSY;
//	}
	addr_map = ioremap_nocache(g_addr[minor], g_size[minor]);
	addr_base = (volatile uint8_t *)addr_map;

	for (i = 0; i < g_size[minor]; i++) {
		if (count >= i * 3) {
			count_rd += snprintf(&buf[i * 3], count - (i * 3), "%02x ", *(addr_base + i));
		} else {
			break;
		}
	}
	buf[count_rd - 1] = '\n';

	/* unmap I/O memory from kernel address space */
	iounmap(addr_map);
//	release_mem_region(g_addr[minor], g_size[minor]);

	return count_rd;
}

/*------------------------------------------------------------------------------
	Functions (Internal)
------------------------------------------------------------------------------*/
/**
 * @brief Register Devices
 *
 * @param nothing
 *
 * @retval 0		success
 * @retval others	failure
 */
static int sRegisterDev(void)
{
	dev_t dev, dev_tmp;
	int ret, i;

	/* acquire major#, minor# */
	if ((ret = alloc_chrdev_region(&dev, D_DEV_MINOR, D_DEV_NUM, D_DEV_NAME)) < 0) {
		printk(KERN_ERR "alloc_chrdev_region() failed\n");
		return ret;
	}

	g_dev_major = MAJOR(dev);
	g_dev_minor = MINOR(dev);

	/* create device class */
	g_class = class_create(THIS_MODULE, D_DEV_NAME);
	if (IS_ERR(g_class)) {
		return PTR_ERR(g_class);
	}

	/* allocate charactor devices */
	g_cdev_array = (struct cdev *)kmalloc(sizeof(struct cdev) * D_DEV_NUM, GFP_KERNEL);

	for (i = 0; i < D_DEV_NUM; i++) {
		dev_tmp = MKDEV(g_dev_major, g_dev_minor + i);
		/* initialize charactor devices */
		cdev_init(&g_cdev_array[i], &g_fops);
		g_cdev_array[i].owner = THIS_MODULE;
		/* register charactor devices */
		if (cdev_add(&g_cdev_array[i], dev_tmp, 1) < 0) {
			printk(KERN_ERR "cdev_add() failed: minor# = %d\n", g_dev_minor + i);
			continue;
		}
		/* create device node */
		device_create(g_class, NULL, dev_tmp, NULL, D_DEV_NAME "%u", g_dev_minor + i);
	}

	return 0;
}

/**
 * @brief Unregister Devices
 *
 * @param nothing
 *
 * @return nothing
 */
static void sUnregisterDev(void)
{
	dev_t dev_tmp;
	int i;

	for (i = 0; i < D_DEV_NUM; i++) {
		dev_tmp = MKDEV(g_dev_major, g_dev_minor + i);
		/* delete charactor devices */
		cdev_del(&g_cdev_array[i]);
		/* destroy device node */
		device_destroy(g_class, dev_tmp);
	}

	/* release major#, minor# */
	dev_tmp = MKDEV(g_dev_major, g_dev_minor);
	unregister_chrdev_region(dev_tmp, D_DEV_NUM);

	/* destroy device class */
	class_destroy(g_class);

	/* deallocate charactor device */
	kfree(g_cdev_array);
}
