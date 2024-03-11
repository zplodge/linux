#include <linux/delay.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include "usb_driver.h"


/* table of devices that work with this driver */
static struct usb_device_id su_usb_ids[] = {
	{USB_DEVICE(0x3452, 0x6600)},
	{ /* Terminating entry */ },
};

static struct usb_ops su_usb_hif_ops = {
    .xmit   = usb_hif_xmit,
    .start  = usb_hif_start,
    .write  = usb_hif_write_raw,
    .wait_ack   = usb_hif_wait_ack,
    .suspend    = usb_hif_suspend,
    .resume     = usb_hif_resume,
};


static struct usb_driver su_usb_driver = {
    .name = "su usb dirver",
    .probe = su_usb_probe,
    .dicconnect = su_usb_disconnect,
    .suspend = su_usb_suspend,
    .resume = su_usb_resume,
    .id_table = su_usb_ids,
    .supports_autosuspend = true,
};

static int su_usb_probe (struct usb_interface *intf, const struct usb_device_id *id)
{
    int ret = 0;
    struct su_usb_priv* su_usb = NULL;
    struct usb_host_interface *iface_desc = interface->cur_altsetting;
    struct usb_device *dev = interface_to_usbdev(interface);

    su_usb = kzmalloc(sizeof(struct su_usb_priv), GFP_KERNEL);

    if(!su_usb){
        printk("probe err, no memory !! \n");
        return -ENOMEM;
    }

    su_usb->dev = dev;
    su_usb->ops = &su_usb_driver;

    retunr 0;
}

void su_usb_disconnect (struct usb_interface *intf)
{
    return;
}

int su_usb_suspend (struct usb_interface *intf, pm_message_t message)
{
    return 0;
}

int su_usb_resume (struct usb_interface *intf)
{
    return 0;
}

int usb_start(struct su_usb_priv *tr)
{
    return 0;
}

int usb_xmit (struct su_usb_priv *tr, struct sk_buff *skb)
{
    return 0;
}

int usb_suspend (struct su_usb_priv *tr)
{
    return 0;
}

int usb_resume (struct su_usb_priv *tr)
{
    return 0;
}
int usb_write(struct su_usb_priv *tr, const void* data, const u32 len)
{
    return 0;
}

int usb_wait_ack(struct su_usb_priv *tr, void* data, const u32 len)
{
    return 0;
}

static int usb_test_init(void)
{
    int ret = 0;
    printk("%s exit!", __func__);

    ret = usb_composite_probe(&usb_driver);
    return ret;
}

module_init(usb_test_init);
module_exit(usb_test_exit);

MODULE_AUTHOR("zhangpeng");
MODULE_LICENSE("Dual BSD/GPL");
