#include <linux/delay.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/usb.h>
#include "usb_driver.h"

static struct usb_host_priv* g_host_priv = NULL;
static void free_urb_context(struct usb_urb_context* urb_context);
static int usb_refill_rcv_transfer(struct usb_infac_pipe* pipe_rx);
static struct usb_urb_context* alloc_urb_context(struct usb_infac_pipe* pipe);


static void core_register_work(struct work_struct *work)
{
    struct usb_host_priv *tr = container_of(work, struct usb_host_priv, register_work.work);
    u8* buff;
    static int count = 0;
    printk(DEBUG_FN_ENTRY_STR);

    if (tr->ops->xmit) {
        struct sk_buff *skb;
        skb = dev_alloc_skb(MAX_PACKET_SIZE);
        buff = (u8*)skb->data;
        skb->len = MAX_PACKET_SIZE>>2;
        memset(buff, 0x22, MAX_PACKET_SIZE>>2);
        printk(" xmit test, count:%d\n", count++);
		tr->ops->xmit(tr, skb);
	} if(tr->ops->write){
        struct sk_buff *skb;
        struct sk_buff *skb1;
	    skb = dev_alloc_skb(MAX_PACKET_SIZE);
        buff = (u8*)skb->data;
        skb->len = MAX_PACKET_SIZE>>3;
        memset(buff, 0x88, MAX_PACKET_SIZE>>3);
	    printk(" write test\n");
        tr->ops->xmit(tr, skb);

	    skb1 = dev_alloc_skb(MAX_PACKET_SIZE);
        buff = (u8*)skb1->data;
        skb1->len = MAX_PACKET_SIZE>>4;
	    memset(buff, 0xaa, MAX_PACKET_SIZE>>4);
	    //tr->ops->xmit(tr, skb);
        tr->ops->write(tr, skb1->data, skb1->len);
	} else {
        printk("eswin_sdio_work error, ops->xmit is null\n");
	}

	schedule_delayed_work(&tr->register_work, msecs_to_jiffies(1000));
}

static void usb_transmit_complete(struct urb* urb)
{
    struct usb_urb_context* urb_context = urb->context;
    struct usb_infac_pipe * pipe = urb_context->pipe;
    struct sk_buff *skb = urb_context->skb;

    printk(DEBUG_FN_ENTRY_STR);
    if(urb->status == 0 && urb->actual_length) {
        printk("%s:data:%p, len:%d, actual_length:%d \n", __func__, skb->data, skb->len, urb->actual_length);
        //print_hex_dump(KERN_DEBUG, "usb_transmit_complete: ", DUMP_PREFIX_NONE, 32, 1, skb->data, urb->actual_length, false);
        skb_queue_tail(&pipe->io_complete_queue, skb);

#ifdef CONFIG_WORKQUEUE
        schedule_work(&pipe->io_complete_work);
#endif
    }

    free_urb_context(urb_context);
}

static int usb_host_init(struct usb_host_priv *tr)
{
    printk(DEBUG_FN_ENTRY_STR);

    if(tr->register_flag == false) {
        INIT_DELAYED_WORK(&tr->register_work, core_register_work);
        tr->register_flag = true;
    }

    //tr->ops->start(tr);
    schedule_delayed_work(&tr->register_work, msecs_to_jiffies(1000));

    return 0;
}

static void usb_recv_complete(struct urb* urb)
{
    struct usb_urb_context* urb_context = (struct usb_urb_context*)urb->context;
    struct sk_buff* skb = urb_context->skb;
    struct usb_infac_pipe* pipe = urb_context->pipe;
    printk(DEBUG_FN_ENTRY_STR);

    printk("%s:data:%p, len:%d, actual_length:%d, urb status:%d \n", __func__, skb->data, skb->len, urb->actual_length, urb->status);
    if(urb->status == 0 && urb->actual_length) {
        print_hex_dump(KERN_DEBUG, "usb_recv_complete: ", DUMP_PREFIX_NONE, 32, 1, skb->data, urb->actual_length, false);
        skb->len = urb->actual_length;
        skb_queue_tail(&pipe->io_complete_queue, skb);

#ifdef CONFIG_WORKQUEUE
        schedule_work(&pipe->io_complete_work);
#endif
    }

    free_urb_context(urb_context);
    usb_refill_rcv_transfer(pipe);
}

