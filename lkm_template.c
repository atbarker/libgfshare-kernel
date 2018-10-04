#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/rslib.h>
#include <linux/random.h>
#include "libgfshare.h"

MODULE_LICENSE("RMS");
MODULE_AUTHOR("AUSTEN BARKER");
MODULE_DESCRIPTION("TEMPLATE");
MODULE_VERSION("0.01");

static int __init km_template_init(void){
    uint8_t random[512];
//    uint8_t output[512];
    printk(KERN_INFO "THIS IS A KERNEL MODULE\n");
    get_random_bytes(&random, 512);
    
    return 0;
}

static void __exit km_template_exit(void){
    printk(KERN_INFO "Removing kernel module\n");
}

module_init(km_template_init);
module_exit(km_template_exit);
