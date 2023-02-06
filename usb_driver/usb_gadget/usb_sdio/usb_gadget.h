/*
* usb gadget driver header file
*/
#ifndef __USB_GADGET_H_
#define __USB_GADGET_H_

#define USB_INTERFACE_NUM 2
#define DEBUG_FN_ENTRY_STR "%s() enter, line:%d\n", __func__, __LINE__
#define QUEUE_SIZE 128
#define HS_NOTIFY_MAXPACKET 512
#define GS_NOTIFY_INTERVAL_MS 32

struct usb_infac_dev {
    struct usb_function func;
    struct usb_composite_dev *cdev;
    spinlock_t dev_lock;

    struct usb_ep *ep_in;
    struct usb_ep *ep_out;

    struct delayed_work push;
    struct list_head read_pool;
    int read_allocated;
    int read_started;
    struct list_head	read_queue;

    struct list_head write_pool;
    int write_allocated;
    int write_started;
    bool write_busy;

    u8 id;
};

struct usb_gadget_dev_t {
    struct usb_infac_dev infac_dev[USB_INTERFACE_NUM];
};

#endif