static int usb_init_rcv_transfer(struct usb_infac_pipe* pipe_rx)
{
    //struct sk_buff* skb;
    struct urb* urb;
    int rx_status;
    struct usb_urb_context* urb_context = alloc_urb_context(pipe_rx);

    printk(DEBUG_FN_ENTRY_STR);
    while(urb_context) {
        urb_context->skb = alloc_skb(USB_RX_MAX_BUF_SIZE, GFP_ATOMIC);
        if (!urb_context->skb) {
            return -ENOMEM;
        }

        /*urb = usb_alloc_urb(0, GFP_ATOMIC);
        if (!urb) {
            return -ENOMEM;
        }*/

        urb = urb_context->urb;
        urb_context->pipe = pipe_rx;
        urb->context = urb_context;

        usb_fill_bulk_urb(urb,
            pipe_rx->infac->udev,
            pipe_rx->usb_pipe_handle,
            urb_context->skb->data,
            USB_RX_MAX_BUF_SIZE,
            usb_recv_complete, urb_context);

        usb_anchor_urb(urb, &pipe_rx->urb_submitted);
        rx_status = usb_submit_urb(urb, GFP_ATOMIC);

        if (rx_status) {
            printk("usb_refill_recv_transfer, usb bulk recv failed: %d\n", rx_status);
            usb_unanchor_urb(urb);
            usb_free_urb(urb);
            kfree_skb(urb_context->skb);
        }

        urb_context = alloc_urb_context(pipe_rx);
    }
}

static int usb_refill_rcv_transfer(struct usb_infac_pipe* pipe_rx)
{
    //struct sk_buff* skb;
    struct urb* urb;
    int rx_status;
    struct usb_urb_context* urb_context = alloc_urb_context(pipe_rx);

    printk(DEBUG_FN_ENTRY_STR);
    if(!urb_context) {
        return -ENOMEM;
    }

    urb_context->skb = alloc_skb(USB_RX_MAX_BUF_SIZE, GFP_ATOMIC);
    if (!urb_context->skb) {
        return -ENOMEM;
    }

    /*urb = usb_alloc_urb(0, GFP_ATOMIC);
    if (!urb) {
        return -ENOMEM;
    }*/

    //urb_context->urb = urb;
    urb = urb_context->urb;
    urb_context->pipe = pipe_rx;
    urb->context = urb_context;

    usb_fill_bulk_urb(urb,
        pipe_rx->infac->udev,
        pipe_rx->usb_pipe_handle,
        urb_context->skb->data,
        USB_RX_MAX_BUF_SIZE,
        usb_recv_complete, urb_context);

    usb_anchor_urb(urb, &pipe_rx->urb_submitted);
    rx_status = usb_submit_urb(urb, GFP_ATOMIC);

    if (rx_status) {
        printk("usb_refill_recv_transfer, usb bulk recv failed: %d\n", rx_status);
        usb_unanchor_urb(urb);
        usb_free_urb(urb);
        kfree_skb(urb_context->skb);
    }

    return 0;
}

static int usb_hif_start(struct usb_host_priv *tr)
{
    printk(DEBUG_FN_ENTRY_STR);
    usb_init_rcv_transfer(&tr->usb_data.pipe_rx);
    usb_init_rcv_transfer(&tr->usb_msg.pipe_rx);
    return 0;
}

int usb_hif_xmit(struct usb_host_priv *tr, struct sk_buff *skb)
{
    struct urb*urb;
    struct usb_urb_context* urb_context = NULL;
    int ret;

    urb_context = alloc_urb_context(&tr->usb_data.pipe_tx); //data

    if(!urb_context) {
        return -ENOMEM;
    }

    /*urb = usb_alloc_urb(0, GFP_ATOMIC);
    if (!urb) {
        return -ENOMEM;
    }*/

    urb = urb_context->urb;
    urb_context->pipe = &tr->usb_data.pipe_tx;
    urb_context->urb = urb;
    urb_context->skb = skb;
    urb->context = urb_context;

    usb_fill_bulk_urb(urb,
        tr->udev,
        tr->usb_data.pipe_tx.usb_pipe_handle,
        skb->data,
        skb->len,
        usb_transmit_complete, urb_context);

    if (!(skb->len % tr->usb_data.max_packet_size)) {
        /* hit a max packet boundary on this pipe */
        urb->transfer_flags |= URB_ZERO_PACKET;
    }

    usb_anchor_urb(urb, &tr->usb_data.pipe_tx.urb_submitted);

    print_hex_dump(KERN_DEBUG, "usb_hif_xmit: ", DUMP_PREFIX_NONE, 32, 1, skb->data, skb->len, false);
    ret = usb_submit_urb(urb, GFP_ATOMIC);
    if (ret) {
        printk("usb_hif_xmit, usb bulk transmit failed: %d\n", ret);
        usb_unanchor_urb(urb);
        ret = -EINVAL;
        //goto err_free_urb_to_pipe;
    }

    return 0;
}

