#include <linux/delay.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/list.h>

#define SKB_LENGTH 1024
#define TX_MAX_HEADROOM 100

static int skb_debug(struct sk_buff *skb)
{
    printk("skb:%p, skb_data:%p, skb_len:%d, skb_head:%p, skb_tail:%p \n", skb, skb->data, skb->len, skb->head, skb->tail);
    printk("skb_shared:%d, skb_headroom:%d, skb_cloned:%d ", skb_shared(skb), skb_headroom(skb), skb_cloned(skb));

    if(skb->len)
    {
        //print_hex_dump(KERN_INFO, "skb:", DUMP_PREFIX_ADDRESS, 32, 1, skb->data, skb->len, false);
    }

    return 1;
}

/*
push data to skb, just the skb->data will move up, skb->head and skb->tail will not changed
*/
static int skb_push_t(struct sk_buff *skb)
{
    printk("%s enter!", __func__);
    skb_push(skb, 0x0f);
    skb_debug(skb);
    return 1;
}

/*
put data to skb tail, the skb->tail will move down, skb->head and skb->data will not changed
*/
static int skb_put_t(struct sk_buff *skb)
{
    printk("%s enter!", __func__);
    skb_put(skb, 0x0f);
    skb_debug(skb);
    return 1;
}

/*
pul data from skb data, the skb->data will move down, skb->head and skb->tail will not changed
*/
static int skb_pull_t(struct sk_buff *skb)
{
    printk("%s enter!", __func__);
    skb_pull(skb, 0x0f);
    skb_debug(skb);
    return 1;
}

/*
reserve buffer from skb data, the skb->data and skb->tail will move down, skb->head will not changed
*/
static int skb_reserve_t(struct sk_buff *skb)
{
    printk("%s enter!", __func__);
    skb_reserve(skb, 0x200);
    skb_debug(skb);
    return 1;
}

/*
copy and expand the skb, all the skb info will be changed
*/
static int skb_copy_expand_t(struct sk_buff *skb)
{
    struct sk_buff *newskb = skb_copy_expand(skb, TX_MAX_HEADROOM, 0, GFP_ATOMIC);
	printk("%s enter!", __func__);
    skb_debug(newskb);
    return 1;
}

static int st_init(void)
{
    struct sk_buff *skb = dev_alloc_skb(SKB_LENGTH);
	printk("%s enter!", __func__);
    skb_debug(skb);

    skb_reserve_t(skb);
    skb_push_t(skb);
    skb_put_t(skb);
    skb_pull_t(skb);
    skb_copy_expand_t(skb);
    
    return 0;
}

static void st_exit(void)
{
    printk("%s exit!", __func__);

    return;
}

module_init(st_init);
module_exit(st_exit);

MODULE_AUTHOR("zhangpeng");
MODULE_LICENSE("Dual BSD/GPL");
