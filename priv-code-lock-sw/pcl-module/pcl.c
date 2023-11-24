#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>


#define D_DEV_NAME		"priv-code-lock"		/**< device name */
#define D_DEV_MAJOR		(0)					/**< major# */
#define D_DEV_MINOR		(0)					/**< minor# */
#define D_DEV_NUM		(1)					/**< number of device */
#define D_BUF_SIZE		(32)				/**< buffer size (for sample-code) */



static struct class *g_class;
static int g_dev_major = D_DEV_MAJOR;		
static int g_dev_minor = D_DEV_MINOR;	    
static struct cdev* g_cdev;

static int register_dev(void);
static int pcl_module_open(struct inode *inode, struct file *filep);

static int pcl_module_release(struct inode *inode, struct file *filep);

static ssize_t pcl_module_write(struct file *filep, const char __user *buf, size_t count,
        loff_t *f_pos);
static ssize_t pcl_module_read(struct file *filep, char __user *buf, size_t count,
        loff_t *f_pos);

static struct file_operations g_fops = {
	.open    = pcl_module_open,
	.release = pcl_module_release,
	.write   = pcl_module_write,
	.read    = pcl_module_read,
};





static int pcl_module_init(void) {
    pr_info("Loading %s kernel module\n", D_DEV_NAME);
    return 0;
}


static void pcl_module_exit(void) {
    pr_info("Unloading %s module\n", D_DEV_NAME);
}


static int register_dev(void) {
    dev_t  dev, dev_tmp;
    int ret;


    if ((ret = alloc_chrdev_region(&dev, D_DEV_MINOR, D_DEV_NUM, D_DEV_NAME)) < 0) {
        pr_err("alloc_chrdev_region failed\n");
        return ret;
    }

    g_dev_major = MAJOR(dev);
    g_dev_minor = MINOR(dev);

    g_class = class_create(THIS_MODULE, D_DEV_NAME);
    if (IS_ERR(g_class)) {
        return PTR_ERR(g_class);
    }

    g_cdev = (struct cdev*)kmalloc(sizeof(struct cdev), GFP_KERNEL);

    dev_tmp = MKDEV(g_dev_major, g_dev_minor);
    cdev_init(g_cdev, &g_fops);

}
static int pcl_module_open(struct inode *inode, struct file *filep) {
    return 0;
}

static int pcl_module_release(struct inode *inode, struct file *filep) {
    return 0;
}

static ssize_t pcl_module_write(struct file *filep, const char __user *buf, size_t count,
        loff_t *f_pos) {
    return 0;
}
static ssize_t pcl_module_read(struct file *filep, char __user *buf, size_t count,
        loff_t *f_pos) {
    return 0;
}


module_init(pcl_module_init);
module_exit(pcl_module_exit);


MODULE_LICENSE("Dual MIT/GPL");

/*
 * Reference: https://github.com/ngtkt0909/linux-kernel-module-template
 */
