#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/rslib.h>
#include <linux/slab.h>
#include <linux/random.h>
#include "libgfshare.h"

MODULE_LICENSE("RMS");
MODULE_AUTHOR("AUSTEN BARKER");

static int __init km_template_init(void){
    uint8_t random[4096];
    uint8_t output[4096];
    uint8_t* share1, share2, share3;
    uint8_t* sharenrs(
    printk(KERN_INFO "THIS IS A KERNEL MODULE\n");
    get_random_bytes(&random, 512);
    
    return 0;
}

static void __exit km_template_exit(void){
    printk(KERN_INFO "Removing kernel module\n");
}

module_init(km_template_init);
module_exit(km_template_exit);