int usb_hif_suspend(struct usb_host_priv *tr)
{
    return -EOPNOTSUPP;
}

int usb_hif_resume(struct usb_host_priv *tr)
{
    return -EOPNOTSUPP;
}

int usb_hif_write(struct usb_host_priv *tr, const void* data, const u32 len)
{
    printk(DEBUG_FN_ENTRY_STR);
    print_hex_dump(KERN_DEBUG, "usb_hif_write: ", DUMP_PREFIX_NONE, 32, 1, data, len, false);
    return usb_bulk_msg(tr->udev, tr->usb_data.pipe_tx.usb_pipe_handle, (void*)data, len, NULL, 2000);
}

int usb_hif_wait_ack(struct usb_host_priv *tr, void* data, const u32 len)
{
    printk(DEBUG_FN_ENTRY_STR);
    print_hex_dump(KERN_DEBUG, "usb_hif_wait_ack: ", DUMP_PREFIX_NONE, 32, 1, data, len, false);
    return usb_bulk_msg(tr->udev, tr->usb_data.pipe_rx.usb_pipe_handle, (void*)data, len, NULL, 1000);
}

static struct usb_ops usb_hif_ops = {
    .xmit   = usb_hif_xmit,
    .start  = usb_hif_start,
    .write  = usb_hif_write,
    .wait_ack   = usb_hif_wait_ack,
    .suspend    = usb_hif_suspend,
    .resume     = usb_hif_resume,
};

static void usb_tx_comp_work(struct work_struct* com_work)
{
    struct usb_infac_pipe* pipe = container_of(com_work, struct usb_infac_pipe, io_complete_work);
    struct sk_buff* skb = NULL;

    printk(DEBUG_FN_ENTRY_STR);

    while(skb = skb_dequeue(&pipe->io_complete_queue)) {
        printk("%s: skb_len:%d; \n", __func__, skb->len);
        print_hex_dump(KERN_DEBUG, "usb_tx_comp_work: ", DUMP_PREFIX_NONE, 32, 1, skb->data, skb->len, false);
        kfree_skb(skb);
    }
}


static void usb_tx_comp_tasklet(unsigned long data)
{
}

static void usb_rx_comp_work(struct work_struct* com_work)
{
    struct usb_infac_pipe* pipe = container_of(com_work, struct usb_infac_pipe, io_complete_work);
    struct sk_buff* skb = NULL;

    printk(DEBUG_FN_ENTRY_STR);
    while(skb = skb_dequeue(&pipe->io_complete_queue)) {
        printk("%s: skb_len:%d; \n", __func__, skb->len);
        print_hex_dump(KERN_DEBUG, "usb_rx_comp_work: ", DUMP_PREFIX_NONE, 32, 1, skb->data, skb->len, false);;
    }
}

static void usb_rx_comp_tasklet(unsigned long data)
{
}

static void usb_free_urb_to_infac(struct usb_infac_pipe * pipe,
					struct usb_urb_context *urb_context)
{
	unsigned long flags;

	//spin_lock_irqsave(&g_usb->cs_lock, flags);
	pipe->urb_cnt++;
	list_add(&urb_context->link, &pipe->urb_rx_list_head);
    if(urb_context->urb)
    {
        usb_unanchor_urb(urb_context->urb);
        //usb_free_urb(urb_context->urb);
        urb_context->urb = NULL;
    }
	//spin_unlock_irqrestore(&g_usb->cs_lock, flags);
}


