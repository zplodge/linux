/*
* usb gadget source header file
*/
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/usb/composite.h>
#include <linux/err.h>
#include <linux/usb/cdc.h>

#include "usb_gadget.h"

/***  device description ***/
#define SU_VENDOR_ID		0x19d2
#define SU_PRODUCT_ID		0x0256
#define SU_BCD_DEVICE		0x7510

struct usb_gadget_dev_t usb_gadget_dev;
USB_GADGET_COMPOSITE_OPTIONS();

static struct usb_configuration usb_gadget_config_driver = {
    .label = "usb_config",
    .bConfigurationValue = 1,
    .MaxPower = 250,
};

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


/* interface and class descriptors: */
static struct usb_interface_descriptor usb_data_interface_desc = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	/* .bInterfaceNumber = DYNAMIC */
	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_VENDOR_SPEC, //USB_CLASS_CDC_DATA,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	/* .iInterface = DYNAMIC */
};

#if 0
static struct usb_cdc_header_desc usb_header_desc = {
	.bLength =		sizeof(usb_header_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_HEADER_TYPE,
	.bcdCDC =		cpu_to_le16(0x0110),
};

static struct usb_cdc_call_mgmt_descriptor usb_call_mgmt_descriptor = {
	.bLength =		sizeof(usb_call_mgmt_descriptor),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_CALL_MANAGEMENT_TYPE,
	.bmCapabilities =	0,
	/* .bDataInterface = DYNAMIC */
};

static struct usb_cdc_union_desc usb_union_desc = {
	.bLength =		sizeof(usb_union_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_UNION_TYPE,
	/* .bMasterInterface0 =	DYNAMIC */
	/* .bSlaveInterface0 =	DYNAMIC */
};


/* full speed support: */
static struct usb_endpoint_descriptor usb_fs_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(HS_NOTIFY_MAXPACKET),
	.bInterval =		GS_NOTIFY_INTERVAL_MS,
};
#endif

static struct usb_endpoint_descriptor usb_fs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor usb_fs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header *usb_fs_function[] = {
	//(struct usb_descriptor_header *) &usb_header_desc,
	//(struct usb_descriptor_header *) &usb_call_mgmt_descriptor,
	//(struct usb_descriptor_header *) &usb_union_desc,
	//(struct usb_descriptor_header *) &usb_fs_notify_desc,
	(struct usb_descriptor_header *) &usb_data_interface_desc,
	(struct usb_descriptor_header *) &usb_fs_in_desc,
	(struct usb_descriptor_header *) &usb_fs_out_desc,
	NULL,
};

/* high speed support: */
static struct usb_endpoint_descriptor usb_hs_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(HS_NOTIFY_MAXPACKET),
	.bInterval =		USB_MS_TO_HS_INTERVAL(GS_NOTIFY_INTERVAL_MS),
};

static struct usb_endpoint_descriptor usb_hs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_endpoint_descriptor usb_hs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_descriptor_header *usb_hs_function[] = {
	//(struct usb_descriptor_header *) &usb_header_desc,
	//(struct usb_descriptor_header *) &usb_call_mgmt_descriptor,
	//(struct usb_descriptor_header *) &usb_union_desc,
	//(struct usb_descriptor_header *) &usb_hs_notify_desc,
	(struct usb_descriptor_header *) &usb_data_interface_desc,
	(struct usb_descriptor_header *) &usb_hs_in_desc,
	(struct usb_descriptor_header *) &usb_hs_out_desc,
	NULL,
};

static struct usb_endpoint_descriptor usb_ss_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_endpoint_descriptor usb_ss_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_ss_ep_comp_descriptor usb_ss_bulk_comp_desc = {
	.bLength =              sizeof(usb_ss_bulk_comp_desc),
	.bDescriptorType =      USB_DT_SS_ENDPOINT_COMP,
};

