#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/rslib.h>
#include <linux/slab.h>
#include <linux/random.h>
#include "libgfshare.h"

MODULE_LICENSE("MIT");
MODULE_AUTHOR("AUSTEN BARKER");

static int __init km_template_init(void){
    uint8_t* secret = kmalloc(512, GFP_KERNEL);
    uint8_t* recombine = kmalloc(512, GFP_KERNEL);
    uint8_t* shard1 = kmalloc(512, GFP_KERNEL);
    uint8_t* shard2 = kmalloc(512, GFP_KERNEL);
    uint8_t* shard3 = kmalloc(512, GFP_KERNEL);
    uint8_t* sharenrs = "012";
    int i;
    gfshare_ctx *G;
    gfshare_ctx *G_dec;

    printk(KERN_INFO "Inserting kernel module\n");
    
    //populate everything with random bytes
    get_random_bytes(secret, 512);
    
    printk(KERN_INFO "Splitting randomly generated secret\n");
    G = gfshare_ctx_init_enc(sharenrs, 3, 2, 512); 
    gfshare_ctx_enc_setsecret(G, secret);
    gfshare_ctx_enc_getshare(G, 0, shard1);
    gfshare_ctx_enc_getshare(G, 1, shard2);
    gfshare_ctx_enc_getshare(G, 2, shard3);
    
    //recombine the secret
    G_dec = gfshare_ctx_init_dec(sharenrs, 3, 512);
    gfshare_ctx_dec_giveshare(G_dec, 0, shard1);
    gfshare_ctx_dec_giveshare(G_dec, 1, shard2);
    gfshare_ctx_dec_giveshare(G_dec, 2, shard3);
    gfshare_ctx_dec_extract(G_dec, recombine);
    
    //verify the recombination succeeded
    for(i = 0; i < 512; i++){
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
    kfree(shard1);
    kfree(shard2);
    kfree(shard3);
    return 0;
}

static void __exit km_template_exit(void){
    printk(KERN_INFO "Removing kernel module\n");
}

module_init(km_template_init);
module_exit(km_template_exit);
