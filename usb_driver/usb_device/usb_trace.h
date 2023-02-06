#undef TRACE_SYSTEM
#define TRACE_SYSTEM usb_driver

#if !defined(__USB_TRACE_H__) || defined(TRACE_HEADER_MULTI_READ)

//#ifndef __USB_TRACE_H__ //this line should be must delete
#define __USB_TRACE_H__

#include <linux/tracepoint.h>


TRACE_EVENT(usb_driver_tx,
    TP_PROTO(struct sk_buff* skb, unsigned int pipe),
    TP_ARGS(skb, pipe),
    TP_STRUCT__entry(
        __field(struct sk_buff*, skb)
        __field(u32, pipe)
    ),

    TP_fast_assign(
        __entry->skb = skb;
        __entry->pipe = pipe;
    ),

    TP_printk("skb:%pM, skb_len:%d, pipe:%d ", __entry->skb, __entry->skb->len, __entry->pipe)
);


//#endif
#endif

#undef TRACE_INCLUDE_PATH
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE usb_trace
#include <trace/define_trace.h>