static struct usb_descriptor_header *usb_ss_function[] = {
	//(struct usb_descriptor_header *) &usb_header_desc,
	//(struct usb_descriptor_header *) &usb_call_mgmt_descriptor,
	//(struct usb_descriptor_header *) &usb_union_desc,
	//(struct usb_descriptor_header *) &usb_hs_notify_desc,
	(struct usb_descriptor_header *) &usb_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &usb_data_interface_desc,
	(struct usb_descriptor_header *) &usb_ss_in_desc,
	(struct usb_descriptor_header *) &usb_ss_bulk_comp_desc,
	(struct usb_descriptor_header *) &usb_ss_out_desc,
	(struct usb_descriptor_header *) &usb_ss_bulk_comp_desc,
	NULL,
};

static inline struct usb_infac_dev *func_to_dev(struct usb_function *f)
{
	return container_of(f, struct usb_infac_dev, func);
}

struct usb_request *hs_alloc_req(struct usb_ep *ep, unsigned len, gfp_t kmalloc_flags)
{
	struct usb_request *req;

	req = usb_ep_alloc_request(ep, kmalloc_flags);

	if (req != NULL) {
		req->length = len;
		req->buf = kmalloc(len, kmalloc_flags);
		if (req->buf == NULL) {
			usb_ep_free_request(ep, req);
			return NULL;
		}
	}

	return req;
}

static int hs_alloc_requests(struct usb_ep *ep, struct list_head *head, \
    void (*fn)(struct usb_ep *, struct usb_request *), int *allocated)
{
	int			i;
	struct usb_request	*req;
	int n = allocated ? QUEUE_SIZE - *allocated : QUEUE_SIZE;

	/* Pre-allocate up to QUEUE_SIZE transfers, but if we can't
	 * do quite that many this time, don't fail ... we just won't
	 * be as speedy as we might otherwise be.
	 */
	for (i = 0; i < n; i++) {
		req = hs_alloc_req(ep, ep->maxpacket, GFP_ATOMIC);
		if (!req)
			return list_empty(head) ? -ENOMEM : 0;
		req->complete = fn;
		list_add_tail(&req->list, head);
		if (allocated)
			(*allocated)++;
	}
	return 0;
}


static void hs_free_requests(struct usb_ep *ep, struct list_head *head, int *allocated)
{
	struct usb_request	*req;

	while (!list_empty(head)) {
		req = list_entry(head->next, struct usb_request, list);
		list_del(&req->list);
		kfree(req->buf);
	    usb_ep_free_request(ep, req);
		if (allocated)
			(*allocated)--;
	}
}

static void hs_rx_push(struct work_struct *work)
{
    struct delayed_work	*w = to_delayed_work(work);
    struct usb_infac_dev *infac_dev = container_of(w, struct usb_infac_dev, push);
    struct list_head *queue = &infac_dev->read_queue;
    bool restart = true;
    printk(DEBUG_FN_ENTRY_STR);

    spin_lock_irq(&infac_dev->dev_lock);

    while(!list_empty(queue)) {
		struct usb_request	*req;

		req = list_first_entry(queue, struct usb_request, list);

		switch (req->status) {
		case -ESHUTDOWN:
			restart = false;
			pr_debug("hs_rx_push: shutdown\n");
			break;

		default:
			/* presumably a transient fault */
			pr_warn("hs_rx_push: unexpected RX status %d\n", req->status);
			fallthrough;
		case 0:
			/* normal completion */
            printk("usb rx length:%d \n", req->length);
            print_hex_dump(KERN_DEBUG, "usb_rx: ", DUMP_PREFIX_NONE, 32, 1, req->buf, req->length, false);
			break;
		}

		list_move(&req->list, &infac_dev->read_pool);
		infac_dev->read_started--;
	}

    if(restart > 0) {
        restart = 1;
    }

	spin_unlock_irq(&infac_dev->dev_lock);

	if (restart == 1) {
			int ack[] = {0x12, 0x34, 0x56, 0x78};
			printk("usb tx ack:%d, read:%d, write:%d \n", sizeof(ack), infac_dev->read_allocated, infac_dev->write_allocated);
			//usb_xmit(infac_dev->ep_in, (void*)ack, sizeof(ack));
			//su_function_start(infac_dev);
		} else {
			printk("[su] entry %s, stop!\n", __func__);
		}
}

