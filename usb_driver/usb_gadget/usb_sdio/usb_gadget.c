/*
* usb gadget driver
*/

#include <linux/delay.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/usb/composite.h>

#include "usb_gadget.h"

static t_usb_sdio_dev su_dev;

unsigned int need_size = 0;

static struct dentry *p_debugfs_su;
struct usb_request * su_alloc_req(struct usb_ep *ep, unsigned len, gfp_t kmalloc_flags);
static int usb_xmit(struct usb_ep *ep_in, void *data, int len);

static int debugfs_su_read(void *data, u64 *val)
{
	printk("[su] entry %s\n", __func__);
	return 0;
}

static struct usb_request * req_tx;
unsigned long speed_test_rx_pkt_cnt = 0;
unsigned long speed_test_last_jiffies;

static void usb_write_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct usb_infac_dev *infac_dev = &su_dev.usb_infac[0];
	usb_ep_queue(infac_dev->ep_in, req_tx, GFP_ATOMIC);

			if ( speed_test_rx_pkt_cnt == 0) {
				speed_test_last_jiffies = jiffies;
			}

			speed_test_rx_pkt_cnt ++;
			if (time_after(jiffies, (speed_test_last_jiffies + msecs_to_jiffies(1000)))) {
				unsigned long mbits;

				mbits = (1514 * speed_test_rx_pkt_cnt) / 128 / 1024;	// 128 = 1024 / 8

				pr_info("[wangc]usb tx throughput speed (%d Mbps), cnt (%d)\n", 
					mbits, speed_test_rx_pkt_cnt);

				//speed_test_last_jiffies = jiffies;
				speed_test_rx_pkt_cnt = 0;
			}
	
	
	
	//struct sk_buff	*skb = req->context;
	//printk("[su] entry %s\n", __func__);
	//dev_kfree_skb(skb);
	//usb_ep_free_request(ep, req);
}
static int debugfs_su_write(void *data1, u64 val)
{
	int ret = 0;
	struct usb_infac_dev *infac_dev = &su_dev.usb_infac[0];
	unsigned len = (unsigned)val;
	struct usb_request * req;

	printk("[su] entry %s\n", __func__);

	req = su_alloc_req(infac_dev->ep_in, len, GFP_ATOMIC);
	req->length = len;
	req->complete = usb_write_complete;
	memset(req->buf, 0x23, len);
	req->length = len;
	
	req_tx = req;
	usb_ep_queue(infac_dev->ep_in, req, GFP_ATOMIC);

	return ret;
}

DEFINE_SIMPLE_ATTRIBUTE(debugfs_su,
			debugfs_su_read,
			debugfs_su_write,
			"%llu\n");

void debugfs_su_init(void)
{
	pr_info("[su]%s\n", __func__);
	p_debugfs_su = debugfs_create_file("sdio_usb", 0777, NULL, NULL, &debugfs_su);
}

void debugfs_su_exit(void)
{
	debugfs_remove(p_debugfs_su);
}


USB_GADGET_COMPOSITE_OPTIONS();

// usb desc 

/***  configuration description ***/
static struct usb_configuration su_config_driver = {
	.label = "usb config",
	.bConfigurationValue = 1,
	/* .iConfiguration = DYNAMIC */
	/* .bmAttributes = USB_CONFIG_ATT_SELFPOWER, */
	.MaxPower = 250,
};


/* interface descriptor: */

static struct usb_interface_descriptor su_interface_desc = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	/* .bInterfaceNumber = DYNAMIC */
	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_VENDOR_SPEC,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	/* .iInterface = DYNAMIC */
};

/* full speed support: */

static struct usb_endpoint_descriptor su_fs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor su_fs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header *su_fs_function[] = {
	(struct usb_descriptor_header *) &su_interface_desc,
	(struct usb_descriptor_header *) &su_fs_in_desc,
	(struct usb_descriptor_header *) &su_fs_out_desc,
	NULL,
};