static struct usb_urb_context* alloc_urb_context(struct usb_infac_pipe* pipe)
{
    unsigned long flag;
    struct usb_urb_context* urb_context = NULL;
    if(!list_empty(&pipe->urb_rx_list_head)) {
        spin_lock_irqsave(&pipe->urb_lock, flag);
        urb_context = list_first_entry(&pipe->urb_rx_list_head, struct usb_urb_context, link);
        list_del(&urb_context->link);
        if(pipe->urb_cnt) {
            pipe->urb_cnt--;
        } else {
            printk("error:no urb buffer; \n");
        }
        spin_unlock_irqrestore(&pipe->urb_lock, flag);
    }

    return urb_context;
}

static void free_urb_context(struct usb_urb_context* urb_context)
{
    unsigned long flag;
    struct usb_infac_pipe* pipe = urb_context->pipe;

    spin_lock_irqsave(&pipe->urb_lock, flag);
    list_add(&urb_context->link, &pipe->urb_rx_list_head);
    usb_unanchor_urb(urb_context->urb);
    urb_context->skb = NULL;
    pipe->urb_cnt++;
    if(urb_context->urb) {
        usb_unanchor_urb(urb_context->urb);
        usb_free_urb(urb_context->urb);
    }
    spin_unlock_irqrestore(&pipe->urb_lock, flag);
}

static int usb_create_pipe(struct usb_infac_pipe * pipe, int dir, bool flag)
{
    int i;
    struct usb_urb_context *urb_context = NULL;
    struct urb* urb = NULL;
    unsigned long lock_flag;
    printk("%s entry, dir: %d!!", __func__, dir);

    pipe->dir = dir;
    pipe->urb_cnt = 0;
    init_usb_anchor(&pipe->urb_submitted);
    INIT_LIST_HEAD(&pipe->urb_rx_list_head);
    INIT_LIST_HEAD(&pipe->skb_rx_head);
    INIT_LIST_HEAD(&pipe->skb_tx_head);
    spin_lock_init(&pipe->urb_lock);
    skb_queue_head_init(&pipe->io_complete_queue);

    for (i = 0; i < (flag ? USB_MSG_URB_NUM : USB_DATA_URB_NUM); i++) {
        urb_context = kzalloc(sizeof(struct usb_urb_context), GFP_KERNEL);
        if (!urb_context) {
            return -ENOMEM;
        }

        urb_context->pipe = pipe;
        pipe->urb_cnt++;

        spin_lock_irqsave(&pipe->urb_lock, lock_flag);
        list_add(&urb_context->link, &pipe->urb_rx_list_head);

        /*if(urb_context->urb) {
            usb_unanchor_urb(urb_context->urb);
            usb_free_urb(urb_context->urb);
            urb_context->urb = NULL;
        }*/

        urb_context->urb = usb_alloc_urb(0, GFP_ATOMIC);
        if (!urb_context->urb) {
            kfree(urb_context);
            return -ENOMEM;
        }
        usb_unanchor_urb(urb_context->urb);

        spin_unlock_irqrestore(&pipe->urb_lock, lock_flag);
    }

    if (dir) {
#ifdef CONFIG_WORKQUEUE
        INIT_WORK(&pipe->io_complete_work, usb_tx_comp_work);
#endif
#ifdef CONFIG_TASKLET
        tasklet_init(&pipe->tx_tasklet, usb_tx_comp_tasklet, (unsigned long) pipe);
#endif
    } else {
#ifdef CONFIG_WORKQUEUE
        INIT_WORK(&pipe->io_complete_work, usb_rx_comp_work);
#endif
#ifdef CONFIG_TASKLET
        tasklet_init(&pipe->rx_tasklet, usb_rx_comp_tasklet, (unsigned long) pipe);
#endif
    }

    skb_queue_head_init(&pipe->io_complete_queue);
    return 0;
}

static int usb_create_infac(struct usb_interface *interface, struct usb_infac_data_t* iface_data, int flag)
{
    struct usb_device* dev = interface_to_usbdev(interface);
    struct usb_host_interface* iface_desc = interface->cur_altsetting;
    struct usb_endpoint_descriptor *endpoint = NULL;
    int i;

    printk(DEBUG_FN_ENTRY_STR);
    //usb_get_dev(dev);
    usb_set_intfdata(interface, iface_data);

    iface_data->udev = dev;
    iface_data->interface = interface;

    for(i=0; i<iface_desc->desc.bNumEndpoints; i++) {
        endpoint = &iface_desc->endpoint[i].desc;
        if(endpoint->bEndpointAddress & USB_DIR_MASK) {
            iface_data->pipe_rx.usb_pipe_handle = usb_rcvbulkpipe(dev, endpoint->bEndpointAddress);
            iface_data->pipe_rx.infac = iface_data;
            usb_create_pipe(&iface_data->pipe_rx, USB_DIR_RX, flag);
        } else {
            iface_data->pipe_tx.usb_pipe_handle = usb_sndbulkpipe(dev, endpoint->bEndpointAddress);
            iface_data->pipe_tx.infac = iface_data;
            usb_create_pipe(&iface_data->pipe_tx, USB_DIR_TX, flag);
        }
    }

    iface_data->max_packet_size = le16_to_cpu(endpoint->wMaxPacketSize);
    return 0;
}

