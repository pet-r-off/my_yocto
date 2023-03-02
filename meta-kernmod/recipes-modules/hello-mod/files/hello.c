#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
 
static int __init test_module_init(void) {
    printk(KERN_INFO "Hello kenrel DRIVER!!!\n");
    printk(KERN_INFO "This is my first LKM.\n");
    return 0;
}
 
static void __exit test_module_exit(void) {
    printk(KERN_INFO "Goodbuy Kernel!\n");
}
 
module_init(test_module_init);
module_exit(test_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pet_r_off");
MODULE_DESCRIPTION("A simple loadable Linux module.");
MODULE_VERSION("0.1");