/* high speed support: */

static struct usb_endpoint_descriptor su_hs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_endpoint_descriptor su_hs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_descriptor_header *su_hs_function[] = {
	(struct usb_descriptor_header *) &su_interface_desc,
	(struct usb_descriptor_header *) &su_hs_in_desc,
	(struct usb_descriptor_header *) &su_hs_out_desc,
	NULL,
};

static struct usb_endpoint_descriptor su_ss_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_endpoint_descriptor su_ss_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_ss_ep_comp_descriptor su_ss_bulk_comp_desc = {
	.bLength =              sizeof su_ss_bulk_comp_desc,
	.bDescriptorType =      USB_DT_SS_ENDPOINT_COMP,
};

static struct usb_descriptor_header *su_ss_function[] = {
	(struct usb_descriptor_header *) &su_interface_desc,
	(struct usb_descriptor_header *) &su_ss_in_desc,
	(struct usb_descriptor_header *) &su_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &su_ss_out_desc,
	(struct usb_descriptor_header *) &su_ss_bulk_comp_desc,
	NULL,
};



/***  device description ***/
#define SU_VENDOR_ID		0x19d2
#define SU_PRODUCT_ID		0x0256
#define SU_BCD_DEVICE		0x7510

static struct usb_device_descriptor device_desc = {
	.bLength =		USB_DT_DEVICE_SIZE,
	.bDescriptorType =	USB_DT_DEVICE,
	.bcdUSB = 0x0200, 
	.bDeviceClass = 0,
	.bDeviceSubClass =	0,
	.bDeviceProtocol =	0,
	.bMaxPacketSize0 = 64,
	.idVendor = cpu_to_le16(SU_VENDOR_ID),
	.idProduct = cpu_to_le16(SU_PRODUCT_ID),
	.bcdDevice = cpu_to_le16(SU_BCD_DEVICE),
	/* .iManufacturer = DYNAMIC */
	/* .iProduct = DYNAMIC */
	.bNumConfigurations =	1,
};

/***  string description ***/
#define STRING_DESCRIPTION_IDX		USB_GADGET_FIRST_AVAIL_IDX

static struct usb_string strings_dev[] = {
	[USB_GADGET_MANUFACTURER_IDX].s = "Leon",
	[USB_GADGET_PRODUCT_IDX].s = "BootLoader",
	[USB_GADGET_SERIAL_IDX].s = "usbBootloder",
	[STRING_DESCRIPTION_IDX].s = NULL, 
	{  } /* end of list */
};

static struct usb_gadget_strings stringtab_dev = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings_dev,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_dev,
	NULL,
};

#define QUEUE_SIZE		16//128
#define ITEM_BUF_SIZE		2048

static inline struct usb_infac_dev *func_to_su(struct usb_function *f)
{
	return container_of(f, struct usb_infac_dev, function);
}

static void su_start_tx(struct usb_infac_dev * infac_dev)
{
	struct list_head *pool = &infac_dev->write_queue;
	struct usb_ep *in = infac_dev->ep_in;

	while (!list_empty(pool)) {
		struct usb_request	*req;
		int			status;

		req = list_entry(pool->next, struct usb_request, list);

		list_del(&req->list);

		spin_unlock(&infac_dev->lock);
		status = usb_ep_queue(in, req, GFP_ATOMIC);
		spin_lock(&infac_dev->lock);

		if (status) {
			pr_debug("%s: %s %s err %d\n",
					__func__, "queue", in->name, status);
			list_add(&req->list, &infac_dev->write_pool);
			break;
		}
	}
}