static void hs_read_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct usb_infac_dev *infac_dev = ep->driver_data;
    printk(DEBUG_FN_ENTRY_STR);

	/* Queue all received data until the tty layer is ready for it. */
	spin_lock(&infac_dev->dev_lock);
	list_add_tail(&req->list, &infac_dev->read_queue);
	schedule_delayed_work(&infac_dev->push, 0);
	spin_unlock(&infac_dev->dev_lock);
}

static int hs_start_rx(struct usb_infac_dev *infac_dev)
{
    struct list_head* pool = &infac_dev->read_pool;
    struct usb_ep *out = infac_dev->ep_out;
    int status = 0;
    printk(DEBUG_FN_ENTRY_STR);

    while(!list_empty(pool)) {
		struct usb_request *req;

		if (infac_dev->read_started >= QUEUE_SIZE)
			break;

		req = list_entry(pool->next, struct usb_request, list);
		list_del(&req->list);
		req->length = out->maxpacket;

		/* drop lock while we call out; the controller driver
		 * may need to call us back (e.g. for disconnect)
		 */
		spin_unlock(&infac_dev->dev_lock);
		status = usb_ep_queue(out, req, GFP_ATOMIC);
		spin_lock(&infac_dev->dev_lock);

		if (status) {
			pr_debug("%s: %s %s err %d\n",
					__func__, "queue", out->name, status);
			list_add(&req->list, pool);
			break;
		}
		infac_dev->read_started++;
	}

	return status;
}

static int hs_start_tx(struct usb_infac_dev *infac_dev)
{
	struct list_head *pool = &infac_dev->write_pool;
	struct usb_ep *in = infac_dev->ep_in;
	int status = 0;

    printk(DEBUG_FN_ENTRY_STR);

	while (!infac_dev->write_busy && !list_empty(pool)) {
		struct usb_request	*req;

		if (infac_dev->write_started >= QUEUE_SIZE)
			break;

		req = list_entry(pool->next, struct usb_request, list);
		

		infac_dev->write_busy = true;
		spin_unlock(&infac_dev->dev_lock);
		status = usb_ep_queue(in, req, GFP_ATOMIC);
		spin_lock(&infac_dev->dev_lock);
		infac_dev->write_busy = false;

		if (status) {
			pr_debug("%s: %s %s err %d\n",
					__func__, "queue", in->name, status);
			list_add(&req->list, pool);
			break;
		}

		infac_dev->write_started++;
	}

	return status;
}

static void hs_write_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct usb_infac_dev *infac_dev = ep->driver_data;
    printk(DEBUG_FN_ENTRY_STR);

	spin_lock(&infac_dev->dev_lock);
	list_add(&req->list, &infac_dev->write_pool);
	infac_dev->write_started--;

	switch (req->status) {
	default:
		/* presumably a transient fault */
		pr_warn("%s: unexpected status %d\n", __func__, req->status);
		fallthrough;
	case 0:
		/* normal completion */
		hs_start_tx(infac_dev);
		break;

	case -ESHUTDOWN:
		/* disconnect */
		pr_debug("%s: shutdown\n", __func__);
		break;
	}

	spin_unlock(&infac_dev->dev_lock);
}

