#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("andrei.khorokhorin@cloudbear.ru");
MODULE_DESCRIPTION("Provide device for logging into dmesg buffer");
MODULE_VERSION("0.1.0");

dev_t dev = 0;
static struct cdev klog_dev;
static struct class *cls;
#define DEVICE_NAME "klog"
#define N 512

static ssize_t klog_read(struct file *filp,
                         char __user *buf,
                         size_t len,
                         loff_t *off) {
    pr_info("KLOG: read from process %d\n", current->pid);
    return 0;
}


static ssize_t klog_write(struct file *filp, const char __user *ubuf,
                         size_t len, loff_t *off) {
    int to_copy;
    char kbuf[N];
    char msg[2 * N];
    to_copy = (len > N ? N : len);
    pr_info("KLOG: process %d try to write %lu bytes\n", current->pid, len);
    if (copy_from_user(&kbuf, ubuf, to_copy)) {
        pr_info("KLOG: copy from user failed, return EFAULT\n");
        return -EFAULT;
    }
    for (int i = 0; i < to_copy; ++i) {
        sprintf(msg + i * 2, "%02x", kbuf[i]);
    }
    pr_info("Write successful, see hex dump below:\n");
    pr_info("%s", msg);
    pr_info("\n");
    return len;
}

static struct file_operations klog_ops = {
    .owner      = THIS_MODULE,
    .read       = klog_read,
    .write      = klog_write,
};

static int __init klog_init(void)
{
    int res;
    pr_info("KLOG: module load\n");

    if ((res = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME)) < 0) {
        pr_err("Error allocating major number\n");
        return res;
    }

    cdev_init(&klog_dev, &klog_ops);

    if ((res = cdev_add(&klog_dev, dev, 1)) < 0) {
        pr_err("KLOG: error on dev_add\n");
        goto fail1;
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(6,4,0)
    if (IS_ERR(cls = class_create(THIS_MODULE, DEVICE_NAME))) {
#else
    if (IS_ERR(cls = class_create(DEVICE_NAME))) {
#endif
        pr_err("KLOG: error on class_create\n");
        res = -1;
        goto fail2;
    }

    if (IS_ERR(device_create(cls, NULL, dev, 0, DEVICE_NAME))) {
        pr_err("KLOG: error on device_create\n");
        res = -1;
        goto fail3;
    }

    pr_info("KLOG: device created successfully\n");
    return 0;

    fail3:
    class_destroy(cls);
    fail2:
    cdev_del(&klog_dev);
    fail1:
    unregister_chrdev_region(dev, 1);
    return res;
}

static void __exit klog_cleanup(void) {
    device_destroy(cls, dev);
    class_destroy(cls);
    cdev_del(&klog_dev);
    unregister_chrdev_region(dev, 1);
    pr_info("KLOG: module unload\n");
}

module_init(klog_init);
module_exit(klog_cleanup);
