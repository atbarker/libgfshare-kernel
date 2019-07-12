#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/rslib.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/timekeeping.h>
#include "libgfshare.h"

#define SECRET_SIZE 4096

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AUSTEN BARKER");

static int __init km_template_init(void){
   /* uint8_t* secret = kmalloc(SECRET_SIZE, GFP_KERNEL);
    uint8_t* recombine = kmalloc(SECRET_SIZE, GFP_KERNEL);
    uint8_t** shards = kmalloc(sizeof(uint8_t*) * 3, GFP_KERNEL);
    //uint8_t* shard1 = kmalloc(SECRET_SIZE, GFP_KERNEL);
    //uint8_t* shard2 = kmalloc(SECRET_SIZE, GFP_KERNEL);
    //uint8_t* shard3 = kmalloc(SECRET_SIZE, GFP_KERNEL);
    uint8_t* sharenrs = "012";
    int i;
    uint64_t time = 0;

    gfshare_ctx *G;
    gfshare_ctx *G_dec;

    printk(KERN_INFO "Inserting kernel module\n");
    for(i = 0; i < 3; i++){
        shards[i] = kmalloc(SECRET_SIZE, GFP_KERNEL);
    }
    
    //populate everything with random bytes
    get_random_bytes(secret, SECRET_SIZE);
    
    printk(KERN_INFO "Splitting randomly generated secret\n");
    G = gfshare_ctx_init_enc(sharenrs, 3, 2, SECRET_SIZE); 
    time = ktime_get_ns();
    gfshare_ctx_enc_getshare(G, secret, 3, shards);
    printk(KERN_INFO "time to split: %lld", ktime_get_ns() - time);
    
    //recombine the secret
    G_dec = gfshare_ctx_init_dec(sharenrs, 3, SECRET_SIZE);
    time = ktime_get_ns();
    gfshare_ctx_dec_giveshare(G_dec, 0, shards[0]);
    gfshare_ctx_dec_giveshare(G_dec, 1, shards[1]);
    gfshare_ctx_dec_giveshare(G_dec, 2, shards[2]);
    gfshare_ctx_dec_extract(G_dec, recombine);
    printk(KERN_INFO "time to reconstruct: %lld", ktime_get_ns() - time);
    
    //verify the recombination succeeded
    for(i = 0; i < SECRET_SIZE; i++){
        if(secret[i] != recombine[i]){
             printk(KERN_INFO "Recombine failed at character %d\n", i);
	     goto exit;
	}	
    }

    printk(KERN_INFO "Recombine with all shares succeeded\n");
    
exit:
    gfshare_ctx_free(G);
    gfshare_ctx_free(G_dec); 
    kfree(secret);
    kfree(recombine);
    kfree(shards);
    //kfree(shard1);
    //kfree(shard2);
    //kfree(shard3);*/
    return 0;
}

static void __exit km_template_exit(void){
    printk(KERN_INFO "Removing kernel module\n");
}

module_init(km_template_init);
module_exit(km_template_exit);
