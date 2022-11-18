#ifndef __USB_DRIVER_H__
#define __USB_DRIVER_H__

#include <linux/workqueue.h>

#define DEBUG_FN_ENTRY_STR "%s() enter, line:%d\n", __func__, __LINE__
#define URB_MAX_ITEM_SIZE   128
#define USB_RX_MAX_BUF_SIZE 2048
#define MAX_PACKET_SIZE 512
#define USB_NUM_MASK    0x7F

#define USB_DATA_URB_NUM 64
#define USB_MSG_URB_NUM	16

#define USB_ENDPOINT_DIR_MASK 0x80
#define USB_DIR_IN      0x80
#define USB_DIR_OUT     0

struct usb_infac_pipe {
	u32 urb_cnt;
	struct usb_infac_data_t *infac;
    spinlock_t urb_lock;
	struct usb_anchor urb_submitted;
	unsigned int usb_pipe_handle;
	struct list_head urb_list_head;
#ifdef CONFIG_WORKQUEUE
	struct work_struct io_complete_work;
#elif defined(CONFIG_TASKLET)
	struct tasklet_struct tx_tasklet;
	struct tasklet_struct rx_tasklet;
#endif
	struct sk_buff_head io_skb_queue;
};

struct usb_urb_context {
	struct list_head link;
	struct sk_buff *skb;
    struct urb *urb;
	struct usb_infac_pipe * pipe;
};
struct usb_infac_data_t {
	struct usb_interface *interface;
	struct usb_device *udev;
	u16 max_packet_size;

	struct usb_infac_pipe pipe_rx;
	struct usb_infac_pipe pipe_tx;

	#ifdef CONFIG_KTHREAD
	struct task_struct* usb_rx_thread;
	struct task_struct* usb_tx_thread;

	wait_queue_head_t rx_queue;
	wait_queue_head_t tx_queue;
	#endif
};

struct usb_host_priv{
    struct usb_device* udev;
    struct usb_infac_data_t usb_data;
    struct usb_infac_data_t usb_msg;
    struct mutex usb_mutex;
    struct delayed_work register_work;
    bool register_flag;
    spinlock_t txq_lock;
    struct usb_ops *ops;

#ifdef CONFIG_KTHREAD
    struct task_struct *kthread_tx;
    struct task_struct *kthread_rx;
    wait_queue_head_t wait_tx_comp;
	wait_queue_head_t wait_rx_comp;
#endif
};

struct usb_ops{
    int (*start)(struct usb_host_priv *tr);
    int (*xmit)(struct usb_host_priv *tr, struct sk_buff *skb);
    int (*suspend)(struct usb_host_priv *tr);
    int (*resume)(struct usb_host_priv *tr);
    int (*write)(struct usb_host_priv *tr, const void* data, const u32 len);
    int (*wait_ack)(struct usb_host_priv *tr, void* data, const u32 len);
};

enum
{
    USB_INFAC_DATA = 0,
    USB_INFAC_MSG = 1,
};


#endif /* __USB_DRIVER_H__ */