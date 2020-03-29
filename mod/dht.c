#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Emil Gedda");
MODULE_DESCRIPTION("Interface with DHT22 modules");
MODULE_VERSION("1.0.0");

static int __init dht_init(void) {
	printk(KERN_INFO "Hello, World!\n");
	return 0;
}
static void __exit dht_exit(void) {
	printk(KERN_INFO "Goodbye, World!\n");
}

module_init(dht_init);
module_exit(dht_exit);