static unsigned su_start_rx(struct usb_infac_dev * infac_dev)
{
	struct list_head *pool = &infac_dev->read_pool;
	struct usb_ep *out = infac_dev->ep_out;
	//printk("[su] entry %s\n", __func__);

	while (!list_empty(pool)) {
		struct usb_request	*req;
		int			status;

		if (infac_dev->read_started >= QUEUE_SIZE)
			break;

		req = list_entry(pool->next, struct usb_request, list);

		list_del(&req->list);

		req->length = ITEM_BUF_SIZE;

		/* drop lock while we call out; the controller driver
		 * may need to call us back (e.g. for disconnect)
		 */
		spin_unlock(&infac_dev->lock);
		status = usb_ep_queue(out, req, GFP_ATOMIC);
		spin_lock(&infac_dev->lock);

		if (status) {
			pr_debug("%s: %s %s err %d\n",
					__func__, "queue", out->name, status);
			list_add(&req->list, pool);
			break;
		}
		infac_dev->read_started++;
	}

	return infac_dev->read_started;
}




struct usb_request *
su_alloc_req(struct usb_ep *ep, unsigned len, gfp_t kmalloc_flags)
{
	struct usb_request *req;
	struct sk_buff	*skb;

	skb = dev_alloc_skb(len);
	if (!skb) {
		printk("[su] entry %s, no skb!\n", __func__);
		return NULL;
	}

	req = usb_ep_alloc_request(ep, kmalloc_flags);
	if (req != NULL) {
		req->length = len;
		req->buf = skb->data;
		req->context = skb; 
	} else {
		printk("[su] entry %s, no req!\n", __func__);
		dev_kfree_skb(skb);
	}

	return req;
}

static void su_write_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct usb_infac_dev *infac_dev = ep->driver_data;

	//printk("[su] entry %s\n", __func__);

	spin_lock(&infac_dev->lock);
	list_add_tail(&req->list, &infac_dev->write_pool);
	spin_unlock(&infac_dev->lock);
}


static void su_read_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct usb_infac_dev *infac_dev = ep->driver_data;

	//printk("[su] entry %s\n", __func__);

	spin_lock(&infac_dev->lock);
	list_add_tail(&req->list, &infac_dev->read_queue);
	queue_work(su_dev.workqueue, &su_dev.work);
	spin_unlock(&infac_dev->lock);
	
	//if(req->actual > 500)
	//	print_hex_dump(KERN_INFO, "READ:", DUMP_PREFIX_ADDRESS, 32, 1,
	//		       req->buf, req->actual, false);
}


static void su_function_free_req_all(struct usb_ep *ep, struct list_head *head,
							 int *allocated)
{
	struct usb_request	*req;

	printk("[su] entry %s\n", __func__);
	while (!list_empty(head)) {
		req = list_entry(head->next, struct usb_request, list);
		list_del(&req->list);
		dev_kfree_skb(req->context);
		usb_ep_free_request(ep, req);
		if (allocated)
			(*allocated)--;
	}
}



static int su_function_alloc_req_all(struct usb_ep *ep, struct list_head *head,
		void (*fn)(struct usb_ep *, struct usb_request *),
		int *allocated)
{
	int			i;
	struct usb_request	*req;
	int n = allocated ? QUEUE_SIZE - *allocated : QUEUE_SIZE;

	//printk("[su] entry %s\n", __func__);
	/* Pre-allocate up to QUEUE_SIZE transfers, but if we can't
	 * do quite that many this time, don't fail ... we just won't
	 * be as speedy as we might otherwise be.
	 */
	for (i = 0; i < n; i++) {
		req = su_alloc_req(ep, ITEM_BUF_SIZE, GFP_ATOMIC);
		if (!req) {
			printk("[su] entry %s: nomem!\n", __func__);
			return list_empty(head) ? -ENOMEM : 0;
		}
		req->complete = fn;
		list_add_tail(&req->list, head);
		if (allocated)
			(*allocated)++;
	}
	return 0;
}



