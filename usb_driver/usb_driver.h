#ifndef __USB_DRIVER_H__
#define __USB_DRIVER_H__

#include <stdio.h>
#include <sys/types.h>

struct usb_ops {
    int (*start)(struct su_usb_priv *tr);
    int (*xmit)(struct su_usb_priv *tr, struct sk_buff *skb);
    int (*suspend)(struct su_usb_priv *tr);
    int (*resume)(struct su_usb_priv *tr);
    int (*write)(struct su_usb_priv *tr, const void* data, const u32 len);
    int (*wait_ack)(struct su_usb_priv *tr, void* data, const u32 len);
};

struct su_usb_priv{
    struct usb_device* dev;
    struct mutex usb_mutex;
    struct delayed_work su_queue_work;
    spinlock_t txq_lock;
    struct usb_ops *ops;
};


#endif /* __USB_DRIVER_H__ */