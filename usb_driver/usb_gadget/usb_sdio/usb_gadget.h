/*
* wangc add for usb to sdio driver
*/

#define USB_INFAC_NUM		2

struct _t_usb_sdio_dev;

struct usb_infac_dev {
	struct usb_function function;
	struct usb_composite_dev *cdev;
	spinlock_t lock;
//	int already_load_fw;

	struct usb_ep *ep_in;
	struct usb_ep *ep_out;

	struct list_head read_pool;
	struct list_head read_queue;
	int read_allocated;	
	int read_started;

	struct list_head write_pool;
	struct list_head write_queue;
	int write_allocated;

	u8 id;
};

#define	SU_SDIO_BLK_SIZE 	512
#define SDIO_ADDR_INFO		        (unsigned int)(0x081)
#define SDIO_ADDR_DATA		        (unsigned int)(0x080)
#define SDIO_ADDR_INFO_ASYNC		(unsigned int)(0x082)

struct sdio_dev {
	struct sdio_func   *func;
	spinlock_t lock;

	unsigned int recv_len;
	unsigned int next_rx_size;

	unsigned int curr_tx_size;

	unsigned int slave_avl_buf;
	atomic_t slave_buf_suspend;
	struct delayed_work work;
};


typedef struct _t_usb_sdio_dev
{
	struct usb_infac_dev usb_infac[USB_INFAC_NUM];
	struct sdio_dev sdio;
	struct semaphore sem;

	// usb to sdio
	struct work_struct work;
	struct workqueue_struct * workqueue;
	wait_queue_head_t us_wait; /* wait queue */

	// sdio to usb
	struct task_struct *kthread;
	wait_queue_head_t wait; /* wait queue */
} t_usb_sdio_dev;