static int su_function_start(struct usb_infac_dev * infac_dev)
{
	struct list_head *head = &infac_dev->read_pool;
	struct usb_ep *ep = infac_dev->ep_out;
	int ret;
	unsigned started;

	if (infac_dev->read_allocated == 0) {
		//printk("[su] entry %s, id: %d\n", __func__, infac_dev->id);
		ret = su_function_alloc_req_all(ep, head, su_read_complete,
			&infac_dev->read_allocated);
		if (ret)
			return ret;
	}

	if (infac_dev->write_allocated == 0) {
		ret = su_function_alloc_req_all(infac_dev->ep_in, &infac_dev->write_pool,
			su_write_complete, &infac_dev->write_allocated);
		if (ret) {
			su_function_free_req_all(ep, head, &infac_dev->read_allocated);
			return ret;
		}
	}

	started = su_start_rx(infac_dev);

	if (!started) {
		su_function_free_req_all(ep, head, &infac_dev->read_allocated);
		su_function_free_req_all(infac_dev->ep_in, &infac_dev->write_pool, 
			&infac_dev->write_allocated);
		ret = -EIO;
	}
	su_start_tx(infac_dev);
	return ret;
}

// ---------------------------------------------------------------------------

static int su_function_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct usb_infac_dev	*infac_dev = func_to_su(f);
	int			status;
	struct usb_ep		*ep;

	printk("[su] entry %s\n", __func__);

	/* allocate instance-specific interface IDs */
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	infac_dev->id = status;
	su_interface_desc.bInterfaceNumber = status;

	status = -ENODEV;

	/* allocate instance-specific endpoints */
	ep = usb_ep_autoconfig(cdev->gadget, &su_fs_in_desc);
	if (!ep)
		goto fail;
	infac_dev->ep_in = ep;

	ep = usb_ep_autoconfig(cdev->gadget, &su_fs_out_desc);
	if (!ep)
		goto fail;
	infac_dev->ep_out = ep;

	/* support all relevant hardware speeds... we expect that when
	 * hardware is dual speed, all bulk-capable endpoints work at
	 * both speeds
	 */
	su_hs_in_desc.bEndpointAddress = su_fs_in_desc.bEndpointAddress;
	su_hs_out_desc.bEndpointAddress = su_fs_out_desc.bEndpointAddress;

	su_ss_in_desc.bEndpointAddress = su_fs_in_desc.bEndpointAddress;
	su_ss_out_desc.bEndpointAddress = su_fs_out_desc.bEndpointAddress;

	status = usb_assign_descriptors(f, su_fs_function, su_hs_function,
			su_ss_function, NULL);
	if (status)
		goto fail;

	return 0;

fail:
	ERROR(cdev, "%s: can't bind, err %d\n", f->name, status);

	return status;
}


static void su_function_unbind(struct usb_configuration *c, struct usb_function *f)
{
	usb_free_all_descriptors(f);
}


static int su_function_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct usb_infac_dev *infac_dev = func_to_su(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	int ret;
	unsigned long flags;

	printk("[su] entry %s,  intf: %d alt: %d\n", __func__, intf, alt);
	DBG(cdev, "su_function_set_alt intf: %d alt: %d\n", intf, alt);

	if (!infac_dev->ep_in->desc || !infac_dev->ep_out->desc) {
		dev_dbg(&cdev->gadget->dev,
			"activate generic infac-su%d\n", infac_dev->id);
		if (config_ep_by_speed(cdev->gadget, f, infac_dev->ep_in) ||
		    config_ep_by_speed(cdev->gadget, f, infac_dev->ep_out)) {
			infac_dev->ep_in->desc = NULL;
			infac_dev->ep_out->desc = NULL;
			return -EINVAL;
		}
	}

	ret = usb_ep_enable(infac_dev->ep_in);
	if (ret)
		return ret;
	infac_dev->ep_in->driver_data = infac_dev;

	ret = usb_ep_enable(infac_dev->ep_out);
	if (ret) {
		usb_ep_disable(infac_dev->ep_in);
		return ret;
	}
	infac_dev->ep_out->driver_data = infac_dev;


    spin_lock_irqsave(&infac_dev->lock, flags);
    su_function_start(infac_dev);
    spin_unlock_irqrestore(&infac_dev->lock, flags);

    return 0;
}