static int usb_gadget_function_start(struct usb_infac_dev *infac_dev)
{
    int status;
    unsigned started;
    struct list_head *head = &infac_dev->read_pool;

    printk(DEBUG_FN_ENTRY_STR);

    if(!infac_dev->read_allocated) {
        status = hs_alloc_requests(infac_dev->ep_in, &infac_dev->read_pool, hs_read_complete, &infac_dev->read_allocated);

        if(status) {
            return status;
        }
    }

    if(!infac_dev->write_allocated) {
        status = hs_alloc_requests(infac_dev->ep_out, &infac_dev->write_pool, hs_write_complete, &infac_dev->write_allocated);

        if(status) {
            hs_free_requests(infac_dev->ep_in, head, &infac_dev->read_allocated);
            return status;
        }
    }

    started = hs_start_rx(infac_dev);

    if(!started) {
        printk("hs rx start fail; \n");
        hs_free_requests(infac_dev->ep_in, &infac_dev->read_pool, &infac_dev->read_allocated);
        hs_free_requests(infac_dev->ep_out, &infac_dev->write_pool, &infac_dev->write_allocated);
        status = -EIO;
    }else {
        hs_start_tx(infac_dev);
    }

    return status;
}

static int usb_func_bind(struct usb_configuration *c, struct usb_function *f)
{
    struct usb_composite_dev *cdev = c->cdev;
    struct usb_infac_dev	*infac_dev = func_to_dev(f);
    int			status;
    struct usb_ep		*ep;


    printk(DEBUG_FN_ENTRY_STR);
    /* allocate instance-specific interface IDs, and patch descriptors */
    status = usb_interface_id(c, f);
    printk("usb_interface_id status:%d \n", status);
    if (status < 0)
        goto fail;

    infac_dev->id = status;
    usb_data_interface_desc.bInterfaceNumber = status;
    status = -ENODEV;

    /* allocate instance-specific endpoints */
	ep = usb_ep_autoconfig(cdev->gadget, &usb_fs_in_desc);
	if (!ep)
		goto fail;

	infac_dev->ep_in = ep;

	ep = usb_ep_autoconfig(cdev->gadget, &usb_fs_out_desc);
	if (!ep)
		goto fail;

	infac_dev->ep_out = ep;

	/* support all relevant hardware speeds... we expect that when
	 * hardware is dual speed, all bulk-capable endpoints work at
	 * both speeds
	 */
	usb_hs_in_desc.bEndpointAddress = usb_fs_in_desc.bEndpointAddress;
	usb_hs_out_desc.bEndpointAddress = usb_fs_out_desc.bEndpointAddress;

	usb_ss_in_desc.bEndpointAddress = usb_fs_in_desc.bEndpointAddress;
	usb_ss_out_desc.bEndpointAddress = usb_fs_out_desc.bEndpointAddress;

	status = usb_assign_descriptors(f, usb_fs_function, usb_hs_function, usb_ss_function, usb_ss_function);
	//status = usb_assign_descriptors(f, usb_fs_function, usb_hs_function, usb_ss_function, NULL);

	if (status)
		return status;

	return 0;
fail:
    printk("usb func bind error! \n");
    return status;

}

static int usb_func_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
    unsigned long   flags;
    struct usb_infac_dev *infac_dev = func_to_dev(f);
    struct usb_composite_dev *cdev = f->config->cdev;

    printk(DEBUG_FN_ENTRY_STR);

    if (!infac_dev->ep_in->desc || !infac_dev->ep_out->desc) {
        printk("init usb gadget;\n");
        if (config_ep_by_speed(cdev->gadget, f,
                       infac_dev->ep_in) ||
            config_ep_by_speed(cdev->gadget, f,
                       infac_dev->ep_out)) {
            infac_dev->ep_in->desc = NULL;
            infac_dev->ep_out->desc = NULL;
            goto fail;
        }
    }

    usb_ep_enable(infac_dev->ep_in);
    usb_ep_enable(infac_dev->ep_out);

    infac_dev->ep_in->driver_data = infac_dev;
    infac_dev->ep_out->driver_data = infac_dev;

    spin_lock_irqsave(&infac_dev->dev_lock, flags);
    usb_gadget_function_start(infac_dev);
    spin_unlock_irqrestore(&infac_dev->dev_lock, flags);
    return 0;

fail:
    return -EINVAL;
}

static void usb_func_unbind(struct usb_configuration *c, struct usb_function *f)
{
    printk(DEBUG_FN_ENTRY_STR);
    usb_free_all_descriptors(f);
}