static int usb_host_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    int ret = 0;
    static bool probe_flag = false;
    struct usb_host_priv* usb_host = NULL;
    struct usb_host_interface *iface_desc = interface->cur_altsetting;
    struct usb_device *udev = interface_to_usbdev(interface);

    printk(DEBUG_FN_ENTRY_STR);
    printk("bInterfaceNumber:%d \n", iface_desc->desc.bInterfaceNumber);
    //printk("driver info:match_flag:%d, vendor:%d, product:%d, class:%d, protocol:%d, subclass:%d, ifacenum:%d \n", id->match_flags, id->idVendor, id->idProduct, id->bDeviceClass, id->bDeviceProtocol, id->bDeviceSubClass, id->bInterfaceNumber);
    if(USB_INFAC_DATA == iface_desc->desc.bInterfaceNumber) {
        usb_host = kzalloc(sizeof(struct usb_host_priv), GFP_KERNEL);

        if(!usb_host){
            printk("probe err, no memory !! \n");
            return -ENOMEM;
        }

        g_host_priv = usb_host;
        usb_host->udev = udev;
        usb_host->ops = &usb_hif_ops;
        usb_set_intfdata(interface, usb_host);
        usb_create_infac(interface, &usb_host->usb_data, 0);
        //usb_refill_rcv_transfer(&usb_host->usb_data.pipe_rx);
        usb_init_rcv_transfer(&usb_host->usb_data.pipe_rx);
    }else if(USB_INFAC_MSG == iface_desc->desc.bInterfaceNumber) {
        usb_create_infac(interface, &g_host_priv->usb_msg, 1);
        //usb_init_rcv_transfer(&g_host_priv->usb_msg.pipe_rx);
        probe_flag = true;
    }

    if(probe_flag) {
        ret = usb_host_init(g_host_priv);
        
        if(ret) {
            goto error;
        }
    }

    return 0;

error:
    usb_set_intfdata(interface, NULL);
	kfree(usb_host);
	return ret;
}

void usb_host_remove(struct usb_interface *interface)
{
    printk(DEBUG_FN_ENTRY_STR);
    kfree(g_host_priv);
    usb_set_intfdata(interface, NULL);
	usb_put_dev(interface_to_usbdev(interface));
    return;
}

int usb_host_suspend(struct usb_interface *intf, pm_message_t message)
{
    printk(DEBUG_FN_ENTRY_STR);
    return 0;
}

int usb_host_resume(struct usb_interface *intf)
{
    printk(DEBUG_FN_ENTRY_STR);
    return 0;
}

//{USB_DEVICE(0x062a, 0x4106)}
/* table of devices that work with this driver */
static struct usb_device_id usb_host_ids[] = {
	{USB_DEVICE(0x19d2, 0x0256)},
	{ /* Terminating entry */ },
};

MODULE_DEVICE_TABLE(usb, usb_host_ids);
static struct usb_driver usb_host_driver = {
    .name = "su usb dirver",
    .probe = usb_host_probe,
    .disconnect = usb_host_remove,
    .suspend = usb_host_suspend,
    .resume = usb_host_resume,
    .id_table = usb_host_ids,
    .supports_autosuspend = true,
};

static int usb_test_init(void)
{
    int ret = 0;
    printk(DEBUG_FN_ENTRY_STR);

    ret = usb_register(&usb_host_driver);
    return ret;
}

static void usb_test_exit(void)
{
    printk(DEBUG_FN_ENTRY_STR);

    usb_deregister(&usb_host_driver);
    return;
}

module_init(usb_test_init);
module_exit(usb_test_exit);

MODULE_AUTHOR("zhangpeng");
MODULE_LICENSE("Dual BSD/GPL");