static void su_function_disable(struct usb_function *f)
{
	struct usb_infac_dev *infac_dev = func_to_su(f);
	//struct usb_composite_dev	*cdev = infac_dev->cdev;
	unsigned long	flags;

	printk("[su] entry %s\n", __func__);

	usb_ep_disable(infac_dev->ep_in);
	usb_ep_disable(infac_dev->ep_out);

	spin_lock_irqsave(&infac_dev->lock, flags);

	//su_function_free_req_all(infac_dev->ep_out, &infac_dev->read_pool, NULL);
	//su_function_free_req_all(infac_dev->ep_out, &infac_dev->read_queue, NULL);
	//su_function_free_req_all(infac_dev->ep_in, &infac_dev->write_pool, NULL);
	//infac_dev->read_allocated = infac_dev->read_started = 0;

	spin_unlock_irqrestore(&infac_dev->lock, flags);
}



static int su_register_ports(struct usb_composite_dev *cdev,
		struct usb_configuration *c)
{
	int i;
	int ret;
	struct usb_infac_dev * infac_dev;

	printk("[su] entry %s\n", __func__);
	ret = usb_add_config_only(cdev, c);
	if (ret)
		return ret;

	for (i = 0; i < USB_INFAC_NUM; i++) {

		infac_dev = &su_dev.usb_infac[i];
		infac_dev->cdev = c->cdev;
		if (i)
			infac_dev->function.name = "su1";
		else
			infac_dev->function.name = "su0";
		infac_dev->function.bind = su_function_bind;
		infac_dev->function.unbind = su_function_unbind;
		infac_dev->function.set_alt = su_function_set_alt;
		infac_dev->function.disable = su_function_disable;

		spin_lock_init(&infac_dev->lock);

		INIT_LIST_HEAD(&infac_dev->read_pool);
		INIT_LIST_HEAD(&infac_dev->read_queue);
		INIT_LIST_HEAD(&infac_dev->write_pool);
		INIT_LIST_HEAD(&infac_dev->write_queue);

		usb_add_function(c, &infac_dev->function);
	}

	return 0;
}



static int su_bind(struct usb_composite_dev *cdev)
{
	int	status;

	/* Allocate string descriptor numbers ... note that string
	 * contents can be overridden by the composite_dev glue.
	 */

	printk("[su] entry %s\n", __func__);

	status = usb_string_ids_tab(cdev, strings_dev);
	if (status < 0)
		goto fail;
	device_desc.iManufacturer = strings_dev[USB_GADGET_MANUFACTURER_IDX].id;
	device_desc.iProduct = strings_dev[USB_GADGET_PRODUCT_IDX].id;
	status = strings_dev[STRING_DESCRIPTION_IDX].id;
	su_config_driver.iConfiguration = status;

	status = su_register_ports(cdev, &su_config_driver);
	if (status < 0)
		goto fail;

	usb_composite_overwrite_options(cdev, &coverwrite);
	return 0;
fail:
	return status;
}



static int su_unbind(struct usb_composite_dev *cdev)
{
	printk("[su] entry %s\n", __func__);
	return 0;
}

static struct usb_composite_driver usb_gadget_driver = {
	.name		= "su_usb",
	.dev		= &device_desc,
	.strings	= dev_strings,
	.max_speed	= USB_SPEED_HIGH,
	.bind		= su_bind,
	.unbind		= su_unbind,
};

#define NEXT_BUF_SZ_OFFSET			(0)