static void usb_func_disable(struct usb_function *f)
{
    struct usb_infac_dev	*infac_dev = func_to_dev(f);
    printk(DEBUG_FN_ENTRY_STR);
    usb_ep_disable(infac_dev->ep_in);
    usb_ep_disable(infac_dev->ep_out);
    cancel_delayed_work_sync(&infac_dev->push);
}

static int usb_register_ports(struct usb_composite_dev *cdev, struct usb_configuration* conf)
{
    int status = 0, i;
    struct usb_infac_dev* infac_dev;

    printk(DEBUG_FN_ENTRY_STR);
    status = usb_add_config_only(cdev, conf);
    if(status)
        return status;

    printk("%s: cdev:%p, conf->cdev:%p \n", __func__, cdev, conf->cdev);
    for(i=0; i<USB_INTERFACE_NUM; i++) {
        infac_dev = &usb_gadget_dev.infac_dev[i];
        infac_dev->cdev = conf->cdev;
        infac_dev->func.name = i?"hs1":"hs0";
        infac_dev->func.bind = usb_func_bind;
        infac_dev->func.set_alt = usb_func_set_alt;
        infac_dev->func.unbind = usb_func_unbind;
        infac_dev->func.disable = usb_func_disable;

        spin_lock_init(&infac_dev->dev_lock);
        INIT_LIST_HEAD(&infac_dev->read_pool);
        INIT_LIST_HEAD(&infac_dev->read_queue);
        INIT_LIST_HEAD(&infac_dev->write_pool);
        INIT_DELAYED_WORK(&infac_dev->push, hs_rx_push);

        status = usb_add_function(conf, &infac_dev->func);

        if (status) {
            printk("usb_add_function fail; \n");
            usb_put_function(&infac_dev->func);
        }
    }

    return status;
}

static int hs_usb_gadget_bind(struct usb_composite_dev *cdev)
{
    int status;
    printk(DEBUG_FN_ENTRY_STR);

    status = usb_string_ids_tab(cdev, strings_dev);

    if(status < 0) {
        return status;
    }

    device_desc.iManufacturer = strings_dev[USB_GADGET_MANUFACTURER_IDX].id;
    device_desc.iProduct = strings_dev[USB_GADGET_PRODUCT_IDX].id;
    status = strings_dev[STRING_DESCRIPTION_IDX].id;
    usb_gadget_config_driver.iConfiguration = status;

    status = usb_register_ports(cdev, &usb_gadget_config_driver);

    if(status) {
        printk("usb_register_ports error!!! \n");
        return status;
    }

    usb_composite_overwrite_options(cdev, &coverwrite);

    return 0;
}

static int hs_usb_gadget_unbind(struct usb_composite_dev *cdev)
{
    printk(DEBUG_FN_ENTRY_STR);
    usb_put_function(&usb_gadget_dev.infac_dev[0].func);
    usb_put_function(&usb_gadget_dev.infac_dev[1].func);

    return 0;
}


static struct usb_composite_driver hs_usb_gadget_driver = {
    .name = "usb_gadget",
    .dev = &device_desc,
    .strings = dev_strings,
    .max_speed = USB_SPEED_HIGH,
    .bind = hs_usb_gadget_bind,
    .unbind = hs_usb_gadget_unbind,
};

#if 0
static int usb_gadget_init(void)
{
    int ret = 0;

    printk(DEBUG_FN_ENTRY_STR);
    ret = usb_composite_probe(&usb_gadget_driver);
    if(ret) {
        printk("usb composite error!! \n");
    }

    return ret;
}

static void usb_gadget_exit(void)
{
    printk(DEBUG_FN_ENTRY_STR);
}

module_init(usb_gadget_init);
module_exit(usb_gadget_exit);
#endif

module_usb_composite_driver(hs_usb_gadget_driver);

MODULE_DESCRIPTION("raspberry Pi4 usb gadget driver");
//MODULE_AOTHOR("Leon");
MODULE_LICENSE("Dual BSD/GPL");