static void sdio_tx_work(struct work_struct *work)
{
	t_usb_sdio_dev * p_su = container_of(work, t_usb_sdio_dev, work);
	int i;

	//printk("[su] entry %s\n", __func__);

	for (i=0; i < USB_INFAC_NUM; i++) {
		struct usb_infac_dev * infac_dev = &p_su->usb_infac[i];
		struct list_head *queue = &infac_dev->read_queue;
		int restart = 0;

		spin_lock_irq(&infac_dev->lock);
		while (!list_empty(queue)) {
			struct usb_request	*req;
			req = list_first_entry(queue, struct usb_request, list);

			switch (req->status) {
				case -ESHUTDOWN:
					restart = -1;
					printk("[su] us%d: shutdown\n", infac_dev->id);
					break;

				default:
					/* presumably a transient fault */
					pr_warn("us%d: unexpected RX status %d\n",
						infac_dev->id, req->status);
					/* FALLTHROUGH */
				case 0:
					/* normal completion */
					printk("usb rx length:%d \n", req->length);
					print_hex_dump(KERN_DEBUG, "usb_rx: ", DUMP_PREFIX_NONE, 32, 1, req->buf, req->length, false);
					break;
			}

			spin_unlock_irq(&infac_dev->lock);

			if (restart >= 0) {
				//sdio_xmit(req);
				restart = 1;
			} 

			spin_lock_irq(&infac_dev->lock);
			list_move(&req->list, &infac_dev->read_pool);
			infac_dev->read_started--;
		}

		if (restart == 1) {
			int ack[] = {0x12, 0x34, 0x56, 0x78};
			printk("usb tx ack:%d, read:%d, write:%d \n", sizeof(ack), infac_dev->read_allocated, infac_dev->write_allocated);
			usb_xmit(p_su->usb_infac[0].ep_in, (void*)ack, sizeof(ack));
			su_function_start(infac_dev);
		} else if (restart < 0){
			printk("[su] entry %s, stop!\n", __func__);
		}

		spin_unlock_irq(&infac_dev->lock);
	}
}

#define FW_BLOCK_SIZE 4096


#include <linux/sched.h>
#include <uapi/linux/sched/types.h>

static int usb_xmit(struct usb_ep *ep_in, void *data, int len)
{
    struct usb_request *req;
    struct sk_buff  *skb;

     printk("[su] entry %s, len:%d! \n", __func__, len);
    skb = dev_alloc_skb(1024);
    if (!skb) {
        printk("[su] entry %s, no skb!\n", __func__);
        return NULL;
    }

    memcpy(skb->data, data, len);
    skb->len = len;
    req = usb_ep_alloc_request(ep_in, GFP_ATOMIC);
    if (req != NULL) {
        req->length = len;
        req->buf = skb->data;
        req->context = skb;
    } else {
        printk("[su] entry %s, no req!\n", __func__);
        dev_kfree_skb(skb);
    }

    usb_ep_queue(ep_in, req, GFP_ATOMIC);

    return 0;
}

static int usb_gadget_init(void)
{
	int ret;
	t_usb_sdio_dev * p_su = &su_dev;
	printk("[su] entry %s\n", __func__);

	strings_dev[STRING_DESCRIPTION_IDX].s = su_config_driver.label;

	INIT_WORK(&p_su->work, sdio_tx_work);

	p_su->workqueue = create_singlethread_workqueue("wq_sdio_tx");
	if (! p_su->workqueue)
		return  -ENOMEM;

	ret =  usb_composite_probe(&usb_gadget_driver);
	if (ret) {
		destroy_workqueue(p_su->workqueue);
		return ret;
	}

	debugfs_su_init();
	return ret;
}

static void usb_gadget_exit(void)
{
	printk("[su] entry %s\n", __func__);
	usb_composite_unregister(&usb_gadget_driver);
	destroy_workqueue(su_dev.workqueue);
	debugfs_su_exit();
}


module_init(usb_gadget_init);
module_exit(usb_gadget_exit);


MODULE_AUTHOR("Leon");
MODULE_LICENSE("Dual BSD/GPL");
